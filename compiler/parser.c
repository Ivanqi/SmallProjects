#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "lexer.h"
#include "ast.h"
#include "util.h"

extern char* cur;

struct Token* t = NULL;

typedef enum {
    VT_INT,
} VarType;

typedef struct {
    char* var_name;
    VarType var_type;
} Var;

LIST(Var)
LIST_APPEND(Var)

VarList symbol_table;

Node* if_stmt();
Node* while_stmt();
Node* declare_stmt();
Node* asn_stmt();
Node* print_stmt();
Node* stmts();
Node* condition();
Node* expr();
Node* term();
Node* factor();

int match(enum TokenType tt) {
    if (t->_type != tt) {
        printf("Parse Error, expected %d, but got %d\n", tt, t->_type);
        return 0;
    }

    t = next_token();

    return 1;
}

Node* stmts() {
    ListNode* list = create_list();
    while (t != NULL) {

        if (t->_type == TT_IF) {
            append(list, if_stmt());
        } else if (t->_type == TT_INTEGER) {
            append(list, expr());
            assert(t->_type == TT_SEMICON);
            t = next_token();
        } else if (t->_type == TT_KW_INT) {
            Node* stmt = declare_stmt();

            /* declared with initial value. */
            if (stmt != NULL) {
                append(list, stmt);
            }

            assert(t->_type == TT_SEMICON);
            t = next_token();
        } else if (t->_type == TT_VAR) {
            append(list, asn_stmt());
            assert(t->_type == TT_SEMICON);
            t = next_token();
        } else if (t->_type == TT_KW_WHILE) {
            append(list, while_stmt());
        } else if (t->_type == TT_KW_PRINT) {
            append(list, print_stmt());
            assert(t->_type == TT_SEMICON);
            t = next_token();
        } else {
            break;
        }
    }

    return (Node *)list;
}

Node* asn_stmt() {
    Node *a = expr();

    if (t->_type == TT_ASSIGN) {
        t = next_token();
        a = create_assign(a, expr());
    }

    return a;
}

Node* print_stmt() {
    assert (t->_type == TT_KW_PRINT);
    t = next_token();

    assert (t->_type == TT_LEFT_PAR);
    t = next_token();

    Node *a = expr();
    a = create_print(a);
    assert (t->_type == TT_RIGHT_PAR);
    t = next_token();
    
    return a;
}

Node* if_stmt() {
    Node *then_clause, *else_clause;
    assert(t->_type == TT_IF);

    t = next_token();
    assert(t->_type == TT_LEFT_PAR);

    t = next_token();
    Node* cond = condition();

    assert(t->_type == TT_RIGHT_PAR);

    t = next_token();
    assert(t->_type == TT_LEFT_BRACKET);

    t = next_token();
    then_clause = stmts();
    assert(t->_type == TT_RIGHT_BRACKET);

    t = next_token();
    if (t->_type == TT_ELSE) {
        t = next_token();
        assert(t->_type == TT_LEFT_BRACKET);

        t = next_token();
        else_clause = stmts();
        assert(t->_type == TT_RIGHT_BRACKET);

        t = next_token();
    } else {
        return create_if(cond, then_clause, NULL);
    }

    return create_if(cond, then_clause, else_clause);
}

Node* while_stmt() {
    assert(t->_type == TT_KW_WHILE);

    t = next_token();
    assert(t->_type == TT_LEFT_PAR);

    t = next_token();
    Node* cond = condition();

    assert(t->_type == TT_RIGHT_PAR);

    t = next_token();
    assert(t->_type == TT_LEFT_BRACKET);

    t = next_token();
    Node* body = stmts();
    assert(t->_type == TT_RIGHT_BRACKET);
    t = next_token();

    return create_while(cond, body);
}

Node* declare_stmt() {
    assert(t->_type == TT_KW_INT);
    
    t = next_token();
    assert(t->_type == TT_VAR);

    /* add varialbe into symbol table.*/
    Node* var_name = create_var(t->_value._str);
    Var entry = {((VarNode*)var_name)->name, 0};
    append_Var(&symbol_table, entry);

    t = next_token();
    if (t->_type == TT_ASSIGN) {
        t = next_token();
        return create_assign(var_name, expr());
    }

    return NULL;
}

Node* condition() {
    Node* a, *b;
    a = expr();
    
    if (t->_type == TT_LT) {
        t = next_token();
        b = expr();
        return create_binop(TT_LT, a, b);
    }

    return a;
}

Node* expr() {
    Node* a = NULL, *b = NULL;
    a = term();

    while (t->_type == TT_ADD || t->_type == TT_SUB) {
        if (t->_type == TT_ADD) {
            t = next_token();
            b = term();
            a = create_binop(TT_ADD, a, b);
        } else if (t->_type == TT_SUB) {
            t = next_token();
            b = term();
            a = create_binop(TT_SUB, a, b);
        }
    }

    return a;
}

Node* term() {
    Node* a = NULL, *b = NULL;
    a = factor();

    while (t->_type == TT_MUL || t->_type == TT_DIV) {
        if (t->_type == TT_MUL) {
            t = next_token();
            b = factor();
            a = create_binop(TT_MUL, a, b);
        } else if (t->_type == TT_DIV) {
            t = next_token();
            b = factor();
            a = create_binop(TT_DIV, a, b);
        }
    }

    return a;
}

Node* factor() {
    if (t->_type == TT_INTEGER) {
        Node* a = create_int(t->_value._int);
        t = next_token();
        return a;
    } else if (t->_type == TT_VAR) {
        Node* a = create_var(t->_value._str);
        t = next_token();
        return a;
    } else if (t->_type == TT_LEFT_PAR) {
        t = next_token();
        Node* a = expr();
        if (!match(TT_RIGHT_PAR)) {
            return NULL;
        } else {
            return a;
        }
    } else {
        printf("Parse Error\n");
        return NULL;
    }
}

Node* eval(char* s) {
    symbol_table.size = 8;
    symbol_table.length = 0;
    symbol_table.array = (Var*)malloc(8 * sizeof(Var));

    cur = s;
    t = next_token();
    return stmts();
}