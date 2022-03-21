#ifndef BCGEN_H
#define BCGEN_H

enum BCGEN_H {
    Store,
    Load,
};

typedef struct {
    enum StoreLoad is_store;
} Context;

#endif