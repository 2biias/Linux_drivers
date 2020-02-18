
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

int main(char argc, char* argv[]){

  int fd, fdstatus, bytes;

  char gpio[3];
  gpio[2] = '\0';
  switch(argv[1][0]){
    case '1' : //if LED1
      gpio[0] = '2';
      gpio[1] = '6';
      break;
    case '2' : //if LED2
      gpio[0] = '2';
      gpio[1] = '0';
      break;
    case '3' :
      gpio[0] = '2';
      gpio[1] = '1';
      break;
    default :
      printf("Could not switch led %s\nexitting...\n", argv[1]);
      exit(1);
  }
  /*string to check if given gpio already exists in gpio directory */
  char gpiodir[strlen("/sys/class/gpio/gpio") + strlen(gpio) + 1];
  snprintf(gpiodir, sizeof gpiodir, "/sys/class/gpio/gpio%s", gpio);

  /* if file doesnt exist, create and write a new gpio to export */
  if(access(gpiodir, F_OK) == -1){

    fd = open("/sys/class/gpio/export", O_WRONLY | O_CREAT); //ONLY WRITE
    if(fd == -1){
      printf("Error opening export-file with write permissions\n");
      exit(1);
    }
    /*Writing to export*/
    bytes = write(fd, gpio, strlen(gpio));
    if(bytes == -1){
      printf("Error writing to export\n");
      exit(1);
    }
    fdstatus = close(fd);
    if(fdstatus == -1){
      printf("Error closing fd\n");
      exit(1);
    }
  }

  /*write to direction*/
  char gpiodirection[strlen("/sys/class/gpio/gpio") + strlen(gpio) + strlen("/direction") + 1];
  snprintf(gpiodirection, sizeof gpiodirection, "/sys/class/gpio/gpio%s/direction", gpio);

  fd = open(gpiodirection, O_WRONLY);
  if(fd == -1){
    printf("Error opening diretion-file with write permissions\n");
    exit(1);
  }
  /*Writing to direction*/
  bytes = write(fd, "out", 3);
  if(bytes == -1){
    printf("Error writing to direction\n");
    exit(1);
  }
  fdstatus = close(fd);
  if(fdstatus == -1){
    printf("Error closing fd\n");
    exit(1);
  }
  /*Open and write value to switch led*/
  char gpioval[strlen("/sys/class/gpio/gpio") + strlen(gpio) + strlen("/value") + 1];
  snprintf(gpioval, sizeof gpioval, "/sys/class/gpio/gpio%s/value", gpio);

  fd = open(gpioval, O_WRONLY);
  if(fd == -1){
    printf("Error opening value-file with write permissions\n");
    exit(1);
  }
  else{
    int sleep;
    while(1){
      sleep = usleep(1000000);
      if(sleep == -1){
        printf("error sleeping");
        exit(1);
      }
      bytes = write(fd, "1", 1);
      if(bytes == -1){
        printf("Error writing to value\n");
        exit(1);
      }
      sleep = usleep(1000000);
      if(sleep == -1){
        printf("error sleeping");
        exit(1);
      }
      bytes = write(fd, "0", 1);
      if(bytes == -1){
        printf("Error writing to value\n");
        exit(1);
      }
    }
  }
}
