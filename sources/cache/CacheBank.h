#ifndef _CACHE_BANK_H_
#define _CACHE_BANK_H_

#include <iostream>
#include <queue>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include "transaction.h"
//#include "way.h"
#include "DelayObject.h"
#include "SimObject.h"
#include "CasHMCWrapper.h"
#include "Transaction.h"
#include <cstdlib>
#include "cache_config.h"
#include "block_requests.h"
#include "utils.h"
#include "types.h"
#include "TagArray.h"
#include "DataArray.h"

using namespace std;
using namespace CasHMC;

class CacheBank : public SimObject {
    DelayObject tag_check_delay;
    DelayObject data_access_delay;
    unordered_map <unsigned, transaction> *pending_transactions;
    queue <uint64_t> tag_check_q;
    queue <uint64_t> data_access_q;
    queue <uint64_t> evict_q;
    // vector <way> ways;
    TagArray tag_array;
    DataArray data_array;
    int miss_time;
    int hit_time;
    // unsigned next_req_id;
    // cache* p_cache;
    // queue <Transaction>* HMC_q;
    CasHMCWrapper* casHMCWrapper;
    unordered_map <uint64_t, unsigned> pending_HMC_reqs;
    vector <uint64_t>* completed_HMC_reqs;
    block_requests block_requests_table;

    public :
    // vector <unsigned> completed_transactions;
    unordered_set <unsigned> *completed_transactions;

    CacheBank (int miss_time, int hit_time, unordered_map <unsigned, transaction>*, unordered_set <unsigned> *);

    ~CacheBank () {}

    // void new_request (unsigned req_id);
    void read (uint64_t addr, unsigned req_id);

    void write (uint64_t addr, unsigned req_id);

    void process_request ();

    void update ();
        
    bool check_tag (unsigned req_id);

    // void get_index_and_tag (uint64_t addr, unsigned &row_index, unsigned &tag);

    // bool is_transaction_complete (unsigned req_id);

    void print ();

    // bool get_free_way (unsigned index, unsigned &free_way);

    // void perform_data_operation (unsigned req_id);

    void set_wrapper_ptr (CasHMCWrapper* ptr, vector <uint64_t>* c_ptr) {
        casHMCWrapper = ptr;
        completed_HMC_reqs = c_ptr;
    }

    void do_tag_check ();

    void do_data_access ();

    void check_hmc_resp ();
};

// bool CacheBank::is_transaction_complete (unsigned req_id) {
//     unordered_set <unsigned>::const_iterator found = completed_transactions.find(req_id);
//     if (found == completed_transactions.end()) {
//         return false;
//     } else {
//         completed_transactions.erase (req_id);
//         return true;
//     }
// }

// void CacheBank::set_cache_pointer (cache* cache_ptr) {
//     p_cache = cache_ptr;
// }


// bool CacheBank::check_tag (uint64_t addr, unsigned &way);
// {
//     unsigned row_index, tag;
//     bool hit;
//     // uint64_t addr = pending_transactions[req_id].addr;
//     cout << "Doing tag check" << endl;
//     get_index_and_tag(addr, row_index, tag);
//     for (unsigned i = 0; i < ways.size(); i++) {
//         ways[i].increment_age ();
//          if (ways[i].check_tag(row_index, tag)) {
//             hit = true;
//             ways[i].reset_age ();
//             way = i;
//             // break;
//             // return true;
//         }
//     }
// 
//     if (hit) {
//         cout << "Tag Hit" << endl;
//     } else {
//         cout << "Tag Miss" << endl;
//     }
//     return hit;
// }

void CacheBank::print () {
    tag_array.print ();
}

CacheBank::CacheBank (int miss_time, int hit_time, unordered_map <unsigned, transaction> *pt, unordered_set <unsigned> *ct) : 
        tag_check_delay(miss_time),
        data_access_delay(hit_time-miss_time),
        pending_transactions (pt), 
        completed_transactions (ct),
        block_requests_table (block_requests(pt, ct)) {
    this->miss_time = miss_time;
    this->hit_time = hit_time;
    // next_req_id = 0;

    // ways.resize(NUM_WAYS);
    // cout << "reserved 4 ways" << endl;

    // for (unsigned i = 0; i < 128; i++) {
    //     ways[0].write (i, i);
    // }
    // cout << "initialized ways" << endl;
}

