#define MAX_COUNTER_NAME 12

struct counter_infra{
   char counter_name[MAX_COUNTER_NAME];
   int id;
   struct counter_infra *next;
};

char *get_counter_file(int id);

int create_counter(const char *name);

int counter_inc(int id, int value);
