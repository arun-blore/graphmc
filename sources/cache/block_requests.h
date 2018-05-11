#ifndef _BLOCK_REQUESTS_H_
#define _BLOCK_REQUESTS_H_
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "transaction.h"
#include "types.h"
#include "utils.h"

using namespace std;

/*
Each entry has the following fields:
1. Block address
2. block_entry_id
3. vector of request ids
4. block of data
5. block state
6. boolean indicating if all requests are reads or are some of the requests, writes?
*/

enum block_state {
    /*
    State name consists of 2 parts:
    1. The first part is either VALID or INVALID. This represents the state of the corresponding 'data' field in the block_request table.
    2. The second part represents the status of the block
    */
    INVALID__CACHE_CHECK,
    INVALID__RD_REQ_SENT_TO_CACHE,
    INVALID__RW_REQ_SENT_TO_CACHE,
    INVALID__LINEFILL_REQ_SENT,
    INVALID__RD_REQ_SENT_TO_CACHE_NEW_WR,
    VALID__WR_REQ_SENT_TO_CACHE,
    VALID__CACHE_CHECK,
    VALID__RW_REQ_SENT_TO_CACHE // indicates that the data is valid, a read request to evict and write request to update the cacheline has been sent to the data array.
};

struct block_entry {
    vector <unsigned> req_ids;
    block_state state;
    bool all_reads; // 1 => all requests are reads, 0 => atleast one write among the requests
    unsigned way; // The cache way in which this block is stored (validity of this field depends on the state)
    cacheblock data;
};

class block_requests {
    // vector <block_entry> table;
    unordered_map <uint64_t, block_entry> table;
    unsigned next_block_entry_id = 0;
    unordered_map <unsigned, transaction> *pending_transactions;
    // vector <unsigned> *completed_transactions;
    unordered_set <unsigned> *completed_transactions;

    public:
    // block_requests (unordered_map <unsigned, transaction> *pt, vector<unsigned>* ct) : pending_transactions (pt), completed_transactions (ct) {}
    block_requests (unordered_map <unsigned, transaction> *pt, unordered_set<unsigned> *ct) : pending_transactions (pt), completed_transactions (ct) {}
    /*
    uint64_t get_addr (unsigned b_id) {
        for (unsigned i = 0; i < block_data.size(); i++) {
            if (block_data[i].block_entry_id == b_id) {
                return block_data[i].block_addr;
            }
        }
    }
    */

    block_state get_state (uint64_t addr) {
        cout << "block_requests::get_state" << endl;
        return table[addr].state;
    }

    /*
    unsigned new_evict_entry (uint64_t adr) {
        uint64_t addr;
        // Search for the address corresponding to entry with id = b_id

        block_entry new_entry = {next_block_entry_id, addr, vector <unsigned> {}, TO_EVICT, true};
        block_data.push_back (new_entry);
        next_block_entry_id ++;
        return (next_block_entry_id-1);
    }

    unsigned new_trans (transaction *trans, unsigned req_id, bool &new_entry_created, bool &transaction_complete) {
        block_state cur_state;
        unsigned cur_index;
        bool block_found = false;
        for (unsigned i = 0; i < block_data.size(); i++) {
            if ((block_data[i].block_addr == trans->addr) && (block_data[i].state != TO_EVICT)) {
                cur_index = i;
                cur_state = block_data[i].state;
                block_found = true;
                break;
            }
        }

        if (block_found == false) {
            bool read_write = (trans->type == READ);
            block_entry new_entry = {next_block_entry_id, trans->addr, vector <unsigned> {req_id}, CACHE_CHECK, read_write};
            block_data.push_back(new_entry);
            next_block_entry_id ++;
            new_entry_created = true;
            transaction_complete = false;
            return (next_block_entry_id-1);
        } else if (cur_state == BLOCK_WRITE) {
            new_entry_created = false;
            transaction_complete = true;
            return 0;
        } else {
            block_data[cur_index].req_ids.push_back (req_id);
            new_entry_created = false;
            transaction_complete = false;
            return 0;
        }
    }

    void tag_check_done (unsigned b_id, bool hit, unsigned way) {
        unsigned cur_index;
        for (unsigned i = 0; i < block_data.size(); i++) {
            if (block_data[i].block_entry_id == b_id) {
                cur_index = i;
                break;
            }
        }

        if (block_data[cur_index].state == CACHE_CHECK) {
            if (hit) {
                block_data[cur_index].state = WORD_WRITE;
            } else {
                block_data[cur_index].state = LINE_FILL;
            }
        } else {
            // block state is LINE_FILL
        }
        block_data[cur_index].dest_way = way;
    }

    vector <unsigned> data_access_done (unsigned b_id) {
        unsigned cur_index;
        vector <unsigned> completed_req_ids;
        for (unsigned i = 0; i < block_data.size(); i++) {
            if (block_data[i].block_entry_id == b_id) {
                cur_index = i;
                break;
            }
        }
        completed_req_ids = block_data[cur_index].req_ids;
        block_data.erase (block_data.begin() + cur_index);
        return completed_req_ids;
    }

    vector <unsigned> hmc_data_received (uint64_t addr) {
        unsigned cur_index;
        vector <unsigned> completed_req_ids;
        for (unsigned i = 0; i < block_data.size(); i++) {
            if ((block_data[i].block_addr == addr) && (block_data[i].state == LINE_FILL)) {
                cur_index = i;
                break;
            }
        }

        completed_req_ids = block_data[cur_index].req_ids;

        block_data[cur_index].state = BLOCK_WRITE;
        block_data[cur_index].req_ids.clear ();
        
        return completed_req_ids;
    }
    */

