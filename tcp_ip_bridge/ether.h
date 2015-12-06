#ifndef __EHTER__
#define __EHTER__
#include <rte_common.h>
#define HW_ADDRESS_LEN 6
#define MAX_INTERFACES 10

struct Interface {
   unsigned char HwAddress[HW_ADDRESS_LEN];
   uint8_t InterfaceNumber;
   unsigned char IP[4];
   struct Interface *Next;
};

unsigned char InterfaceHwAddr[MAX_INTERFACES][HW_ADDRESS_LEN];
uint32_t GetIntAddFromChar(unsigned char *address, uint8_t order);

uint8_t GetInterfaceMac(uint8_t InterfaceNumber, uint8_t *mac);

extern struct Interface *InterfaceList;
#endif
