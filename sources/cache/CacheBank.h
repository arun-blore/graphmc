#ifdef  _CACHE_BANK_H_
#define _CACHE_BANK_H_

#include <iostream>
#include <queue>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include "transaction.h"
#include "way.h"
#include "DelayObject.h"
#include "SimObject.h"

using namespace std;

class CacheBank : public SimObject {
    DelayObject tag_check_delay;
    DelayObject data_access_delay;
    unordered_map <unsigned, uint64_t> pending_transactions;
    queue <unsigned> tag_check_q;
    queue <unsigned> data_access_q;
    vector <way> ways;
    int miss_time;
    int hit_time;
    // unsigned next_req_id;
    unordered_set <unsigned> completed_transactions;
    int num_rows = 128;

    public :
    CacheBank (int miss_time, int hit_time);

    ~CacheBank () {}

    // void new_request (unsigned req_id);
    unsigned read (uint64_t addr, unsigned req_id);

    void process_request ();

    void update ();
        
    bool check_tag (unsigned req_id);

    void get_row_index_and_tag (uint64_t addr, unsigned &row_index, unsigned &tag);

    bool is_transaction_complete (unsigned req_id);

    void print ();
};

bool CacheBank::is_transaction_complete (unsigned req_id) {
    unordered_set <unsigned>::const_iterator found = completed_transactions.find(req_id);
    if (found == completed_transactions.end()) {
        return false;
    } else {
        completed_transactions.erase (req_id);
        return true;
    }
}

void CacheBank::get_row_index_and_tag (uint64_t addr, unsigned &row_index, unsigned &tag) {
    // Assuming 32K, 4 way cache with 64B blocks
    tag       = unsigned(addr >> 13);
    row_index = unsigned((addr >> 6) & 0x7f);
}

bool CacheBank::check_tag (unsigned req_id) {
    unsigned row_index, tag;
    uint64_t addr = pending_transactions[req_id];
    cout << "Doing tag check" << endl;
    get_row_index_and_tag(addr, row_index, tag);
    for (unsigned i = 0; i < ways.size(); i++) {
        if (ways[i].check_tag(row_index, tag)) {
            cout << "Tag Hit" << endl;
            return true;
        }
    }
    cout << "Tag Miss" << endl;
    return false;
}

void CacheBank::print () {
    unsigned tag;
    bool valid;
    for (unsigned row = 0; row < num_rows; row++) {
        for (unsigned i = 0; i < ways.size(); i++) {
            ways[i].get_tag (row, tag, valid);
            if (valid == false)
                cout << setw(20) << "invalid";
            else
                cout << setw(20) << tag;
        }
        cout << endl;
    }
}

CacheBank::CacheBank (int miss_time, int hit_time) : tag_check_delay(miss_time), data_access_delay(hit_time-miss_time) {
    this->miss_time = miss_time;
    this->hit_time = hit_time;
    next_req_id = 0;

    ways.resize(4);
    cout << "reserved 4 ways" << endl;

    for (unsigned i = 0; i < 128; i++) {
        ways[0].write (i, i);
    }
    cout << "initialized ways" << endl;
}

unsigned CacheBank::read (uint64_t addr, unsigned req_id) {
    // unsigned req_id;

    // Add the transaction to the pending_transactions table
    // push the req_id to the tag_lookup_q;
    // Initiate tag check

    tag_check_q.push (addr);
    process_request ();

    // req_id = next_req_id;
    // next_req_id++;

    return req_id;
}

void CacheBank::process_request () {
    cout << "process request" << endl;

    // if nothing is being processed
    cout << "State of tag_check_delay object is ";
    tag_check_delay.print ();
    if (tag_check_delay.is_idle()) {
        if (!tag_check_q.empty()) {
            tag_check_delay.start ();
        }
    }

    if (data_access_delay.is_idle()) {
        if (!data_access_q.empty()) {
            data_access_delay.start ();
        }
    }
}

void CacheBank::update () {
    bool hit;
    unsigned req;

    // Update tag_check_delay
    tag_check_delay.update ();
    // Update data_access_delay
    data_access_delay.update ();

    cout << "=== tag and data delay updated ===" << endl;

    // Check if tag check is complete
    // if yes, get the result (hit/miss)
    // if hit, push the req_id to the data_access_q
    // if miss, ??
    cout << "State of tag_check_delay object is ";
    tag_check_delay.print ();
    if (tag_check_delay.delay_complete()) {
        cout << "Tag check delay complete" << endl;
        req = tag_check_q.front();
        hit = check_tag (req);
        if (hit) {
            tag_check_q.pop();
            data_access_q.push(req);
        } else {
        }
    } else {
        cout << "tag check in progress" << endl;
    }

    // Check if data access is complete
    // if yes, return the data
    // remove the req_id from the pending_transactions hash table
    // call the appropriate transaction_done function
    if (data_access_delay.delay_complete ()) {
        req = data_access_q.front ();
        data_access_q.pop ();
        cout << "Completed request to address " << pending_transactions[req] << endl;
        pending_transactions.erase (req);
        completed_transactions.insert(req);
    }

    process_request ();
    Step ();
}

/*
int main () {
    DelayObject D (4);
    D.start ();
    for (int i = 0; i < 10; i++) {
        D.PrintCycle ();
        if (D.delay_complete ()) {
            cout << "delay complete" << endl;
        } else {
            cout << "delay not complete" << endl;
        }
        D.update ();
        cout << "Update" << endl;
    }
    return 0;
}
*/

/*
int main () {
    CacheBank C(4,6);
    // C.print ();
    unsigned id = C.read (0);
    unsigned id1 = C.read (1);
    while (true) {
        if (C.is_transaction_complete (id1)) {
            break;
        } else {
            C.update ();
        }
    }
        
    // // C.new_request (1);
    // C.update ();
    // C.update ();
    // C.read (2);
    // for (int i = 0; i < 10; i++) {
    //     // cout << "Update" << endl;
    //     C.update ();
    // }
    return 0;
}
*/

#endif
