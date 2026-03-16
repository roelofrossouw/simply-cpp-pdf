#include <sc.h>

using namespace std;

int main() {
    const auto data = sc::rest::fetch("https://www.1web.co.za/myip.php");
    cout << data << endl;
}
