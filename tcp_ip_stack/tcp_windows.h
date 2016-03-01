#ifndef __TCP_WINDOWS__
#define __TCP_WINDOWS__
#include <sys/types.h>
#include <inttypes.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include "types.h"
#include "common_header.h"
#include <rte_tcp.h>
#include <rte_ip.h>
#include <rte_ether.h>
#include <rte_mbuf.h>
#include <assert.h>
//#include "tcp.h"
#include "tcp_tcb.h"

struct tcb; // forward declaration

// this holds the sequence number and length to which we have continuous data.
/*
struct SequenceLengthPair{
   uint32_t SequenceNumber;
   uint16_t Length;
   struct SequenceLengthPair *Next;
};
*/
struct OutOfSeqPair {
   uint32_t SequenceNumber;
   uint16_t Length; //  
   struct rte_mbuf *mbuf; // needed to hold data buffer.
   uint8_t Flags;  // FIN and SYN are more important here, to adjust seq number.
   int TcpLen;
   uint8_t HasFin;
   struct OutOfSeqPair *Next;
};

struct ReceiveWindow_ {
   int MaxSize;
   uint32_t CurrentSize;
   uint32_t StartSequenceNumber; // needed for buffer maangament
   uint32_t CurrentSequenceNumber; // the next sequence number from which data to socket has to send.
   //struct SequenceLengthPair *SeqPairs;
   struct OutOfSeqPair *SeqPairs;
};
typedef struct ReceiveWindow_ ReceiveWindow;
struct SendSeqPair {
   uint32_t m_StartSeqNumber;
   uint32_t m_EndSeqNumber;
   struct rte_mbuf *m_mbuf;
   struct SendSeqPair *Next;
};

struct SendWindow_ {
   int MaxSize;
   uint32_t CurrentSize;
   uint32_t StartSequenceNumber; // needed for buffer maangament
   uint32_t CurrentSequenceNumber; // the next least sequence number from which data to wire has to acknowledged.
   //struct SequenceLengthPair *SeqPairs;
   struct SendSeqPair *Head;
   struct SendSeqPair *Last;
};
typedef struct SendWindow_ SendWindow;

int
PushDataToSendWindow(struct tcb *ptcb, struct rte_mbuf* mbuf, uint32_t StartSeq, uint32_t EndSeq);  

int
AdjustSendWindow(struct tcb *ptcb, uint32_t AckValue);

ReceiveWindow *AllocWindow(int MaxSize, int CurrentSize);
int FreeWindow(ReceiveWindow *Window);
int PushData(struct rte_mbuf *mbuf, struct tcp_hdr* ptcphdr, uint16_t Length, struct tcb *ptcb);
int GetData(int identifier, unsigned char *Buffer, uint32_t len);
int SendAck(void);
int AddData(unsigned char *data, uint16_t Length, uint32_t SequenceNumber, ReceiveWindow *Window, int identifier);
uint32_t AdjustPair(ReceiveWindow *Window, uint32_t StartSeqNumber, uint16_t Length, struct rte_mbuf *mbuf, int TcpLen, uint8_t TcpFlags);
void InitSocketTcbRing(void);
int DeletePair(struct OutOfSeqPair  *Pair);
#endif
