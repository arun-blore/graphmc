#ifndef _CACHE_H_
#define _CACHE_H_

#include "CacheBank.h"
#include "transaction.h"
#include <stdint.h>
#include "SimObject.h"
#include <vector>
#include <unordered_set>

using namespace std;

class cache : public SimObject {
    vector <CacheBank> banks;
    unsigned next_req_id;
    unordered_map <unsigned, transaction> pending_transactions;
    unordered_set <unsigned> completed_transactions;
    vector <uint64_t> completed_HMC_reqs;

    public:
    cache (unsigned miss_time, unsigned hit_time);
    unsigned read (uint64_t addr);
    unsigned write (uint64_t addr, uint64_t data);
    // void transaction_complete (unsigned req_id);
    void wait (unsigned req_id);
    void update ();
    void set_wrapper_ptr (CasHMCWrapper* ptr) {
        for (int i = 0; i < banks.size(); i++) {
            banks[i].set_wrapper_ptr(ptr, &completed_HMC_reqs);
        }
    }
    void HMC_req_complete (uint64_t addr) {
        completed_HMC_reqs.push_back (addr);
    }
    void print_banks () {
        for (unsigned i = 0; i < banks.size(); i++) {
            cout << "======== BANK " << i << "===========" << endl;
            banks[i].print();
            cout << endl;
        }
    }
    void run (unsigned);
};

#endif
