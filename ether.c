#include "types.h"
#include "ether.h"

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
   }
}