void CacheBank::read (uint64_t addr, unsigned req_id) {
    bool new_entry;
    uint64_t block_addr = get_block_addr (addr);
    block_requests_table.new_read (addr, req_id, new_entry);
    if (new_entry) {
        tag_check_q.push(block_addr);
    }
    process_request ();

    //    // Add the transaction to the pending_transactions table
    //    // push the req_id to the tag_lookup_q;
    //    // Initiate tag check
    //    transaction t = {addr, 0, trans_type::READ};
    //    tag_check_q.push (req_id);
    //    pending_transactions.insert ({req_id, t});
    //    process_request ();

    //    // req_id = next_req_id;
    //    // next_req_id++;
}

void CacheBank::write (uint64_t addr, unsigned req_id) {
    bool new_entry;
    uint64_t block_addr = get_block_addr (addr);
    cout << "CacheBank::write - write to block " << block_addr << endl;
    block_requests_table.new_write (addr, req_id, new_entry);
    if (new_entry) {
        cout << "Adding new entry to block_requests_table" << endl;
        tag_check_q.push(block_addr);
    }
    process_request ();

    // // Add the transaction to the pending_transactions table
    // // push the req_id to the tag_lookup_q;
    // // Initiate tag check

    // transaction t = {addr, data, trans_type::WRITE};
    // tag_check_q.push (req_id);
    // pending_transactions.insert ({req_id, t});
    // process_request ();

    // // req_id = next_req_id;
    // // next_req_id++;
}

