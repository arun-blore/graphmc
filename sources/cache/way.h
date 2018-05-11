#include <vector>
#include <stdint.h>
#include "cache_config.h"

class way {
    std::vector <uint64_t> tag_array;
    std::vector <bool> valid_array  ;
    std::vector <unsigned> age;
    
    public:
    way () {
        // Assuming depth of 128
        tag_array.resize(NUM_ROWS);
        valid_array.resize(NUM_ROWS, false);
        age.resize (NUM_ROWS, 0);
        // for (unsigned i = 0; i < ; i++) {
        //     valid_array[i] = false;
        // }
    }
    
    bool check_tag (unsigned row, uint64_t tag) {
        if (valid_array[row] & (tag_array[row] == tag))
            return true;
        else
            return false;
    }

    unsigned write (unsigned row, uint64_t tag) {
        cout << "writing to index = " << row << endl;
        valid_array[row] = true;
        tag_array[row] = tag;
    }

    void get_tag (unsigned row, uint64_t &tag, bool &valid) {
        tag = tag_array[row];
        valid = valid_array[row];
    }

    bool is_valid (unsigned index) {
        return valid_array[index];
    }

    void increment_age (unsigned index) {
        age[index]++;
    }

    void reset_age (unsigned index) {
        age[index] = 0;
    }

    unsigned get_age (unsigned index) {
        return age[index];
    }
};

/*
int main () {
    way W;
    return 0;
}
*/
