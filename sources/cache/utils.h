#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>
#include "cache_config.h"

void get_index_and_tag (uint64_t addr, unsigned &row_index, uint64_t &tag);
uint64_t make_addr (uint64_t addr, uint64_t tag);
unsigned get_bank (uint64_t addr);
uint64_t get_block_addr (uint64_t addr);

#endif
