#include "tcp_windows.h"
#include "types.h"
#include "tcp_tcb.h"

int AdjustPair(ReceiveWindow *Window, uint16_t StartSeqNumber, uint16_t Length)
{
   struct SequenceLengthPair *Pair = Window->SeqPairs;
   struct SequenceLengthPair *LastPair = NULL;
   struct SequenceLengthPair *NextPair = NULL;
   while(Pair && (Pair->SequenceNumber <= StartSeqNumber)) {
      LastPair = Pair;
      Pair = Pair->Next;
   }
   Pair = LastPair;
   if((Pair->SequenceNumber + Pair->Length) > StartSeqNumber) {  // if we fall inside existing pair
      if(Pair->Length < (StartSeqNumber - Pair->SequenceNumber + Length)) { // do we need to increase length of existing pair
         Pair->Length = Length + StartSeqNumber - Pair->SequenceNumber;
      }
   }
   else { // create new pair
      struct SequenceLengthPair *NewPair = malloc(sizeof (struct SequenceLengthPair));
      NewPair->SequenceNumber = StartSeqNumber;
      NewPair->Length = Length;
      NewPair->Next = Pair->Next;
      Pair->Next = NewPair;
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
}

int AddData(unsigned char *data, uint16_t Length, uint16_t SequenceNumber, ReceiveWindow *Window)
{
   uint16_t CurrentPointer = (SequenceNumber - Window->StartSequenceNumber) % Window->MaxSize;
   memcpy(Window->Data, data, Length);
}

int PushData(unsigned char *data, uint16_t SequenceNumber, uint16_t Length, struct tcb *ptcb)
{
   ReceiveWindow *Window = ptcb->RecvWindow;
   if((SequenceNumber - Window->SeqPairs->SequenceNumber + Length) < Window->CurrentSize) { // sequence number out of receive window size 
      return -1;
   }
   AdjustPair(Window, SequenceNumber, Length);
   AddData(data, Length, SequenceNumber, Window);
   // unlock the sem if socket waiting for it.
}

ReceiveWindow *AllocWindow(int MaxSize, int CurrentSize, uint16_t StartSequenceNumber)
{
   ReceiveWindow *Window = malloc(sizeof(ReceiveWindow));
   Window->MaxSize = MaxSize;
   Window->CurrentSize = CurrentSize;
   Window->StartSequenceNumber = StartSequenceNumber;
   Window->SeqPairs = NULL;
   Window->Data = malloc(MaxSize);
   return Window;
}
