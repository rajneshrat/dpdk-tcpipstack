#ifndef __LOGGER__
#define __LOGGER__

typedef enum {
   ARP,
   IP,
   TCP,
   TOTAL_FEATURE,

}FEATURE;

typedef enum {
   CRITICAL,
   NORMAL,
   ALL,
}TRACE_LEVEL;

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logger \
   printf("%s(%s):%d :: ", __FILENAME__, __func__, __LINE__); log_print
void log_print(FEATURE feature, TRACE_LEVEL Level, const char *format,  ...);

#endif
