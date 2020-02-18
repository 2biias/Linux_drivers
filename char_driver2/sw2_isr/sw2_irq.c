#include <linux/gpio.h> //API for lowlevel handling of gpio
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/module.h>

//interrupt-related
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>

//free license
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gruppe 42");
MODULE_DESCRIPTION("Chardriver til SW2");

//major 64 for SW's //number of devices are 0.
#define SW2_MAJOR 64
#define SW2_MINOR 0
#define NUM_OF_DEVS 1

//char device struct
static struct cdev sw_dev;

//Declaration of interruptible wait queue and flag
static DECLARE_WAIT_QUEUE_HEAD(wq);
static int flag = 0;

//variables and flags
static int devno;
static int err;
static uint8_t gpio = 16; //gpio of SW2.
static int irq_line;

//interrup handler
static irqreturn_t gpio_isr_handler(int irq_line, void* dev_id){
  flag = 1;
  wake_up_interruptible(&wq);
  printk("IRQ event on line: %i\n", irq_line);
  return IRQ_HANDLED;
}

//read data of sw2 inode
ssize_t sw2_read(struct file* filep, char __user* buf, size_t count, loff_t* f_pos){

  printk("Read function called\n");

  //room for 0-255 and EOF
  char readbuffer[2];

  printk("Blocking read going to sleep\n");
  //Block read until interrupt event triggers
  wait_event_interruptible(wq, flag != 0);
  //reset flag
  flag = 0;
  printk("Blocking read awake\n");

  //read from sw2
  sprintf(readbuffer, "%d", gpio_get_value(gpio));
  int buf_len = strlen(readbuffer) + 1;
  //truncate to requested lenght, good practice and suffecient for this purpose
  buf_len = buf_len > count ? count : buf_len;
  //copy data to user-space, returns bytes that could not be written to US
  if(copy_to_user(buf, readbuffer, buf_len)){
    printk(KERN_ALERT "Error: cpoy_to_user\n");
  }
  //offset
  *f_pos += buf_len;
  return buf_len;
}

//open of sw2 inode and file when open() is called in user-space
int sw2_open(struct inode *inode, struct file *filep){
  int major, minor;

  major = MAJOR(inode->i_rdev);
  minor = MINOR(inode->i_rdev);
  printk("Opening sw2 device: %i, %i\n", major, minor);

  return 0;
}

//release of sw2 when close() is called in user-space
int sw2_release(struct inode *inode, struct file *filep){
  int minor, major;

  major = MAJOR(inode->i_rdev);
  minor = MINOR(inode->i_rdev);
  printk("Releasing sw2 device: %i, %i\n", major, minor);

  return 0;
 }

// file_operations function pointers to this child functions
 struct file_operations my_fops =
 {
   .open = sw2_open,
   .release = sw2_release,
   .read = sw2_read,
 };

//initializing resources and driver
static int sw2_init(void){
    //request GPIO of SW2
    printk("Init sw2\n");
    err = gpio_request(gpio, "SW2");
    if(err < 0){
      goto err_req;
    }
    //Set gpio directions
    err = gpio_direction_input(gpio);
    if(err < 0){
      goto err_dir;
    }
    //specifying major and minor number for sw2
    devno = MKDEV(SW2_MAJOR, SW2_MINOR);
    //Registering device number, checking for error;
    err = register_chrdev_region(devno, NUM_OF_DEVS, "SW2");
    if(err < 0){
      goto err_dir;
    }
    //Char device init
    cdev_init(&sw_dev, &my_fops);
    //register char device
    err = cdev_add(&sw_dev, devno, NUM_OF_DEVS);
    if(err < 0){
      goto err_add;
    }
    //get irq_line
    irq_line = gpio_to_irq(gpio);
    printk("irqline of %d is %d\n", gpio, irq_line);

    //request and enable irq
    err = request_irq(irq_line, gpio_isr_handler, IRQF_TRIGGER_LOW, "SW2", &sw_dev);
    if(err < 0){
      if(err == -EBUSY){
        printk("Another device is using the requested interrupt line\n");
      }else{
        printk("Reqeust of irq failed\n");
      }
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
static void sw2_exit(void){
  printk("Releasing sw2\n");
  free_irq(irq_line, &sw_dev);
  cdev_del(&sw_dev);
  unregister_chrdev_region(devno, NUM_OF_DEVS);
  gpio_free(gpio);
}

module_init(sw2_init);
module_exit(sw2_exit);
