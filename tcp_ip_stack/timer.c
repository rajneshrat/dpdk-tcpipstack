#include "timer.h"
#include "logger.h"
#include "tcp_tcb.h"
#include "tcp_windows.h"
#include "tcp_out.h"
#include "debug.h"

// hard coded. need to be dynamically caluclated
#define CPU_FREQ (2.2 * 1000000000l)

uint64_t rdtsc(void);

uint64_t rdtsc(void){
       unsigned int lo,hi;
           __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
               return ((uint64_t)hi << 32) | lo;
}

static __inline__ unsigned long long get_time_msec(void)
{
    unsigned long long time = rdtsc();
   time = (time / (2.2 *100000)); 
   return time;
}

static __inline__ unsigned long long get_time_usec(void)
{
    unsigned long long time = rdtsc();
   time = (time / (2.2 *100)); 
   return time;
}

static __inline__ unsigned long long get_time_sec(void)
{
    unsigned long long time = rdtsc();
   time = time / CPU_FREQ; 
   return time;
}

void DoTimer(void)
{
   static unsigned long long time_last_usec = 0;
   unsigned long long time_now_usec = get_time_sec();
   if(time_now_usec > time_last_usec) {
      // this if for all timers which need to be called per micro sec.
   //  printf("1 sec expored %lld %lld\n", time_now, time_last);
     time_last_usec = time_now_usec;
       DoRtoTimer();
       if((time_now_usec - time_last_usec) > 1) {
// it means we have missed timer. timer overrun .
       }
   } 

}

void DoRtoTimer(void)
{
   int i;
   struct tcb *ptcb = NULL;
   for(i=0; i<Ntcb; i++) {  // change it to hash type later
      ptcb = tcbs[i];
      if(ptcb && ptcb->rto_timer >= 0) {  // non negative  value means timer is running.
         if(ptcb->rto_timer >= ptcb->rto_value) {
    //          printf("*************** retrnsmission happen %d %d\n", ptcb->rto_timer, ptcb->rto_value);
              logger(LOG_TCP, LOG_LEVEL_NORMAL, "retransmission timeout happens for ptcb %u.", ptcb->identifier);
              struct rte_mbuf *mbuf;
              int data_len;
              GetFirstUnAckedPacket(ptcb, &data_len, &mbuf);
              logger(LOG_TCP, LOG_LEVEL_NORMAL, "data len %u and pkt len %u\n", data_len, rte_pktmbuf_data_len(mbuf));
              uint16_t headersize = rte_pktmbuf_data_len(mbuf) - data_len;
              assert(headersize >= 20);
              if(headersize == 54) {
                    mbuf->udata64 |= 0x7;
              }
              if(headersize == 40) {
                    mbuf->udata64 |= 0x3;
              }

              logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "starting the rto timer\n");
              ptcb->rto_timer = 0;
              rte_mbuf_sanity_check(mbuf, 1);
              RetransmitPacket(mbuf, ptcb, data_len);
          //    DumpMbuf(mbuf);
              //logger("retransmission timeout happens for ptcb);
              //rto has expired, send the first non ack packet again. 
              // create a send window and use it to send data. 
              // set the value of rto_value aggain.
              // adjust the window for slow start.
         }
         else {
            logger(LOG_TCP_WINDOW, LOG_LEVEL_NORMAL, "retransmission timeout happens for ptcb %u resetting the rto value. rto timer = %u and rto value = %u\n", ptcb->identifier, ptcb->rto_timer, ptcb->rto_value);
            ptcb->rto_timer ++;
            // increase the rto_timer value.
         }
      }
   }
}

