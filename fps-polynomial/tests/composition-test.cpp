// g++-14 -std=c++17 -O2 composition-test.cpp -o /tmp/fps-composition-test
// /tmp/fps-composition-test [benchmark_size]
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
using namespace std;
#include "../fps-composition.cpp"

template <class Mint>
FormalPowerSeries<Mint> naive_mul(const FormalPowerSeries<Mint>& a,
                                const FormalPowerSeries<Mint>& b, int n) {
  FormalPowerSeries<Mint> c(n);
  for (int i = 0; i < int(a.size()); ++i)
    for (int j = 0; j < int(b.size()) && i + j < n; ++j) c[i + j] += a[i] * b[j];
  return c;
}

template <class Mint>
FormalPowerSeries<Mint> naive_compose(const FormalPowerSeries<Mint>& f,
                                    const FormalPowerSeries<Mint>& g, int n) {
  using F = FormalPowerSeries<Mint>;
  F answer(n), power(n);
  if (n) power[0] = Mint(1);
  for (int i = 0; i < min(n, int(f.size())); ++i) {
    for (int j = 0; j < n; ++j) answer[j] += f[i] * power[j];
    power = naive_mul(power, g, n);
  }
  return answer;
}

template <class Mint>
void check_mod() {
  using F = FormalPowerSeries<Mint>;
  mt19937 rng(947592);
  assert(fps_compose(F{}, F{}, 0).empty());
  assert(fps_compositional_inverse(F{}, 0).empty());
  assert(fps_power_projection(F{}, 0).empty());
  for (int n = 1; n <= 80; ++n) {
    for (int t = 0; t < 6; ++t) {
      F f(n + t % 3), g(n), unit(n);
      for (auto& x : f) x = Mint(rng() % 1000);
      for (int i = 1; i < n; ++i) g[i] = Mint(rng() % 1000);
      if (t == 0) g = F{};
      if (t == 1) f = F{};
      if (t == 2) g.assign(n, Mint(0));
      if (t == 3 && n > 1) g[1] = Mint(0);
      assert(fps_compose(f, g, n) == naive_compose(f, g, n));
      F projection = fps_power_projection(g, n), power(n);
      power[0] = Mint(1);
      for (int i = 0; i < n; ++i) {
        assert(projection[i] == power[n - 1]);
        power = naive_mul(power, g, n);
      }
      g.resize(max(2, n));
      g[1] = Mint(1 + rng() % 1000);
      F inv = fps_compositional_inverse(g, n);
      if (n > 1) unit[1] = Mint(1);
      assert(naive_compose(g, inv, n) == unit);
      assert(naive_compose(inv, g, n) == unit);
    }
  }
  // Explicit precision exceeds the input lengths; all omitted terms are zero.
  F short_f{0, 7, 3};
  F short_inv = fps_compositional_inverse(short_f, 257);
  F unit(257);
  unit[1] = Mint(1);
  assert(naive_compose(short_f, short_inv, 257) == unit);
  F f(311), g(311);
  for (auto& x : f) x = Mint(rng());
  for (int i = 1; i < 311; ++i) g[i] = Mint(rng());
  assert(fps_compose(f, g) == naive_compose(f, g, 311));
  cerr << "composition random checks passed, mod = " << Mint::get_mod() << '\n';
}

int main(int argc, char** argv) {
  if (argc == 1) {
    check_mod<LazyMontgomeryModInt<998244353>>();
    check_mod<LazyMontgomeryModInt<1000000007>>();
    return 0;
  }
  using M = LazyMontgomeryModInt<998244353>;
  using F = FormalPowerSeries<M>;
  const int n = stoi(argv[1]);
  F f(n);
  mt19937 rng(192382);
  for (int i = 1; i < n; ++i) f[i] = M(rng());
  f[1] = M(1);
  auto start = chrono::steady_clock::now();
  F g = fps_compositional_inverse(f);
  auto mid = chrono::steady_clock::now();
  F h = fps_compose(f, g), identity(n);
  identity[1] = M(1);
  assert(h == identity);
  auto end = chrono::steady_clock::now();
  cerr << "N=" << n << ": inverse " << chrono::duration<double>(mid - start).count()
       << "s, composition " << chrono::duration<double>(end - mid).count() << "s\n";
}
