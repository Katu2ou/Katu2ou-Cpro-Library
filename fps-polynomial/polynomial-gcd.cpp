/*
<half-GCD / 多項式 gcd・拡張 gcd・mod 逆元>
  [使い方]
    #include "polynomial-gcd.cpp"
    fps d = polynomial_gcd(a, b); // 非零なら最高次係数 1 の gcd
    auto [g, s, t] = polynomial_extgcd(a, b); // s*a + t*b == g
    auto [ok, h] = polynomial_mod_inverse(a, b); // a*h % b == 1
    auto matrix = half_gcd(a, b);
    auto [c, d2] = matrix.apply(a, b);
    // size(a)>=size(b) が前提。size(d2)<=floor(size(a)/2) まで互除法を進める。

  [各関数の入力と返り値]
    polynomial_gcd(a,b)
      入力: 2 つの多項式 a,b の係数列（fps）。零多項式も渡せる。
      返り値: 最大公約多項式の係数列（fps）。非零なら最高次係数を 1 に正規化。
        a=b=0 のときは空列を返す。
    polynomial_extgcd(a,b)
      入力: 2 つの多項式 a,b の係数列（fps）。
      返り値: tuple<fps,fps,fps>{g,s,t}。
        g は最高次係数を 1 にした gcd、s,t は s*a+t*b=g を満たす係数列。
        a=b=0 のときは g,s,t のすべてが空列。
    polynomial_mod_inverse(a,b)
      入力: 逆元を求める多項式 a と、法とする非零多項式 b。
      返り値: pair<bool,fps>{ok,h}。
        逆元があれば ok=true、h は a*h=1 mod b かつ deg(h)<deg(b) の係数列。
        逆元がなければ {false,{}} を返す。
        b が非零の定数多項式なら剰余環は零環なので {true,{}} を返す。
    PolyGCD(a,b), PolyInv(a,b)
      入力・返り値: それぞれ polynomial_gcd, polynomial_mod_inverse と同じ。
    half_gcd(a,b)
      入力: 多項式 a,b の係数列。末尾の 0 を除いた後で a.size()>=b.size() が必要。
      返り値: PolynomialGCDMatrix<mint> 型の 2 行 2 列の多項式行列 M。
        (c,d)^T=M*(a,b)^T としたとき、gcd を保ったまま互除法を途中まで進め、
        d.size()<=floor(a.size()/2) を満たすようにする。行列の成分は M.entry[i][j]。
    PolynomialGCDMatrix<mint>()
      入力: なし。結果: 2 行 2 列の単位行列を作る。
    M.apply(a,b)
      入力: 行列 M を作用させる 2 つの多項式 a,b。
      返り値: pair<fps,fps>{M00*a+M01*b, M10*a+M11*b}。末尾の 0 は除去する。
    M * R
      入力: 同じ係数型の多項式行列 R。返り値: 行列積 M*R。
    M.euclidean_step(q)
      入力: 互除法の 1 ステップで使う商の多項式 q。
      結果: M を [[0,1],[1,-q]]*M に更新する。返り値なし。

  [仕様 / 前提]
    係数は体（素数 mod）。多項式末尾の 0 は自動で除去する。
    零多項式は空列で返す。gcd(0,0)=0、そのとき extgcd は {0,0,0}。
    mod 逆元では b!=0。定数の法にも対応。存在しない場合は {false,{}} を返す。
    逆元が存在する場合、deg(h)<deg(b)。
    PolyGCD(a,b), PolyInv(a,b) は互換用の別名。

  [計算量]
    N=max(a.size(),b.size()) として O(N log^2 N) 時間、O(N log N) 空間。
    高次半分から互除法の変換行列を求め、元の多項式に適用する half-GCD。
    小さい部分問題のみ通常の互除法を使う。

  [参考]
    https://nyaannyaan.github.io/library/fps/polynomial-gcd.hpp.html
    https://scrapbox.io/37zigen-43465887/half-GCD
*/
#ifndef POLYNOMIAL_GCD_CPP
#define POLYNOMIAL_GCD_CPP
#include "formal-power-series.cpp"
#include <tuple>
#include <utility>

template <typename mint>
struct PolynomialGCDMatrix {
  using FPS = FormalPowerSeries<mint>;
  FPS entry[2][2];

  PolynomialGCDMatrix() {
    entry[0][0] = entry[1][1] = FPS{mint(1)};
  }

  pair<FPS, FPS> apply(const FPS& a, const FPS& b) const {
    FPS c = entry[0][0] * a + entry[0][1] * b;
    FPS d = entry[1][0] * a + entry[1][1] * b;
    c.shrink();
    d.shrink();
    return {std::move(c), std::move(d)};
  }

