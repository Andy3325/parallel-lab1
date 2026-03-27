#include <chrono>
#include <iostream>
using namespace std;

int main() {
    volatile long long s = 0;

    auto start = chrono::high_resolution_clock::now();

    for (long long i = 0; i < 100000000; ++i) {
        s += i;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> elapsed = end - start;

    cout << "sum = " << s << endl;
    cout << "time = " << elapsed.count() << " ms" << endl;

    return 0;
}
