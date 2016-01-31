#include "tcp_windows.h"
#include "tcp_tcb.h"
#include "tcp.h"

//static struct rte_ring *socket_tcb_ring_send;

//static const char *TCB_TO_SOCKET = "TCB_TO_SOCKET";
static const char *_MSG_POOL = "MSG_POOL";
const unsigned int pool_size = 1024;
const unsigned int socket_tcb_ring_size = 1024;
const unsigned int buffer_size = 1500;
static struct rte_mempool *buffer_message_pool;

void InitSocketTcbRing(void)
{
 //  socket_tcb_ring_send = rte_ring_create(TCB_TO_SOCKET, socket_tcb_ring_size, SOCKET_ID_ANY, 0);
   buffer_message_pool = rte_mempool_create(_MSG_POOL, pool_size,
            buffer_size, 32, 0,
            NULL, NULL, NULL, NULL,
            SOCKET_ID_ANY, 0);
   if(buffer_message_pool == NULL) {
      printf("ERROR **** socket tcb Message pool send side failed\n");
   }
   else {
      printf("socket tcb send side OK.\n");
   }
}


int AdjustPair(ReceiveWindow *Window, uint32_t StartSeqNumber, uint16_t Length)
{
   struct SequenceLengthPair *Pair = Window->SeqPairs;
   struct SequenceLengthPair *LastPair = NULL;
   struct SequenceLengthPair *NextPair = NULL;
   while(Pair && (Pair->SequenceNumber <= StartSeqNumber)) {
      LastPair = Pair;
      Pair = Pair->Next;
   }
   Pair = LastPair;
   if(Pair && (Pair->SequenceNumber + Pair->Length) > StartSeqNumber) {  // if we fall inside existing pair
      if(Pair->Length < (StartSeqNumber - Pair->SequenceNumber + Length)) { // do we need to increase length of existing pair
         Pair->Length = Length + StartSeqNumber - Pair->SequenceNumber;
      }
   }
   else { // create new pair
      struct SequenceLengthPair *NewPair = malloc(sizeof (struct SequenceLengthPair));
      NewPair->SequenceNumber = StartSeqNumber;
      NewPair->Length = Length;
      if(Pair) {
         NewPair->Next = Pair->Next;
         Pair->Next = NewPair;
      }
      else {
         NewPair->Next = NULL;
         Window->SeqPairs = NewPair;
      }
      Pair = NewPair;  // set pair as newpair to do adjustment as down.
   }
   NextPair = Pair->Next;
   while(NextPair && ((Pair->SequenceNumber + Pair->Length) > NextPair->SequenceNumber)) { // after increasing length we may have overlaps next few existing pairs.
      if((Pair->SequenceNumber + Pair->Length) < (NextPair->SequenceNumber + NextPair->Length)){ // do we need to adjust our pair length.
         Pair->Length = NextPair->SequenceNumber - Pair->SequenceNumber + NextPair->Length; 
      }
      Pair->Next = NextPair->Next;
      free(NextPair);
      NextPair = Pair->Next;
   }
   return 0;
}
static int PushDataInQueue(int identifier)
{
   unsigned char Buffer[2000];
   int len = 0;
   struct tcb *ptcb = get_tcb_by_identifier(identifier);
   if((len = GetData(identifier, Buffer))) {
      void *msg = NULL;
      if (rte_mempool_get(buffer_message_pool, &msg) < 0) {
         printf ("Failed to get message buffer\n");
/// / put assert ;
      }
      memcpy(msg, Buffer, len);
      if (rte_ring_enqueue(ptcb->socket_tcb_ring_send, msg) < 0) {
         printf("Failed to send message - message discarded\n");
         rte_mempool_put(buffer_message_pool, msg);
      }
   }
   return 0;
}

int AddData(unsigned char *data, uint16_t Length, uint32_t SequenceNumber, ReceiveWindow *Window, int identifier)
{
   if(Window->StartSequenceNumber == 0) {
      Window->StartSequenceNumber = SequenceNumber;
   }
   uint16_t CurrentPointer = (SequenceNumber - Window->StartSequenceNumber) % Window->MaxSize;
   memcpy(Window->Data + CurrentPointer, data, Length);
   printf("pushing data from index %d to length %d\n", CurrentPointer, Length);
   PushDataInQueue(identifier);
   return 0;
}

int GetData(int identifier, unsigned char *Buffer)
{
   struct tcb *ptcb = get_tcb_by_identifier(identifier);
   if(ptcb == NULL) {
      return -1;
   }
   ReceiveWindow *Window = (ReceiveWindow *) ptcb->RecvWindow;
   struct SequenceLengthPair *Pair = Window->SeqPairs;
   if(Pair) {
      int index = (Pair->SequenceNumber - Window->StartSequenceNumber) % Window->MaxSize;
      printf("copying data from index %d to length %d\n", index, Pair->Length);
      memcpy(Buffer, Window->Data + index, Pair->Length);
      Window->SeqPairs = Pair->Next;
      free(Pair);
      return Pair->Length;
   }
   return 0;
}

int SendAck(void)
{
   printf("Sending Syn Ack\n");
   return 0;

}

int PushData(unsigned char *data, struct tcp_hdr* ptcphdr, uint16_t Length, struct tcb *ptcb)
{
// future add assert if SequenceNumber is 0.
   uint32_t SequenceNumber = ntohl(ptcphdr->sent_seq);
   ReceiveWindow *Window = (ReceiveWindow *) ptcb->RecvWindow;
   if(Window->SeqPairs && ((SequenceNumber - Window->SeqPairs->SequenceNumber + Length) < Window->CurrentSize)) { // sequence number out of receive window size 
      printf("WARNING :: Out of window data, dropping all\n");
      return -1;
   }
   ptcb->ack = SequenceNumber + Length;
   if(ptcphdr->tcp_flags & FIN || ptcphdr->tcp_flags & SYN) {
      ptcb->ack = ptcb->ack + 1;
   }
   printf("**** setting ack for %u\n", ptcb->ack); 
   sendack(ptcb);
   AdjustPair(Window, SequenceNumber, Length);
   struct SequenceLengthPair *Pair = Window->SeqPairs;
   ptcb->ack = Pair->SequenceNumber + Pair->Length;
   AddData(data, Length, SequenceNumber, Window, ptcb->identifier);
   return 0;
   // unlock the sem if socket waiting for it.
}

int FreeWindow(ReceiveWindow *Window)
{
   free(Window->Data);
   free(Window);
   return 0;
}

ReceiveWindow *AllocWindow(int MaxSize, int CurrentSize)
{
   ReceiveWindow *Window = malloc(sizeof(ReceiveWindow));
   Window->MaxSize = MaxSize;
   Window->CurrentSize = CurrentSize;
   Window->StartSequenceNumber = 0;
   Window->SeqPairs = NULL;
   Window->Data = malloc(MaxSize);
   return Window;
}
