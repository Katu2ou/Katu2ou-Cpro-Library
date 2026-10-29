/*
  g++-14 -std=c++17 -O2 tests/matrix-bsgs-benchmark.cpp -o /tmp/fps-bsgs-benchmark
  /tmp/fps-bsgs-benchmark
  実時間は環境依存。BSGS と独立した全項計算の答えも比較する。
*/
#define FPS_NO_DEFAULT_ALIASES
#include "../polynomial-matrix-product.cpp"
#include <chrono>
#include <iomanip>
using mint = LazyMontgomeryModInt<998244353>;
using fps = FormalPowerSeries<mint>;

int main() {
  using Clock = std::chrono::steady_clock;
  std::cout << std::fixed << std::setprecision(6);
  for (int64_t n : {1000000LL,10000000LL}) {
    auto start = Clock::now();
    auto a = fps_pow_coefficient_bsgs(fps{1,3,1},2*n,n-1)/mint(n);
    auto seconds = std::chrono::duration<double>(Clock::now()-start).count();
    std::cout << "BSGS n=" << n << " answer=" << a << " seconds=" << seconds << '\n';
    start = Clock::now();
    auto f = fps_pow_sparse(fps{1,3,1},2*n,int(n));
    auto b = f.back()/mint(n);
    seconds = std::chrono::duration<double>(Clock::now()-start).count();
    assert(a == b);
    std::cout << "sparse n=" << n << " answer=" << b << " seconds=" << seconds << '\n';
  }
  // C(p-1,n)=(-1)^n (mod p) を使い、O(N) の比較をせず大きな次数も検証。
  for (int64_t n : {100000000LL,998244351LL,998244352LL}) {
    auto start = Clock::now();
    auto value = fps_pow_coefficient_bsgs(fps{1,1},998244352,n);
    assert(value == mint(n&1 ? -1 : 1));
    auto seconds = std::chrono::duration<double>(Clock::now()-start).count();
    std::cout << "binomial n=" << n << " answer=" << value << " seconds=" << seconds << '\n';
  }
}
