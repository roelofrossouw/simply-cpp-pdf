#include "include/color.h"

#include <sc.h>

using namespace std;

std::ostream &operator<<(std::ostream &lhs, const sc::color &rhs) {
    lhs << "cmyk(" << rhs.cyan() << "," << rhs.magenta() << "," << rhs.yellow() << "," << rhs.black() << "," << rhs.alpha() << ")"
            << " rgb(" << rhs.red() << "," << rhs.green() << "," << rhs.blue() << "," << rhs.alpha() << ")"
            << endl;
    return lhs;
}

int main() {
    sc::color k{"DarkRed"};
    cout << k << endl;

    k = sc::color::from_string("None");
    cout << k << endl;

    k = sc::color::from_string("#C00");
    cout << k << endl;

    k = sc::color::from_string("#c00000");
    cout << k << endl;

    k = sc::color::from_string("hsl(120,100%,50%)");
    cout << k << endl;

    k = sc::color::from_string("rgba(255,0,200, 0.2)");
    cout << k << endl;

    k = sc::color::from_web(255, 0, 200);
    cout << k << endl;

    sc::color c1{"blue"};
    sc::color c2{0,0,1};

    if (c1 != c2) {
        cout << "c1 != c2" << endl;
    } else {
        cout << "c1 == c2" << endl;
    }
}
