#include <linux/gpio.h> //API for lowlevel handling of gpio
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>

//free license
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gruppe 42");
MODULE_DESCRIPTION("Driver platform til LED");

//Number of devices are 2.
#define NUM_OF_LEDS 2

//*********** platform variables ************//
//class pointer to this class
static struct class* LED_class;

//Platform driver variables
//static struct platform_driver my_platform_driver;
static dev_t devno;
static struct cdev LED_p_cdev;
static struct platform_driver LED_p_driver;
static int err;
static int dev_count = 0;
static int gpios_in_dt = 0;
static int index = 0;

//gpio device struct
struct gpio_dev{
  uint8_t no; //GPIO number
  uint8_t dir; //direction of device
};

//struct with devices
static struct gpio_dev gpio_devs[255];


//read data of device
ssize_t LED_p_read(struct file* filep, char __user* buf, size_t count, loff_t* f_pos){
  //room for 0-255 and EOF
  char readbuffer[2];
  //getting the device minor
  int minor;
  minor = iminor(filep->f_inode);
  //read from device
  sprintf(readbuffer, "%d", gpio_get_value(gpio_devs[minor].no));
  int buf_len = strlen(readbuffer) + 1;
  //truncate to requested lenght, good practice and suffecient for this purpose
  buf_len = buf_len > count ? count : buf_len;
  //copy data to user-space, returns bytes that could not be written to US
  if(copy_to_user(buf, readbuffer, buf_len)){
    printk(KERN_ALERT "Error: copy_to_user\n");
  }
  //offset
  *f_pos += buf_len;
  return buf_len;
}

//write data to device
ssize_t LED_p_write(struct file* filep, const char __user* buf, size_t count, loff_t *f_pos){

  char writebuffer[2];
  int value;
  int buf_len = 2;

  //truncating length
  buf_len = buf_len > count ? count : buf_len;

  //copy data from user into writebuffer
  if(copy_from_user(writebuffer, buf, count)){
    printk(KERN_ALERT "Error: copy_from_user\n");
  }

  value = writebuffer[0]-48;
  //getting the device minor
  int minor;
  minor = iminor(filep->f_inode);
  //write recieved value to device
  printk(KERN_ALERT "writing to gpio%d\n", gpio_devs[minor].no);
  gpio_set_value(gpio_devs[minor].no, value);

  *f_pos +=  buf_len;
  return buf_len;
}

//open of device inode and file when open() is called in user-space
int LED_p_open(struct inode *inode, struct file *filep){
  int major, minor;

  major = MAJOR(inode->i_rdev);
  minor = MINOR(inode->i_rdev);
  printk("Opening LED device: %i, %i\n", major, minor);

  return 0;
}

//release of device when close() is called in user-space
int LED_p_release(struct inode *inode, struct file *filep){
  int minor, major;

  major = MAJOR(inode->i_rdev);
  minor = MINOR(inode->i_rdev);
  printk("Releasing LED device: %i, %i\n", major, minor);

  return 0;
 }

// file_operations function pointers to this child functions
 struct file_operations my_fops =
 {
   .open = LED_p_open,
   .release = LED_p_release,
   .read = LED_p_read,
   .write = LED_p_write,
 };

