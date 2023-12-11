#include "list.h"
#include <string.h>
#include <stdio.h>

#define MAX_NAME_LEN 10
#define I2C_TYPE 0

struct  mylist {
    int type;
    char name[MAX_NAME_LEN];
    struct list_head list;
};

int main () {

    struct list_head myhead; 
    INIT_LIST_HEAD(&myhead);

    struct mylist node1;
    node1.type = I2C_TYPE;
    strcpy(node1.name, "ivan1");
    
    printf("node1.list:%p, node1.list.prev:%p, node1.list.next:%p, myhead:%p\n", 
        &node1.list, &(node1.list.prev),  &(node1.list.next), &myhead);

    list_add(&node1.list, &myhead);

    struct mylist node2;
    node2.type = I2C_TYPE;
    strcpy(node2.name, "ivan2");
    list_add(&node2.list, &myhead);
    return 0;
}