/*
<FPS に対する汎用 Newton 法>
  #include "fps-newton.cpp"
  [各関数の入力と返り値]
    fps_newton(initial, N, equation)
      入力: 解の初期近似 initial（fps）、求める項数 N（非負整数）、
        方程式と導関数を評価する関数 equation。
        initial は、m=initial.size() として Phi(initial)=0 mod x^m を満たすこと。
        N>0 のとき initial は空にできない。
      返り値: Phi(g)=0 mod x^N を満たす解 g の係数列（長さ N の fps）。
        N=0 なら空列。initial が既に N 項以上なら先頭 N 項を返す。
    equation(g, n)（呼出側が用意する関数）
      入力: 現在の近似解 g（長さ n の fps）と、評価に必要な項数 n。
      返り値: pair<fps,fps>{Phi(g), (dPhi/dg)(g)}。
        どちらも次数 n 未満の係数が必要。省略した高次係数は 0 として扱う。
        second は x による微分ではなく、未知の FPS g に関する導関数。
        その定数項が非零でなければならない。
    fps_pow_unit(f, alpha, N=-1)
      入力: 定数項が 1 の FPS f、mint 型の指数 alpha、求める項数 N。
        N を省略または -1 にすると f.size() 項を求める。
      返り値: exp(alpha*log(f)) の次数 N 未満の係数列（長さ N の fps）。
        N=0 なら空列。alpha=mint(a)/mint(b) で分母が可逆な有理数べきも扱える。

  [更新方法]
  fps_newton(initial, N, equation)
    equation(g, n) は {Phi(g) mod x^n, (dPhi/dg)(g) mod x^n} を返す。
    initial は mod x^initial.size() の解。導関数の定数項は非零が必要。
    g <- g - Phi(g)/(dPhi/dg)(g) で精度を倍増し、N 項を返す。
    Phi は FPS に関する形式的な解析関数を想定する。
    g' を含む微分方程式には、この単純な除算による更新は使えない。

  例: g^2 = 1+x, g(0)=1 を解く:
    fps f = {1, 1};
    auto g = fps_newton(fps{1}, 100, [&](const fps& g, int n) {
      return make_pair((g*g-f).pre(n), g*mint(2));
    });
  M(N) を N 項の多項式乗算の計算量とすると、
  Phi の評価が O(M(N)) なら全体 O(M(N)) (このライブラリでは N log N)。
  initial が正しい解であることは呼出側の責任。
  基本演算の inv/exp は formal-power-series.cpp に専用 Newton 法を実装済み。

  fps_pow_unit(f, alpha, N): f(0)=1 に対する exp(alpha*log(f))。
    alpha=mint(a)/mint(b) とすれば有理数べき。N<=p、b!=0 (mod p)。
    O(N log N)。非負整数べき (先頭が 0 の場合を含む) は f.pow(k,N)。
*/
#ifndef FPS_NEWTON_CPP
#define FPS_NEWTON_CPP
#include "formal-power-series.cpp"
#include <utility>

template <typename Mint, typename Equation>
FormalPowerSeries<Mint> fps_newton(FormalPowerSeries<Mint> initial,
                                   int n, Equation equation) {
  assert(n >= 0);
  if (n == 0) return {};
  assert(!initial.empty());
  if ((int)initial.size() >= n) return initial.pre(n);
  while ((int)initial.size() < n) {
    int next = min(n, int(initial.size()) * 2);
    auto value_derivative = equation(initial.pre(next), next);
    auto value = value_derivative.first.pre(next);
    auto derivative = value_derivative.second.pre(next);
    assert(derivative[0] != Mint(0));
    initial = (initial - (value * derivative.inv(next)).pre(next)).pre(next);
  }
  return initial;
}

template <typename Mint>
FormalPowerSeries<Mint> fps_pow_unit(const FormalPowerSeries<Mint>& f,
                                     Mint alpha, int n = -1) {
  if (n == -1) n = int(f.size());
  assert(n >= 0);
  if (n == 0) return {};
  assert(!f.empty() && f[0] == Mint(1));
  return (f.log(n) * alpha).exp(n);
}
#endif
