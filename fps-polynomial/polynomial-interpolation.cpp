/*
<多項式補間>
  [使い方]
    #include "polynomial-interpolation.cpp"
    vector<mint> xs = {0, 1, 2}, ys = {1, 6, 17};
    fps f = polynomial_interpolation(xs, ys); // {1, 2, 3}
    PolynomialInterpolation(xs, ys) も同じ関数。
    ProductTree<mint> tree(xs);
    auto g = polynomial_interpolation(tree, ys); // 積木の構築を再利用

  [各関数の入力と返り値]
    polynomial_interpolation(xs,ys)
      入力: 相異なる評価点 xs（vector<mint>）と、各点での値 ys（vector<mint>）。
        xs と ys は同じ長さ n で、ys[i] が点 xs[i] での値。
      返り値: すべての i で f(xs[i])=ys[i] を満たす、次数 n 未満の多項式 f。
        返す形式は長さ n の fps で、返り値の j 番目が x^j の係数。
        高次の係数が 0 でも長さ n を保つ。n=0 なら空列を返す。
    polynomial_interpolation(tree,ys)
      入力: 評価点から構築した ProductTree<mint> tree と、各点での値 ys。
        tree.xs は相異なり、ys.size()==tree.xs.size() が必要。
      返り値: polynomial_interpolation(tree.xs,ys) と同じ多項式の係数列（fps）。
        与えられた積木を再利用する。
    PolynomialInterpolation(xs,ys)
      入力・返り値: polynomial_interpolation(xs,ys) と同じ。互換用の別名。

  [仕様 / 前提]
    xs.size() == ys.size()。xs は mod 上で相異なること（assert で確認）。
    f(xs[i]) = ys[i] を満たす次数 < n の多項式を、長さ n の係数列で返す。
    n=0 なら空列。係数は体（素数 mod）。

  [計算量 / 方法]
    O(n log^2 n) 時間、O(n log n) 空間。
    P=prod(x-xs[i])、w[i]=ys[i]/P'(xs[i]) とおく。
    葉の w[i] を、左右の値 * 反対側の積木の和で併合する。
*/
#ifndef POLYNOMIAL_INTERPOLATION_CPP
#define POLYNOMIAL_INTERPOLATION_CPP
#include "multipoint-evaluation.cpp"

template <typename mint>
FormalPowerSeries<mint> polynomial_interpolation(
    const ProductTree<mint>& tree, const vector<mint>& ys) {
  using FPS = FormalPowerSeries<mint>;
  assert((int)ys.size() == tree.xsz);
  if (ys.empty()) return {};
  auto derivative_values = tree.evaluate(tree.buf[1].diff());
  vector<FPS> partial(2 * tree.N);
  for (int i = 0; i < tree.xsz; ++i) {
    assert(derivative_values[i] != mint(0));
    partial[tree.N + i] = {ys[i] / derivative_values[i]};
  }
  for (int i = tree.N - 1; i; --i) {
    partial[i] = partial[2 * i] * tree.buf[2 * i + 1]
               + partial[2 * i + 1] * tree.buf[2 * i];
    partial[2 * i].clear();
    partial[2 * i + 1].clear();
  }
  return partial[1].pre(tree.xsz);
}

template <typename mint>
FormalPowerSeries<mint> polynomial_interpolation(const vector<mint>& xs,
                                                const vector<mint>& ys) {
  assert(xs.size() == ys.size());
  return polynomial_interpolation(ProductTree<mint>(xs), ys);
}

template <typename mint>
FormalPowerSeries<mint> PolynomialInterpolation(const vector<mint>& xs,
                                               const vector<mint>& ys) {
  return polynomial_interpolation(xs, ys);
}
#endif