//device probe
static int LED_p_probe(struct platform_device *pdev){
  printk(KERN_ALERT "New platform device: %s\n", pdev->name);

  //derive platform device
  struct device *my_device = &pdev->dev;
  //derive device tree node pointer
  struct device_node *np = my_device->of_node;
  if(!np){
    printk("nodepointer equals 0\n");
    return 0;
  }
  //flag to hold derived direction
  enum of_gpio_flags flag;

  //Retrieving number of gpios from node pointer
  if((gpios_in_dt = of_gpio_count(np)) < 0){
    dev_err(&pdev->dev, "Failed to retrieve number of gpios\n");
    return -EINVAL;
  }else if(gpios_in_dt == 0){
    printk("gpios count equals 0\n");
  }

  printk(KERN_ALERT "Number of gpios:%d\n", gpios_in_dt);

  //loop through device tree and get gpio-numbers and direction
  for(index = 0; index < gpios_in_dt; index++){
    gpio_devs[index].no = of_get_gpio(np, index);
    if(gpio_devs[index].no < 0){
      if(gpio_devs[index].no != -EPROBE_DEFER){
        dev_err(my_device, "Failed to parse gpio %d\n", gpio_devs[index].no);
      }
    }
    //get and assign flag to device struct
    err = of_get_gpio_flags(np, index, &flag);
    gpio_devs[index].dir = flag;
  }

  for(index = 0; index < gpios_in_dt; index++){
    err = gpio_request(gpio_devs[index].no, pdev->name);
    if(err < 0){
      printk("Failed to request gpio%d\n", gpio_devs[index].no);
      goto err_req;
    }

    //Set gpio directions and initial value, flag = 1 is input, flag = 0 is output
    err = gpio_direction_output(gpio_devs[index].no, !(gpio_devs[index].dir));
    if(err < 0){
      printk("Failed to set gpio direction of gpio%d\n", gpio_devs[index].no);
      goto err_dir;
    }

    //create device with given major and minor as mydev<gpio>
    my_device = device_create(LED_class, NULL, MKDEV(MAJOR(devno), dev_count),
                              NULL, "mydev%d", gpio_devs[index].no);
    if(IS_ERR(my_device)){
      printk(KERN_ALERT "Failed to create device\n");
      return -EFAULT;
    }
    else
      printk(KERN_ALERT "Created device /dev/mydev%d with major:%d and minor:%d\n",
              gpio_devs[index].no, MAJOR(devno), dev_count);
    //use next minor for device
    if(dev_count < (gpios_in_dt-1)){
        dev_count++;
    }
  }
  //success
  printk(KERN_ALERT "GPIO devices succesfully initialized\n");
  //reset index
  index = 0;
  return 0;

  err_dir:
    //freeing claimed gpio
    gpio_free(gpio_devs[index].no);
  err_req:
    if(index > 0){
      gpio_free(gpio_devs[index].no-1);
    }
    index = 0;
    printk("Failed to setup gpio%d\n", gpio_devs[index].no);
    return err;
}

//device remove
static int LED_p_remove(struct platform_device *pdev){
  printk(KERN_ALERT "Removing device: %s\n", pdev->name);

  //destroy device of given Major and minor numbers
  for(index = 0; index < gpios_in_dt; index++){
    device_destroy(LED_class, MKDEV(MAJOR(devno), dev_count));
    gpio_free(gpio_devs[index].no);
    printk("Freeing gpio%d\n", gpio_devs[index].no);
    if(dev_count > 0){
        dev_count--;
    }
  }
  dev_count = 0;
  index = 0;
  return 0;
}

//Platform driver struct holding probe and remove functionpointers
//and the name under which is registered
static const struct of_device_id of_LED_p_driver_match[] = {
  {.compatible = "ase, plat_drv", }, {},
};

static struct platform_driver LED_p_driver = {
  .probe    = LED_p_probe,
  .remove   = LED_p_remove,
  .driver   = {
    .name   = "plat_drv",
              .of_match_table = of_LED_p_driver_match,
              .owner = THIS_MODULE
  },
};

//initializing resources and driver
static int LED_p_init(void){

    printk("initializing LED driver\n");

    // Allocate and reserve devicenumbers
    err = alloc_chrdev_region(&devno, 0, NUM_OF_LEDS, "my_leds");
    if(err < 0){
      printk("Failed to allocate device major\n");
      goto err_alloc;
    }else if(MAJOR(devno) <= 0){
      printk("Major number zero or less\n");
      goto err_class;
    }

    printk(KERN_ALERT "Assigned major number: %i\n", MAJOR(devno));

    //Create sysFS class
    LED_class = class_create(THIS_MODULE, "my_led_cls");
    if(IS_ERR(LED_class)){
      goto err_class;
    }

    //registering platform driver
    err = platform_driver_register(&LED_p_driver);
    if(err < 0){
      goto err_register;
    }

    //Char device init
    cdev_init(&LED_p_cdev, &my_fops);
    //Register char device
    err = cdev_add(&LED_p_cdev, devno, NUM_OF_LEDS);
    if(err < 0){
      goto err_platform;
    }

    printk("Init succesful\n");
    return 0; //success!

    //handle errors
    //unregister platform driver
    err_platform:
      platform_driver_unregister(&LED_p_driver);
    err_register:
      //destroy class
      class_destroy(LED_class);
    err_class:
      //unregistering device numbers
      unregister_chrdev_region(devno, NUM_OF_LEDS);
    err_alloc:
      printk("Failed to initialize device\n");
      return err;
}
//releasing resources
static void LED_p_exit(void){
  printk(KERN_ALERT "Exit called\n");
  platform_driver_unregister(&LED_p_driver);
  cdev_del(&LED_p_cdev);
  class_destroy(LED_class);
  unregister_chrdev_region(devno, NUM_OF_LEDS);
}

module_init(LED_p_init);
module_exit(LED_p_exit);
