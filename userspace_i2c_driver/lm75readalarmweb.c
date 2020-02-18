#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

void initLED(){

  int fd, byteswritten, fdstatus;
  //if gpio26 doesnt exit, export it
  if(access("/sys/class/gpio/gpio26", F_OK) == -1){

    fd = open("/sys/class/gpio/export", O_WRONLY | O_CREAT); //ONLY WRITE
    if(fd == -1){
      printf("Error opening export-file for LED1 with write permissions\n");
      exit(1);
    }
    /*Writing to export*/
    byteswritten = write(fd, "26", 2);
    if(byteswritten == -1){
      printf("Error writing LED1 to export\n");
      exit(1);
    }
    fdstatus = close(fd);
    if(fdstatus == -1){
      printf("Error closing fd for export\n");
      exit(1);
    }
  }else{
    fd = open("/sys/class/gpio/gpio26/direction", O_WRONLY);
    if(fd == -1){
      printf("Error opening LED1 diretion-file with write permissions\n");
      exit(1);
    }
    /*Writing to direction*/
    byteswritten = write(fd, "out", 3);
    if(byteswritten == -1){
      printf("Error writing to direction\n");
      exit(1);
    }
    fdstatus = close(fd);
    if(fdstatus == -1){
      printf("Error closing fd\n");
      exit(1);
    }
  }
}

void ToggleLED(char* state){

  int byteswritten, fd, fdstatus;

  fd = open("/sys/class/gpio/gpio26/value", O_WRONLY);
  if(fd == -1){
    printf("Error opening value-file with write permissions\n");
    exit(1);
  }
  if(*state){
    byteswritten = write(fd, "1", 1);
  }else{
    byteswritten = write(fd, "0", 1);
  }
  fdstatus = close(fd);
  if(fdstatus == -1){
    printf("Error closing fd\n");
    exit(1);
  }
}

void write2web(char* data){
  int byteswritten, fd, fdstatus;
  char information[strlen("<html><body><h1> Temperature: c,c </h1></body></html>") + 1];

  fd = open("/www/pages/index.html", O_WRONLY);
  if(fd == -1){
    printf("Error opening webpage with write permissions\n");
    exit(1);
  }

  snprintf(information, sizeof information, "<html><body><h1> Temperature: %d,%d </h1></body></html>", data[0], (data[1] >= 128 ? 5: 0));

  byteswritten = write(fd, information, strlen(information));
  if(byteswritten == -1){
    printf("Error writing to webpage\n");
    exit(1);
  }
  fdstatus = close(fd);
  if(fdstatus == -1){
    printf("Error closing fd\n");
    exit(1);
  }
}

int main(void){

  int fd, bytesread, i2c_handle, i2c_addr;
  char state[1];
  state[0] = 0; //state of LED1.
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
    ToggleLED(state);
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
    if(data[0] > 34){
      printf("Warning high temperature\n");
      state[0] = 1;
    }else{
      state[0] = 0;
    }
    ToggleLED(state); //write state
    write2web(data); //write data to webpage
  }
}
