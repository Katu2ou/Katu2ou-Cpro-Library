/*
  多項式 GCD・多点評価・補間の回帰テスト。
  g++-14 -std=c++17 -O2 tests/polynomial-test.cpp -o /tmp/polynomial-test
  /tmp/polynomial-test
  小規模は独立した筆算除算・互除法・Horner 法と比較する。
*/
#define FPS_NO_DEFAULT_ALIASES
#include "../polynomial-gcd.cpp"
#include "../polynomial-interpolation.cpp"
#include <chrono>
#include <random>

template <typename T> using Poly = FormalPowerSeries<T>;

template <typename T>
Poly<T> slow_multiply(const Poly<T>& a, const Poly<T>& b) {
  if (a.empty() || b.empty()) return {};
  Poly<T> result(a.size() + b.size() - 1);
  for (int i = 0; i < (int)a.size(); ++i)
    for (int j = 0; j < (int)b.size(); ++j) result[i + j] += a[i] * b[j];
  result.shrink();
  return result;
}

template <typename T>
Poly<T> slow_remainder(Poly<T> a, Poly<T> b) {
  a.shrink();
  b.shrink();
  assert(!b.empty());
  while (a.size() >= b.size()) {
    T c = a.back() / b.back();
    int shift = (int)a.size() - (int)b.size();
    for (int j = 0; j < (int)b.size(); ++j) a[j + shift] -= c * b[j];
    a.shrink();
  }
  return a;
}

template <typename T>
Poly<T> slow_gcd(Poly<T> a, Poly<T> b) {
  a.shrink();
  b.shrink();
  while (!b.empty()) {
    auto c = slow_remainder(a, b);
    a = b;
    b = c;
  }
  if (!a.empty()) a *= a.back().inverse();
  return a;
}

template <typename T>
T horner(const Poly<T>& a, T x) {
  T result = 0;
  for (int i = (int)a.size() - 1; i >= 0; --i) result = result * x + a[i];
  return result;
}

template <typename T>
void verify_pair(Poly<T> a, Poly<T> b) {
  auto expected = slow_gcd(a, b);
  auto [g, s, t] = polynomial_extgcd(a, b);
  assert(g == expected);
  auto combination = slow_multiply(s, a) + slow_multiply(t, b);
  combination.shrink();
  assert(combination == g);
  a.shrink();
  b.shrink();
  if (!b.empty()) {
    auto [ok, inverse] = polynomial_mod_inverse(a, b);
    assert(ok == (g.size() == 1));
    if (ok) {
      assert(inverse.size() < b.size());
      assert(slow_remainder(slow_multiply(a, inverse), b)
             == slow_remainder(Poly<T>{T(1)}, b));
    }
  }
  if (a.size() < b.size()) swap(a, b);
  auto matrix = half_gcd(a, b);
  auto [c, d] = matrix.apply(a, b);
  if (d.size() > a.size() / 2) {
    std::cerr << "half reduction failed: " << a.size() << ' ' << b.size()
              << " -> " << c.size() << ' ' << d.size() << '\n';
    abort();
  }
  assert(slow_gcd(c, d) == expected);
  auto determinant = matrix.entry[0][0] * matrix.entry[1][1]
                   - matrix.entry[0][1] * matrix.entry[1][0];
  determinant.shrink();
  assert(determinant.size() == 1);
  assert(determinant[0] == T(1) || determinant[0] == T(-1));
}

