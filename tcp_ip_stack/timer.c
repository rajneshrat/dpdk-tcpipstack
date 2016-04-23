#include "timer.h"
#include "logger.h"
#include "tcp_tcb.h"
#include "tcp_windows.h"
#include "tcp_out.h"

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
              logger(LOG_TCP, LOG_LEVEL_NORMAL, "retransmission timeout happens for ptcb %u.", ptcb->identifier);
              struct rte_mbuf *mbuf;
              int data_len;
              GetFirstUnAckedPacket(ptcb, &data_len, &mbuf);
              logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "starting the rto timer\n");
              ptcb->rto_timer = 0;
              RetransmitPacket(mbuf, ptcb, data_len);
              //logger("retransmission timeout happens for ptcb);
              //rto has expired, send the first non ack packet again. 
              // create a send window and use it to send data. 
              // set the value of rto_value aggain.
              // adjust the window for slow start.
         }
         else {
            logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "retransmission timeout happens for ptcb %u resetting the rto value.", ptcb->identifier);
            ptcb->rto_timer ++;
            // increase the rto_timer value.
         }
      }
   }
}

