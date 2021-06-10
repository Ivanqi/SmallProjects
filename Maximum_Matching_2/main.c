#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 参考资料: https://www.iteye.com/blog/josh-persistence-2243380
#define HASH_LEN 39841  // 定义散列表大小
#define STACK_LEN 100   // 定义栈大小
#define MAX 5           // 最大匹配字数 

typedef struct hash_node {
    char *data;
    struct hash_node *next;
} HASH_NODE;    // 散列表下来链表结果

typedef struct {
    int len;
    HASH_NODE *data;
} HASH; // 散列表数据结构

typedef struct {
    int len;
    char *data[STACK_LEN];
} STACK;    // 栈数据结构

// 哈希函数，计算哈希值
unsigned int hash_fun(char *key) {
    unsigned int seed = 131;
    unsigned int hash = 0;

    while (*key) {
        hash = hash * seed + *key;
        ++key;
    }

    return hash & 0x7FFFFFFF;
}

// 初始化散列表
void hash_init(HASH *hash) {
    int i;

    // 所有表中数据为空
    for (i = 0; i != HASH_LEN; ++i) {
        hash[i].len = 0;
    }
}

// 冲突时下拉链表
void hash_list_insert(HASH_NODE *data, char *key) {
    HASH_NODE *temp;

    while (data->next != NULL) {
        data = data->next;
    }

    temp = malloc(sizeof(HASH_NODE));
    temp->data = key;
    temp->next = NULL;
    data->next = temp;
}

// 插入数据
void hash_insert(HASH *hash, char *key) {
    int h;
    
    h = hash_fun(key) % HASH_LEN;

    // 如果当前表中没有数据则直接插入
    // 否则插入链表中
    if (hash[h].len > 0) {
        hash_list_insert(hash[h].data, key);
    } else {
        hash[h].data = malloc(sizeof(HASH_NODE));
        hash[h].data->data = key;
        hash[h].data->next = NULL;
    }

    // 当前表中数据值加1
    ++hash[h].len;
}

// 从该地址中进行搜索
char *hash_node_key(HASH_NODE *node, char *key) {
    while (node) {
        if (strcmp(node->data, key) == 0) {
            return node->data;
        }

        node = node->next;
    }

    return NULL;
}

// 从散列表中查找
char *hash_get(HASH *hash, char *key) {
    int h;
    
    h = hash_fun(key) % HASH_LEN;
    if (hash[h].len == 0) {
        return NULL;
    }
    return hash_node_key(hash[h].data, key);
}

// 判断数据是否在该表中
int hash_node_in(HASH_NODE *node, char *key) {
    while (node) {
        if (strcmp(node->data, key) == 0) {
            return 1;
        }
        node = node->next;
    }
    return 0;
}

/**
 * 从表中搜索关键词
 * 如若存在则返回1
 * 否则返回0
 */
int hash_in(HASH *hash, char *key) {
    int h;

    h = hash_fun(key) % HASH_LEN;
    if (hash[h].len == 0) {
        return 0;
    }

    return hash_node_in(hash[h].data, key);
}

// 打印表的所有数据
void hash_list_print(HASH_NODE *data) {
    while (data != NULL) {
        printf("%s", data->data);
        data = data->next;
    }
}

// 打印散列表
void hash_print(HASH *hash) {
    int i;

    for (i = 0; i != HASH_LEN; ++i) {
        hash_list_print(hash[i].data);
        printf("\n");
    }
}

// 加载词典数据并存入散列表中
void load_data(HASH *hash, char *path) {
    char *buf = NULL;
    char *temp;
    size_t len;
    int s;
    FILE *fp;

    if ((fp = fopen(path, "rb")) == NULL) {
        return ;
    }

    // 按行读取
    while ((s = getline(&buf, &len, fp)) > 0) {
        temp = malloc(sizeof(char) *s);
        snprintf(temp, sizeof(char) *s, "%s", buf);
        // 去除回车符
        hash_insert(hash, temp);
        // 插入数据

        free(buf);
        buf = NULL;
    }

    fclose(fp);
}

// 初始化栈
void stack_init(STACK *stack) {
    int i;

    // 栈数据置零
    stack->len = 0;
    for (i = 0; i != STACK_LEN; ++i) {
        stack->data[i] = NULL;
    }
}

