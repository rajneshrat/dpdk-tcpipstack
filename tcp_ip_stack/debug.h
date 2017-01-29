#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <rte_tcp.h>
#include <rte_common.h>
#include <rte_ether.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include "tcp_tcb.h"

void DumpTcpHdr(struct tcp_hdr *hdr);
void DumpMbuf(struct rte_mbuf *mbuf);
void DumpIpHdr(struct ipv4_hdr *hdr);
void DumpTcb(struct tcb *ptcb);
uint32_t GetSeqFromMbuf(struct rte_mbuf *mbuf);
void DumpMbufToBuf(struct rte_mbuf *mbuf, char *buf, size_t len);
int DumpMbufInBuf(struct rte_mbuf *mbuf, char *buf);
int DumpIpHdrToBuf(struct ipv4_hdr *hdr, char *buf);
int DumpTcpHdrToBuf(struct tcp_hdr *hdr, char *buf);

#endif
