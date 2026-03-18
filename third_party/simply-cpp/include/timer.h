#ifndef SC_TIMER_TIMER_H
#define SC_TIMER_TIMER_H

class ostream;

namespace sc {
    namespace impl {
        class timer;
    }

    class timer {
    public:
        timer();

        ~timer();

        void reset();

        void start();

        void stop();

        void lap();

    private:
        impl::timer *impl;

        friend std::ostream &operator<<(std::ostream &lhs, timer &rhs);
    };
}


#endif //SC_TIMER_TIMER_H
