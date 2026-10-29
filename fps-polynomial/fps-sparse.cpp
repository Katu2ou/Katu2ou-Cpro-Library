/*
<疎な FPS の基本演算>
  #include "fps-sparse.cpp"
  SparseFPS<mint> s = {{0, 1}, {3, 2}, {10, 5}}; // 1+2x^3+5x^10
  auto s2 = fps_to_sparse(f);                   // 通常の FPS から変換
  auto a = fps_mul_sparse(f, s, N);             // f*s mod x^N
  auto b = fps_div_sparse(f, s, N);             // f/s mod x^N、s(0)!=0
  auto c = fps_inv_sparse(s, N);                // 1/s、s(0)!=0
  auto d = fps_log_sparse(s, N);                // log(s)、s(0)=1
  auto e = fps_exp_sparse(s, N);                // exp(s)、s(0)=0
  auto h = fps_pow_sparse(s, k, N);             // s^k、int64_t k>=0
  auto q = fps_pow_unit_sparse(s, mint(1)/mint(2), N); // 定数項1の有理数べき
  auto r = fps_sqrt_sparse(s, N);               // r*r=s mod x^N、なければ空列
  inv/log/exp/pow/pow_unit/sqrt は通常の FPS を渡すことも可能。

  [各関数の入力と返り値]
    以下の f は通常の係数列 fps、s は SparseFPS<mint>、N は非負の項数。
    fps_to_sparse 以外の返り値は通常の係数列 fps（疎な形式ではない）。
    fps_to_sparse(f)
      入力: 通常の係数列 f。
      返り値: 非零係数だけを (次数,係数) の組にした SparseFPS<mint>。
        次数の昇順で並び、零多項式なら空列。
    fps_mul_sparse(f,s,N)
      入力: 通常の係数列 f、掛ける疎な多項式 s、必要な項数 N。
      返り値: f(x)*s(x) の次数 N 未満の係数を持つ長さ N の fps。
    fps_div_sparse(f,s,N)
      入力: 通常の係数列 f、定数項が非零の疎な除数 s、必要な項数 N。
      返り値: s(x)*g(x)=f(x) mod x^N を満たす長さ N の fps g。
        多項式除算の商ではなく、FPS として割った結果を返す。
    fps_inv_sparse(s,N)
      入力: 定数項が非零の疎な FPS s と必要な項数 N。
      返り値: s(x)*g(x)=1 mod x^N を満たす長さ N の逆元の係数列 g。
    fps_log_sparse(s,N)
      入力: 定数項が 1 の疎な FPS s と必要な項数 N。
      返り値: log(s(x)) の次数 N 未満の係数列（長さ N）。定数項は 0。
    fps_exp_sparse(s,N)
      入力: 定数項が 0 の疎な FPS s と必要な項数 N。
      返り値: exp(s(x)) の次数 N 未満の係数列（長さ N）。N>0 なら定数項は 1。
    fps_pow_sparse(s,k,N)
      入力: 疎な FPS s、非負の int64_t の指数 k、必要な項数 N。
      返り値: s(x)^k の次数 N 未満の係数列（長さ N）。s の先頭が 0 でもよい。
        k=0 なら 1 mod x^N の係数列を返す。
    fps_pow_unit_sparse(s,alpha,N)
      入力: 定数項が 1 の疎な FPS s、mint 型の指数 alpha、必要な項数 N。
      返り値: exp(alpha*log(s(x))) の次数 N 未満の係数列（長さ N）。
        alpha=mint(a)/mint(b) で、分母が可逆な有理数べきも指定できる。
    fps_sqrt_sparse(s,N)
      入力: 疎な FPS s と必要な項数 N。
      返り値: g(x)^2=s(x) mod x^N を満たす長さ N の係数列 g を一つ返す。
        解がなければ空列。s=0 mod x^N なら N 項すべて 0 を返す。
    通常の FPS を受け取る inv/log/exp/pow/pow_unit/sqrt の同名関数
      入力: 上記の s を通常の係数列 f に置き換えたもの。他の引数は同じ。
        この形式だけは N を省略または -1 にでき、その場合は f.size() 項を求める。
      返り値: f を疎な形式に変換して上記の関数を実行した結果と同じ。

  入力 s は「非負次数の昇順・重複なし・非零係数」の列。
  fps_to_sparse 以外で作った s も上記条件を守ること (assert で検査)。
  N は項数。N を受け取る関数は正常時に N 項を返す。N=0 では空列。
  p は奇素数、log/exp/pow/pow_unit/sqrt は N<=p。
  sqrt の高次の自由な係数は 0 とする。0^0=1。
  計算量 O(N(K+1)+K)、領域 O(N+K)、K は疎な入力の非零項数。
  pow の定数項の累乗は追加 O(log k)、sqrt は追加 O(log^2 p)。
  通常の FPS 入力を変換する際は追加 O(f.size())。
  漸化式: h=exp(f) なら h'=f'h、h=f^a なら fh'=af'h。
*/
#ifndef FPS_SPARSE_CPP
#define FPS_SPARSE_CPP
#include "fps-sqrt.cpp"
#include <utility>