void CacheBank::process_request () {
    cout << "process request" << endl;

    // if nothing is being processed
    cout << "State of tag_check_delay object is ";
    // tag_check_delay.print ();
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

void CacheBank::do_tag_check () {
    bool hit, all_reads;
    unsigned way, index;
    uint64_t evicted_tag, tag, evict_addr, addr;
    block_state state, next_state;
    
    if (tag_check_q.size()) {
        addr = tag_check_q.front (); // pop the next entry from the tag_check queue
        cout << "Doing tag check for addr " << addr << endl;
        tag_check_q.pop ();
        // cout << "Here 1" << endl;
        get_index_and_tag (addr, index, tag);
        state = block_requests_table.get_state (addr);
        // cout << "Here 2" << endl;
        if (state == block_state::INVALID__CACHE_CHECK) { // data in the request_block is invalid, we want to do a cache check
            cout << "State is INVALID__CACHE_CHECK" << endl;
            hit = tag_array.check_tag (addr, way);
            if (hit) {
                cout << "Cache hit" << endl;
                all_reads = block_requests_table.is_all_reads (addr);
                if (all_reads) { // All the requests to this block are reads.
                    cout << "All Reads" << endl;
                    // push data read from (index, way); // add a data access request to the data queue.
                    data_access_q.push (addr);
                    next_state = block_state::INVALID__RD_REQ_SENT_TO_CACHE;
                } else { // There are a few writes among the requests. We need to read and write back into cache
                    // push data read from (index, way);
                    data_access_q.push (addr);
                    // push data write to (index, way);
                    data_access_q.push (addr);
                    next_state = block_state::INVALID__RW_REQ_SENT_TO_CACHE;
                }
                block_requests_table.set_way (addr, way); // set the 'way' field of the block in the block_requests table
            } else {
                // send linefill request to HMC from address addr
                cout << "Cache Miss" << endl;
                Transaction* trans;
        	    trans = new Transaction(DATA_READ, addr, 64, casHMCWrapper);		// Read transaction
                casHMCWrapper->ReceiveTran(trans);
                cout << "Sent request to HMC" << endl;
                // pending_HMC_reqs.insert (addr);
        
                next_state = block_state::INVALID__LINEFILL_REQ_SENT;
            }
        } else if (state == block_state::VALID__CACHE_CHECK) { // state must be VALID__CACHE_CHECK
            // hit is true if either there was a hit or if there was an empty way.
            hit = tag_array.check_tag_evict (addr, way, evicted_tag);
            if (hit) {
                // push data write to (index, way);
                data_access_q.push (addr);
                next_state = block_state::VALID__WR_REQ_SENT_TO_CACHE;
            } else {
                // push address make_addr (addr, tag) to eviction queue.
                evict_q.push (make_addr(addr, evicted_tag));
        
                // push data read from (index, way); // for eviction
                data_access_q.push (addr);
        
                // push data write to (index, way);
                data_access_q.push (addr);
        
                next_state = block_state::VALID__RW_REQ_SENT_TO_CACHE;
            }
            block_requests_table.set_way (addr, way);
        } else {
            cout << "Error" << endl;
            while (1);
        }
        
        cout << "Here 3" << endl;
        block_requests_table.set_state (addr, next_state);
    } else {
        cout << "Error" << endl;
        while (1);
    }
}

void CacheBank::do_data_access () {
    bool hit, all_reads;
    unsigned way, index;
    uint64_t evicted_tag, tag, evict_addr, addr;
    block_state state, next_state;

    if (data_access_q.size()) {
        // Data check operations
        addr = data_access_q.front (); // pop the next entry from the data_access queue.
        data_access_q.pop ();
        get_index_and_tag (addr, index, tag);
        state = block_requests_table.get_state (addr);
        way = block_requests_table.get_way (addr);
        cacheblock data;

        // cout << "Here D1" << endl;
        cout << "Doing data access. addr = " << addr << endl;
        
        // if (operation == BLOCK_READ) {
        //     data = read data from (index, way);
        // } else {
        //     data = block_requests_table.get_data (addr);
        //     write data to (index, way);
        // }
        
        if (state == block_state::INVALID__RD_REQ_SENT_TO_CACHE) {
            // read data from data array
            // data = read data from (index, way);
            data = data_array.get_data (index, way);
            // write data into the request_table
            block_requests_table.put_data (addr, data);
            // service all pending requests to this block (all of them must be reads)
            block_requests_table.service_reads (addr);
            // remove entry from the table
            block_requests_table.erase (addr);
        } else if (state == block_state::INVALID__RW_REQ_SENT_TO_CACHE) {
            // data = read data from (index, way);
            data = data_array.get_data (index, way);
            // write data into the request_table
            block_requests_table.put_data (addr, data);
            // service all reads in order until the first write is encountered
            block_requests_table.service_all (addr);
            next_state = block_state::VALID__WR_REQ_SENT_TO_CACHE;
        } else if (state == block_state::VALID__WR_REQ_SENT_TO_CACHE) {
            cout << "State = VALID__WR_REQ_SENT_TO_CACHE" << endl;
            data = block_requests_table.get_data (addr);
            cout << "Here D2" << endl;
            // service all requests to the current block (first one will be a write).
            // Update the data field of this block with various write data
            // write data to (index, way);
            data_array.put_data (index, way, data);
            cout << "Here D3" << endl;
            // remove entry from table
            block_requests_table.erase (addr);
            cout << "Here D4" << endl;
        } else if (state == block_state::INVALID__RD_REQ_SENT_TO_CACHE_NEW_WR) {
            cout << "State = INVALID__RD_REQ_SENT_TO_CACHE_NEW_WR" << endl;
            // data = read data from (index, way);
            data = data_array.get_data (index, way);
            // write data into the request_table
            block_requests_table.put_data (addr, data);
            // service all requests to the current block until the first write is encountered
            block_requests_table.service_leading_reads (addr);
            // push a new tag check request to the tag_check queue.
            tag_check_q.push (addr);
            next_state = block_state::VALID__CACHE_CHECK;
        } else if (state == block_state::VALID__RW_REQ_SENT_TO_CACHE) {
            // data = read data from (index, way);
            data = data_array.get_data (index, way);
        
            // send read data to HMC (evict)
            Transaction* trans;
            evict_addr = evict_q.front ();
            evict_q.pop ();
        	trans = new Transaction(DATA_WRITE, evict_addr, 64, casHMCWrapper);		// Write transaction
            casHMCWrapper->ReceiveTran(trans);
            // pending_HMC_reqs.insert ({addr, req}); // dont do this since HMC does not generate an ack on write.
            // HMC.write (data, evict_addr)
            next_state = VALID__WR_REQ_SENT_TO_CACHE;
        } else {
            cout << "Error" << endl;
            cout << "Unknown state, " << state << endl;
            while (1);
        }
        
        block_requests_table.set_state (addr, next_state);
    } else {
        cout << "Error" << endl;
        while (1);
    }
}

void CacheBank::check_hmc_resp () {
    block_state state, next_state;

    // When data arrives from HMC
    cout << "Size of completed_HMC_reqs = " << completed_HMC_reqs->size() << endl;
    for (int i = 0; i < completed_HMC_reqs->size(); i++) {
        cout << "HMC resp" << endl;
        uint64_t hmc_resp_addr = (*completed_HMC_reqs)[i];
        cout << "address = " << hmc_resp_addr << endl;
        if (block_requests_table.find_addr (hmc_resp_addr)) {
            cout << "Addr found" << endl;
            state = block_requests_table.get_state (hmc_resp_addr);
            if (state == block_state::INVALID__LINEFILL_REQ_SENT) {
                // write the data received from HMC into the block_requests table (TODO)
                block_requests_table.put_data (hmc_resp_addr, 0); // TODO: change 0 to the data received
                // service all requests (reads and writes) to the current block.
                block_requests_table.service_all (hmc_resp_addr);
                cout << "service all done" << endl;
                // push a new tag check request to the tag_check queue.
                tag_check_q.push (hmc_resp_addr);
                next_state = VALID__CACHE_CHECK;
                block_requests_table.set_state (hmc_resp_addr, next_state);
            } else {
                cout << "Error" << endl;
                while (1);
            }
        }
    }
}

void CacheBank::update () {
    bool hit;
    unsigned req;
    uint64_t addr;

    // Update tag_check_delay
    tag_check_delay.update ();
    // Update data_access_delay
    data_access_delay.update ();

    cout << "=== tag and data delay updated ===" << endl;

    /*
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
            // HMC.read ()
            cout << "Sending request to HMC " << endl;
            Transaction* trans;
            addr = pending_transactions[req].addr;
			trans = new Transaction(DATA_READ, addr, 64, casHMCWrapper);		// Read transaction
            casHMCWrapper->ReceiveTran(trans);
            tag_check_q.pop();
            pending_HMC_reqs.insert ({addr, req});
        }
    } else {
        cout << "tag check in progress" << endl;
    }

    // Check if tag check is complete
    if (tag_check_delay.delay_complete ()) {
        bool writeback_required; // indicates that a way has to be cleared. This happens when the linefill data is available.
        unsigned new_req, evicted_tag, evicted_way = 0;
        uint64_t evicted_addr;
        block_state state;
        req = tag_check_q.front();
        tag_check_q.pop();
        state = block_requests_table.get_state (req);

        addr = block_requests_table.get_addr (req);

        if (state == CACHE_CHECK) {
            hit = check_tag (addr);
        } else if (state == LINE_FILL) {
            // hit = check_tag (req, true);
            hit = check_tag_evict (addr, evicted_tag, evicted_way, writeback_required);
        }
            
        block_requests_table.tag_check_done (req, hit, evicted_way);
        evicted_addr = f (addr, tag);

        if (hit) {
            // do data access
            data_access_q.push(req);
        else {
            if (state == LINE_FILL) {
                // if doing linefill
                // add 2 data requests
                // 1. read the address to be evicted
                new_req = block_requests_table.new_evict_entry (evicted_addr);
                data_access_q.push(new_req);
                // 2. write the data to cache
                data_access_q.push(req);
            } else {
                // HMC read
            }
        }
    }

    // Check if data access is complete
    // if yes, return the data
    // remove the req_id from the pending_transactions hash table
    // call the appropriate transaction_done function
    if (data_access_delay.delay_complete ()) {
        req = data_access_q.front ();
        perform_data_operation (req);
        data_access_q.pop ();
        cout << "Completed request to address " << pending_transactions[req].addr << endl;
        // p_cache->transaction_complete (req);
        pending_transactions.erase (req);
        completed_transactions.push_back(req);
    }

    // Check if HMC Q has any completed transactions
    // if any transaction is complete, update the cache and return
    for (int i = 0; i < completed_HMC_reqs->size(); i++) {
        uint64_t addr = (*completed_HMC_reqs)[i];
        unordered_map <uint64_t, unsigned>::const_iterator found = pending_HMC_reqs.find(addr);
        if (found == pending_HMC_reqs.end()) {
        } else {
            req = found->second;
            perform_data_operation (req);
            completed_transactions.push_back (req);
            pending_HMC_reqs.erase (found);
        }
    }
    */

    if (tag_check_delay.delay_complete()) {
        cout << "Doing tag check" << endl;
        do_tag_check ();
    }

    if (data_access_delay.delay_complete ()) {
        cout << "Doing data access" << endl;
        do_data_access ();
    }

    check_hmc_resp ();

    process_request ();
    Step ();
}

/*
bool CacheBank::get_free_way (unsigned index, unsigned &free_way) {
    for (unsigned i = 0; i < ways.size(); i++) {
        if (!ways[i].is_valid(index)) {
            free_way = i;
            return true;
        }
    }
    return false;
}
*/

/*
void CacheBank::perform_data_operation (unsigned req_id) {
    uint64_t addr   = pending_transactions[req_id].addr;
    trans_type type = pending_transactions[req_id].type;
    unsigned free_way, index, tag;
    get_index_and_tag (addr, index, tag);
    if (type == trans_type::READ) {
        return;
    } else {
        // write
        // check if there is space in the cache
        if (get_free_way(index, free_way)) {
            ways[free_way].write (index, tag);
        } else {
            // evict way 0 always
            // HMC.write ()
            int way_evicted = rand() % num_ways;
            ways[way_evicted].write (index, tag);
        }
    }
}
*/

/*
uint64_t CacheBank::get_block_addr (uint64_t addr) {
    uint64_t tmp = (0xffffffffffffffff << NUM_BYTE_OFFSET_BITS);
    return (addr & tmp);
}
*/

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
