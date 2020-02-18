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
  int fd, fdstatus, bytesread;
  char buffer[2];

  fd = open("/dev/sw2", O_RDONLY);
  if(fd == -1){
    printf("Error opening file\n%s\n", strerror(errno));
    exit(1);
  }

  while(1){
    //read from sw2
    bytesread = read(fd, buffer, 2);
    if(bytesread == -1){
      printf("Error reading from sw2\n%s\n", strerror(errno));
      exit(1);
    }
    printf("%c\n", buffer[0]);
    usleep(1000000);
  }

  fdstatus = close(fd);
  if(fdstatus == -1){
    printf("Error closing fd\n");
    exit(1);
  }
  return 0;
}