template <typename Mint>
using SparseFPS = vector<pair<int, Mint>>;

template <typename Mint>
SparseFPS<Mint> fps_to_sparse(const FormalPowerSeries<Mint>& f) {
  SparseFPS<Mint> ret;
  for (int i = 0; i < (int)f.size(); ++i)
    if (f[i] != Mint(0)) ret.emplace_back(i, f[i]);
  return ret;
}

namespace fps_sparse_detail {
template <typename Mint>
void validate(const SparseFPS<Mint>& f, int n) {
  assert(n >= 0);
  int last = -1;
  for (auto [d, c] : f) {
    assert(d > last && c != Mint(0));
    last = d;
  }
}
template <typename Mint>
Mint constant(const SparseFPS<Mint>& f) {
  return !f.empty() && f[0].first == 0 ? f[0].second : Mint(0);
}
template <typename Mint>
vector<Mint> inverses(int n) {
  assert(uint64_t(n) <= uint64_t(Mint::get_mod()));
  vector<Mint> inv(n);
  if (n > 1) inv[1] = 1;
  for (int i = 2; i < n; ++i)
    inv[i] = -inv[Mint::get_mod() % i] * Mint(Mint::get_mod() / i);
  return inv;
}
}  // namespace fps_sparse_detail

template <typename Mint>
FormalPowerSeries<Mint> fps_mul_sparse(const FormalPowerSeries<Mint>& f,
                                      const SparseFPS<Mint>& g, int n) {
  fps_sparse_detail::validate(g, n);
  FormalPowerSeries<Mint> ret(n);
  for (auto [d, c] : g) {
    if (d >= n) break;
    for (int i = 0; i < min(int(f.size()), n-d); ++i) ret[i+d] += f[i]*c;
  }
  return ret;
}

template <typename Mint>
FormalPowerSeries<Mint> fps_div_sparse(const FormalPowerSeries<Mint>& f,
                                      const SparseFPS<Mint>& g, int n) {
  fps_sparse_detail::validate(g, n);
  if (n == 0) return {};
  Mint c = fps_sparse_detail::constant(g);
  assert(c != Mint(0));
  Mint ic = c.inverse();
  auto ret = f.pre(n);
  for (int i = 0; i < n; ++i) {
    for (auto [d, v] : g) {
      if (d > i) break;
      if (d) ret[i] -= v * ret[i-d];
    }
    ret[i] *= ic;
  }
  return ret;
}

template <typename Mint>
FormalPowerSeries<Mint> fps_inv_sparse(const SparseFPS<Mint>& f, int n) {
  return fps_div_sparse(FormalPowerSeries<Mint>{1}, f, n);
}

template <typename Mint>
FormalPowerSeries<Mint> fps_log_sparse(const SparseFPS<Mint>& f, int n) {
  fps_sparse_detail::validate(f, n);
  assert(uint64_t(n) <= uint64_t(Mint::get_mod()));
  if (n == 0) return {};
  assert(fps_sparse_detail::constant(f) == Mint(1));
  FormalPowerSeries<Mint> df(max(0, n-1));
  for (auto [d, c] : f) {
    if (d >= n) break;
    if (d) df[d-1] = Mint(d) * c;
  }
  return fps_div_sparse(df, f, n-1).integral();
}

template <typename Mint>
FormalPowerSeries<Mint> fps_exp_sparse(const SparseFPS<Mint>& f, int n) {
  fps_sparse_detail::validate(f, n);
  auto inv = fps_sparse_detail::inverses<Mint>(n);
  if (n == 0) return {};
  assert(fps_sparse_detail::constant(f) == Mint(0));
  FormalPowerSeries<Mint> ret(n);
  ret[0] = 1;
  for (int i = 1; i < n; ++i) {
    for (auto [d, c] : f) {
      if (d > i) break;
      ret[i] += Mint(d) * c * ret[i-d];
    }
    ret[i] *= inv[i];
  }
  return ret;
}

