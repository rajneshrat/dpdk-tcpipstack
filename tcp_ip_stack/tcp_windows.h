#ifndef __TCP_WINDOWS__
#define __TCP_WINDOWS__
#include <sys/types.h>
#include <inttypes.h>
struct SequenceLengthPair{
   uint32_t SequenceNumber;
   uint16_t Length;
   struct SequenceLengthPair *Next;
};

struct ReceiveWindow_ {
   int MaxSize;
   int CurrentSize;
   uint32_t StartSequenceNumber; // needed for buffer maangament
   struct SequenceLengthPair *SeqPairs;
   unsigned char *Data;
};

typedef struct ReceiveWindow_ ReceiveWindow;
ReceiveWindow *AllocWindow(int MaxSize, int CurrentSize);
#endif
