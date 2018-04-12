#include <iostream>
#include <queue>

using namespace std;

class SimObject {
    int currentClockCycle;

    public:
    SimObject () {
        currentClockCycle = 0;
    }

    ~SimObject () {}

    virtual void update () = 0;

    void Step () {
        currentClockCycle++;
    }

    void PrintCycle () {
        cout << "========= Clock Cycle " << currentClockCycle << " ===========" << endl;
    }
};

class DelayObject : public SimObject {
    int delay;
    int counter;

    public:
    DelayObject (int d) {
        // SimObject ();
        delay = d;
        counter = 0;
    }

    void update () {
        // if counter is 0, object is free and we can restart the count
        // if not, decrement the counter.
        if (counter != 0) {
            counter--;
        }
        Step ();
    }

    bool delay_complete () {
        return (counter == 0);
    }

    void start () {
        counter = delay;
    }
};

class CacheBank : public SimObject {
    DelayObject tag_access_delay;
    queue <unsigned> request_q;
    int miss_time;

    public :
    CacheBank (int miss_time);

    ~CacheBank () {}

    void new_request (unsigned req_id);

    void process_request (int req_id);

    void update ();
        
};

CacheBank::CacheBank (int miss_time) : tag_access_delay(miss_time) {
    this->miss_time = miss_time;
}

void CacheBank::new_request (unsigned req_id) {
    request_q.push (req_id);
    process_request (req_id);
}

void CacheBank::process_request (int req_id) {
    // if nothing is being processed
    if (tag_access_delay.delay_complete()) {
        if (!request_q.empty()) {
            tag_access_delay.start ();
        }
    }
}

void CacheBank::update () {
    tag_access_delay.update ();
    if (tag_access_delay.delay_complete ()) {
        if (!request_q.empty()) {
            cout << "Req " << request_q.front() << " complete" << endl;
            request_q.pop ();
        }
    }
    process_request (request_q.front());
    Step ();
    cout << "end of Update" << endl;
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

int main () {
    CacheBank C(4);
    C.new_request (0);
    // C.new_request (1);
    C.update ();
    C.update ();
    C.new_request(2);
    for (int i = 0; i < 10; i++) {
        // cout << "Update" << endl;
        C.update ();
    }
    return 0;
}
