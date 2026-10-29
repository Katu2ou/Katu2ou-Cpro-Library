/*
<多項式行列の連続積 / P-recursive 数列の第 N 項 / BSGS>
  [使い方 / C++17]
    #define FPS_NO_DEFAULT_ALIASES
    #include "polynomial-matrix-product.cpp"
    using mint = LazyMontgomeryModInt<998244353>;
    using fps = FormalPowerSeries<mint>;

    // a_(i+1)=(i+1)*a_i, a_0=1: 階乗を遠くの 1 項だけ計算する。
    vector<fps> p = {{-1, -1}, {1}};
    auto value = p_recursive_nth(p, vector<mint>{1}, 10000000);
    if (value) cout << *value << '\n';

    // A(x) = [[x+1,1],[1,0]]。右側の A(0) から順に列ベクトルに作用。
    BSGSPolynomialMatrix<mint> A = {{{1,1},{1}}, {{1},{}}};
    auto product = polynomial_matrix_product_bsgs(A, 10000000);

    // [x^(N-1)] (1+3x+x^2)^(2N) / N。全 N 項は作らない。
    int64_t N = 10000000;
    mint answer = fps_pow_coefficient_bsgs(fps{1,3,1}, 2*N, N-1) / mint(N);

  [各型・関数の入力と返り値]
    BSGSMatrix<mint>
      vector<vector<mint>>。添字は [行][列]。
    BSGSPolynomialMatrix<mint>
      vector<vector<fps>>。A[i][j] は (i,j) 成分の多項式の係数列。
      係数は昇べき順。空の fps は零多項式。末尾の 0 も許す。
    polynomial_matrix_product_bsgs(A,n,start=mint(0))
      入力: r*r の正方多項式行列 A、非負 int64_t の積の個数 n、
        最初の評価点 start (mint)。評価点は start,start+1,...。
      返り値: A(start+n-1)*...*A(start+1)*A(start) を BSGSMatrix<mint> で返す。
        n=0 は単位行列。0*0 行列なら空列。特異行列・零行列も許す。
        行列の積の順序に注意。start は添字の整数上限ではなく体の元。
    polynomial_product_bsgs(f,n,start=mint(0))
      入力: 多項式 f (fps)、非負 int64_t の個数 n、最初の評価点 start。
      返り値: prod(i=0..n-1) f(start+i) を mint で返す。n=0 は 1。
        例: polynomial_product_bsgs(fps{0,1},n,mint(1)) は n!。
    p_recursive_nth(p,initial,n)
      入力: p={p_0(x),...,p_r(x)} (vector<fps>, r>=1)、
        initial={a_0,...,a_(r-1)} (vector<mint>, 必ず r 個)、非負 int64_t の n。
        漸化式は sum(j=0..r) p_j(i)*a_(i+j)=0 (i=0,1,...)。
      返り値: optional<mint> に格納した a_n。n<r は初期値をそのまま返す。
        必要な範囲 i=0..n-r のどこかで p_r(i)=0 なら nullopt。
        nullopt は「この漸化式では順方向に決定できない」の意味で、解の不存在とは限らない。
    fps_pow_coefficient_bsgs(f,k,n)
      入力: 次数の小さい多項式 f (fps)、非負 int64_t の指数 k と次数 n。
      返り値: [x^n] f(x)^k を mint で返す。長さ n+1 の係数列は作らない。
        k=0 は定数 1。f=0、先頭の零係数にも対応。0^0=1。
        この関数は 0<=n<p を要求する。

  [条件・計算量]
    係数は 2<p<2^30 の素数体。本ライブラリの固定/動的 Montgomery 型に対応。
    r は行列サイズ、d は成分多項式の最大次数、M(s) は s 項の畳み込みの計算量。
    d>=1 では B をおおむね sqrt(n/d) の 2 冪に取り、S=dB+n/B とする。
    BSGS 経路の時間 O(r^2*M(d)*log(d+2) + r^2*M(S) + r^3*S)、
      空間 O(r^2*S+d*log(d+2))。
    r,d 固定なら時間 O(sqrt(n)*log(n+2))、空間 O(sqrt(n))。
    小さい n / B=1 は O(n*(r^2*(d+1)+r^3)) の逐次計算。
    定数行列は O(r^3*log(n+1)) の行列累乗。
    n>=p は周期 A(x+p)=A(x) により 1 周期の積の累乗と端数に分解する。
      この場合、上記 n を p に置き換えた計算量に O(r^3*log(n/p+1)) を加える。
    補間点数 dB+1<p となるよう B を制限する。内部畳み込みは FPS の対応長以内が必要。
    上記は体上の演算回数。メモリ・時間の上限を保証するものではない。
    p_recursive_nth は r 次元の行列と、分母のスカラー積を計算する。
    fps_pow_coefficient_bsgs は先頭零を除いた f の次数を r とし、
      初期値の計算に追加 O(r^2+log k)。非零項が少なくても次数が大きい場合には不向き。

  [原理]
    F_s(x)=A(start+B*x+s-1)*...*A(start+B*x) の評価値を保持する。
    F_(2s)(x)=F_s(x+s/B)*F_s(x) を評価点シフトで構築し、s を倍々に増やす。
    F_B(0),F_B(1),... が各ブロックの積。末尾 B 未満の項は直接掛ける。
    参考: Bostan, Gaudry, Schost, Linear Recurrences with Polynomial Coefficients (2007)
    https://mathexp.eu/bostan/publications/BoGaSc07.pdf

  [検証]
    tests/run-tests.sh: ランダム比較・境界・法をまたぐ区間・動的 mod・分母の零。
    tests/matrix-bsgs-benchmark.cpp: N=10^7 の疎な累乗版との比較、大きな二項係数。
    tests/bsgs-verification/RESULTS.txt: Library Checker の結果と再現手順。
*/
#ifndef POLYNOMIAL_MATRIX_PRODUCT_CPP
#define POLYNOMIAL_MATRIX_PRODUCT_CPP
#include "sampling-point-shift.cpp"
#include "multipoint-evaluation.cpp"
#include "fps-sparse.cpp"
#include <optional>

