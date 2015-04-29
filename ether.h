#ifndef __EHTER__
#define __EHTER__
#define HW_ADDRESS_LEN 6
struct Interface {
   unsigned char HwAddress[HW_ADDRESS_LEN];
   unsigned int InterfaceNumber;
   unsigned char IP[4];
   struct Interface *Next;
};
#endif
