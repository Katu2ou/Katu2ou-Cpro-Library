/*
<FPS の平方根 (Newton 法 + Tonelli-Shanks)>
  #include "fps-sqrt.cpp"
  auto g = fps_sqrt(f, N);  // g*g == f (mod x^N)、N 項を返す

  [各関数の入力と返り値]
    fps_sqrt(f, deg=-1)
      入力: FPS f の係数列と、求める項数 deg。
        deg を省略または -1 にすると f.size() 項を求める。
      返り値: g(x)^2=f(x) mod x^deg を満たす g の係数列（長さ deg の fps）。
        解が複数ある場合はそのうち一つを返す。先頭が 0 の場合に自由に選べる
        高次係数は 0 とする。f=0 mod x^deg なら deg 項すべて 0 を返す。
        平方根が存在しなければ空列。deg=0 の正常な結果も空列。
    mod_sqrt(a,p)
      入力: 整数 a と法 p（ともに int64_t）、0<=a<p、2<p<2^30 の素数 p。
      返り値: r*r=a mod p を満たす 0<=r<p の整数 r（int64_t）。
        存在しなければ -1。複数ある場合、最小の平方根を返す保証はない。

  [計算量・条件]
    fps_sqrt は O(N log N + log^2 p)。法は 2<p<2^30 の素数。
*/

#ifndef FPS_SQRT_CPP
#define FPS_SQRT_CPP
#include "formal-power-series.cpp"

inline int64_t mod_sqrt(const int64_t &a, const int64_t &p) {
  assert(0 <= a && a < p);
  if (a < 2) return a;
  using Mint = ArbitraryLazyMontgomeryModInt<409075245>;
  Mint::set_mod(p);
  if (Mint(a).pow((p - 1) >> 1) != 1) return -1;
  Mint b = 1, one = 1;
  while (b.pow((p - 1) >> 1) == 1) b += one;
  int64_t m = p - 1, e = 0;
  while (m % 2 == 0) m >>= 1, e += 1;
  Mint x = Mint(a).pow((m - 1) >> 1);
  Mint y = Mint(a) * x * x;
  x *= a;
  Mint z = Mint(b).pow(m);
  while (y != 1) {
    int64_t j = 0;
    Mint t = y;
    while (t != one) {
      j += 1;
      t *= t;
    }
    z = z.pow(int64_t(1) << (e - j - 1));
    x *= z;
    z *= z;
    y *= z;
    e = j;
  }
  return x.get();
}

template <typename mint>
FormalPowerSeries<mint> fps_sqrt(const FormalPowerSeries<mint> &f, int deg = -1) {
  if (deg == -1) deg = (int)f.size();
  assert(deg >= 0);
  if (deg == 0) return {};
  if ((int)f.size() == 0) return FormalPowerSeries<mint>(deg, 0);
  if (f[0] == mint(0)) {
    for (int i = 1; i < min(deg, (int)f.size()); i++) {
      if (f[i] != mint(0)) {
        if (i & 1) return {};
        if (deg - i / 2 <= 0) break;
        auto ret = fps_sqrt((f >> i).pre(deg - i), deg - i);
        if (ret.empty()) return {};
        ret = ret << (i / 2);
        if ((int)ret.size() < deg) ret.resize(deg, mint(0));
        return ret;
      }
    }
    return FormalPowerSeries<mint>(deg, 0);
  }

  int64_t sqr = mod_sqrt(f[0].get(), mint::get_mod());
  if (sqr == -1) return {};
  assert(sqr * sqr % mint::get_mod() == f[0].get());
  FormalPowerSeries<mint> ret = {mint(sqr)};
  mint inv2 = mint(2).inverse();
  for (int i = 1; i < deg; i <<= 1) {
    int next = min(deg, i << 1);
    ret = (ret + (f.pre(next) * ret.inv(next)).pre(next)) * inv2;
  }
  return ret.pre(deg);
}

#endif  // FPS_SQRT_CPP
