#include <stdarg.h>
#include "logger.h"
#include <rte_common.h>

//uint32_t LoggingFeatures = 0;

struct LoggerFeature *LogFeature;

void InitLogger(void)
{
   LogFeature = calloc(LOG_TOTAL_FEATURE, sizeof(struct LoggerFeature));
//   EnableTrace(LOG_ARP, ALL);
//   EnableTrace(LOG_IP, ALL);
//   EnableTrace(LOG_TCP, ALL);
   EnableTrace(LOG_TCP_WINDOW, ALL);
   //EnableTrace(SOCKET, ALL);
}

void EnableTrace(FEATURE Feature, TRACE_LEVEL Level)
{
   LogFeature[Feature].Enable = 1;
   LogFeature[Feature].Level = Level;
}

void log_print(FEATURE Feature, TRACE_LEVEL Level, const char *format,  ...)
{
   va_list(arglist);
   if((LogFeature[Feature].Enable == 1) && (LogFeature[Feature].Level >= Level)) {
      va_start(arglist, format);
      printf("Log feature %d ---- ", Feature);
      vprintf(format, arglist);
      va_end(arglist);
   }
}

