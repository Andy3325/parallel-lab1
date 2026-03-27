#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

using Clock = chrono::high_resolution_clock;

void init_data(size_t n, vector<uint64_t>& a) {
    a.resize(n);
    for (size_t i = 0; i < n; ++i) {
        a[i] = static_cast<uint64_t>(i % 100);
    }
}

// 平凡链式累加
uint64_t sum_chain(const vector<uint64_t>& a) {
    uint64_t s = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        s += a[i];
    }
    return s;
}

// 两路链式累加
uint64_t sum_twoway(const vector<uint64_t>& a) {
    uint64_t s0 = 0, s1 = 0;
    size_t i = 0;
    size_t n = a.size();

    for (; i + 1 < n; i += 2) {
        s0 += a[i];
        s1 += a[i + 1];
    }
    if (i < n) s0 += a[i];

    return s0 + s1;
}

double benchmark_ms(function<uint64_t()> fn, int repeat, uint64_t& last_sum) {
    last_sum = fn();  // warm-up

    auto start = Clock::now();
    for (int r = 0; r < repeat; ++r) {
        last_sum = fn();
    }
    auto end = Clock::now();

    chrono::duration<double, milli> elapsed = end - start;
    return elapsed.count() / repeat;
}

int default_repeat(size_t n) {
    if (n <= (1u << 10)) return 20000;
    if (n <= (1u << 12)) return 5000;
    if (n <= (1u << 14)) return 2000;
    if (n <= (1u << 16)) return 500;
    if (n <= (1u << 18)) return 150;
    if (n <= (1u << 20)) return 50;
    if (n <= (1u << 22)) return 10;
    return 3;
}

void run_check(size_t n) {
    vector<uint64_t> a;
    init_data(n, a);

    uint64_t s1 = sum_chain(a);
    uint64_t s2 = sum_twoway(a);

    cout << "n=" << n << "\n";
    cout << "sum_chain=" << s1 << "\n";
    cout << "sum_twoway=" << s2 << "\n";
    cout << "equal=" << (s1 == s2 ? "true" : "false") << "\n";
}

void run_single(const string& mode, size_t n, int repeat) {
    vector<uint64_t> a;
    init_data(n, a);

    uint64_t last_sum = 0;
    double ms = 0.0;

    if (mode == "chain") {
        ms = benchmark_ms([&]() { return sum_chain(a); }, repeat, last_sum);
    } else if (mode == "twoway") {
        ms = benchmark_ms([&]() { return sum_twoway(a); }, repeat, last_sum);
    } else {
        cerr << "Unknown mode: " << mode << "\n";
        exit(1);
    }

    cout << fixed << setprecision(6);
    cout << "mode=" << mode
         << " n=" << n
         << " repeat=" << repeat
         << " avg_ms=" << ms
         << " checksum=" << last_sum << "\n";
}

void run_bench() {
    vector<size_t> sizes = {
        1u << 10, 1u << 12, 1u << 14, 1u << 16,
        1u << 18, 1u << 20, 1u << 22
    };

    cout << fixed << setprecision(6);
    cout << "n,repeat,chain_ms,twoway_ms,speedup,checksum\n";

    for (size_t n : sizes) {
        vector<uint64_t> a;
        init_data(n, a);

        int repeat = default_repeat(n);
        uint64_t s1 = 0, s2 = 0;

        double t1 = benchmark_ms([&]() { return sum_chain(a); }, repeat, s1);
        double t2 = benchmark_ms([&]() { return sum_twoway(a); }, repeat, s2);

        if (s1 != s2) {
            cerr << "Mismatch at n=" << n << ": " << s1 << " vs " << s2 << "\n";
            exit(1);
        }

        cout << n << ","
             << repeat << ","
             << t1 << ","
             << t2 << ","
             << (t1 / t2) << ","
             << s1 << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1 || string(argv[1]) == "bench") {
        run_bench();
        return 0;
    }

    string mode = argv[1];

    if (mode == "check") {
        size_t n = 16;
        if (argc >= 3) n = static_cast<size_t>(stoull(argv[2]));
        run_check(n);
        return 0;
    }

    if (mode == "chain" || mode == "twoway") {
        if (argc < 4) {
            cerr << "Usage: ./build/sum [chain|twoway] n repeat\n";
            return 1;
        }
        size_t n = static_cast<size_t>(stoull(argv[2]));
        int repeat = stoi(argv[3]);
        run_single(mode, n, repeat);
        return 0;
    }

    cerr << "Usage:\n"
         << "  ./build/sum bench\n"
         << "  ./build/sum check [n]\n"
         << "  ./build/sum chain  n repeat\n"
         << "  ./build/sum twoway n repeat\n";
    return 1;
}
