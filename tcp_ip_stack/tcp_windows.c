#include "tcp_windows.h"
#include "tcp_tcb.h"
#include "tcp.h"
#include "logger.h"

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

int DeletePair(struct OutOfSeqPair  *Pair)
{
   logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "Deleting Seq Pair with Seq No. %u\n", Pair->SequenceNumber);
   rte_pktmbuf_free(Pair->mbuf);
   free(Pair);
   return 0;
}

// This holds the out of sequence packets. Here we store the complete mbuf and create the link list of them.
// The logic here is simple, just add the new incoming buffer and then adjust the complete list if we need to delete any node.
//
uint32_t AdjustPair(ReceiveWindow *Window, uint32_t StartSeqNumber, uint16_t Length, struct rte_mbuf *mbuf, int TcpLen, uint8_t TcpFlags)
{
   struct OutOfSeqPair *Pair = Window->SeqPairs;
   struct OutOfSeqPair *PrePair = NULL;
   struct OutOfSeqPair *NextPair = NULL;
   while(Pair && (Pair->SequenceNumber <= StartSeqNumber)) {
      PrePair = Pair;
      Pair = Pair->Next;
      // get the pair whose next is more than this Seq no.
   }
   NextPair = Pair;
   Pair = PrePair; // we will add new pair after this.
   struct OutOfSeqPair *NewPair = malloc(sizeof (struct OutOfSeqPair));
   NewPair->SequenceNumber = StartSeqNumber;
   NewPair->Length = Length;
   NewPair->mbuf = mbuf;
   NewPair->TcpLen = TcpLen;
   NewPair->HasFin = TcpFlags & TCP_FLAG_FIN;
   NewPair->Flags = TcpFlags & TCP_FLAG_FIN;
   NewPair->Next = NextPair;
   if(Pair) {
      Pair->Next = NewPair;
   }
   else {
      Window->SeqPairs = NewPair;
   }
   logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "adding new seq pair with seq no %u length %u\n", StartSeqNumber, Length);
// now delete extra pairs -
   Pair = Window->SeqPairs;
   NextPair = Pair->Next;
   PrePair = NULL;
   while(Pair && NextPair) {
       assert(Pair->SequenceNumber < NextPair->SequenceNumber);
       if(Pair->SequenceNumber == NextPair->SequenceNumber) {
           // only one pair should exist now, and the one with more length.
           if(Pair->Length >= NextPair->Length) {
               Pair->Next = NextPair->Next;
               DeletePair(NextPair);
           }
           else {
                if(PrePair) {
                    PrePair->Next = NextPair;
                }
                else {
                    Window->SeqPairs = NextPair;
                }
                DeletePair(Pair);
                Pair = NextPair; // set the current pair now, as all adjustment for next and pre will be done using it.
           }
       }
       else {
           // we need current Pair as it has somethig at starting but do we need its next also.
           if((Pair->SequenceNumber + Pair->Length) >= (NextPair->SequenceNumber + NextPair->Length)) {
               Pair->Next = NextPair->Next;
               DeletePair(NextPair);
           }
       }
       PrePair = Pair;
       Pair = Pair->Next;
       NextPair = Pair->Next;
   }
   Pair = Window->SeqPairs;
   uint8_t FlagBit = 0;
   if(Pair->HasFin != 0) {
      FlagBit = 1;  //FIN has one len of data.
   }
   return (Pair->SequenceNumber + Pair->Length + FlagBit); // maximum contiuous data we have received.
}

static int PushDataInQueue(int identifier)
{
   unsigned char Buffer[2000];
   // check for buffer overflow.
   int len = 0;
   struct tcb *ptcb = get_tcb_by_identifier(identifier);
   if((len = GetData(identifier, Buffer, 2000))) {
      void *msg = NULL;
      logger(LOG_TCP, LOG_LEVEL_NORMAL, "Pushing data of len %u to tcb %u\n", len, identifier);
      if (rte_mempool_get(buffer_message_pool, &msg) < 0) {
         logger (LOG_TCP, LOG_LEVEL_CRITICAL, "Failed to get message buffer for tcb %u\n", identifier);
/// / put assert ;
      }
      memcpy(msg, Buffer, len);
      if (rte_ring_enqueue(ptcb->socket_tcb_ring_send, msg) < 0) {
         logger(LOG_TCP, LOG_LEVEL_CRITICAL, "Failed to send message - message discarded for tcb %u\n", identifier);
         rte_mempool_put(buffer_message_pool, msg);
      }
   }
   return 0;
}

