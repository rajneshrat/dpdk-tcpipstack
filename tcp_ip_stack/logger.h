#ifndef __LOGGER__
#define __LOGGER__
#include <stdarg.h>
#include "logger.h"
#include <rte_common.h>

typedef enum {
   ARP,
   IP,
   TCP,
   SOCKET,
   TCB,
   TOTAL_FEATURE,

}FEATURE;

typedef enum {
   CRITICAL,
   NORMAL,
   ALL,
}TRACE_LEVEL;

struct LoggerFeature {
   TRACE_LEVEL Level;
   uint8_t Enable;
};

extern struct LoggerFeature *LogFeature;

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logger log_print
//      printf("%s(%s):%d :: ", __FILENAME__, __func__, __LINE__); log_print 

void log_print(FEATURE feature, TRACE_LEVEL Level, const char *format,  ...);

#endif