template <typename Mint>
using BSGSMatrix = vector<vector<Mint>>;
template <typename Mint>
using BSGSPolynomialMatrix = vector<vector<FormalPowerSeries<Mint>>>;

namespace matrix_bsgs_detail {
template <typename Mint>
BSGSMatrix<Mint> identity(int r) {
  BSGSMatrix<Mint> a(r, vector<Mint>(r));
  for (int i = 0; i < r; ++i) a[i][i] = 1;
  return a;
}

template <typename Mint>
BSGSMatrix<Mint> multiply(const BSGSMatrix<Mint>& a, const BSGSMatrix<Mint>& b) {
  const int r = (int)a.size();
  BSGSMatrix<Mint> c(r, vector<Mint>(r));
  for (int i = 0; i < r; ++i)
    for (int k = 0; k < r; ++k) {
      if (a[i][k] == Mint(0)) continue;
      for (int j = 0; j < r; ++j) c[i][j] += a[i][k] * b[k][j];
    }
  return c;
}

template <typename Mint>
BSGSMatrix<Mint> power(BSGSMatrix<Mint> a, int64_t n) {
  auto out = identity<Mint>((int)a.size());
  while (n) {
    if (n & 1) out = multiply(a, out);
    n >>= 1;
    if (n) a = multiply(a, a);
  }
  return out;
}

template <typename Mint>
BSGSMatrix<Mint> evaluate(const BSGSPolynomialMatrix<Mint>& a, Mint x) {
  const int r = (int)a.size();
  BSGSMatrix<Mint> out(r, vector<Mint>(r));
  for (int i = 0; i < r; ++i)
    for (int j = 0; j < r; ++j) out[i][j] = a[i][j].eval(x);
  return out;
}

// n<=p。sampling_point_shift の入力点数だけを p 未満に保つ。
template <typename Mint>
BSGSMatrix<Mint> interval(const BSGSPolynomialMatrix<Mint>& a, int64_t n,
                          Mint start, int d) {
  const int r = (int)a.size();
  auto out = identity<Mint>(r);
  if (!n) return out;
  if (!d) return power(evaluate(a, start), n);
  int64_t b = 1;
  while ((__int128)d * b * b < n && (__int128)d * (2*b) + 1 < Mint::get_mod()) b *= 2;
  if (n <= 64 || b == 1) {
    for (int64_t i = 0; i < n; ++i, start += Mint(1))
      out = multiply(evaluate(a, start), out);
    return out;
  }

  // 各成分の F_s(0),...,F_s(ds) を、成分ごとの連続配列に格納。
  assert((int64_t)r*r <= numeric_limits<int>::max());
  vector<vector<Mint>> values(r*r);
  vector<Mint> xs(d+1);
  for (int i = 0; i <= d; ++i) xs[i] = start + Mint(b)*Mint(i);
  if (d <= 32) {
    for (int i = 0; i < r; ++i) for (int j = 0; j < r; ++j) {
      values[i*r+j].resize(d+1);
      for (int k = 0; k <= d; ++k) values[i*r+j][k] = a[i][j].eval(xs[k]);
    }
  } else {
    ProductTree<Mint> tree(xs);
    for (int i = 0; i < r; ++i) for (int j = 0; j < r; ++j)
      values[i*r+j] = tree.evaluate(a[i][j]);
  }
  const Mint inverse_b = Mint(b).inverse();
  for (int64_t s = 1; s < b; s *= 2) {
    const int old = (int)values[0].size(), count = 2*old-1;
    vector<vector<Mint>> left(r*r), right(r*r), next(r*r, vector<Mint>(count));
    for (int k = 0; k < r*r; ++k) {
      auto tail = sampling_point_shift(values[k], Mint(old), old-1);
      left[k] = values[k];
      left[k].insert(left[k].end(), tail.begin(), tail.end());
      right[k] = sampling_point_shift(values[k], Mint(s)*inverse_b, count);
    }
    for (int i = 0; i < r; ++i) for (int k = 0; k < r; ++k) for (int j = 0; j < r; ++j)
      for (int t = 0; t < count; ++t)
        next[i*r+j][t] += right[i*r+k][t] * left[k*r+j][t];
    values.swap(next);
  }
  const int blocks = int(n/b);
  if (blocks > (int)values[0].size()) {
    for (auto& v : values) {
      const int old = (int)v.size();
      auto tail = sampling_point_shift(v, Mint(old), blocks-old);
      v.insert(v.end(), tail.begin(), tail.end());
    }
  }
  BSGSMatrix<Mint> block(r, vector<Mint>(r));
  for (int t = 0; t < blocks; ++t) {
    for (int i = 0; i < r; ++i) for (int j = 0; j < r; ++j)
      block[i][j] = values[i*r+j][t];
    out = multiply(block, out);
  }
  start += Mint(int64_t(blocks)*b);
  for (int64_t i = int64_t(blocks)*b; i < n; ++i, start += Mint(1))
    out = multiply(evaluate(a, start), out);
  return out;
}
}  // namespace matrix_bsgs_detail

