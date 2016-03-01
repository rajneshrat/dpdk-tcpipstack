#include "timer.h"
#include "tcp_tcb.h"

void DoTimer(void)
{


}

void DoRtoTimer(void)
{
   int i;
   struct tcb *ptcb = NULL;
   for(i=0; i<Ntcb; i++) {  // change it to hash type later
      ptcb = tcbs[i];
      if(ptcb && ptcb->rto_timer >= 0) {  // non negative  value means timer is running.
         if(ptcb->rto_timer >= ptcb->rto_value) {
              //logger("retransmission timeout happens for ptcb);
              //rto has expired, send the first non ack packet again. 
              // create a send window and use it to send data. 
              // set the value of rto_value aggain.
              // adjust the window for slow start.
         }
         else {
            ptcb->rto_timer ++;
            // increase the rto_timer value.
         }
      }
   }
}