// 压入一个数据到栈中
int stack_push(STACK *stack, char *data) {
    if (stack->len >= STACK_LEN) {
        return 0;
    }

    stack->data[stack->len] = data;
    ++stack->len;
}

// 从栈中弹出一个数据
char *stack_pop(STACK *stack) {
    if (stack->len <= 0) {
        return NULL;
    }

    --stack->len;
    return stack->data[stack->len];
}

// 最大正向匹配
int for_match(HASH *hash, STACK *stack, char *data, int *index) {
    int len = strlen(data);

    while (len) {
        /**
         * 判断词典中是否有该词
         * 有则将该词压入栈中
         * 否则从字符串减一个字进行循环
         */
        if (hash_in(hash, data)) {
            stack_push(stack, data);
            *index += len;
            return 1;
        }

        len -= 3;
        data = realloc(data, sizeof(char) * len + 1);
        data[len] = '\0';
    }

    return 0;
}

// 逆向最大匹配
int re_match(HASH *hash, STACK *stack, char *data, int *index) {
    int len = strlen(data);

    while (len) {
        /**
         * 判断词典中是否有该词
         * 有则将该词压入栈中
         * 否则从字符串前减一个字进行循环
         */
        if (hash_in(hash, data)) {
            stack_push(stack, data);
            *index -= len;
            return 1;
        }

        data += 3;
        len -= 3;
    }
}

// 预处理字符串
void pre_set(char *str) {
    char temp[600] = {0};
    int i = 0;
    int index = 0;

    while (i < strlen(str)) {
        if ((str[i] & 0xe0) == 0xe0) {
            snprintf(temp + index, sizeof(char) * 3 + 1, "%s", str + i);
            i += 3;
            index += 3;
        } else if ((str[i] & 0xc0) == 0xc0) {   // 标点等
            i += 2;
        } else if ((str[i] & 0x80) == 0) {      // 英文字符
            ++i;
        }
    }

    // 重新设置字符串
    memset(str, 0, strlen(str) + 1);
    snprintf(str, strlen(temp) + 1, "%s", temp);
}

// 输入：计算语言学课程有意思
int main(int argc, char **argv) {

    HASH hash[HASH_LEN];        // 散列表
    STACK stack;                // 压入匹配到的词的栈
    STACK res;                  // 以顺序打印正向匹配结果的交换栈
    char string[600] = {0};     // 输入的字符串
    int i = 0;
    int index = 0;
    char *temp;                 // 取出的字符串

    // 初始化
    hash_init(hash);
    stack_init(&stack);
    stack_init(&res);

    load_data(hash, "./data/dict.txt");

    printf("请输入一个字符串: ");
    fgets(string, 600, stdin);

    // 预处理字符串，去除英文和其他非中文字符
    pre_set(string);

    // 开始正向取出字符串
    while (i < strlen(string)) {
        // 申请5个字符的临时变量
        temp = malloc(sizeof(char) * 3 * MAX + 1);
        snprintf(temp, sizeof(char) * 3 * MAX + 1, "%s", string + i);
        printf("temp:%s\n", temp);

        // 正向最大匹配
        if (!for_match(hash, &stack, temp, &i)) {
            i += 3;
        }
    }

    // 将匹配结果重新排序并打印
    while (temp = stack_pop(&stack)) {
        stack_push(&res, temp);
    }

    printf("正向最大匹配:\n");
    while (temp = stack_pop(&res)) {
        printf("%s ", temp);
    }

    // 取出逆向匹配字符串
    printf("\n\n");
    i = strlen(string);

    while (i > 0) {
        int index = 0;

        // 如果当前字符串不够5个，则从头开始截取
        if (i < 3 * MAX) {
            index = 0;
        } else {
            index = i - 3 * MAX;
        }

        temp = malloc(sizeof(char) * 3 * MAX + 1);
        snprintf(temp, sizeof(char) * 3 * MAX + 1, string + index);
        printf("temp:%s\n", temp);

        // 开始逆向匹配
        if (!re_match(hash, &stack, temp, &i)) {
            i -= 3;
        }

        // 去除已经匹配的字符串
        string[i] = '\0';
    }

    // 打印匹配结果
    printf("逆向最大匹配\n");
    while (temp = stack_pop(&stack)) {
        printf("%s ",temp);
    }

    printf("\n");

    return 0;
}