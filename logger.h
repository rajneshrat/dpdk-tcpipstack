#ifndef __LOGGER__
#define __LOGGER__

typedef enum {
   ARP,
   TOTAL_FEATURE,

}FEATURE;

typedef enum {
   CRITICAL,

}TRACE_LEVEL;

void logger(FEATURE feature, TRACE_LEVEL Level, const char *format,  ...);

#endif
