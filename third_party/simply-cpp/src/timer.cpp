#include "sc.h"
#include <chrono>

namespace sc {
    namespace impl {
        class timer {
        public:
            timer() {
                since = std::chrono::steady_clock::now();
                stopped = false;
            }

            bool stopped = false;
            std::chrono::steady_clock::time_point since;
            std::chrono::steady_clock::time_point until;
            std::chrono::milliseconds taken{};
        };
    }

    timer::timer() {
        impl = new impl::timer();
    }

    timer::~timer() {
        delete impl;
    }

    void timer::reset() {
        impl->since = std::chrono::steady_clock::now();
        impl->taken.zero();
    }

    void timer::start() {
        if (!impl->stopped) return;
        impl->since = std::chrono::steady_clock::now();
        impl->stopped = false;
    }

    void timer::stop() {
        impl->until = std::chrono::steady_clock::now();
        impl->stopped = true;
        impl->taken += std::chrono::duration_cast<std::chrono::milliseconds>(impl->until - impl->since);
    }

    void timer::lap() {
        if (impl->stopped) {
            // Work like a real stopwatch button where lap/reset is the same button?
            // reset();
            return;
        }
        impl->until = std::chrono::steady_clock::now();
        impl->taken += std::chrono::duration_cast<std::chrono::milliseconds>(impl->until - impl->since);
        impl->since = impl->until;
    }


    std::ostream &operator<<(std::ostream &lhs, timer &rhs) {
        rhs.lap();
#if defined(__cpp_lib_format)
        lhs << std::format("{:%H:%M:%S}", rhs.impl->taken);
#else
        auto ms = rhs.impl->taken;
        auto h = std::chrono::duration_cast<std::chrono::hours>(ms);
        ms -= h;
        auto m = std::chrono::duration_cast<std::chrono::minutes>(ms);
        ms -= m;
        auto s = std::chrono::duration_cast<std::chrono::seconds>(ms);
        ms -= s;
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(ms);

        lhs << std::setfill('0')
                << std::setw(2) << h.count() << ":"
                << std::setw(2) << m.count() << ":"
                << std::setw(2) << s.count() << "."
                << std::setw(3) << millis.count();
#endif
        return lhs;
    }
} // sc
