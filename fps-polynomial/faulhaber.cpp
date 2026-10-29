/*
<Bernoulli 数・Faulhaber の公式・べき乗和>
  [使い方]
    #include "faulhaber.cpp"
    auto B = bernoulli_numbers<mint>(k);   // B[0],...,B[k]。B[1] = -1/2
    fps S = faulhaber_polynomial<mint>(k); // S(N) = sum_{i=0}^N i^k
    fps T = power_sum_polynomial<mint>(k);// T(N) = sum_{i=0}^{N-1} i^k
    auto v = power_sums(mint(N), k);       // v[j] = sum_{i=0}^{N-1} i^j, 0<=j<=k
    mint s = polynomial_sum(f, mint(N));  // sum_{i=0}^{N-1} f(i)
  [各関数の入力と返り値]
    bernoulli_numbers<mint>(k)
      入力: 求める Bernoulli 数の最大の添字 k（非負整数）。
      返り値: vector<mint>{B[0],...,B[k]}（長さ k+1）。
        B[0]=1, B[1]=-1/2 という規約の Bernoulli 数を返す。
    faulhaber_polynomial<mint>(k)
      入力: 各整数を何乗するかを表す非負整数 k。
      返り値: S(t)=sum_{i=0}^t i^k を満たす多項式 S の係数列（長さ k+2 の fps）。
        総和の上端 t を含む。特定の N に対する値は S.eval(mint(N)) で得る。
    power_sum_polynomial<mint>(k)
      入力: 各整数を何乗するかを表す非負整数 k。
      返り値: T(t)=sum_{i=0}^{t-1} i^k を満たす多項式 T の係数列（長さ k+2 の fps）。
        総和の上端 t を含まず、定数項は 0。
    power_sums(N,k)
      入力: 総和の上端 N（mint）と、列挙する指数の最大値 k（非負整数）。
      返り値: 長さ k+1 の vector<mint>。j 番目は sum_{i=0}^{N-1} i^j の値。
        N を固定し、指数 j=0,1,...,k ごとの総和を列挙する。係数列ではない。
    polynomial_sum(f,N)
      入力: 多項式 f の係数列（fps）と、総和の上端 N（mint）。
      返り値: sum_{i=0}^{N-1} f(i) の値を mint で返す。空の f なら 0。
    上の総和は非負整数の上端に対する意味。整数 N は mint(N) に変換して渡す。
    0^0=1 とするため、k=0 では S(t)=t+1, T(t)=t。
  [条件・返り値]
    法 p は素数、0 <= k、k+1 < p。0^0 は 1 とする。
    polynomial_sum は f.size() < p。N は整数を mint に変換して渡す。
    べき乗和の多項式は長さ k+2、power_sums は長さ k+1。
  [計算量]
    すべて O(k log k) 時間、O(k) 空間（polynomial_sum では k=deg(f)）。
    畳み込みの長さは formal-power-series.cpp の上限以内であること。
  [原理]
    x/(exp(x)-1) = sum B[j] x^j/j! を FPS 逆元で求める。
    sum_{i=0}^{N-1} exp(ix) = (exp(Nx)-1)/(exp(x)-1) を用いる。
*/
#ifndef FAULHABER_CPP
#define FAULHABER_CPP
#include "formal-power-series.cpp"

namespace fps_sum_detail {
template <typename mint>
struct Factorials {
  vector<mint> fact, ifact;
  explicit Factorials(int n) : fact(n + 1, mint(1)), ifact(n + 1, mint(1)) {
    assert(n >= 0 && (uint64_t)n < mint::get_mod());
    for (int i = 1; i <= n; ++i) fact[i] = fact[i - 1] * mint(i);
    ifact[n] = fact[n].inverse();
    for (int i = n; i > 0; --i) ifact[i - 1] = ifact[i] * mint(i);
  }
};

template <typename mint>
FormalPowerSeries<mint> bernoulli_egf(int k, const Factorials<mint> &table) {
  FormalPowerSeries<mint> denominator(k + 1);
  for (int i = 0; i <= k; ++i) denominator[i] = table.ifact[i + 1];
  return denominator.inv(k + 1);
}
}  // namespace fps_sum_detail

template <typename mint>
vector<mint> bernoulli_numbers(int k) {
  assert(k >= 0 && (uint64_t)k + 1 < mint::get_mod());
  fps_sum_detail::Factorials<mint> table(k + 1);
  auto result = fps_sum_detail::bernoulli_egf(k, table);
  for (int i = 0; i <= k; ++i) result[i] *= table.fact[i];
  return vector<mint>(result.begin(), result.end());
}

template <typename mint>
FormalPowerSeries<mint> power_sum_polynomial(int k) {
  assert(k >= 0 && (uint64_t)k + 1 < mint::get_mod());
  fps_sum_detail::Factorials<mint> table(k + 1);
  auto b = fps_sum_detail::bernoulli_egf(k, table);
  FormalPowerSeries<mint> result(k + 2);
  for (int j = 1; j <= k + 1; ++j)
    result[j] = table.fact[k] * table.ifact[j] * b[k + 1 - j];
  return result;
}

template <typename mint>
FormalPowerSeries<mint> faulhaber_polynomial(int k) {
  auto result = power_sum_polynomial<mint>(k);
  result[k] += mint(1);
  return result;
}

template <typename mint>
vector<mint> power_sums(mint n, int k) {
  assert(k >= 0 && (uint64_t)k + 1 < mint::get_mod());
  fps_sum_detail::Factorials<mint> table(k + 1);
  auto b = fps_sum_detail::bernoulli_egf(k, table);
  FormalPowerSeries<mint> numerator(k + 1);
  mint power = n;
  for (int i = 0; i <= k; ++i, power *= n) numerator[i] = power * table.ifact[i + 1];
  auto result = (numerator * b).pre(k + 1);
  for (int i = 0; i <= k; ++i) result[i] *= table.fact[i];
  return vector<mint>(result.begin(), result.end());
}

template <typename mint>
mint polynomial_sum(const FormalPowerSeries<mint> &f, mint n) {
  if (f.empty()) return mint(0);
  auto sums = power_sums(n, (int)f.size() - 1);
  mint answer = 0;
  for (int i = 0; i < (int)f.size(); ++i) answer += f[i] * sums[i];
  return answer;
}
#endif
