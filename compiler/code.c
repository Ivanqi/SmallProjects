#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code.h"
#include "opcode.h"

CodeObject* create_code_object() {
    CodeObject *co = (CodeObject *)malloc(sizeof(CodeObject));
    co->nested = 0;
    co->bytecodes = create_byte_list();
    co->constant_pool = create_int_list();
    co->var_table = create_string_list();
    co->reloc_while_end = NULL;
}

void code_object_append(CodeObject *co, byte b) {
    append_byte(co->bytecodes, b);
}

void code_object_append_bytecode(CodeObject *co, byte op_code, byte parameter) {
    code_object_append(co, op_code);
    if (op_code >= OP_CODE_PARAMETER) {
        code_object_append(co, parameter);
    }
}