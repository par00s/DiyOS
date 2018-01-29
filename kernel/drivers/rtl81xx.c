#include <pci.h>
#include <x86/x86.h>
#include <small.h>

const struct _rtl81xx_id {
  uint16_t  vendor;
  uint16_t  device;
} rtl81xx_id[] = {
  { 0x10EC, 0x8168 }, // RTL-8029
  { 0, 0}
};

#define RX_DESCRIPTORS    64    // can be up to 1024
#define RX_BUFFER_SIZE    1024

struct x_descriptor {
    uint32_t cmd_status;
    uint32_t reserved;
    uint32_t low_mem; // 0 .. 32
    uint32_t hi_mem;  // 33 .. 64 (for 64-bits system)
};

uint8_t               rx_buffer[RX_BUFFER_SIZE];  // each
struct x_descriptor   rx_descriptor[64] __attribute__ ((aligned (256)));

uint16_t  rtl81xx_io_base;
uint8_t   rtl81xx_MAC[6];
uint8_t   rtl81xx_bus, rtl81xx_dev, rtl81xx_function;

void
rtl81xx_handler() {
  printf(".");
}
IRQN(3,rtl81xx_handler);

void
setup_rtl81xx() {
  uint32_t i;
  uint8_t rtl81xx_present;

  printf("Realtek 81xx network card: ");

  rtl81xx_present = 0;
  i = 0;
  while(rtl81xx_id[i].vendor) {
    if(pci_find(rtl81xx_id[i].vendor, rtl81xx_id[i].device, &rtl81xx_bus,
                &rtl81xx_dev, &rtl81xx_function)) {
        rtl81xx_present = 1;
        break;
      }
    i++;
  }

  if(rtl81xx_present == 0) {
    printf("not found\n");
    return;
  }

  // okay, device was found

  //get IO base addr
  rtl81xx_io_base = pci_read(rtl81xx_bus, rtl81xx_dev, rtl81xx_function, PCI_BAR0);
  rtl81xx_io_base = rtl81xx_io_base & 0xFFFC; // 4-byte aligned

  pci_write(rtl81xx_bus, rtl81xx_dev, rtl81xx_function, 0x3c, 0x103); // IRQ3
  rtl81xx_reset();

  printf("\n\tio_base=0x%x, ", rtl81xx_io_base);

  for(i = 0; i < 6; i++)
    rtl81xx_MAC[i] = inb(rtl81xx_io_base + i);

  printf("MAC=%x:%x:%x:%x:%x:%x\n", rtl81xx_MAC[0],rtl81xx_MAC[1],rtl81xx_MAC[2],
                        rtl81xx_MAC[3],rtl81xx_MAC[4],rtl81xx_MAC[5]);

  // starts configuration

  outw(rtl81xx_io_base + 0xE0, 0x20); // configure C+ Command. no VLAN de-taging; Checksum offload.
  outb(rtl81xx_io_base + 0x37, 0x00);  // configure Command. disable RX/TX

  // setup rx descriptors
  for( i = 0; i < RX_DESCRIPTORS; i++) {
      rx_descriptor[i].cmd_status = 0x80000000 | (RX_BUFFER_SIZE & 0x3FFF);
      rx_descriptor[i].hi_mem = 0;
      rx_descriptor[i].low_mem = (uint32_t)VIRTUAL_TO_PHYSICAL(&rx_buffer);
  }
  rx_descriptor[i-1].cmd_status |= 0x40000000; // set End Of Ring flag on last entry

  rtl81xx_unlock();   // allow modify config registers

  outb(rtl81xx_io_base + 0x3C, 0xFF); // enable all interrupts
  outb(rtl81xx_io_base + 0x3D, 0xFF);

  // about RX packets
  outw(rtl81xx_io_base + 0xDA, 8192);   // max  tx packet size
  outl(rtl81xx_io_base + 0x44, 0xE70F); // accept anything (test)

  // about TX packets
  //TODO:

  //outb(rtl81xx_io_base + 0x52, 0); // configure LED

  outw(rtl81xx_io_base + 0xE4, (uint32_t)VIRTUAL_TO_PHYSICAL(&rx_descriptor));

  outb(rtl81xx_io_base + 0x37, 0x0C);  // configure Command. enable RX/TX
  rtl81xx_lock();

  irq_install(3,irq3);
}

void
rtl81xx_reset() {
    outb(rtl81xx_io_base + 0x37, 0x10);
    while(inb(rtl81xx_io_base + 0x37) && 0x10);
}

void
rtl81xx_unlock() {
    outb(rtl81xx_io_base + 0x50, 0xC0);
}

void
rtl81xx_lock() {
    outb(rtl81xx_io_base + 0x50, 0x0);
}
