#include <stdarg.h>
#include "logger.h"
#include <rte_common.h>

//uint32_t LoggingFeatures = 0;
#define LOG_FILE "TcpStack.log"
struct LoggerFeature *LogFeature;

void InitLogger(void)
{
   LogFeature = calloc(LOG_TOTAL_FEATURE, sizeof(struct LoggerFeature));
//   EnableTrace(LOG_ARP, ALL);
 //  EnableTrace(LOG_IP, LOG_LEVEL_CRITICAL);
 //  EnableTrace(LOG_TCP, ALL);
 //  EnableTrace(LOG_ETHER, ALL);
  // EnableTrace(LOG_TCP_STATE, ALL);
////   EnableTrace(LOG_TCB, ALL);
 //  EnableTrace(LOG_TCP_WINDOW, ALL);
   EnableTrace(LOG_SOCKET, ALL);
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
      FILE *fd = fopen(LOG_FILE, "a");
      //vfprintf(fd, "Log feature %d ---- ", Feature);
      vfprintf(fd, format, arglist);
      fprintf(fd, "\n");
      fclose(fd);
      va_end(arglist);
   }
}

