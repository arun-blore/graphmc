#include "SimObject.h"

enum delay_state {
    IDLE,
    IN_PROGRESS,
    COMPLETE
};

class DelayObject : public SimObject {
    int delay;
    int counter;
    delay_state state;

    public:
    DelayObject (int d) {
        // SimObject ();
        delay = d;
        counter = 0;
        state = IDLE;
    }

    void update () {
        // if counter is 0, object is free and we can restart the count
        // if not, decrement the counter.
        if (counter > 1) {
            counter--;
        } else if (counter == 1) {
            counter--;
            state = COMPLETE;
        }
        Step ();
    }

    bool delay_complete () {
        if ((counter == 0) && (state == COMPLETE)) {
            state = IDLE;
            return true;
        } else {
            return false;
        }
    }

    void start () {
        counter = delay;
        state = IN_PROGRESS;
        // cout << "start: counter = " << counter << endl;
    }

    bool is_idle () {
        return (state == IDLE);
    }

    void print () {
        cout << "counter = " << counter << " state = " << state << endl;
    }
};
