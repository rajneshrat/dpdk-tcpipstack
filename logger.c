#include <stdarg.h>
#include "logger.h"
#include <rte_common.h>

uint32_t LoggingFeatures = 0;

struct LoggerFeature {
   TRACE_LEVEL Level;
   uint8_t Enable;
};

struct LoggerFeature *LogFeature;

void InitLogger()
{
   LogFeature = calloc(TOTAL_FEATURE, sizeof(struct LoggerFeature));
}

void EnableTrace(FEATURE Feature, TRACE_LEVEL Level)
{
   LogFeature[Feature].Enable = 1;
   LogFeature[Feature].Level = Level;
}

void logger(FEATURE Feature, TRACE_LEVEL Level, const char *format,  ...)
{
   va_list(arglist);
   if((LogFeature[Feature].Enable == 1) && (LogFeature[Feature].Level >= Level)) {
      va_start(arglist, format);
      vprintf(format, arglist);
      va_end(arglist);
   }
}