  PolynomialGCDMatrix operator*(const PolynomialGCDMatrix& rhs) const {
    PolynomialGCDMatrix product;
    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 2; ++j) {
        product.entry[i][j].clear();
        for (int k = 0; k < 2; ++k)
          product.entry[i][j] += entry[i][k] * rhs.entry[k][j];
        product.entry[i][j].shrink();
      }
    }
    return product;
  }

  // 左から [[0,1],[1,-q]] を掛ける。
  void euclidean_step(const FPS& q) {
    for (int j = 0; j < 2; ++j) {
      entry[0][j] -= q * entry[1][j];
      entry[0][j].shrink();
      swap(entry[0][j], entry[1][j]);
    }
  }
};

namespace polynomial_gcd_detail {
template <typename mint>
void step(FormalPowerSeries<mint>& a, FormalPowerSeries<mint>& b,
          PolynomialGCDMatrix<mint>& matrix) {
  auto q = a / b;
  auto remainder = a - q * b;
  remainder.shrink();
  matrix.euclidean_step(q);
  a = std::move(b);
  b = std::move(remainder);
}

template <typename mint>
PolynomialGCDMatrix<mint> reduce_half(FormalPowerSeries<mint> a,
                                       FormalPowerSeries<mint> b) {
  const int target = (int)a.size() / 2;
  PolynomialGCDMatrix<mint> matrix;
  if ((int)b.size() <= target) return matrix;
  if (a.size() <= 32) {
    while ((int)b.size() > target) step(a, b, matrix);
    return matrix;
  }

  matrix = reduce_half(a >> target, b >> target);
  tie(a, b) = matrix.apply(a, b);
  if ((int)b.size() <= target) return matrix;
  step(a, b, matrix);
  if ((int)b.size() <= target) return matrix;

  // 既に減った次数に合わせ、残る半分の問題も高次項のみで解く。
  const int shift = 2 * target - ((int)a.size() - 1);
  assert(shift >= 0);
  return reduce_half(a >> shift, b >> shift) * matrix;
}
}  // namespace polynomial_gcd_detail

template <typename mint>
PolynomialGCDMatrix<mint> half_gcd(FormalPowerSeries<mint> a,
                                  FormalPowerSeries<mint> b) {
  a.shrink();
  b.shrink();
  assert(a.size() >= b.size());
  return polynomial_gcd_detail::reduce_half(std::move(a), std::move(b));
}

template <typename mint>
tuple<FormalPowerSeries<mint>, FormalPowerSeries<mint>, FormalPowerSeries<mint>>
polynomial_extgcd(FormalPowerSeries<mint> a, FormalPowerSeries<mint> b) {
  using FPS = FormalPowerSeries<mint>;
  a.shrink();
  b.shrink();
  if (a.empty() && b.empty()) return {FPS{}, FPS{}, FPS{}};
  PolynomialGCDMatrix<mint> total;
  if (a.size() < b.size()) {
    swap(a, b);
    for (int j = 0; j < 2; ++j) swap(total.entry[0][j], total.entry[1][j]);
  }
  while (!b.empty()) {
    auto block = polynomial_gcd_detail::reduce_half(a, b);
    tie(a, b) = block.apply(a, b);
    if (!b.empty()) polynomial_gcd_detail::step(a, b, block);
    total = block * total;
  }
  const mint scale = a.back().inverse();
  a *= scale;
  total.entry[0][0] *= scale;
  total.entry[0][1] *= scale;
  return {std::move(a), std::move(total.entry[0][0]), std::move(total.entry[0][1])};
}

template <typename mint>
FormalPowerSeries<mint> polynomial_gcd(const FormalPowerSeries<mint>& a,
                                      const FormalPowerSeries<mint>& b) {
  return get<0>(polynomial_extgcd(a, b));
}

template <typename mint>
pair<bool, FormalPowerSeries<mint>> polynomial_mod_inverse(
    const FormalPowerSeries<mint>& a, FormalPowerSeries<mint> modulus) {
  modulus.shrink();
  assert(!modulus.empty());
  if (modulus.size() == 1) return {true, {}};
  auto [g, s, t] = polynomial_extgcd(a, modulus);
  if (g.size() != 1) return {false, {}};
  return {true, s % modulus};
}

template <typename mint>
FormalPowerSeries<mint> PolyGCD(const FormalPowerSeries<mint>& a,
                               const FormalPowerSeries<mint>& b) {
  return polynomial_gcd(a, b);
}

template <typename mint>
pair<bool, FormalPowerSeries<mint>> PolyInv(const FormalPowerSeries<mint>& a,
                                          const FormalPowerSeries<mint>& b) {
  return polynomial_mod_inverse(a, b);
}
#endif
