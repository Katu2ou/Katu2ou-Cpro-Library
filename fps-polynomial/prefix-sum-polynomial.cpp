/*
<Prefix Sum Polynomial: 多項式の離散積分>
  [使い方]
    #include "prefix-sum-polynomial.cpp"
    fps F = prefix_sum_polynomial(f);           // F(N) = sum_{i=0}^N f(i)
    fps G = prefix_sum_polynomial_exclusive(f); // G(N) = sum_{i=0}^{N-1} f(i)
    // 例: f={0,1} (f(x)=x) なら F={0,1/2,1/2}, G={0,-1/2,1/2}。
  [各関数の入力と返り値]
    prefix_sum_polynomial(f)
      入力: 多項式 f(x) の係数列（fps）。f[i] は x^i の係数。
      返り値: 非負整数 N に対して F(N)=sum_{i=0}^N f(i) を満たす多項式 F の
        係数列（fps）。総和の上端 N を含む。
        f が n 項なら返り値は n+1 項。特定の N の値は F.eval(mint(N)) で求める。
    prefix_sum_polynomial_exclusive(f)
      入力: 多項式 f(x) の係数列（fps）。
      返り値: 非負整数 N に対して G(N)=sum_{i=0}^{N-1} f(i) を満たす多項式 G の
        係数列（fps）。総和の上端 N を含まず、G(0)=0。
        f が n 項なら返り値は n+1 項。
    どちらも空の入力には空列を返す。
    入力・返り値はどちらも多項式の係数列であり、係数の単純な累積和を取る関数ではない。
  [条件・返り値]
    法 p は素数、f.size() < p。返り値は通常の単項式基底の係数列。
    非空入力の返り値の長さは f.size()+1。空入力には空列を返す。
    F(x)-F(x-1)=f(x), F(-1)=0。G(x+1)-G(x)=f(x), G(0)=0。
  [計算量]
    d=deg(f) として O(d log d) 時間、O(d) 空間。
    畳み込みの長さは formal-power-series.cpp の上限以内であること。
  [原理]
    Faulhaber の公式を各係数に適用し、f[k]*k! と B[j]/j! を畳み込む。
*/
#ifndef PREFIX_SUM_POLYNOMIAL_CPP
#define PREFIX_SUM_POLYNOMIAL_CPP
#include "faulhaber.cpp"

template <typename mint>
FormalPowerSeries<mint> prefix_sum_polynomial_exclusive(const FormalPowerSeries<mint> &f) {
  using FPS = FormalPowerSeries<mint>;
  const int n = (int)f.size();
  assert((uint64_t)n < mint::get_mod());
  if (n == 0) return {};
  fps_sum_detail::Factorials<mint> table(n);
  FPS a(n);
  for (int i = 0; i < n; ++i) a[n - 1 - i] = f[i] * table.fact[i];
  auto b = fps_sum_detail::bernoulli_egf(n - 1, table);
  auto c = a * b;
  FPS result(n + 1);
  for (int j = 1; j <= n; ++j) result[j] = c[n - j] * table.ifact[j];
  return result;
}

template <typename mint>
FormalPowerSeries<mint> prefix_sum_polynomial(const FormalPowerSeries<mint> &f) {
  return prefix_sum_polynomial_exclusive(f) + f;
}
#endif
