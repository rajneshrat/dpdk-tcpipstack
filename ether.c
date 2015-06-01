#include "types.h"
#include "ether.h"
#include <rte_common.h>

UINT TotalInterface = 0;

struct Interface *InterfaceList = NULL;

uint32_t GetIntAddFromChar(unsigned char *address, uint8_t order)
{
   uint32_t ip_add = 0;
   int i;
   for(i=0;i<4;i++) {
      ip_add = ip_add << 8;
      if(order == 1) {
         ip_add = ip_add | address[3 - i];
      }
      if(order == 0) {
         ip_add = ip_add | address[i];
      }
   }
   return ip_add;
}

void InitInterface(struct Interface *IfList[], UINT Count)
{
   struct Interface *ptr = NULL;
   int i = 0;
   for(i=0; i<Count; i++) {
      ptr = malloc(sizeof(struct Interface));
      memcpy(ptr, IfList[i], sizeof(struct Interface));
      ptr->Next = NULL;
      if(i==0) {
         InterfaceList = ptr;
      }
      else {
         InterfaceList->Next = ptr;
      }
      uint32_t Ipv4Addr = GetIntAddFromChar(ptr->IP, 0);
 //  ptr->IP[0] | ptr->IP[1] << 8 | ptr->IP[2] << 16 | ptr->IP[3] << 24 ;
      //printf("assembled mac address = %x\n", Ipv4Addr);
      add_mac(Ipv4Addr, ptr->HwAddress);
   }
}

uint8_t GetInterfaceMac(uint8_t InterfaceNumber, uint8_t *mac)
{
   struct Interface *temp = NULL;
   temp = InterfaceList;

   while(temp && (temp->InterfaceNumber != InterfaceNumber)) {
      temp = temp->Next;
   }

   if(temp) {
      memcpy(mac, temp->HwAddress, 6);
      return 1;
   }
   return 0;
}


