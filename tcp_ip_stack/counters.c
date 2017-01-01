#include "counters.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define COUNTER_FOLDER "samples"

struct counter_infra *counter_list_head = NULL;
struct counter_infra *counter_list_tail = NULL;

static unsigned int counterid = 0;

void init_counters(void)
{
   char command[128];
   sprintf(command, "rm -rf %s.old; mv %s %s.old; mkdir %s", COUNTER_FOLDER, COUNTER_FOLDER, COUNTER_FOLDER, COUNTER_FOLDER);
   system(command);
}

char *get_counter_file(int id)
{
    struct counter_infra *tmp = counter_list_head;
    while(tmp) {
       if(tmp->id == id) {
            return tmp->counter_name;
       }
       tmp = tmp->next;
    }
    return NULL;
}

struct counter_infra *get_counter_obj(int id)
{
    struct counter_infra *tmp = counter_list_head;
    while(tmp) {
       if(tmp->id == id) {
            return tmp;
       }
       tmp = tmp->next;
    }
    return NULL;
}

int create_counter(const char *name)
{
   char command[128];
   sprintf(command, "mv %s/%s /tmp", COUNTER_FOLDER, name);
   system(command);
   struct counter_infra *obj = malloc(sizeof(struct counter_infra));
   memset(obj, 0, sizeof(struct counter_infra));
   strncpy(obj->counter_name, name, MAX_COUNTER_NAME);
   obj->id = counterid;
   obj->next = NULL;
   counterid++;
   if(counter_list_head == NULL) {
      counter_list_head = obj;
      counter_list_tail = obj;
   }
   else {
      counter_list_tail->next = obj;
      counter_list_tail = obj;
   }
   return obj->id;;
}

int counter_inc(int id, int value)
{
   struct counter_infra *tmp  = get_counter_obj(id);
   char *file_name = tmp->counter_name;
   char file[128];
   value = value + tmp->last;
   tmp->last = value;
   snprintf(file, 128, "%s/%s",COUNTER_FOLDER, file_name);
   FILE *fp = fopen(file, "a");
   if(fp == NULL) {
      printf("file is null %s\n", file);
   }
   fprintf(fp, "%d,%d\n", value, value);
   fclose(fp);
   return id;
}

int counter_abs(int id, int value)
{
   char *file_name = get_counter_file(id);
   char file[128];
   snprintf(file, 128, "%s/%s",COUNTER_FOLDER, file_name);
   FILE *fp = fopen(file, "a");
   if(fp == NULL) {
      printf("file is null %s\n", file);
   }
   fprintf(fp, "%d,%d\n", value, value);
   fclose(fp);
   return id;
}
