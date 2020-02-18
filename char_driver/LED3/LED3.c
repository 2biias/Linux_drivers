#include <linux/gpio.h> //API for lowlevel handling of gpio
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/module.h>

//free license
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gruppe 42");
MODULE_DESCRIPTION("Chardriver til LED3");

//major 62 for LED's //number of devices are 1.
#define LED3_MAJOR 62
#define LED3_MINOR 0
#define NUM_OF_DEVS 1

//char device struct
static struct cdev LED3_dev;

static int devno;
static int err;
static uint8_t gpio = 21; //gpio of LED3.

//read data of LED3 inode
ssize_t LED3_read(struct file* filep, char __user* buf, size_t count, loff_t* f_pos){
  //room for 0-255 and EOF
  char readbuffer[2];
  //read from LED3
  sprintf(readbuffer, "%d", gpio_get_value(gpio));
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

//write data to LED3
ssize_t LED3_write(struct file* filep, const char __user* buf, size_t count, loff_t *f_pos){

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
  //write recieved value to LED3 (gpio40)
  gpio_set_value(gpio, value);

  *f_pos +=  buf_len;
  return buf_len;
}

//open of LED3 inode and file when open() is called in user-space
int LED3_open(struct inode *inode, struct file *filep){
  int major, minor;

  major = MAJOR(inode->i_rdev);
  minor = MINOR(inode->i_rdev);
  printk("Opening LED3 device: %i, %i\n", major, minor);

  return 0;
}

//release of LED3 when close() is called in user-space
int LED3_release(struct inode *inode, struct file *filep){
  int minor, major;

  major = MAJOR(inode->i_rdev);
  minor = MINOR(inode->i_rdev);
  printk("Releasing LED3 device: %i, %i\n", major, minor);

  return 0;
 }

// file_operations function pointers to this child functions
 struct file_operations my_fops =
 {
   .open = LED3_open,
   .release = LED3_release,
   .read = LED3_read,
   .write = LED3_write,
 };

//initializing resources and driver
static int LED3_init(void){
    //request GPIO of LED3
    printk("Init LED3\n");
    err = gpio_request(gpio, "LED3");
    if(err < 0){
      goto err_req;
    }
    //Set gpio directions and initial value
    err = gpio_direction_output(gpio, 0);
    if(err < 0){
      goto err_dir;
    }
    //specifying major and minor number for LED3
    devno = MKDEV(LED3_MAJOR, LED3_MINOR);
    //Registering device number, checking for error;
    err = register_chrdev_region(devno, NUM_OF_DEVS, "LED3");
    if(err < 0){
      goto err_dir;
    }
    //Char device init
    cdev_init(&LED3_dev, &my_fops);
    //register char device
    err = cdev_add(&LED3_dev, devno, NUM_OF_DEVS);
    if(err < 0){
      goto err_add;
    }

    printk("Init succesful\n");
    return 0; //success!

    //handle errors
    err_add:
      //unregistering device numbers
      unregister_chrdev_region(devno, NUM_OF_DEVS);
    err_dir:
      //freeing claimed gpio
      gpio_free(gpio);
    err_req:
      printk("Init failed\n");
      return err;
}
//releasing resources
static void LED3_exit(void){
  printk("Releasing LED3\n");
  cdev_del(&LED3_dev);
  unregister_chrdev_region(devno, NUM_OF_DEVS);
  gpio_free(gpio);
}

module_init(LED3_init);
module_exit(LED3_exit);