int GetData(int identifier, unsigned char *Buffer, uint32_t len)
{
   struct tcb *ptcb = get_tcb_by_identifier(identifier);
   if(ptcb == NULL) {
      assert(0);
      return -1;
   }
   ReceiveWindow *Window = (ReceiveWindow *) ptcb->RecvWindow;
   if(Window == NULL) {
      logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "no data to send no seq pair available for %u for current seq %u\n", identifier, Window->CurrentSequenceNumber);
      return 0;
   }
      
   assert(Window->CurrentSequenceNumber != 0); // this means it is not yet intialized.
   struct OutOfSeqPair *Pair = Window->SeqPairs;
   struct rte_mbuf *mbuf = Pair->mbuf;
   logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "attempt to get data for tcb %u from seq no %u.\n", identifier, Window->CurrentSequenceNumber);
   if(Pair)
      logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "for tcb %u current seq no %u and first pair seq no %u.\n", identifier, Window->CurrentSequenceNumber, Pair->SequenceNumber);
   if(Pair) {
      if(Pair->SequenceNumber <= Window->CurrentSequenceNumber) {
              // we have something to send to socket.
         int tcp_len = Pair->TcpLen;
         int DataSent = 0;
         if(Pair->Length != 0) {
            assert((Pair->SequenceNumber + Pair->Length) > Window->CurrentSequenceNumber); // we should have sent this sequence number to socket by now. how come it is  still here. I don't now if there is any scenrio where i will see this. 
            char *data =  (char *)(rte_pktmbuf_mtod(mbuf, unsigned char *) + 
                           sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) +  
                               tcp_len);
            int datalen = Pair->Length;
            //int datalen = rte_pktmbuf_pkt_len(mbuf) - (sizeof(struct ipv4_hdr) + sizeof(struct ether_hdr) + tcp_len);  // can we use pair length here. it should be same.
            assert(datalen == Pair->Length);
            uint32_t offset = Window->CurrentSequenceNumber - Pair->SequenceNumber;
            assert((Pair->Length-offset) < len);
            memcpy(Buffer, data + offset, Pair->Length - offset); 
            DataSent = Pair->Length - offset;
         }
         else {
            logger(LOG_TCP_WINDOW, LOG_LEVEL_CRITICAL, "Seen receive pair of 0 len this must be fin.");
            assert(Pair->Flags != 0);
         }
         Window->SeqPairs = Pair->Next;
         Window->CurrentSequenceNumber = Pair->SequenceNumber + Pair->Length;
         DeletePair(Pair);
         return DataSent;
      }
   }
   return 0;
}

int SendAck(void)
{
   assert(0); // remove this function at all.
   printf("Sending Syn Ack\n");
   return 0;

}

int
GetFirstUnAckedPacket(struct tcb *ptcb, int *data_len, struct rte_mbuf **mbuf)
{
   SendWindow *Window = ptcb->SendWindow;
   if(Window->Head == NULL) {
      logger(LOG_TCP, LOG_LEVEL_CRITICAL, "Attempt to get unacked packet from empty window");
      *mbuf = NULL;
   }
   struct SendSeqPair *temp = Window->Head;
   *data_len = temp->m_DataLen;
   *mbuf = temp->m_mbuf; 
   rte_pktmbuf_refcnt_update(temp->m_mbuf, 1);
   return 0;
}

int
AdjustSendWindow(struct tcb *ptcb, uint32_t AckValue)
{
   SendWindow *Window = ptcb->SendWindow;
   if(Window->Head == NULL) {
      assert(ptcb->rto_timer == -1); // rto timer must be stop
      return 0;
      // no data to ack. SendWindow is empty.
   }
   if(Window->Head->m_StartSeqNumber >= AckValue){
      assert(ptcb->rto_timer == -1); // rto timer must be stop
      return 0;
      // no new data ack.
   }
   if(Window->Last->m_EndSeqNumber < AckValue) {
      // not possible but ok what can be done now.
   }
   struct SendSeqPair *temp = NULL;
   struct SendSeqPair *head = Window->Head;
   while(head && (head->m_EndSeqNumber < AckValue)) {
      temp = head;
      head = head->Next;
      Window->CurrentSize -= (temp->m_EndSeqNumber - temp->m_StartSeqNumber);
      rte_pktmbuf_free(temp->m_mbuf);
      free(temp);
   }
   if(head == NULL) {
      Window->Last = NULL;
      ptcb->rto_timer = -1; // stop the timer
   }
   Window->Head = head;
   return 1;
}

