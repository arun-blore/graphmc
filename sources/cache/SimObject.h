#ifndef _SIM_OBJECT_H_
#define _SIM_OBJECT_H_

#include <iostream>

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

#endif
