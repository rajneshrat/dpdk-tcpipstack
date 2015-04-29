#include "types.h"

#define HW_ADDRESS_LEN 6
UINT TotalInterface = 0

struct Interface {
   unsigned char Hw_Address[HW_ADDRESS_LEN];
   unsigned int InterfaceNumber;
   struct Interface *Next;
};

struct Interface *InterfaceList = NULL;

void Init()
{
   InterfaceList = malloc(sizeof(struct Interface));  
}
