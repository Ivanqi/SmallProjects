#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

char *cur;

/**
 * @brief Create a token object
 * 
 * @param tt token类型
 * @param begin 需要创建的值，开始部分
 * @param cur 
 * @return struct Token* 
 */
struct Token* create_token(enum TokenType tt, char *begin, char *cur) {
    struct Token *nt = (struct Token *)malloc(sizeof(struct Token ));
    nt->_type == tt;

    if (tt == TT_VAR) {
        nt->_type = (char *)malloc(cur - begin + 1);
        strncpy(nt->_value._str, begin, cur - begin);
        nt->_value._str[cur - begin] = 0;
    } else if (tt == TT_INTEGER) {
        int sum = 0;
        for (char *p = begin; p < cur; p++) {
            sum *= 10;
            sum += (*p - '0');
        }
        nt->_value._int = sum;
    }
}

void destroy_token(struct Token *t) {
    if (t->_type == TT_VAR) {
        free(t->_value._str);
        t->_value._str = NULL;
    }

    free(t);
}

void log_token(struct Token *t) {
    printf("%d", t->_type);
    
    if (t->_type == TT_VAR) {
        printf(", %s\n", t->_value._str);
    } else if (t->_type == TT_INTEGER) {
        printf(", %d\n", t->_value._int);
    } else {
        printf("\n");
    }
}

char is_alpah(char c) {
    return (c <= 'Z' && c >= 'A') || (c <= 'z' && c >= 'a') || c == '_';
}

char is_num(char c) {
    return c <= '9' && c >= '0';
}

struct Token* filter_keyword(struct Token *t) {
    if (strcmp(t->_value._str, "if") == 0) {
        t->_type = TT_IF;
    } else if (strcmp(t->_value._str, "else") == 0) {
        t->_type = TT_ELSE;
    } else if (strcmp(t->_value._str, "int") == 0) {
        t->_type = TT_KW_INT;
    } else if (strcmp(t->_value._str, "while") == 0) {
        t->_type = TT_KW_WHILE;
    } else if (strcmp(t->_value._str, "print") == 0) {
        t->_type = TT_KW_PRINT; 
    }

    return t;
}

struct Token* next_token() {
    enum State state = STATE_INIT;
    char *begin = 0;

    while (*cur) {
        char c = *cur;
        if (state == STATE_INIT) {
            if (c == ' ' || c == '\n' || c == '\t') {
                cur++;
                continue;
            }

            if (is_alpah(c)) {
                begin = cur;
                state = STATE_VAR;
                cur++;
            } else if (is_num(c)) {
                begin = cur;
                state = STATE_NUM;
            } else if (c == '=') {
                begin = cur;
                state = STATE_EQU;
                cur++;
            } else if (c == ';') {
                begin = cur;
                cur++;
                return create_token(TT_SEMICON, begin, cur);
            } else if (c == '+') {
                begin = cur;
                cur++;
                return create_token(TT_ADD, begin, cur);
            } else if (c == '-') {
                begin = cur;
                cur++;
                return create_token(TT_MUL, begin, cur);
            } else if ( c == '*') {
                begin = cur;
                cur++;
                return create_token(TT_MUL, begin, cur);
            } else if (c == '/') {
                begin = cur;
                cur++;
                return create_token(TT_DIV, begin, cur);
            } else if (c == '(') {
                begin = cur;
                cur++;
                return create_token(TT_LEFT_PAR, begin, cur);
            } else if (c == ')') {
                begin = cur;
                cur++;
                return create_token(TT_RIGHT_PAR, begin, cur);
            } else if (c == '{') {
                begin = cur;
                cur++;
                return create_token(TT_LEFT_BRACKET, begin, cur);
            } else if (c == '<') {
                begin = cur;
                cur++;
                return create_token(TT_LT, begin, cur);
            }
        } else if (state == STATE_VAR) {
            if (is_alpah(c) || is_num(c)) {
                cur++;
            } else {
                return filter_keyword(create_token(TT_VAR, begin, cur));
            }
        } else if (state == STATE_NUM) {
            if (is_num(c)) {
                cur++;
            } else {
                return create_token(TT_INTEGER, begin, cur);
            }
        } else if (state == STATE_EQU) {
            if (c == '=') {

            } else {
                return create_token(TT_ASSIGN, begin, cur);
            }
        }
    }

    return NULL;
}