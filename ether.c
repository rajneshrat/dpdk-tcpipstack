#include "types.h"
#include "ether.h"
#include <rte_common.h>

UINT TotalInterface = 0;

struct Interface *InterfaceList = NULL;

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
      uint32_t Ipv4Addr = ptr->IP[0] | ptr->IP[1] << 8 | ptr->IP[2] << 16 | ptr->IP[3] << 24 ;
      printf("assembled mac address = %x\n", Ipv4Addr);
      add_mac(Ipv4Addr, ptr->HwAddress);
   }
}
