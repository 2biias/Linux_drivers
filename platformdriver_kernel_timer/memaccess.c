
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

/* Peripherals base addr on BCM2835 */
#define BCM2835_PERIPH_BASE 0x20000000
/* Peripherals size*/
#define BCM2835_PERIPH_SIZE 0x01000000

//Function prototypes for memory access
void *mmap(void *addr, size_t length, int prot, int flags,
                    int fd, off_t offset);
int munmap(void *addr, size_t length);

//Function prototypes for utility function
void reg_setbit(volatile unsigned int * addr, unsigned int bit);
void reg_clrbit(volatile unsigned int * addr, unsigned int bit);
unsigned int reg_rd(volatile unsigned int * addr);
void reg_wr(volatile unsigned int* addr, unsigned int value);


int main(void){
  //Pointer to Virtual base address
  unsigned int *virt_base;
  //Let linux choose mapping to virtual memory
  unsigned int *phys_base_addr = NULL;
  //Offset for gpio, clock and PWM
  unsigned int gpio_offset = 0x200000;
  unsigned int clock_offset = 0x101000;
  unsigned int pwm_offset = 0x20c000;

  int fd = open("/dev/mem", O_RDWR, 0);

  virt_base = mmap(phys_base_addr, BCM2835_PERIPH_SIZE, PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd, BCM2835_PERIPH_BASE);

  //Peripherals base and mapped base
  printf("Peripherals base: %x\n", BCM2835_PERIPH_BASE);
  printf("Mapped peripheral base: %p\n", virt_base);

  //Pointer to GPIO peripheral. byte -> int/word
  volatile unsigned int *virt_gpio_addr = virt_base+gpio_offset/4;
  printf("virt_gpio_addr: hex uint: %x, ptr-address: %p\n", (unsigned int)*virt_gpio_addr, virt_gpio_addr);
  //Pointer to CLOCK peripheral
  volatile unsigned int *virt_clock_addr = virt_base+clock_offset/4;
  printf("virt_clock_addr: hex uint: %x, ptr-address: %p\n", (unsigned int)*virt_clock_addr, virt_clock_addr);
  //Pointer to PWM peripheral
  volatile unsigned int *virt_pwm_addr = virt_base+pwm_offset/4;
  printf("virt_pwm_addr: hex uint: %x, ptr-address: %p\n", (unsigned int)*virt_pwm_addr, virt_pwm_addr);


  void reg_setbit(volatile unsigned int *addr, unsigned int bit){
    if(bit > 31 || bit < 0){
      printf("Bit number exceeds unsiged int\n");
      return;
    }
    //Read value at addr
    unsigned int tmp = (unsigned int)*addr;
    //Set bit
    tmp |= 1UL << bit;
    //Change value at addr
    *addr = tmp;
    printf("addr %p, setbit %u\n", addr, bit);
  }

  void reg_clrbit(volatile unsigned int *addr, unsigned int bit){
    if(bit > 31 || bit < 0){
      printf("Bit number exceeds unsigned int\n");
      return;
    }
    unsigned int tmp = (unsigned int)*addr;
    tmp &= ~(1U << bit);
    *addr = tmp;
    printf("addr %p, clrbit %u\n", addr, bit);
  }

  unsigned int reg_rd(volatile unsigned int *addr){
    unsigned int value = (unsigned int)*addr;
    printf("read addr %p, value 0x%X\n", addr, value);
    return value;
  }

  void reg_wr(volatile unsigned int* addr, unsigned int value){
    //Set value at addr, but devide to get bytes to words
    *addr = value;
    printf("write addr %p, value 0x%X\n", addr, *addr);
  }

  //************************ GPIO20 switch on/off ****************************//
  /*
  //GPIO20 configuration, as output (001)
  __sync_synchronize();
  volatile unsigned int *gpfsel2 = virt_gpio_addr + (0x08/4);
  reg_rd(gpfsel2);
  reg_setbit(gpfsel2, 0);
  reg_clrbit(gpfsel2, 1);
  reg_clrbit(gpfsel2, 2);
  reg_rd(gpfsel2);
  __sync_synchronize();
  //Turn GPIO20 once
  printf("Turning gpio20 on\n");
  volatile unsigned int *gpset0 = virt_gpio_addr + (0x1C/4);
  reg_rd(gpset0);
  reg_setbit(gpset0, 20);
  reg_rd(gpset0);
  __sync_synchronize();
  //Wait 2 seconds and turn off with clear
  sleep(2);
  printf("Turning gpio20 off\n");
  volatile unsigned int *gpclr0 = virt_gpio_addr + (0x28/4);
  reg_rd(gpclr0);
  reg_setbit(gpclr0, 20);
  reg_rd(gpclr0);
  */

  //***************************** PWM at gpio19 ******************************//

  //Setting up gpio19 as PWM1 output, memory barrier is called first
  __sync_synchronize();
  printf("Memory Barrier\n");
  //Funcion select 1, alternativ function 5 (010) at bit 29-27
  volatile unsigned int *gpfsel1 = virt_gpio_addr + (0x04/4);
  volatile unsigned int tmp = reg_rd(gpfsel1);
  tmp |= 0x10000000;
  reg_wr(gpfsel1, tmp);
  __sync_synchronize();
  printf("Memory Barrier\n");

  //Setting up clock divider to 16
  volatile unsigned int *cm_pwm_div = virt_clock_addr + (0xa4/4);
  tmp = reg_rd(cm_pwm_div);
  //Clock divisor password = 0b01011010 from temp, bit 31-24
  //PWM Clock Divisor DIVI, 16=0b000000010000, bit 23-12
  //PWM Clock divisor DIVF, 0, bit 11-0
  tmp |= 0x5A010000;
  //Write tmp to Clock divisor register
  reg_wr(cm_pwm_div, tmp);

  //Setting up clock control register
  volatile unsigned int *cm_pwm_ctl = virt_clock_addr + (0xa0/4);
  tmp = reg_rd(cm_pwm_ctl);
  //Clock control password = 0b01011010, bits 31-24
  //Enable clock EN=1, bit 4
  //External oscillator 19.2MHz (SRC=OSC), bit 3-0
  tmp |= 0x5A000011;
  //Write tmp to Clock Control register
  reg_wr(cm_pwm_ctl, tmp);
  __sync_synchronize();
  printf("Memory Barrier\n");

  //Setup PWM ctl register
  volatile unsigned int *pwm_ctl = virt_pwm_addr;
  tmp = reg_rd(pwm_ctl);
  //MSEN2 (MS), bit 15
  //Enable channel 2, bit 8
  tmp |= 0x8100;
  //Write to PWM control register
  reg_wr(pwm_ctl, tmp);

  //Set range RNG2 to 1024
  volatile unsigned int *rng2 = virt_pwm_addr + (0x20/4);
  //Writing 1024 to RNG2 register, bit 31-0
  reg_wr(rng2, 0x400);

  //Setting DAT2 register
  volatile unsigned int *dat2 = virt_pwm_addr + (0x24/4);
  //Writing 512 to dat2 register, bit 31-0
  reg_wr(dat2, 0x200);
  __sync_synchronize();
  printf("Memory Barrier\n");
  return 0;
}
