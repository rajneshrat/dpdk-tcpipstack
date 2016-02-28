#ifndef __LOGGER__
#define __LOGGER__
#include <stdarg.h>
#include "logger.h"
#include <rte_common.h>
#include "common_header.h"

typedef enum {
   LOG_ARP,
   LOG_IP,
   LOG_TCP,
   LOG_TCP_WINDOW,
   LOG_SOCKET,
   LOG_TCB,
   LOG_TOTAL_FEATURE,

}FEATURE;

typedef enum {
   CRITICAL,
   LOG_LEVEL_CRITICAL,
   NORMAL,
   LOG_LEVEL_NORMAL,
   ALL,
}TRACE_LEVEL;

struct LoggerFeature {
   TRACE_LEVEL Level;
   uint8_t Enable;
};

extern struct LoggerFeature *LogFeature;
void InitLogger(void);
void EnableTrace(FEATURE Feature, TRACE_LEVEL Level);

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logger log_print
//      printf("%s(%s):%d :: ", __FILENAME__, __func__, __LINE__); log_print 

void log_print(FEATURE feature, TRACE_LEVEL Level, const char *format,  ...);

#endif
