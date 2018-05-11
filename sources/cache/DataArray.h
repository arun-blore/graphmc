#include <iostream>
#include <vector>
#include <stdint.h>
#include "types.h"
#include "cache_config.h"

using namespace std;

class DataArray {
    vector <vector <cacheblock> > array;

    public:
    DataArray () {
        array.resize(NUM_WAYS);
        for (int i = 0; i < NUM_WAYS; i++) {
            array[i].resize(NUM_ROWS);
        }
    }

    void print () {
        uint64_t data;
        for (unsigned row = 0; row < NUM_ROWS; row++) {
            for (unsigned i = 0; i < NUM_WAYS; i++) {
                data = array[row][i];
                cout << setfill(' ') << setw(30) << data;
            }
            cout << endl;
        }
    }

    cacheblock get_data (unsigned index, unsigned way) {
        return array[way][index];
    }

    void put_data (unsigned index, unsigned way, cacheblock data) {
        cout << "DataArray::put_data, way = " << way << " , index = " << index << endl;
        cout << array[way][index] << endl;
        cout << "Done1" << endl;
        array[way][index] = data;
        cout << "Done" << endl;
    }
};
