#include "cache.h"
#include "transaction.h"
#include <getopt.h>		//getopt_long
#include <stdlib.h>		//exit(0)
#include <fstream>		//ofstream
#include <vector>		//vector

#include "CasHMCWrapper.h"
#include "Transaction.h"
#include "CallBack.h"
#include <cstdlib>
#include <time.h>
#include "cache_config.h"
#include "utils.h"

using namespace std;
using namespace CasHMC;

cache C(MISS_TIME,HIT_TIME);
CasHMCWrapper *casHMCWrapper;

void HMC_read_done (uint64_t addr) {
    cout << "HMC request complete " << addr << endl;
    C.HMC_req_complete (addr);
}

cache::cache (unsigned miss_time, unsigned hit_time) {
    // banks.reserve (NUM_BANKS);
    for (int i = 0; i < NUM_BANKS; i++) {
        banks.push_back(CacheBank(miss_time, hit_time, &pending_transactions, &completed_transactions));
    }
}

unsigned cache::read (uint64_t addr) {
    unsigned dest_bank = 0;
    transaction t = {addr, 0, trans_type::READ};
    pending_transactions.insert ({next_req_id, t});
    // create a new transaction
    // decide which cache bank this request should go to
    // Assuming 4 banks of 32KB each
    // dest_bank = (addr >> 15) & (3);
    dest_bank = get_bank (addr);
    banks[dest_bank].read (addr, next_req_id);
    next_req_id++;
    return (next_req_id-1);
}

unsigned cache::write (uint64_t addr, uint64_t data) {
    unsigned dest_bank = 0;
    transaction t = {addr, data, trans_type::WRITE};
    pending_transactions.insert ({next_req_id, t});
    // create a new transaction
    // decide which cache bank this request should go to
    // Assuming 4 banks of 32KB each
    // dest_bank = (addr >> 15) & (3);
    dest_bank = get_bank (addr);
    banks[dest_bank].write (addr, next_req_id);
    next_req_id++;
    return (next_req_id-1);
}

// void cache::transaction_complete (unsigned req_id) {
//     completed_transactions.insert (req_id);
// }

void cache::wait (unsigned req_id) {
    unordered_set <unsigned>::const_iterator found = completed_transactions.find(req_id);
    while (true) {
        found = completed_transactions.find(req_id);
        if (found == completed_transactions.end()) {
            update ();
            // return false;
        } else {
            cout << "cache::wait - request " << req_id << " complete" << endl;
            completed_transactions.erase (req_id);
            break;
            // return true;
        }
    }
}

void cache::run (unsigned num_cycles) {
    for (unsigned i = 0; i < num_cycles; i++) {
        update ();
    }
}

void cache::update () {
    PrintCycle();
    casHMCWrapper->Update ();
    for (unsigned i = 0; i < banks.size(); i++) {
        banks[i].update();
        // for (unsigned j = 0; j < banks[i].completed_transactions.size(); j++) {
        //     cout << "cache::update - bank " << i << " completd request " << banks[i].completed_transactions[j] << endl;
        //     completed_transactions.insert(banks[i].completed_transactions[j]);
        // }
    }
    completed_HMC_reqs.clear();
    Step();
}

void test1 () {
    unsigned id0, id1, id2;
    id0 = C.write (0x0000, 0);
    id1 = C.write (0x2000, 1);
    id2 = C.read (0x0000);
    C.wait (id0);
    C.wait (id1);
    C.wait (id2);
    C.run (20);
}

void test2 () {
    vector <unsigned> id;
    uint64_t addr = 0;
    int num_writes = 0;
    bool done = false;
    while (!done) {
        for (int i = 0; i < 10; i++) {
            id.push_back(C.write (addr, 0));
            num_writes++;
            addr += 0x40;

            // if (num_writes == 2049) {
            //     done = true;
            //     break;
            // }
        }

        for (int i = 0; i < id.size(); i++) {
            C.wait (id[i]);
        }
        id.clear ();
        // cout << "Test2 : iteration complete" << endl;
    }
    C.run(100);
}

int main () {
	casHMCWrapper = new CasHMCWrapper("../HMC/ConfigSim.ini", "../HMC/ConfigDRAM.ini");
    C.set_wrapper_ptr (casHMCWrapper);
    srand(time(NULL));
    // C.print_banks ();
    // unsigned id0, id1, id2;
    // id0 = C.write (0x0000, 0);
    // id1 = C.write (0x2000, 1);
    // id2 = C.read (0x0000);
    // C.wait (id0);
    // C.wait (id1);
    // C.wait (id2);
    // C.run (20);
    // // id0 = C.write (0x0000, 0);
    // // C.wait (id0);
    // // id0 = C.read (0x0001);
    // // C.wait (id0);
    // C.print_banks ();
    C.print_banks ();
    test2 ();
    C.print_banks ();
    return 0;
}
