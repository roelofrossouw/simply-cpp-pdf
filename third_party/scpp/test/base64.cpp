#include <sc.h>

using namespace std;

int main() {
    string sample = "Hello World in Base64...";
    cout << "Source Text: " << sample << endl;
    auto encoded = sc::base64::encode(sample);
    cout << "Encoded: " << encoded << endl;
    auto decoded = sc::base64::decode(encoded);
    cout << "Decoded: " << decoded << endl;
}