// the logic here is very simple. just add the new pair at last and adjust the size.
int
PushDataToSendWindow(struct tcb *ptcb, struct rte_mbuf* mbuf, uint32_t StartSeq, uint32_t EndSeq, int data_len)
{
   SendWindow *Window = ptcb->SendWindow;
   logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "starting rto timer\n");
   ptcb->rto_timer = 0; // start the timer, 0 is the first value, now we have somthing which should be acked
   struct SendSeqPair *temp = (struct SendSeqPair *)malloc(sizeof (struct SendSeqPair));
   temp->m_StartSeqNumber = StartSeq;
   temp->m_EndSeqNumber = EndSeq;
   temp->m_DataLen = data_len;
   temp->Next = NULL;
   rte_pktmbuf_refcnt_update(mbuf, 1);
   temp->m_mbuf = mbuf;
   if(Window->Head == NULL) {
      assert(Window->Last == NULL);
      Window->Head = temp;
      Window->Last = temp;
   }
   else {
      assert(Window->Last->m_EndSeqNumber == StartSeq);
      Window->Last->Next = temp;
      Window->Last = temp;
   }
   Window->CurrentSize += (EndSeq - StartSeq);
   return 0;
} 

int PushData(struct rte_mbuf *mbuf, struct tcp_hdr* ptcphdr, uint16_t Length, struct tcb *ptcb)
{
// future add assert if SequenceNumber is 0.
   uint32_t SequenceNumber = ntohl(ptcphdr->sent_seq);
   ReceiveWindow *Window = (ReceiveWindow *) ptcb->RecvWindow;
   if(Window->SeqPairs && ((SequenceNumber - Window->SeqPairs->SequenceNumber + Length) < Window->CurrentSize)) { // sequence number out of receive window size 
      logger(LOG_TCP, LOG_LEVEL_NORMAL, "WARNING :: Out of window data, dropping all\n");
      return -1;
   }
   if(Window->CurrentSequenceNumber > (SequenceNumber + Length)) { // must be a duplicate packet.
      logger(LOG_TCP, LOG_LEVEL_NORMAL, "WARNING :: duplicate packet , dropping all; Current Seq = %u, Packet max length %u\n", Window->CurrentSequenceNumber, (SequenceNumber + Length));
      return -1;
   }
   int tcp_len = (ptcphdr->data_off >> 4) * 4;
   ptcb->ack = AdjustPair(Window, SequenceNumber, Length, mbuf, tcp_len, ptcphdr->tcp_flags);
   PushDataInQueue(ptcb->identifier); // Push data from seq pair to socket queue.
   return 0;
}

int FreeWindow(ReceiveWindow *Window)
{
   struct OutOfSeqPair *Pair = Window->SeqPairs;
   while(Pair) {
      DeletePair(Pair);
      Pair = Pair->Next;
   }
   free(Window);
   return 0;
}

ReceiveWindow *AllocReceiveWindow(int MaxSize, int CurrentSize)
{
   ReceiveWindow *Window = malloc(sizeof(ReceiveWindow));
   Window->MaxSize = MaxSize;
   Window->CurrentSize = CurrentSize;
   Window->StartSequenceNumber = 0;
   Window->CurrentSequenceNumber = 0;
   Window->SeqPairs = NULL;
   //Window->Data = malloc(MaxSize);
   return Window;
}

SendWindow *AllocSendWindow(int MaxSize, int CurrentSize)
{
   SendWindow *Window = malloc(sizeof(SendWindow));
   Window->MaxSize = MaxSize;
   Window->CurrentSize = CurrentSize;
   Window->StartSequenceNumber = 0;
   Window->CurrentSequenceNumber = 0;
   Window->Head = NULL;
   Window->Last = NULL;
   //Window->Data = malloc(MaxSize);
   return Window;
}
