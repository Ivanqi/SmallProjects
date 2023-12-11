#include "list.h"
#include <stdio.h>
#include <string.h>

#define MAX_NAME_LEN 32
#define MAX_ID_LEN 10

struct list_head myhead;

#define I2C_TYPE 1
#define SPI_TYPE 2

char *dev_name[]={
  "none",
  "I2C",
  "SPI"
};

struct mylist { 
    int type;
    char name[MAX_NAME_LEN];
    struct list_head list;  
};

void display_list(struct list_head* list_head) {
    int i = 0;
    struct list_head *p;
    struct mylist *entry;
    printf("-------list---------\n");

    list_for_each(p, list_head) {
        printf("node[%d]\n",i++);
        entry = list_entry(p, struct mylist, list);
        printf("\ttype: %s\n", dev_name[entry->type]);
        printf("\tname: %s\n", entry->name);
    }

    printf("-------end----------\n");
}

int main(void) {
    struct mylist node1;
    struct mylist node2;

    INIT_LIST_HEAD(&myhead);

    node1.type = I2C_TYPE;
    strcpy(node1.name, "ivan1");

    node2.type = I2C_TYPE;
    strcpy(node2.name, "ivan2");

    list_add(&node1.list,&myhead);
    list_add(&node2.list,&myhead);

    display_list(&myhead);

    list_del(&node1.list);

    display_list(&myhead);

    return 0;
}