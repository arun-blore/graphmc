#include "utils.h"

void get_index_and_tag (uint64_t addr, unsigned &row_index, uint64_t &tag) {
    uint64_t tmp = ~(0xffffffffffffffff << NUM_INDEX_BITS);
    // Assuming 32K, 4 way cache with 64B blocks
    tag       = unsigned(addr >> (NUM_BYTE_OFFSET_BITS+NUM_INDEX_BITS));
    row_index = unsigned((addr >> NUM_BYTE_OFFSET_BITS) & tmp);
}

uint64_t make_addr (uint64_t addr, uint64_t tag) {
    // Assuming 4 way, 32
    uint64_t tmp = ~(0xffffffffffffffff << (NUM_BYTE_OFFSET_BITS+NUM_INDEX_BITS));
    return ((addr & tmp) | (tag << (NUM_BYTE_OFFSET_BITS + NUM_INDEX_BITS)));
}

unsigned get_bank (uint64_t addr) {
    uint64_t tmp = ~(0xffffffffffffffff << LOG2_NUM_BANKS);
    return (unsigned)((addr >> LOG2_NUM_BYTES_PER_BANK) & (tmp));
}

uint64_t get_block_addr (uint64_t addr) {
    uint64_t tmp = (0xffffffffffffffff << NUM_BYTE_OFFSET_BITS);
    return (addr & tmp);
}
