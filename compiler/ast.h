#ifndef AST_H
#define AST_H

enum TokeType;

// 节点类型
enum NodeType {
    NT_INT,
    NT_VAR,

    NT_ADD,
    NT_SUB,
    NT_MUL,
    NT_DIV,

    NT_ASN,

    NT_LT,
    NT_LIST,
    NT_IF,
    NT_WHILE,
    NT_PRINT,
};

typedef struct {
    enum NodeType ntype;
} Node;

typedef struct {
    Node parent;
    int value;
} IntNode;

typedef struct {
    Node parent;
    Node *name;  
} VarNode;

typedef struct {
    Node parent;
    Node* expr;
} PrintNode;

typedef struct {
    Node parent;
    Node *left;
    Node *right;
} BinOpNode;

typedef struct {
    Node parent;
    Node *cond;
    Node* then_clause;
    Node* else_clause;
} IfNode;

typedef struct {
    Node parent;
    Node *cond;
    Node *body;
} WhileNode;

typedef struct {
    Node parent;
    int size;
    int length;
    Node **array;
} ListNode;

typedef struct {
    Node parent;
    Node *var;
    Node *value;
} AssignNode;

Node *create_int(int v);
Node *create_print(Node *v);
Node *create_var(char *name);
Node *create_assgin(Node *var, Node *value);
Node *create_binop(enum TokenType tt, Node *left, Node *right);
Node* create_if(Node* cond, Node* then_clause, Node* else_clause);
Node* create_while(Node* cond, Node* body);
void post_order(Node* root);

/* method for list */
ListNode* create_list();
void append(ListNode* list, Node* n);

#endif