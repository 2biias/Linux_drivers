#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

int main(){
  int fd, fd2, fdstatus, bytesread, byteswritten;
  char buffer[2];

  //open sw2
  fd = open("/dev/sw2", O_RDONLY);
  if(fd == -1){
    printf("Error opening file\n%s\n", strerror(errno));
    exit(1);
  }
  //open LED3_dev
  fd2 = open("/dev/LED3", O_RDWR);
  if(fd2 == -1){
    printf("Error opening file\n%s\n", strerror(errno));
  }
  while(1){
    //read from sw2
    bytesread = read(fd, buffer, 2);
    if(bytesread == -1){
      printf("Error reading from sw2\n%s\n", strerror(errno));
      exit(1);
    }
    byteswritten = write(fd2, buffer, 2);
    if(byteswritten == -1){
      printf("Error writing to LED3\n%s\n", strerror(errno));
      exit(1);
    }
  }

  fdstatus = close(fd);
  if(fdstatus == -1){
    printf("Error closing fd\n");
    exit(1);
  }
  return 0;
  fdstatus = close(fd2);
  if(fdstatus == -1){
    printf("Error closing fd\n");
    exit(1);
  }
  return 0;
}