template <typename Mint>
FormalPowerSeries<Mint> fps_pow_unit_sparse(const SparseFPS<Mint>& f,
                                           Mint alpha, int n) {
  fps_sparse_detail::validate(f, n);
  auto inv = fps_sparse_detail::inverses<Mint>(n);
  if (n == 0) return {};
  assert(fps_sparse_detail::constant(f) == Mint(1));
  FormalPowerSeries<Mint> ret(n);
  ret[0] = 1;
  for (int i = 1; i < n; ++i) {
    for (auto [d, c] : f) {
      if (d > i) break;
      if (d) ret[i] += ((alpha + Mint(1))*Mint(d) - Mint(i)) * c * ret[i-d];
    }
    ret[i] *= inv[i];
  }
  return ret;
}

template <typename Mint>
FormalPowerSeries<Mint> fps_pow_sparse(const SparseFPS<Mint>& f, int64_t k, int n) {
  fps_sparse_detail::validate(f, n);
  assert(k >= 0 && uint64_t(n) <= uint64_t(Mint::get_mod()));
  if (n == 0) return {};
  if (k == 0) return FormalPowerSeries<Mint>{1}.pre(n);
  if (f.empty()) return FormalPowerSeries<Mint>(n);
  int d = f[0].first;
  if (d && k > (n-1)/d) return FormalPowerSeries<Mint>(n);
  int shift = int(d*k), need = n-shift;
  Mint c = f[0].second, ic = c.inverse();
  SparseFPS<Mint> normalized;
  for (auto [j, v] : f) {
    if (j-d >= need) break;
    normalized.emplace_back(j-d, v*ic);
  }
  auto ret = fps_pow_unit_sparse(normalized, Mint(k), need) * c.pow(k);
  return (ret << shift).pre(n);
}

template <typename Mint>
FormalPowerSeries<Mint> fps_sqrt_sparse(const SparseFPS<Mint>& f, int n) {
  fps_sparse_detail::validate(f, n);
  assert(uint64_t(n) <= uint64_t(Mint::get_mod()));
  if (n == 0) return {};
  if (f.empty() || f[0].first >= n) return FormalPowerSeries<Mint>(n);
  int d = f[0].first;
  if (d & 1) return {};
  int64_t root = mod_sqrt(f[0].second.get(), Mint::get_mod());
  if (root == -1) return {};
  Mint ic = f[0].second.inverse();
  SparseFPS<Mint> normalized;
  for (auto [j, v] : f) {
    if (j >= n) break;
    normalized.emplace_back(j-d, v*ic);
  }
  auto ret = fps_pow_unit_sparse(normalized, Mint(1)/Mint(2), n-d) * Mint(root);
  return (ret << (d/2)).pre(n);
}

template <typename Mint>
FormalPowerSeries<Mint> fps_inv_sparse(const FormalPowerSeries<Mint>& f, int n = -1) {
  return fps_inv_sparse(fps_to_sparse(f), n == -1 ? int(f.size()) : n);
}
template <typename Mint>
FormalPowerSeries<Mint> fps_log_sparse(const FormalPowerSeries<Mint>& f, int n = -1) {
  return fps_log_sparse(fps_to_sparse(f), n == -1 ? int(f.size()) : n);
}
template <typename Mint>
FormalPowerSeries<Mint> fps_exp_sparse(const FormalPowerSeries<Mint>& f, int n = -1) {
  return fps_exp_sparse(fps_to_sparse(f), n == -1 ? int(f.size()) : n);
}
template <typename Mint>
FormalPowerSeries<Mint> fps_pow_sparse(const FormalPowerSeries<Mint>& f,
                                     int64_t k, int n = -1) {
  return fps_pow_sparse(fps_to_sparse(f), k, n == -1 ? int(f.size()) : n);
}
template <typename Mint>
FormalPowerSeries<Mint> fps_pow_unit_sparse(const FormalPowerSeries<Mint>& f,
                                          Mint alpha, int n = -1) {
  return fps_pow_unit_sparse(fps_to_sparse(f), alpha, n == -1 ? int(f.size()) : n);
}
template <typename Mint>
FormalPowerSeries<Mint> fps_sqrt_sparse(const FormalPowerSeries<Mint>& f, int n = -1) {
  return fps_sqrt_sparse(fps_to_sparse(f), n == -1 ? int(f.size()) : n);
}
#endif
