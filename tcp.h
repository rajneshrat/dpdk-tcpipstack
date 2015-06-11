enum TCP_FLAGS {
   CWR = 0x80,
   ECE = 0x40,
   URG = 0x20,
   ACK = 0x10,
   PSH = 0x08,
   RST = 0x04,
   SYN = 0x02,
   FIN = 0x01,
}; 

struct pseudo_tcp_hdr {
   uint32_t src_ip;
   uint32_t dst_ip;
   uint8_t reserved;
   uint8_t protocol;
   uint16_t len;
};
