#include "cache_config.h"
#include <iostream>
#include <vector>
#include "way.h"
#include <stdint.h>
#include "utils.h"

using namespace std;

class TagArray {
    vector <way> ways;

    public:

    TagArray () {
        ways.resize(NUM_WAYS);
    }

    void print () {
        uint64_t tag;
        bool valid;
        for (unsigned row = 0; row < NUM_ROWS; row++) {
            for (unsigned i = 0; i < NUM_WAYS; i++) {
                ways[i].get_tag (row, tag, valid);
                if (valid == false)
                    cout << setfill(' ') << setw(30) << "invalid";
                else
                    cout << setfill(' ') << setw(20) << tag << setw(10) << ways[i].get_age(row);
            }
            cout << endl;
        }
    }

    bool check_tag (uint64_t addr, unsigned &way) {
        cout << "TagArray::check_tag" << endl;
        unsigned index;
        uint64_t tag;
        bool hit = false;
        get_index_and_tag (addr, index, tag);
        for (int i = 0; i < NUM_WAYS; i++) {
            ways[i].increment_age (index);
            if (ways[i].check_tag (index, tag)) {
                ways[i].reset_age (index);
                hit = true;
                way = i;
            }
        }
        cout << "TagArray::check_tag complete" << endl;

        if (hit) {
            return true;
        } else {
            return false;
        }
    }

    bool check_tag_evict (uint64_t addr, unsigned &way, uint64_t &evicted_tag) {
        // Returns true if there is a hit or there is an invalid way.
        cout << "TagArray::check_tag_evict - addr = " << addr << endl;
        if (check_tag(addr, way)) {
            cout << "Hit" << endl;
            return true;
        }

        unsigned way_to_evict = 0;
        unsigned max_age = 0;
        unsigned index;
        uint64_t tag;
        bool valid;

        get_index_and_tag (addr, index, tag);

        // check if there is any invalid way
        for (int i = 0; i < NUM_WAYS; i++) {
            if (!ways[i].is_valid(index)) {
                ways[i].reset_age (index);
                ways[i].write (index, tag);
                way = i;
                cout << "Miss, found invalid way" << endl;
                return true;
            }
        }

        cout << "Miss, have to evict" << endl;
        
        // find the oldest way
        for (int i = 0; i < NUM_WAYS; i++) {
            if (ways[i].get_age (index) > max_age) {
                max_age = ways[i].get_age (index);
                way_to_evict = i;
            }
        }

        cout << "Evicting way " << way_to_evict << endl;

        ways[way_to_evict].reset_age (index);
        ways[way_to_evict].get_tag (index, evicted_tag, valid);
        if (!valid) {
            cout << "Error" << endl;
            while (1);
        }
        ways[way_to_evict].write (index, tag);
        way = way_to_evict;
        cout << "Miss, evicted" << endl;
        return false;
    }
};