template <typename Mint>
BSGSMatrix<Mint> polynomial_matrix_product_bsgs(const BSGSPolynomialMatrix<Mint>& input,
                                               int64_t n, Mint start = Mint(0)) {
  using namespace matrix_bsgs_detail;
  assert(n >= 0 && input.size() <= size_t(numeric_limits<int>::max()));
  const int r = (int)input.size();
  for (const auto& row : input) assert(row.size() == input.size());
  if (!r || !n) return matrix_bsgs_detail::identity<Mint>(r);
  auto a = input;
  int d = 0;
  for (auto& row : a) for (auto& f : row) {
    f.shrink();
    if (!f.empty()) {
      assert(f.size() <= size_t(numeric_limits<int>::max()));
      d = max(d, (int)f.size()-1);
    }
  }
  if (!d) return power(evaluate(a, start), n);
  const int64_t p = Mint::get_mod();
  if (n <= p) return interval(a, n, start, d);
  auto cycle = interval(a, p, start, d);
  return multiply(interval(a, n%p, start, d), power(cycle, n/p));
}

template <typename Mint>
Mint polynomial_product_bsgs(const FormalPowerSeries<Mint>& f, int64_t n,
                             Mint start = Mint(0)) {
  return polynomial_matrix_product_bsgs(BSGSPolynomialMatrix<Mint>{{f}}, n, start)[0][0];
}

template <typename Mint>
optional<Mint> p_recursive_nth(const vector<FormalPowerSeries<Mint>>& p,
                                const vector<Mint>& initial, int64_t n) {
  assert(n >= 0 && p.size() >= 2 && initial.size() == p.size()-1);
  if (uint64_t(n) < initial.size()) return initial[size_t(n)];
  assert(initial.size() <= size_t(numeric_limits<int>::max()));
  const int r = (int)initial.size();
  const int64_t steps = n-r+1;
  Mint denominator = polynomial_product_bsgs(p[r], steps);
  if (denominator == Mint(0)) return nullopt;
  BSGSPolynomialMatrix<Mint> a(r, vector<FormalPowerSeries<Mint>>(r));
  for (int j = 0; j+1 < r; ++j) a[j][j+1] = p[r];
  for (int j = 0; j < r; ++j) a[r-1][j] = -p[j];
  auto product = polynomial_matrix_product_bsgs(a, steps);
  Mint answer = 0;
  for (int j = 0; j < r; ++j) answer += product[r-1][j]*initial[j];
  return answer / denominator;
}

template <typename Mint>
Mint fps_pow_coefficient_bsgs(const FormalPowerSeries<Mint>& f, int64_t k, int64_t n) {
  using FPS = FormalPowerSeries<Mint>;
  assert(k >= 0 && n >= 0 && uint64_t(n) < Mint::get_mod());
  if (!k) return Mint(n == 0);
  int valuation = 0;
  while (valuation < (int)f.size() && f[valuation] == Mint(0)) ++valuation;
  if (valuation == (int)f.size() || (valuation && k > n/valuation)) return Mint(0);
  n -= int64_t(valuation)*k;
  FPS g = (f >> valuation).pre(int(min<int64_t>(f.size()-valuation, n+1)));
  g.shrink();
  const int r = (int)g.size()-1;
  if (!r) return n == 0 ? g[0].pow(k) : Mint(0);
  auto initial = fps_pow_sparse(g, k, int(min<int64_t>(r, n+1)));
  if (n < r) return initial[int(n)];
  vector<FPS> p(r+1);
  for (int j = 0; j < r; ++j)
    p[j] = {g[r-j]*(Mint(r)-(Mint(k)+Mint(1))*Mint(r-j)), g[r-j]};
  p[r] = {g[0]*Mint(r), g[0]};
  auto answer = p_recursive_nth(p, static_cast<const vector<Mint>&>(initial), n);
  assert(answer.has_value());
  return *answer;
}
#endif
