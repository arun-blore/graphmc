#define NUM_WAYS 4
#define NUM_ROWS 128
#define NUM_BANKS 4

#define NUM_BYTE_OFFSET_BITS 6
#define NUM_INDEX_BITS 7
#define LOG2_NUM_BYTES_PER_BANK 15 // log (cacheblock size * number of rows * number of ways) = log (64 * 128 * 4)
#define LOG2_NUM_BANKS 2

#define HIT_TIME 6
#define MISS_TIME 4

#define REQ_BUFFER_DEPTH 10
