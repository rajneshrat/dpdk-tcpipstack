#define MAX_COUNTER_NAME 20

struct counter_infra{
   char counter_name[MAX_COUNTER_NAME];
   int id;
   int last;
   struct counter_infra *next;
};

char *get_counter_file(int id);
struct counter_infra *get_counter_obj(int id);

int create_counter(const char *name);

int counter_inc(int id, int value);
int counter_abs(int id, int value);

void init_counters(void);