    bool is_all_reads (uint64_t addr) {
        return table[addr].all_reads;
    }

    unsigned get_way (uint64_t addr) {
        return table[addr].way;
    }

    void set_way (uint64_t addr, unsigned way) {
        table[addr].way = way;
    }

    void set_state (uint64_t addr, block_state new_state) {
        table[addr].state = new_state;
    }

    void service_reads (uint64_t addr) {
        unsigned req;
        for (int i = 0; i < table[addr].req_ids.size(); i++) {
            req = table[addr].req_ids[i];
            if ((*pending_transactions)[addr].type == trans_type::READ) {
                // TODO : update the data field of the transaction
                (*completed_transactions).insert (req);
            } else {
                cout << "Error" << endl;
                while (1);
            }
        }
    }

    void service_all (uint64_t addr) {
        cout << "block_requests::service_all" << endl;
        unsigned req;
        for (int i = 0; i < table[addr].req_ids.size(); i++) {
            req = table[addr].req_ids[i];
            if ((*pending_transactions)[addr].type == trans_type::READ) {
                // TODO : update the data field of the transaction
                (*completed_transactions).insert (req);
                cout << "trans_type::READ" << endl;
            } else { // write
                // update the data field of the appropriate block_requests row
                // table[addr].data ... // TODO
                (*completed_transactions).insert (req);
                cout << "size = " << (*completed_transactions).size() << endl;
                cout << "trans_type::WRITE" << endl;
            }
            cout << "block_requests::service_all - completed request " << req << endl;
        }
    }

    void service_leading_reads (uint64_t addr) {
        unsigned req;
        for (int i = 0; i < table[addr].req_ids.size(); i++) {
            req = table[addr].req_ids[i];
            if ((*pending_transactions)[addr].type == trans_type::READ) {
                // TODO : update the data field of the transaction
                (*completed_transactions).insert (req);
            } else { // write
                return;
            }
        }
    }

    void put_data (uint64_t addr, cacheblock data) {
        table[addr].data = data;
    }

    void erase (uint64_t addr) {
        table.erase (addr);
    }

    cacheblock get_data (uint64_t addr) {
        return table[addr].data;
    }

    bool find_addr (uint64_t addr) {
        cout << "block_requests::find_addr" << endl;
        unordered_map<uint64_t, block_entry>::const_iterator found = table.find (addr);
        if (found == table.end()) {
            return false;
        } else {
            return true;
        }
    }

    void new_read  (uint64_t addr, unsigned req_id, bool &new_request) {
        uint64_t block_addr = get_block_addr (addr);
        if (find_addr(block_addr)) {
            new_request = false;
            block_state state = table[block_addr].state;
            if ((state == VALID__WR_REQ_SENT_TO_CACHE) | (state == VALID__CACHE_CHECK) | (state == VALID__RW_REQ_SENT_TO_CACHE)) {
                // TODO: do the read operation (update data appropriately)
            } else {
                table[block_addr].req_ids.push_back (req_id);
            }
        } else {
            block_entry new_entry = {vector <unsigned> {req_id}, INVALID__CACHE_CHECK, true, 0, 0};
            table.insert ({block_addr, new_entry});
            new_request = true;
        }
    }

    void new_write (uint64_t addr, unsigned req_id, bool &new_request) {
        uint64_t block_addr = get_block_addr (addr);
        if (find_addr(block_addr)) {
            new_request = false;
            block_state state = table[block_addr].state;
            if ((state == VALID__WR_REQ_SENT_TO_CACHE) | (state == VALID__CACHE_CHECK) | (state == VALID__RW_REQ_SENT_TO_CACHE)) {
                // TODO: do the write operation (update data appropriately)
            } else if (state == INVALID__RD_REQ_SENT_TO_CACHE) {
                table[block_addr].state = INVALID__RD_REQ_SENT_TO_CACHE_NEW_WR;
                table[block_addr].req_ids.push_back (req_id);
                table[block_addr].all_reads = false;
            } else {
                table[block_addr].req_ids.push_back (req_id);
            }
        } else {
            block_entry new_entry = {vector <unsigned> {req_id}, INVALID__CACHE_CHECK, false, 0, 0};
            table.insert ({block_addr, new_entry});
            new_request = true;
        }
    }

};

#endif