template <typename T>
void random_tests() {
  std::mt19937_64 rng(7392947);
  auto random_poly = [&](int size) {
    Poly<T> a(size);
    for (auto& x : a) x = T(rng() % T::get_mod());
    return a;
  };
  vector<Poly<T>> special = {{}, {0}, {0, 0}, {1}, {2, 0, 0}, {0, 1},
                             {1, -1}, {0, 0, 0, 1}, {1, 0, 1}};
  for (auto a : special) for (auto b : special) verify_pair(a, b);
  for (int trial = 0; trial < 420; ++trial) {
    auto a = random_poly(rng() % 200), b = random_poly(rng() % 200);
    if (trial % 4 == 0) {
      auto common = random_poly(1 + rng() % 35);
      a = slow_multiply(a, common);
      b = slow_multiply(b, common);
    }
    if (trial % 9 == 0) b = a;
    if (trial % 11 == 0) a = b * T(3);
    if (trial % 7 == 0) a.resize(a.size() + 3);
    if (trial % 5 == 0) b.resize(b.size() + 5);
    verify_pair(a, b);
  }
  // 疎な高次項・次数の急落を含む互除法。
  for (int n : {31, 32, 33, 63, 64, 65, 127, 128, 129, 255}) {
    Poly<T> a(n + 1), b(n);
    a[n] = 1;
    b[n - 1] = 1;
    b[0] = 1;
    verify_pair(a, b);
    verify_pair(a + T(1), a);
  }

  for (int trial = 0; trial < 250; ++trial) {
    int n = rng() % 280, m = rng() % min<uint32_t>(300, T::get_mod());
    auto f = random_poly(n);
    vector<T> xs(m);
    T offset = T(rng() % T::get_mod());
    for (int i = 0; i < m; ++i) xs[i] = offset + T(i);
    shuffle(xs.begin(), xs.end(), rng);
    ProductTree<T> tree{vector<T>(xs)};
    auto values = tree.evaluate(f);
    for (int i = 0; i < m; ++i) assert(values[i] == horner(f, xs[i]));
    auto interpolated = polynomial_interpolation(tree, values);
    assert(interpolated.size() == xs.size());
    if (m) {
      auto expected = slow_remainder(f, tree.buf[1]);
      assert(interpolated == expected.pre(m));
      for (int i = 0; i < m; ++i) assert(horner(interpolated, xs[i]) == values[i]);
    }
    if (m > 1) {
      xs[1] = xs[0];
      auto duplicates = MultipointEvaluation(f, xs);
      for (int i = 0; i < m; ++i) assert(duplicates[i] == horner(f, xs[i]));
    }
  }
  assert(PolyGCD(Poly<T>{}, Poly<T>{}).empty());
  // Library Checker inv_of_polynomials/example_01: nonzero constant modulus.
  auto [constant_ok, constant_inverse] = polynomial_mod_inverse(
      Poly<T>{5, 4, 3, 2, 1}, Poly<T>{7});
  assert(constant_ok && constant_inverse.empty());
  assert(PolynomialInterpolation(vector<T>{}, vector<T>{}).empty());
  std::cerr << "polynomial random tests passed, mod=" << T::get_mod() << '\n';
}

int main() {
  random_tests<LazyMontgomeryModInt<998244353>>();
  random_tests<LazyMontgomeryModInt<1000000007>>();
  random_tests<LazyMontgomeryModInt<17>>();
  using T = LazyMontgomeryModInt<998244353>;
  std::mt19937 rng(7284);
  for (int n : {1024, 4096}) {
    Poly<T> a(n), b(n - 1), h = {1, 4, 2};
    for (auto& x : a) x = T(rng());
    for (auto& x : b) x = T(rng());
    a *= h;
    b *= h;
    auto start = std::chrono::steady_clock::now();
    auto [g, s, t] = polynomial_extgcd(a, b);
    auto relation = s * a + t * b;
    relation.shrink();
    assert(relation == g);
    assert(g == h * T(2).inverse());
    vector<T> xs(n);
    for (int i = 0; i < n; ++i) xs[i] = T(i);
    auto values = MultipointEvaluation(a, xs);
    auto recovered = polynomial_interpolation(xs, values);
    ProductTree<T> tree(xs);
    assert(recovered == (a % tree.buf[1]).pre(n));
    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cerr << "polynomial scale n=" << n << ", seconds=" << seconds << '\n';
  }
  std::cout << "polynomial tests passed\n";
}
