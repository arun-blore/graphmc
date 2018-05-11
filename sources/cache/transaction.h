#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#include <stdint.h>

enum trans_type {
    READ,
    WRITE
};

struct transaction {
    uint64_t addr;
    uint64_t data;
    trans_type type;
};

#endif
