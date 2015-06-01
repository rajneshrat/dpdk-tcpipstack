#ifndef __LOGGER__
#define __LOGGER__

typedef enum {
   ARP,
   TOTAL_FEATURE,

}FEATURE;

typedef enum {
   CRITICAL,
   NORMAL,
   ALL,
}TRACE_LEVEL;

void logger(FEATURE feature, TRACE_LEVEL Level, const char *format,  ...);

#endif
