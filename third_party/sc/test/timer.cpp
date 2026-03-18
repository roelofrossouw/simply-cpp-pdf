#include <sc.h>
#include <thread>
using namespace std;

int main() {
    sc::timer t, t2;
    // Do some work
    std::this_thread::sleep_for(0.1s);
    cout << "Some work took " << t << endl;
    t2.reset();
    // Do more work
    std::this_thread::sleep_for(0.2s);
    t.stop();
    t2.stop();
    // Some cleanup that we don't care about the timing
    cout << "More work took " << t2 << endl;
    cout << "Together took " << t << endl;
    return 0;
}
