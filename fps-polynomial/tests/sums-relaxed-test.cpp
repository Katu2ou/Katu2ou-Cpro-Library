// g++-14 -std=c++17 -O2 tests/sums-relaxed-test.cpp -o /tmp/sums-relaxed-test
#include "../sampling-point-shift.cpp"
#include "../faulhaber.cpp"
#include "../prefix-sum-polynomial.cpp"
#include "../relaxed-multiplication.cpp"
#include <chrono>
#include <random>

template <typename M>
void check_sampling() {
  using F = FormalPowerSeries<M>;
  std::mt19937 rng(223431);
  for (int trial = 0; trial < 500; ++trial) {
    int n = rng() % 65, m = rng() % 180;
    F f(n);
    for (auto &c : f) c = M(rng());
    std::vector<M> y(n);
    for (int i = 0; i < n; ++i) y[i] = f.eval(M(i));
    M a = trial % 3 == 0 ? M((int)(rng() % 140) - 70) : M(rng());
    auto shifted = sampling_point_shift(y, a, m);
    assert((int)shifted.size() == m);
    for (int i = 0; i < m; ++i) assert(shifted[i] == f.eval(a + M(i)));
    assert(interpolate_consecutive(y, a) == f.eval(a));
  }
  // NTT/CRT 経路、既知点との重複、法をまたぐシフト。
  if (M::get_mod() > 1000) {
    F f(400);
    for (auto &c : f) c = M(rng());
    std::vector<M> y(f.size());
    for (int i = 0; i < (int)y.size(); ++i) y[i] = f.eval(M(i));
    for (M a : {M(17), M(-200), M(99991)}) {
      auto shifted = sampling_point_shift(y, a, 650);
      for (int i = 0; i < (int)shifted.size(); ++i) assert(shifted[i] == f.eval(a + M(i)));
    }
  }
}

template <typename M>
void check_sums() {
  using F = FormalPowerSeries<M>;
  auto bernoulli = bernoulli_numbers<M>(10);
  assert(bernoulli[0] == M(1));
  assert(bernoulli[1] == -M(2).inverse());
  assert(bernoulli[2] == M(6).inverse());
  assert(bernoulli[4] == -M(30).inverse());
  for (int k = 0; k <= 25; ++k) {
    F exclusive = power_sum_polynomial<M>(k), inclusive = faulhaber_polynomial<M>(k);
    M sum = 0;
    for (int n = 0; n <= 45; ++n) {
      assert(exclusive.eval(M(n)) == sum);
      sum += M(n).pow(k);
      assert(inclusive.eval(M(n)) == sum);
    }
  }
  for (int n = 0; n <= 45; ++n) {
    auto sums = power_sums(M(n), 30);
    for (int k = 0; k <= 30; ++k) {
      M sum = 0;
      for (int i = 0; i < n; ++i) sum += M(i).pow(k);
      assert(sums[k] == sum);
    }
  }
  std::mt19937 rng(112379);
  for (int trial = 0; trial < 100; ++trial) {
    F f(rng() % 60);
    for (auto &c : f) c = M(rng());
    F exclusive = prefix_sum_polynomial_exclusive(f), inclusive = prefix_sum_polynomial(f);
    M sum = 0;
    for (int n = 0; n <= 25; ++n) {
      assert(exclusive.eval(M(n)) == sum);
      assert(polynomial_sum(f, M(n)) == sum);
      sum += f.eval(M(n));
      assert(inclusive.eval(M(n)) == sum);
    }
    for (int i = 0; i < 10; ++i) {
      M x = M(rng());
      assert(exclusive.eval(x + M(1)) - exclusive.eval(x) == f.eval(x));
      assert(inclusive.eval(x) - inclusive.eval(x - M(1)) == f.eval(x));
    }
    assert(inclusive.eval(M(-1)) == M(0));
  }
  if (M::get_mod() > 1000) {
    F f(600);
    for (auto &c : f) c = M(rng());
    auto exclusive = prefix_sum_polynomial_exclusive(f);
    for (int i = 0; i < 50; ++i) {
      M x = M(rng());
      assert(exclusive.eval(x + M(1)) - exclusive.eval(x) == f.eval(x));
    }
  }
}

template <typename M>
void check_relaxed() {
  using F = FormalPowerSeries<M>;
  std::mt19937 rng(619273);
  for (int trial = 0; trial < 20; ++trial) {
    RelaxedMultiplication<M> mul;
    F a, b;
    M previous = 0;
    for (int i = 0; i < 300; ++i) {
      a.push_back(M(rng()) + previous);
      b.push_back(M(rng()) - previous);
      M expected = 0;
      for (int j = 0; j <= i; ++j) expected += a[j] * b[i - j];
      previous = mul.append(a[i], b[i]);
      assert(previous == expected);
      assert(mul.size() == i + 1);
    }
    mul.clear();
    assert(mul.size() == 0);
    assert(mul.append(M(7), M(9)) == M(63));
  }
  if (M::get_mod() > 1000) {
    auto start = std::chrono::steady_clock::now();
    constexpr int n = 16384;
    F a(n), b(n), result(n);
    for (int i = 0; i < n; ++i) a[i] = M(rng()), b[i] = M(rng());
    RelaxedMultiplication<M> mul;
    for (int i = 0; i < n; ++i) result[i] = mul.append(a[i], b[i]);
    assert(result == (a * b).pre(n));
    auto seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cerr << "relaxed n=" << n << ", mod=" << M::get_mod() << ": " << seconds << " s\n";
  }
}

template <typename M>
void run() {
  check_sampling<M>();
  check_sums<M>();
  check_relaxed<M>();
}

int main() {
  run<LazyMontgomeryModInt<998244353>>();
  run<LazyMontgomeryModInt<1000000007>>();
  run<LazyMontgomeryModInt<97>>();
  std::cout << "sums / sampling shift / relaxed multiplication: OK\n";
}
