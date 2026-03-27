#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

using Clock = chrono::high_resolution_clock;

static inline size_t idx(size_t i, size_t j, size_t n) {
    return i * n + j;   // row-major
}

void init_data(size_t n, vector<double>& A, vector<double>& x) {
    A.resize(n * n);
    x.resize(n);
    for (size_t i = 0; i < n; ++i) {
        x[i] = static_cast<double>(i);
        for (size_t j = 0; j < n; ++j) {
            A[idx(i, j, n)] = static_cast<double>(i + j);
        }
    }
}

// 平凡算法：逐列访问
void matvec_naive(const vector<double>& A, const vector<double>& x, vector<double>& y, size_t n) {
    y.assign(n, 0.0);
    for (size_t col = 0; col < n; ++col) {
        double sum = 0.0;
        for (size_t row = 0; row < n; ++row) {
            sum += A[idx(row, col, n)] * x[row];
        }
        y[col] = sum;
    }
}

// cache 优化算法：逐行访问
void matvec_opt(const vector<double>& A, const vector<double>& x, vector<double>& y, size_t n) {
    y.assign(n, 0.0);
    for (size_t row = 0; row < n; ++row) {
        const double xr = x[row];
        const double* row_ptr = &A[idx(row, 0, n)];
        for (size_t col = 0; col < n; ++col) {
            y[col] += row_ptr[col] * xr;
        }
    }
}

double max_abs_diff(const vector<double>& a, const vector<double>& b) {
    double diff = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        diff = max(diff, fabs(a[i] - b[i]));
    }
    return diff;
}

double checksum(const vector<double>& y) {
    return accumulate(y.begin(), y.end(), 0.0);
}

int default_repeat(size_t n) {
    if (n <= 64) return 4000;
    if (n <= 128) return 1000;
    if (n <= 256) return 300;
    if (n <= 512) return 80;
    if (n <= 1024) return 20;
    if (n <= 2048) return 5;
    return 1;
}

double benchmark_ms(function<void()> fn, int repeat) {
    fn();  // warm-up

    auto start = Clock::now();
    for (int r = 0; r < repeat; ++r) {
        fn();
    }
    auto end = Clock::now();

    chrono::duration<double, milli> elapsed = end - start;
    return elapsed.count() / repeat;
}

void run_check(size_t n) {
    vector<double> A, x, y1, y2;
    init_data(n, A, x);
    matvec_naive(A, x, y1, n);
    matvec_opt(A, x, y2, n);

    cout << fixed << setprecision(6);
    cout << "n=" << n << "\n";
    cout << "checksum_naive=" << checksum(y1) << "\n";
    cout << "checksum_opt  =" << checksum(y2) << "\n";
    cout << "max_abs_diff  =" << max_abs_diff(y1, y2) << "\n";

    size_t print_n = min<size_t>(n, 8);
    cout << "first_" << print_n << "_values_naive: ";
    for (size_t i = 0; i < print_n; ++i) cout << y1[i] << " ";
    cout << "\n";

    cout << "first_" << print_n << "_values_opt  : ";
    for (size_t i = 0; i < print_n; ++i) cout << y2[i] << " ";
    cout << "\n";
}

void run_single(const string& mode, size_t n, int repeat) {
    vector<double> A, x, y;
    init_data(n, A, x);

    double ms = 0.0;
    if (mode == "naive") {
        ms = benchmark_ms([&]() { matvec_naive(A, x, y, n); }, repeat);
    } else if (mode == "opt") {
        ms = benchmark_ms([&]() { matvec_opt(A, x, y, n); }, repeat);
    } else {
        cerr << "Unknown mode: " << mode << "\n";
        exit(1);
    }

    cout << fixed << setprecision(6);
    cout << "mode=" << mode
         << " n=" << n
         << " repeat=" << repeat
         << " avg_ms=" << ms
         << " checksum=" << checksum(y) << "\n";
}

void run_bench() {
    vector<size_t> sizes = {64, 128, 256, 512, 1024, 2048};
    cout << fixed << setprecision(6);
    cout << "n,repeat,naive_ms,opt_ms,speedup,max_abs_diff,checksum\n";

    for (size_t n : sizes) {
        int repeat = default_repeat(n);

        vector<double> A, x, y1, y2;
        init_data(n, A, x);

        double t1 = benchmark_ms([&]() { matvec_naive(A, x, y1, n); }, repeat);
        double t2 = benchmark_ms([&]() { matvec_opt(A, x, y2, n); }, repeat);

        double diff = max_abs_diff(y1, y2);
        double sumv = checksum(y2);

        cout << n << ","
             << repeat << ","
             << t1 << ","
             << t2 << ","
             << (t1 / t2) << ","
             << diff << ","
             << sumv << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1 || string(argv[1]) == "bench") {
        run_bench();
        return 0;
    }

    string mode = argv[1];

    if (mode == "check") {
        size_t n = 8;
        if (argc >= 3) n = static_cast<size_t>(stoull(argv[2]));
        run_check(n);
        return 0;
    }

    if (mode == "naive" || mode == "opt") {
        if (argc < 4) {
            cerr << "Usage: ./build/matvec [naive|opt] n repeat\n";
            return 1;
        }
        size_t n = static_cast<size_t>(stoull(argv[2]));
        int repeat = stoi(argv[3]);
        run_single(mode, n, repeat);
        return 0;
    }

    cerr << "Usage:\n"
         << "  ./build/matvec bench\n"
         << "  ./build/matvec check [n]\n"
         << "  ./build/matvec naive n repeat\n"
         << "  ./build/matvec opt   n repeat\n";
    return 1;
}
