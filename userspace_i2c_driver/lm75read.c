#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

int main(void){

  int fd, bytesread, i2c_handle, i2c_addr;
  i2c_addr = 0x48; //address found with $i2cdetect -y 1

  char data[3]; //temperature with decimal

  char device[strlen("/dev/i2c-1") + 1];
  snprintf(device, sizeof device, "/dev/i2c-1");

  printf("device: %s\n", device);
  fd = open(device, O_RDONLY);
  if(fd == -1){
    printf("Error opening file\n%s", strerror(errno));
    exit(1);
  }

  i2c_handle = ioctl(fd, 0x0703, i2c_addr); // i2cdev sys call (0x0703) to set I2C addr
  if(i2c_handle == -1){
    printf("Error setting i2c addr to bus 1\n%s", strerror(errno));
    exit(1);
  }
  bytesread = read(fd, data, 2); //read temperature

  if(bytesread == -1){
    printf("Error: %s\n%s", device, strerror(errno));
    exit(1);
  }else{
    printf("Temperature reads from lm75:\n");
  }

  //keep reading temperature until terminated by user or error
  while(1){
    printf("%d,%d\n", data[0], (data[1] >= 128 ? 5 : 0));
    usleep(1000000);
    bytesread = read(fd, data, 2); //read temperature with decimal
    if(bytesread == -1){
      printf("Error reading temperature from %s\n%s", device, strerror(errno));
      exit(1);
    }
  }
}
