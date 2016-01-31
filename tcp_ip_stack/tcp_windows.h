#ifndef __TCP_WINDOWS__
#define __TCP_WINDOWS__
#include <sys/types.h>
#include <inttypes.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include "types.h"
#include "common_header.h"
//#include "tcp.h"
#include "tcp_tcb.h"

struct SequenceLengthPair{
   uint32_t SequenceNumber;
   uint16_t Length;
   struct SequenceLengthPair *Next;
};

struct ReceiveWindow_ {
   int MaxSize;
   uint32_t CurrentSize;
   uint32_t StartSequenceNumber; // needed for buffer maangament
   struct SequenceLengthPair *SeqPairs;
   unsigned char *Data;
};

typedef struct ReceiveWindow_ ReceiveWindow;
ReceiveWindow *AllocWindow(int MaxSize, int CurrentSize);
int FreeWindow(ReceiveWindow *Window);
int PushData(unsigned char *data, struct tcp_hdr* ptcphdr, uint16_t Length, struct tcb *ptcb);
int GetData(int identifier, unsigned char *Buffer);
int SendAck(void);
int AddData(unsigned char *data, uint16_t Length, uint32_t SequenceNumber, ReceiveWindow *Window, int identifier);
int AdjustPair(ReceiveWindow *Window, uint32_t StartSeqNumber, uint16_t Length);
void InitSocketTcbRing(void);
#endif
