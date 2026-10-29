#define KATU2OU_FPS_NO_DEFAULT_ALIASES
/*
<FPS 拡張ライブラリ 全機能 / C++17>
  [使い方]
    #define KATU2OU_FPS_NO_DEFAULT_ALIASES
    #include "fps-all.cpp"
    using mint = LazyMontgomeryModInt<998244353>;
    using fps = FormalPowerSeries<mint>;
    int main() {
      fps f = {0, 1, 1};
      auto g = fps_compositional_inverse(f, 16);
      auto x = fps_compose(f, g, 16); // {0,1,0,...}
    }
    デフォルトの mint / fps は mod 1000000007。上の define/using を省略しても使える。
    必要な機能の .cpp だけを include してもよい（依存先は自動で読み込む）。
    貼り付け運用では依存先を先に貼り、ローカル .cpp の include 行を取り除く。

  [機能一覧: 関数名 / ファイル / 計算量]
    f.inv(N), f.log(N), f.exp(N), f.pow(k,N)
      formal-power-series.cpp / O(N log N)、整数 pow は追加 O(log k)
    fps_newton(initial,N,equation), fps_pow_unit(f,alpha,N)
      fps-newton.cpp / 評価が O(N log N) なら Newton 全体も O(N log N)
    fps_sqrt(f,N)
      fps-sqrt.cpp / O(N log N + log^2 p)
    fps_mul_sparse, fps_div_sparse, fps_inv_sparse, fps_log_sparse,
    fps_exp_sparse, fps_pow_sparse, fps_pow_unit_sparse, fps_sqrt_sparse
      fps-sparse.cpp / O(N(K+1)+K)、K=非零項数（定数の pow/sqrt は別途）
    fps_compose(f,g,N), fps_compositional_inverse(f,N), fps_power_projection(f,N)
      fps-composition.cpp / O(N log^2 N)
    half_gcd(a,b), polynomial_gcd(a,b), polynomial_extgcd(a,b),
    polynomial_mod_inverse(a,b)
      polynomial-gcd.cpp / O(N log^2 N)
    MultipointEvaluation(f,xs), ProductTree<mint>(xs)
      multipoint-evaluation.cpp / O(N log^2 N) (次数と点数が同程度の場合)
    polynomial_interpolation(xs,ys)
      polynomial-interpolation.cpp / O(N log^2 N)
    interpolate_consecutive(ys,a), sampling_point_shift(ys,a,M)
      sampling-point-shift.cpp / O(N), O((N+M) log(N+M))
    bernoulli_numbers<mint>(k), faulhaber_polynomial<mint>(k),
    power_sum_polynomial<mint>(k), power_sums(mint(N),k), polynomial_sum(f,mint(N))
      faulhaber.cpp / O(k log k)
    prefix_sum_polynomial(f), prefix_sum_polynomial_exclusive(f)
      prefix-sum-polynomial.cpp / O(N log N)
    RelaxedMultiplication<mint>::append(a_i,b_i)
      relaxed-multiplication.cpp / N 回合計 O(N log^2 N)

  [各関数の入力と返り値の早見表]
    fps は通常の係数列、SparseFPS<mint> は (次数,非零係数) の列。
    以下の「N 項の係数列」は、次数 0 以上 N 未満を並べた長さ N の fps を指す。
    省略できる引数・入力条件・失敗時の扱いの詳細は各ファイル先頭を参照。

    f.inv(N): 入力は定数項が非零の f と項数 N。
      返り値は 1/f の N 項の係数列。
    f.log(N): 入力は定数項が 1 の f と項数 N。
      返り値は log(f) の N 項の係数列。
    f.exp(N): 入力は定数項が 0 の f と項数 N。
      返り値は exp(f) の N 項の係数列。
    f.pow(k,N): 入力は f、非負整数の指数 k、項数 N。
      返り値は f^k の N 項の係数列。
    fps_newton(initial,N,equation): 入力は初期近似、項数、方程式と導関数を返す関数。
      返り値は方程式を mod x^N で満たす解の N 項の係数列。
    fps_pow_unit(f,alpha,N): 入力は定数項 1 の f、mint 型の指数 alpha、項数 N。
      返り値は exp(alpha*log(f)) の N 項の係数列。
    fps_sqrt(f,N): 入力は f と項数 N。
      返り値は g*g=f mod x^N を満たす N 項の係数列 g。存在しなければ空列。
    mod_sqrt(a,p): 入力は整数 a と素数 p。
      返り値は r*r=a mod p を満たす整数 r。存在しなければ -1。

    fps_to_sparse(f): 入力は通常の係数列 f。
      返り値は非零項を次数の昇順に並べた SparseFPS<mint>。
    fps_mul_sparse(f,s,N): 入力は通常の f、疎な s、項数 N。
      返り値は f*s の N 項の係数列。
    fps_div_sparse(f,s,N): 入力は通常の f、定数項が非零の疎な s、項数 N。
      返り値は f/s を FPS として計算した N 項の係数列。
    fps_inv_sparse(s,N): 入力は疎な s と項数 N。返り値は 1/s の N 項の係数列。
    fps_log_sparse(s,N): 入力は疎な s と項数 N。返り値は log(s) の N 項の係数列。
    fps_exp_sparse(s,N): 入力は疎な s と項数 N。返り値は exp(s) の N 項の係数列。
    fps_pow_sparse(s,k,N): 入力は疎な s、非負整数 k、項数 N。
      返り値は s^k の N 項の係数列。
    fps_pow_unit_sparse(s,alpha,N): 入力は定数項 1 の疎な s、mint 型の指数、項数 N。
      返り値は exp(alpha*log(s)) の N 項の係数列。
    fps_sqrt_sparse(s,N): 入力は疎な s と項数 N。
      返り値は g*g=s mod x^N を満たす N 項の係数列 g。存在しなければ空列。

    fps_compose(f,g,N): 入力は外側の f、代入する g、項数 N。
      返り値は f(g(x)) の N 項の係数列。
    fps_compositional_inverse(f,N): 入力は逆関数を求める f と項数 N。
      返り値は f(g(x))=g(f(x))=x mod x^N を満たす N 項の係数列 g。
    fps_power_projection(f,N): 入力は f と項数 N。
      返り値は f^0,f^1,...,f^(N-1) の x^(N-1) の係数を順に並べた長さ N の fps。
    half_gcd(a,b): 入力は 2 つの多項式 a,b。
      返り値は互除法を途中まで進める 2 行 2 列の多項式行列。apply(a,b) で作用させる。
    polynomial_gcd(a,b): 入力は 2 つの多項式。返り値は最高次係数 1 の gcd の係数列。
    polynomial_extgcd(a,b): 入力は 2 つの多項式。
      返り値は {g,s,t} の tuple。g は gcd、s*a+t*b=g。
    polynomial_mod_inverse(a,b): 入力は逆元を求める a と、法とする多項式 b。
      返り値は {true,h} の pair（a*h=1 mod b）。逆元がなければ {false,{}}。

    MultipointEvaluation(f,xs): 入力は多項式の係数列 f と評価点の列 xs。
      返り値は xs と同じ順序の評価値の列 vector<mint>。
    ProductTree<mint>(xs): 評価点の列を受け取って積木を構築する。
      tree.evaluate(f) は多項式 f を受け取り、保存した各点での値を vector<mint> で返す。
    polynomial_interpolation(xs,ys): 入力は評価点 xs と各点での値 ys。
      返り値はそれらを満たす多項式の係数列 fps（長さ xs.size()）。
    polynomial_interpolation(tree,ys): 入力は構築済みの積木と評価値の列。
      返り値は保存した評価点と ys を満たす多項式の係数列 fps。
    interpolate_consecutive(ys,a): 入力は f(0),...,f(n-1) の値の列と評価点 a。
      返り値は f(a) を表す mint。
    sampling_point_shift(ys,a,M): 入力は f(0),...,f(n-1)、先頭の点 a、個数 M。
      返り値は f(a),...,f(a+M-1) の値を並べた長さ M の vector<mint>。

    bernoulli_numbers<mint>(k): 入力は最大添字 k。
      返り値は B[0],...,B[k] を並べた vector<mint>。
    faulhaber_polynomial<mint>(k): 入力は指数 k。
      返り値は S(t)=sum_{i=0}^t i^k を満たす多項式の係数列 fps（長さ k+2）。
    power_sum_polynomial<mint>(k): 入力は指数 k。
      返り値は T(t)=sum_{i=0}^{t-1} i^k を満たす多項式の係数列 fps（長さ k+2）。
    power_sums(mint(N),k): 入力は総和の上端 N と最大指数 k。
      返り値は sum_{i=0}^{N-1} i^j を j=0,...,k について並べた vector<mint>。
    polynomial_sum(f,mint(N)): 入力は多項式の係数列 f と総和の上端 N。
      返り値は sum_{i=0}^{N-1} f(i) を表す mint。
    prefix_sum_polynomial(f): 入力は多項式の係数列 f。
      返り値は F(t)=sum_{i=0}^t f(i) を満たす多項式の係数列 fps。
    prefix_sum_polynomial_exclusive(f): 入力は多項式の係数列 f。
      返り値は G(t)=sum_{i=0}^{t-1} f(i) を満たす多項式の係数列 fps。

    RelaxedMultiplication<mint> mul: 入力係数がまだない畳み込み器を作る。
    mul.append(a_i,b_i): 入力は次の係数の組。返り値は積の x^i の係数 c_i（mint）。
    mul.size(): 入力なし。追加済みの係数の組数（int）を返す。
    mul.clear(): 入力なし。保存した係数と結果を消去する。返り値なし。

  [共通の約束]
    FPS を次数で打ち切る関数の N は「項数」で、計算は mod x^N。
    既定値 -1 は通常入力の長さ。総和を求める関数の N は総和の上端を表す。
    係数は昇べき順 {定数項,x の係数,x^2 の係数,...}。
    法は 2<p<2^30 の素数。微積分・階乗を使う関数は次数と法の条件に注意。
    f/g は多項式の商。FPS の除算は (f*g.inv(N)).pre(N)。
    合成は g[0]=0、合成逆関数は f[0]=0,f[1]!=0。
    補間では評価点が相異なること。多点評価では重複可。
    Prefix Sum / Faulhaber は inclusive と half-open を別関数で提供。
    返り値・空列・失敗時の仕様など、詳細は各ファイル先頭を参照。

  [検証]
    tests/run-tests.sh で愚直解との比較・恒等式・境界ケースを再検証できる。
    Library Checker の提出結果・対応関数・再現手順は tests/library-checker/VERIFY.md。
    参照記事: https://maspypy.com/多項式・形式的べき級数-高速に計算できるもの
*/
#ifndef KATU2OU_FPS_ALL_CPP
#define KATU2OU_FPS_ALL_CPP
/*
<FPS ライブラリ / C++17>
  [使い方]
    #include "formal-power-series.cpp"
    fps f = {1, 2, 3};
    auto g = f.inv(100);               // mod x^100 で逆元
    auto h = f.pow(1000000000000LL, 100);
    // デフォルトは mod 1000000007。別の型を使う例:
    // #define KATU2OU_FPS_NO_DEFAULT_ALIASES
    // #include "formal-power-series.cpp"
    // using mint = LazyMontgomeryModInt<998244353>;
    // using fps = FormalPowerSeries<mint>;
    // 拡張全体は fps-all.cpp、個別の機能は各 .cpp を include する。

  [入力・返り値の共通形式]
    fps = FormalPowerSeries<mint> は係数列を持つ型。
    f[i] が x^i の係数で、fps{1,2,3} は 1+2x+3x^2 を表す。
    以下の f,g は fps、c,a は mint。係数がない次数は 0 として扱う。
    inv/log/exp/pow の deg は返す項数。省略または -1 なら f.size()。
    これらは次数 deg 未満の係数を長さ deg の fps で返す。deg=0 は空列。

  [各関数・演算子の入力と返り値]
    fps(), fps(n), fps{a0,a1,...}
      入力: なし、非負の項数 n、または係数の列。
      結果: それぞれ空の FPS、n 項すべて 0 の FPS、指定した係数の FPS を作る。
    f.resize(n)
      入力: 変更後の非負の項数 n。
      結果: f を n 項に変更する。延長部分は 0、短縮時は高次項を削除。返り値なし。
    f.size(), f.empty()
      入力: なし。返り値: 現在の項数、または係数列が空かどうかの bool。
      係数がすべて 0 でも、項数が正なら empty() は false。
    f + g, f - g, f + c, f - c, -f
      入力: 加減算する多項式 g または定数 c（-f は追加の入力なし）。
      返り値: 和・差・符号反転した fps。c は定数項だけに加減する。
    f * g, f * c
      入力: 掛ける多項式 g または係数 c。
      返り値: 多項式の積、または全係数を c 倍した fps。自動で次数制限はしない。
    f / g
      入力: 零多項式でない除数 g。
      返り値: f=q*g+r, deg(r)<deg(g) を満たす多項式の商 q を fps で返す。
      FPS の除算 f/g mod x^N が必要なら (f*g.inv(N)).pre(N) とする。
    f % g
      入力: 零多項式でない除数 g。
      返り値: 多項式の剰余 r。末尾の 0 を除去し、零多項式なら空列を返す。
    f += g, f -= g, f *= g, f /= g, f %= g, f += c, f -= c, f *= c
      入力: 上記と同じ演算対象。結果: f 自身を更新し、更新後の f への参照を返す。
    f.shrink()
      入力: なし。結果: f の末尾の 0 をすべて削除する。返り値なし。
    f.pre(sz)
      入力: 非負の項数 sz。
      返り値: 先頭 sz 項を持つ新しい fps。不足分は 0 埋めする。f 自身は変えない。
    f.rev()
      入力: なし。返り値: 現在の係数列を逆順に並べた fps。長さは変わらない。
    f.dot(g)
      入力: 係数列 g。
      返り値: 各成分が f[i]*g[i] の fps。長さは min(f.size(),g.size())。
      係数ごとの積であり、多項式の積や、総和を取る内積ではない。
    f << k
      入力: 非負整数 k。返り値: 先頭に k 個の 0 を付けた、x^k*f(x) の係数列。
    f >> k
      入力: 非負整数 k。返り値: 低次の k 項を取り除いた係数列。
      x^k で多項式除算した商に対応する。k>=f.size() なら空列。
    f.diff()
      入力: なし（f の係数列を使う）。
      返り値: 微分 f'(x) の係数列。長さは max(0,f.size()-1)。
    f.integral()
      入力: なし（f の係数列を使う）。
      返り値: 定数項を 0 とした不定積分の係数列。長さは f.size()+1。
    f.eval(a)
      入力: 評価する点 a。
      返り値: 多項式に a を代入した値 f(a) を mint で返す。
    f.inv(deg=-1)
      入力: 必要な項数 deg。f の定数項は非零。
      返り値: f*g=1 mod x^deg を満たす乗法逆元 g の係数列。
    f.log(deg=-1)
      入力: 必要な項数 deg。f の定数項は 1。
      返り値: 形式的対数 log(f) の係数列。返り値の定数項は 0。
    f.exp(deg=-1)
      入力: 必要な項数 deg。f の定数項は 0（空列も可）。
      返り値: 形式的指数関数 exp(f) の係数列。deg>0 なら定数項は 1。
    f.pow(k,deg=-1)
      入力: 非負の int64_t の指数 k と必要な項数 deg。
      返り値: f(x)^k の係数列。先頭が 0 でもよい。k=0 では 1 mod x^deg。

  [係数型 mint の主な入力と返り値]
    mint(a): 整数 a を法 p で正規化した係数を作る。
    a.get(): 入力なし。a を表す 0 以上 p 未満の整数を返す。
    mint::get_mod(): 入力なし。現在の法 p を整数で返す。
    a.pow(k): 非負整数 k を受け取り、a^k mod p を mint で返す。
    a.inverse(): 入力なし。非零の a に対する乗法逆元を mint で返す。
    動的法の型の set_mod(p): 法 p を設定する。返り値なし。
      法を変更する前に作った係数は、そのまま再利用しないこと。

  [計算時間]
      - 乗算・inv・log・exp: O(N log N)、pow: O(N log N + log k)
      - 加減算・微積分・1 点評価: O(N)
      - 多項式除算・剰余: O(N log N)

  [備考]
      - 係数体の法 p は 2<p<2^30 の素数。log/exp/pow/積分は必要な次数 < p。
      - NTT 対応の固定素数では直接 NTT、それ以外は 3 素数 CRT を使う。
      - CRT 畳み込みの変換長は最大 2^24。直接 NTT は法ごとの上限。
      - .ntt(), .intt(), .ntt_doubling(), ntt_pr() は未対応で、呼ぶと終了する。
        通常の FPS 演算からこれらを呼び出す必要はない。
      - int * mint は不可能! mint * int の順で

  [検証]
      - tests/run-tests.sh でローカル検証を実行できる。

*/

#ifndef KATU2OU_FORMAL_POWER_SERIES_CPP
#define KATU2OU_FORMAL_POWER_SERIES_CPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <vector>
using namespace std;

template <uint32_t mod>
struct LazyMontgomeryModInt {
  using mint = LazyMontgomeryModInt;
  using i32 = int32_t;
  using u32 = uint32_t;
  using u64 = uint64_t;

  static constexpr u32 get_r() {
    u32 ret = mod;
    for (i32 i = 0; i < 4; ++i) ret *= 2 - mod * ret;
    return ret;
  }

  static constexpr u32 r = get_r();
  static constexpr u32 n2 = -u64(mod) % mod;
  static_assert(mod < (1 << 30), "invalid, mod >= 2 ^ 30");
  static_assert((mod & 1) == 1, "invalid, mod % 2 == 0");
  static_assert(r * mod == 1, "this code has bugs.");

  u32 a;

  constexpr LazyMontgomeryModInt() : a(0) {}
  constexpr LazyMontgomeryModInt(const int64_t &b)
      : a(reduce(u64(b % mod + mod) * n2)){};

  static constexpr u32 reduce(const u64 &b) {
    return (b + u64(u32(b) * u32(-r)) * mod) >> 32;
  }

  constexpr mint &operator+=(const mint &b) {
    if (i32(a += b.a - 2 * mod) < 0) a += 2 * mod;
    return *this;
  }

  constexpr mint &operator-=(const mint &b) {
    if (i32(a -= b.a) < 0) a += 2 * mod;
    return *this;
  }

  constexpr mint &operator*=(const mint &b) {
    a = reduce(u64(a) * b.a);
    return *this;
  }

  constexpr mint &operator/=(const mint &b) {
    *this *= b.inverse();
    return *this;
  }

  constexpr mint operator+(const mint &b) const { return mint(*this) += b; }
  constexpr mint operator-(const mint &b) const { return mint(*this) -= b; }
  constexpr mint operator*(const mint &b) const { return mint(*this) *= b; }
  constexpr mint operator/(const mint &b) const { return mint(*this) /= b; }
  constexpr bool operator==(const mint &b) const {
    return (a >= mod ? a - mod : a) == (b.a >= mod ? b.a - mod : b.a);
  }
  constexpr bool operator!=(const mint &b) const {
    return (a >= mod ? a - mod : a) != (b.a >= mod ? b.a - mod : b.a);
  }
  constexpr mint operator-() const { return mint() - mint(*this); }
  constexpr mint operator+() const { return mint(*this); }

  constexpr mint pow(u64 n) const {
    mint ret(1), mul(*this);
    while (n > 0) {
      if (n & 1) ret *= mul;
      mul *= mul;
      n >>= 1;
    }
    return ret;
  }

  constexpr mint inverse() const {
    int x = get(), y = mod, u = 1, v = 0, t = 0, tmp = 0;
    while (y > 0) {
      t = x / y;
      x -= t * y, u -= t * v;
      tmp = x, x = y, y = tmp;
      tmp = u, u = v, v = tmp;
    }
    return mint{u};
  }

  friend ostream &operator<<(ostream &os, const mint &b) {
    return os << b.get();
  }

  friend istream &operator>>(istream &is, mint &b) {
    int64_t t;
    is >> t;
    b = LazyMontgomeryModInt<mod>(t);
    return (is);
  }

  constexpr u32 get() const {
    u32 ret = reduce(a);
    return ret >= mod ? ret - mod : ret;
  }

  static constexpr u32 get_mod() { return mod; }
};

template <typename mint>
struct NTT {
  static constexpr uint32_t get_pr() {
    uint32_t _mod = mint::get_mod();
    using u64 = uint64_t;
    u64 ds[32] = {};
    int idx = 0;
    u64 m = _mod - 1;
    for (u64 i = 2; i * i <= m; ++i) {
      if (m % i == 0) {
        ds[idx++] = i;
        while (m % i == 0) m /= i;
      }
    }
    if (m != 1) ds[idx++] = m;

    uint32_t _pr = 2;
    while (1) {
      int flg = 1;
      for (int i = 0; i < idx; ++i) {
        u64 a = _pr, b = (_mod - 1) / ds[i], r = 1;
        while (b) {
          if (b & 1) r = r * a % _mod;
          a = a * a % _mod;
          b >>= 1;
        }
        if (r == 1) {
          flg = 0;
          break;
        }
      }
      if (flg == 1) break;
      ++_pr;
    }
    return _pr;
  };

  static constexpr uint32_t mod = mint::get_mod();
  static constexpr uint32_t pr = get_pr();
  static constexpr int level = __builtin_ctzll(mod - 1);
  mint dw[level], dy[level];

  void setwy(int k) {
    mint w[level], y[level];
    w[k - 1] = mint(pr).pow((mod - 1) / (1 << k));
    y[k - 1] = w[k - 1].inverse();
    for (int i = k - 2; i > 0; --i)
      w[i] = w[i + 1] * w[i + 1], y[i] = y[i + 1] * y[i + 1];
    dw[1] = w[1], dy[1] = y[1], dw[2] = w[2], dy[2] = y[2];
    for (int i = 3; i < k; ++i) {
      dw[i] = dw[i - 1] * y[i - 2] * w[i];
      dy[i] = dy[i - 1] * w[i - 2] * y[i];
    }
  }

  NTT() { setwy(level); }

  void fft4(vector<mint> &a, int k) {
    if ((int)a.size() <= 1) return;
    if (k == 1) {
      mint a1 = a[1];
      a[1] = a[0] - a[1];
      a[0] = a[0] + a1;
      return;
    }
    if (k & 1) {
      int v = 1 << (k - 1);
      for (int j = 0; j < v; ++j) {
        mint ajv = a[j + v];
        a[j + v] = a[j] - ajv;
        a[j] += ajv;
      }
    }
    int u = 1 << (2 + (k & 1));
    int v = 1 << (k - 2 - (k & 1));
    mint one = mint(1);
    mint imag = dw[1];
    while (v) {
      // jh = 0
      {
        int j0 = 0;
        int j1 = v;
        int j2 = j1 + v;
        int j3 = j2 + v;
        for (; j0 < v; ++j0, ++j1, ++j2, ++j3) {
          mint t0 = a[j0], t1 = a[j1], t2 = a[j2], t3 = a[j3];
          mint t0p2 = t0 + t2, t1p3 = t1 + t3;
          mint t0m2 = t0 - t2, t1m3 = (t1 - t3) * imag;
          a[j0] = t0p2 + t1p3, a[j1] = t0p2 - t1p3;
          a[j2] = t0m2 + t1m3, a[j3] = t0m2 - t1m3;
        }
      }
      // jh >= 1
      mint ww = one, xx = one * dw[2], wx = one;
      for (int jh = 4; jh < u;) {
        ww = xx * xx, wx = ww * xx;
        int j0 = jh * v;
        int je = j0 + v;
        int j2 = je + v;
        for (; j0 < je; ++j0, ++j2) {
          mint t0 = a[j0], t1 = a[j0 + v] * xx, t2 = a[j2] * ww,
               t3 = a[j2 + v] * wx;
          mint t0p2 = t0 + t2, t1p3 = t1 + t3;
          mint t0m2 = t0 - t2, t1m3 = (t1 - t3) * imag;
          a[j0] = t0p2 + t1p3, a[j0 + v] = t0p2 - t1p3;
          a[j2] = t0m2 + t1m3, a[j2 + v] = t0m2 - t1m3;
        }
        jh += 4;
        if (jh < u) xx *= dw[__builtin_ctzll(jh)];
      }
      u <<= 2;
      v >>= 2;
    }
  }

  void ifft4(vector<mint> &a, int k) {
    if ((int)a.size() <= 1) return;
    if (k == 1) {
      mint a1 = a[1];
      a[1] = a[0] - a[1];
      a[0] = a[0] + a1;
      return;
    }
    int u = 1 << (k - 2);
    int v = 1;
    mint one = mint(1);
    mint imag = dy[1];
    while (u) {
      // jh = 0
      {
        int j0 = 0;
        int j1 = v;
        int j2 = v + v;
        int j3 = j2 + v;
        for (; j0 < v; ++j0, ++j1, ++j2, ++j3) {
          mint t0 = a[j0], t1 = a[j1], t2 = a[j2], t3 = a[j3];
          mint t0p1 = t0 + t1, t2p3 = t2 + t3;
          mint t0m1 = t0 - t1, t2m3 = (t2 - t3) * imag;
          a[j0] = t0p1 + t2p3, a[j2] = t0p1 - t2p3;
          a[j1] = t0m1 + t2m3, a[j3] = t0m1 - t2m3;
        }
      }
      // jh >= 1
      mint ww = one, xx = one * dy[2], yy = one;
      u <<= 2;
      for (int jh = 4; jh < u;) {
        ww = xx * xx, yy = xx * imag;
        int j0 = jh * v;
        int je = j0 + v;
        int j2 = je + v;
        for (; j0 < je; ++j0, ++j2) {
          mint t0 = a[j0], t1 = a[j0 + v], t2 = a[j2], t3 = a[j2 + v];
          mint t0p1 = t0 + t1, t2p3 = t2 + t3;
          mint t0m1 = (t0 - t1) * xx, t2m3 = (t2 - t3) * yy;
          a[j0] = t0p1 + t2p3, a[j2] = (t0p1 - t2p3) * ww;
          a[j0 + v] = t0m1 + t2m3, a[j2 + v] = (t0m1 - t2m3) * ww;
        }
        jh += 4;
        if (jh < u) xx *= dy[__builtin_ctzll(jh)];
      }
      u >>= 4;
      v <<= 2;
    }
    if (k & 1) {
      u = 1 << (k - 1);
      for (int j = 0; j < u; ++j) {
        mint ajv = a[j] - a[j + u];
        a[j] += a[j + u];
        a[j + u] = ajv;
      }
    }
  }

  void ntt(vector<mint> &a) {
    if ((int)a.size() <= 1) return;
    fft4(a, __builtin_ctz(a.size()));
  }

  void intt(vector<mint> &a) {
    if ((int)a.size() <= 1) return;
    ifft4(a, __builtin_ctz(a.size()));
    mint iv = mint(a.size()).inverse();
    for (auto &x : a) x *= iv;
  }

  vector<mint> multiply(const vector<mint> &a, const vector<mint> &b) {
    if (a.empty() || b.empty()) return {};
    int l = a.size() + b.size() - 1;
    if (min<int>(a.size(), b.size()) <= 40) {
      vector<mint> s(l);
      for (int i = 0; i < (int)a.size(); ++i)
        for (int j = 0; j < (int)b.size(); ++j) s[i + j] += a[i] * b[j];
      return s;
    }
    int k = 2, M = 4;
    while (M < l) M <<= 1, ++k;
    assert(k <= level);
    setwy(k);
    vector<mint> s(M);
    for (int i = 0; i < (int)a.size(); ++i) s[i] = a[i];
    fft4(s, k);
    if (a.size() == b.size() && a == b) {
      for (int i = 0; i < M; ++i) s[i] *= s[i];
    } else {
      vector<mint> t(M);
      for (int i = 0; i < (int)b.size(); ++i) t[i] = b[i];
      fft4(t, k);
      for (int i = 0; i < M; ++i) s[i] *= t[i];
    }
    ifft4(s, k);
    s.resize(l);
    mint invm = mint(M).inverse();
    for (int i = 0; i < l; ++i) s[i] *= invm;
    return s;
  }

  void ntt_doubling(vector<mint> &a) {
    int M = (int)a.size();
    auto b = a;
    intt(b);
    mint r = 1, zeta = mint(pr).pow((mint::get_mod() - 1) / (M << 1));
    for (int i = 0; i < M; i++) b[i] *= r, r *= zeta;
    ntt(b);
    copy(begin(b), end(b), back_inserter(a));
  }
};

template <typename Int, typename UInt, typename Long, typename ULong, int id>
struct ArbitraryLazyMontgomeryModIntBase {
  using mint = ArbitraryLazyMontgomeryModIntBase;

  inline static UInt mod;
  inline static UInt r;
  inline static UInt n2;
  static constexpr int bit_length = sizeof(UInt) * 8;

  static UInt get_r() {
    UInt ret = mod;
    while (mod * ret != 1) ret *= UInt(2) - mod * ret;
    return ret;
  }
  static void set_mod(UInt m) {
    assert(m < (UInt(1u) << (bit_length - 2)));
    assert((m & 1) == 1);
    mod = m, n2 = -ULong(m) % m, r = get_r();
  }
  UInt a;

  ArbitraryLazyMontgomeryModIntBase() : a(0) {}
  ArbitraryLazyMontgomeryModIntBase(const Long &b)
      : a(reduce(ULong(b % mod + mod) * n2)){};

  static UInt reduce(const ULong &b) {
    return (b + ULong(UInt(b) * UInt(-r)) * mod) >> bit_length;
  }

  mint &operator+=(const mint &b) {
    if (Int(a += b.a - 2 * mod) < 0) a += 2 * mod;
    return *this;
  }
  mint &operator-=(const mint &b) {
    if (Int(a -= b.a) < 0) a += 2 * mod;
    return *this;
  }
  mint &operator*=(const mint &b) {
    a = reduce(ULong(a) * b.a);
    return *this;
  }
  mint &operator/=(const mint &b) {
    *this *= b.inverse();
    return *this;
  }

  mint operator+(const mint &b) const { return mint(*this) += b; }
  mint operator-(const mint &b) const { return mint(*this) -= b; }
  mint operator*(const mint &b) const { return mint(*this) *= b; }
  mint operator/(const mint &b) const { return mint(*this) /= b; }

  bool operator==(const mint &b) const {
    return (a >= mod ? a - mod : a) == (b.a >= mod ? b.a - mod : b.a);
  }
  bool operator!=(const mint &b) const {
    return (a >= mod ? a - mod : a) != (b.a >= mod ? b.a - mod : b.a);
  }
  mint operator-() const { return mint(0) - mint(*this); }
  mint operator+() const { return mint(*this); }

  mint pow(ULong n) const {
    mint ret(1), mul(*this);
    while (n > 0) {
      if (n & 1) ret *= mul;
      mul *= mul, n >>= 1;
    }
    return ret;
  }

  friend ostream &operator<<(ostream &os, const mint &b) {
    return os << b.get();
  }

  friend istream &operator>>(istream &is, mint &b) {
    Long t;
    is >> t;
    b = ArbitraryLazyMontgomeryModIntBase(t);
    return (is);
  }

  mint inverse() const {
    Int x = get(), y = get_mod(), u = 1, v = 0;
    while (y > 0) {
      Int t = x / y;
      swap(x -= t * y, y);
      swap(u -= t * v, v);
    }
    return mint{u};
  }

  UInt get() const {
    UInt ret = reduce(a);
    return ret >= mod ? ret - mod : ret;
  }

  static UInt get_mod() { return mod; }
};

// id に適当な乱数を割り当てて使う
template <int id>
using ArbitraryLazyMontgomeryModInt =
    ArbitraryLazyMontgomeryModIntBase<int, unsigned int, long long,
                                      unsigned long long, id>;
template <int id>
using ArbitraryLazyMontgomeryModInt64bit =
    ArbitraryLazyMontgomeryModIntBase<long long, unsigned long long, __int128_t,
                                      __uint128_t, id>;

namespace ArbitraryNTT {
using i64 = int64_t;
using u128 = __uint128_t;
constexpr int32_t m0 = 167772161;
constexpr int32_t m1 = 469762049;
constexpr int32_t m2 = 754974721;
using mint0 = LazyMontgomeryModInt<m0>;
using mint1 = LazyMontgomeryModInt<m1>;
using mint2 = LazyMontgomeryModInt<m2>;
constexpr int r01 = mint1(m0).inverse().get();
constexpr int r02 = mint2(m0).inverse().get();
constexpr int r12 = mint2(m1).inverse().get();
constexpr int r02r12 = i64(r02) * r12 % m2;
constexpr i64 w1 = m0;
constexpr i64 w2 = i64(m0) * m1;

template <typename T, typename submint>
vector<submint> mul(const vector<T> &a, const vector<T> &b) {
  static NTT<submint> ntt;
  vector<submint> s(a.size()), t(b.size());
  for (int i = 0; i < (int)a.size(); ++i) s[i] = i64(a[i] % submint::get_mod());
  for (int i = 0; i < (int)b.size(); ++i) t[i] = i64(b[i] % submint::get_mod());
  return ntt.multiply(s, t);
}

template <typename T>
vector<int> multiply(const vector<T> &s, const vector<T> &t, int mod) {
  auto d0 = mul<T, mint0>(s, t);
  auto d1 = mul<T, mint1>(s, t);
  auto d2 = mul<T, mint2>(s, t);
  int n = d0.size();
  vector<int> ret(n);
  const int W1 = w1 % mod;
  const int W2 = w2 % mod;
  for (int i = 0; i < n; i++) {
    int n1 = d1[i].get(), n2 = d2[i].get(), a = d0[i].get();
    int b = i64(n1 + m1 - a) * r01 % m1;
    int c = (i64(n2 + m2 - a) * r02r12 + i64(m2 - b) * r12) % m2;
    ret[i] = (i64(a) + i64(b) * W1 + i64(c) * W2) % mod;
  }
  return ret;
}

template <typename mint>
vector<mint> multiply(const vector<mint> &a, const vector<mint> &b) {
  if (a.empty() || b.empty()) return {};
  if (min<int>(a.size(), b.size()) < 128) {
    vector<mint> ret(a.size() + b.size() - 1);
    for (int i = 0; i < (int)a.size(); ++i)
      for (int j = 0; j < (int)b.size(); ++j) ret[i + j] += a[i] * b[j];
    return ret;
  }
  vector<int> s(a.size()), t(b.size());
  for (int i = 0; i < (int)a.size(); ++i) s[i] = a[i].get();
  for (int i = 0; i < (int)b.size(); ++i) t[i] = b[i].get();
  vector<int> u = multiply<int>(s, t, mint::get_mod());
  vector<mint> ret(u.size());
  for (int i = 0; i < (int)u.size(); ++i) ret[i] = mint(u[i]);
  return ret;
}

template <typename T>
vector<u128> multiply_u128(const vector<T> &s, const vector<T> &t) {
  if (s.empty() || t.empty()) return {};
  if (min<int>(s.size(), t.size()) < 128) {
    vector<u128> ret(s.size() + t.size() - 1);
    for (int i = 0; i < (int)s.size(); ++i)
      for (int j = 0; j < (int)t.size(); ++j) ret[i + j] += i64(s[i]) * t[j];
    return ret;
  }
  auto d0 = mul<T, mint0>(s, t);
  auto d1 = mul<T, mint1>(s, t);
  auto d2 = mul<T, mint2>(s, t);
  int n = d0.size();
  vector<u128> ret(n);
  for (int i = 0; i < n; i++) {
    i64 n1 = d1[i].get(), n2 = d2[i].get();
    i64 a = d0[i].get();
    i64 b = (n1 + m1 - a) * r01 % m1;
    i64 c = ((n2 + m2 - a) * r02r12 + (m2 - b) * r12) % m2;
    ret[i] = a + b * w1 + u128(c) * w2;
  }
  return ret;
}
}  // namespace ArbitraryNTT


template <typename mint>
struct FormalPowerSeries : vector<mint> {
  using vector<mint>::vector;
  using FPS = FormalPowerSeries;

  FPS &operator+=(const FPS &r) {
    if (r.size() > this->size()) this->resize(r.size());
    for (int i = 0; i < (int)r.size(); i++) (*this)[i] += r[i];
    return *this;
  }

  FPS &operator+=(const mint &r) {
    if (this->empty()) this->resize(1);
    (*this)[0] += r;
    return *this;
  }

  FPS &operator-=(const FPS &r) {
    if (r.size() > this->size()) this->resize(r.size());
    for (int i = 0; i < (int)r.size(); i++) (*this)[i] -= r[i];
    return *this;
  }

  FPS &operator-=(const mint &r) {
    if (this->empty()) this->resize(1);
    (*this)[0] -= r;
    return *this;
  }

  FPS &operator*=(const mint &v) {
    // f *= f[i] の場合も、全係数を更新前の f[i] で掛ける。
    const mint scale = v;
    for (int k = 0; k < (int)this->size(); k++) (*this)[k] *= scale;
    return *this;
  }

  FPS &operator/=(const FPS &r) {
    FPS divisor(r);
    divisor.shrink();
    assert(!divisor.empty());
    this->shrink();
    if (this->size() < divisor.size()) {
      this->clear();
      return *this;
    }
    int n = this->size() - divisor.size() + 1;
    if ((int)divisor.size() <= 64) {
      FPS f(*this), g(divisor);
      mint coeff = g.back().inverse();
      for (auto &x : g) x *= coeff;
      int deg = (int)f.size() - (int)g.size() + 1;
      int gs = g.size();
      FPS quo(deg);
      for (int i = deg - 1; i >= 0; i--) {
        quo[i] = f[i + gs - 1];
        for (int j = 0; j < gs; j++) f[i + j] -= quo[i] * g[j];
      }
      *this = quo * coeff;
      this->resize(n, mint(0));
      return *this;
    }
    return *this = ((*this).rev().pre(n) * divisor.rev().inv(n)).pre(n).rev();
  }

  FPS &operator%=(const FPS &r) {
    *this -= *this / r * r;
    shrink();
    return *this;
  }

  FPS operator+(const FPS &r) const { return FPS(*this) += r; }
  FPS operator+(const mint &v) const { return FPS(*this) += v; }
  FPS operator-(const FPS &r) const { return FPS(*this) -= r; }
  FPS operator-(const mint &v) const { return FPS(*this) -= v; }
  FPS operator*(const FPS &r) const { return FPS(*this) *= r; }
  FPS operator*(const mint &v) const { return FPS(*this) *= v; }
  FPS operator/(const FPS &r) const { return FPS(*this) /= r; }
  FPS operator%(const FPS &r) const { return FPS(*this) %= r; }
  FPS operator-() const {
    FPS ret(this->size());
    for (int i = 0; i < (int)this->size(); i++) ret[i] = -(*this)[i];
    return ret;
  }

  void shrink() {
    while (this->size() && this->back() == mint(0)) this->pop_back();
  }

  FPS rev() const {
    FPS ret(*this);
    reverse(begin(ret), end(ret));
    return ret;
  }

  FPS dot(FPS r) const {
    FPS ret(min(this->size(), r.size()));
    for (int i = 0; i < (int)ret.size(); i++) ret[i] = (*this)[i] * r[i];
    return ret;
  }

  // 前 sz 項を取ってくる。sz に足りない項は 0 埋めする
  FPS pre(int sz) const {
    assert(sz >= 0);
    FPS ret(begin(*this), begin(*this) + min((int)this->size(), sz));
    if ((int)ret.size() < sz) ret.resize(sz);
    return ret;
  }

  FPS operator>>(int sz) const {
    assert(sz >= 0);
    if ((int)this->size() <= sz) return {};
    FPS ret(*this);
    ret.erase(ret.begin(), ret.begin() + sz);
    return ret;
  }

  FPS operator<<(int sz) const {
    assert(sz >= 0);
    FPS ret(*this);
    ret.insert(ret.begin(), sz, mint(0));
    return ret;
  }

  FPS diff() const {
    const int n = (int)this->size();
    FPS ret(max(0, n - 1));
    mint one(1), coeff(1);
    for (int i = 1; i < n; i++) {
      ret[i - 1] = (*this)[i] * coeff;
      coeff += one;
    }
    return ret;
  }

  FPS integral() const {
    const int n = (int)this->size();
    FPS ret(n + 1);
    ret[0] = mint(0);
    if (n > 0) ret[1] = mint(1);
    auto mod = mint::get_mod();
    assert(uint64_t(n) < uint64_t(mod));
    for (int i = 2; i <= n; i++) ret[i] = (-ret[mod % i]) * (mod / i);
    for (int i = 0; i < n; i++) ret[i + 1] *= (*this)[i];
    return ret;
  }

  mint eval(mint x) const {
    mint r = 0, w = 1;
    for (auto &v : *this) r += w * v, w *= x;
    return r;
  }

  FPS log(int deg = -1) const {
    if (deg == -1) deg = (int)this->size();
    assert(deg >= 0 && uint64_t(deg) <= uint64_t(mint::get_mod()));
    if (deg == 0) return {};
    assert(!this->empty() && (*this)[0] == mint(1));
    return (this->pre(deg).diff() * this->inv(deg)).pre(deg - 1).integral();
  }

  FPS pow(int64_t k, int deg = -1) const {
    const int n = (int)this->size();
    if (deg == -1) deg = n;
    assert(k >= 0 && deg >= 0);
    assert(uint64_t(deg) <= uint64_t(mint::get_mod()));
    if (deg == 0) return {};
    if (k == 0) {
      FPS ret(deg);
      if (deg) ret[0] = 1;
      return ret;
    }
    for (int i = 0; i < min(n, deg); i++) {
      if ((*this)[i] != mint(0)) {
        if (i && k > (deg - 1) / i) return FPS(deg, mint(0));
        int shift = int(i * k), need = deg - shift;
        mint rev = mint(1) / (*this)[i];
        FPS ret = ((((*this >> i).pre(need) * rev).log(need)) * mint(k)).exp(need);
        ret *= (*this)[i].pow(k);
        return (ret << shift).pre(deg);
      }
      if (k > (deg - 1) / (i + 1)) return FPS(deg, mint(0));
    }
    return FPS(deg, mint(0));
  }

  static void *ntt_ptr;
  static void set_fft();
  FPS &operator*=(const FPS &r);
  void ntt();
  void intt();
  void ntt_doubling();
  static int ntt_pr();
  FPS inv(int deg = -1) const;
  FPS exp(int deg = -1) const;
};
template <typename mint>
void *FormalPowerSeries<mint>::ntt_ptr = nullptr;

template <typename mint>
void FormalPowerSeries<mint>::set_fft() {
  ntt_ptr = nullptr;
}

template <typename mint>
void FormalPowerSeries<mint>::ntt() {
  exit(1);
}

template <typename mint>
void FormalPowerSeries<mint>::intt() {
  exit(1);
}

template <typename mint>
void FormalPowerSeries<mint>::ntt_doubling() {
  exit(1);
}

template <typename mint>
int FormalPowerSeries<mint>::ntt_pr() {
  exit(1);
}

namespace katu2ou_fps_detail {
template <typename Mint>
vector<Mint> convolution(const vector<Mint> &a, const vector<Mint> &b) {
  return ArbitraryNTT::multiply(a, b);
}
template <uint32_t Mod>
vector<LazyMontgomeryModInt<Mod>> convolution(
    const vector<LazyMontgomeryModInt<Mod>> &a,
    const vector<LazyMontgomeryModInt<Mod>> &b) {
  // 十分な 2 冪根がある固定法のみ直接 NTT。それ以外は CRT。
  if constexpr (__builtin_ctz(Mod - 1) >= 3) {
    if (a.empty() || b.empty()) return {};
    if (a.size() + b.size() - 1 <= (size_t(1) << __builtin_ctz(Mod - 1))) {
      static NTT<LazyMontgomeryModInt<Mod>> engine;
      return engine.multiply(a, b);
    }
  }
  return ArbitraryNTT::multiply(a, b);
}
}  // namespace katu2ou_fps_detail

template <typename mint>
FormalPowerSeries<mint>& FormalPowerSeries<mint>::operator*=(
    const FormalPowerSeries<mint>& r) {
  if (this->empty() || r.empty()) {
    this->clear();
    return *this;
  }
  auto ret = katu2ou_fps_detail::convolution(
      static_cast<const vector<mint>&>(*this), static_cast<const vector<mint>&>(r));
  return *this = FormalPowerSeries<mint>(ret.begin(), ret.end());
}

template <typename mint>
FormalPowerSeries<mint> FormalPowerSeries<mint>::inv(int deg) const {
  if (deg == -1) deg = (*this).size();
  assert(deg >= 0);
  if (deg == 0) return {};
  assert(!this->empty() && (*this)[0] != mint(0));
  FormalPowerSeries<mint> ret({mint(1) / (*this)[0]});
  for (int i = 1; i < deg; i <<= 1) {
    int next = min(deg, i << 1);
    ret = (ret + ret - ret * (ret * this->pre(next)).pre(next)).pre(next);
  }
  return ret.pre(deg);
}

template <typename mint>
FormalPowerSeries<mint> FormalPowerSeries<mint>::exp(int deg) const {
  if (deg == -1) deg = (int)this->size();
  assert(deg >= 0 && uint64_t(deg) <= uint64_t(mint::get_mod()));
  if (deg == 0) return {};
  assert(this->empty() || (*this)[0] == mint(0));
  FormalPowerSeries<mint> ret({mint(1)});
  for (int i = 1; i < deg; i <<= 1) {
    int next = min(deg, i << 1);
    ret = (ret * (pre(next) + mint(1) - ret.log(next))).pre(next);
  }
  return ret.pre(deg);
}

#ifndef KATU2OU_FPS_NO_DEFAULT_ALIASES
using mint = LazyMontgomeryModInt<1000000007>;
using fps = FormalPowerSeries<mint>;
#endif

#endif  // KATU2OU_FORMAL_POWER_SERIES_CPP

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
#ifndef KATU2OU_FPS_NEWTON_CPP
#define KATU2OU_FPS_NEWTON_CPP
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

#ifndef KATU2OU_FPS_SQRT_CPP
#define KATU2OU_FPS_SQRT_CPP

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

#endif  // KATU2OU_FPS_SQRT_CPP

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
#ifndef KATU2OU_FPS_SPARSE_CPP
#define KATU2OU_FPS_SPARSE_CPP
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

namespace katu2ou_fps_sparse_detail {
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
}  // namespace katu2ou_fps_sparse_detail

template <typename Mint>
FormalPowerSeries<Mint> fps_mul_sparse(const FormalPowerSeries<Mint>& f,
                                      const SparseFPS<Mint>& g, int n) {
  katu2ou_fps_sparse_detail::validate(g, n);
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
  katu2ou_fps_sparse_detail::validate(g, n);
  if (n == 0) return {};
  Mint c = katu2ou_fps_sparse_detail::constant(g);
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
  katu2ou_fps_sparse_detail::validate(f, n);
  assert(uint64_t(n) <= uint64_t(Mint::get_mod()));
  if (n == 0) return {};
  assert(katu2ou_fps_sparse_detail::constant(f) == Mint(1));
  FormalPowerSeries<Mint> df(max(0, n-1));
  for (auto [d, c] : f) {
    if (d >= n) break;
    if (d) df[d-1] = Mint(d) * c;
  }
  return fps_div_sparse(df, f, n-1).integral();
}

template <typename Mint>
FormalPowerSeries<Mint> fps_exp_sparse(const SparseFPS<Mint>& f, int n) {
  katu2ou_fps_sparse_detail::validate(f, n);
  auto inv = katu2ou_fps_sparse_detail::inverses<Mint>(n);
  if (n == 0) return {};
  assert(katu2ou_fps_sparse_detail::constant(f) == Mint(0));
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
  katu2ou_fps_sparse_detail::validate(f, n);
  auto inv = katu2ou_fps_sparse_detail::inverses<Mint>(n);
  if (n == 0) return {};
  assert(katu2ou_fps_sparse_detail::constant(f) == Mint(1));
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
  katu2ou_fps_sparse_detail::validate(f, n);
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
  katu2ou_fps_sparse_detail::validate(f, n);
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

/*
<FPS 合成・合成逆関数 O(N log^2 N)>
  [使い方]
    #include "fps-composition.cpp"
    fps h = fps_compose(f, g, n);               // f(g(x)) mod x^n
    fps r = fps_compositional_inverse(f, n);    // f(r(x)) = r(f(x)) = x mod x^n
    auto a = fps_power_projection(f, n);        // a[i] = [x^(n-1)] f(x)^i
    n を省略すると f.size()。返り値はちょうど n 項（0 次〜n-1 次）。
    係数型は FormalPowerSeries<mint> の mint から推論する。

  [各関数の入力と返り値]
    fps_compose(f,g,deg=-1)
      入力: 外側の FPS f の係数列、代入する FPS g の係数列、求める項数 deg。
        g の定数項は 0（空列も可）。deg を省略または -1 にすると f.size()。
      返り値: 合成 f(g(x)) の次数 deg 未満の係数を持つ長さ deg の fps。
        f の deg 次以上の項は不要。入力にない係数は 0 として扱う。
    fps_compositional_inverse(f,deg=-1)
      入力: 定数項 0、1 次の係数が非零の FPS f と、求める項数 deg。
        deg を省略または -1 にすると f.size()。deg>0 では f は 2 項以上必要。
      返り値: f(g(x))=g(f(x))=x mod x^deg を満たす逆関数 g の係数列。
        長さ deg の fps で、定数項は 0。f.inv() が返す 1/f とは異なる。
    fps_power_projection(f,deg=-1)
      入力: 定数項が 0 の FPS f（空列も可）と、項数 deg。
        deg を省略または -1 にすると f.size()。
      返り値: 長さ deg の fps a。a[i] は f(x)^i の x^(deg-1) の係数。
        i=0,1,...,deg-1 の各べきから同じ次数の係数を一つずつ取り出した列。
        例えば f=x, deg=4 のとき {0,0,0,1}。
    いずれも deg=0 なら空列を返す。

  [前提]
    合成: g.empty() または g[0] == 0。外側 f の n 次以上は参照しない。
    逆関数: f[0] == 0, f[1] != 0、素数 mod、n < mod。
    power projection: f.empty() または f[0] == 0、n >= 0。
    合成逆関数は乗法逆元 f.inv() とは異なる。
    n=0 は空列を返す。逆関数の前提は n>0 のとき検査する。

  [計算量 / 実装]
    合成・逆関数・power projection: O(N log^2 N) 時間。
    合成は O(N log N) メモリ、他の 2 つは O(N) メモリ。
    2 変数 Bostan–Mori とその転置を、Kronecker 置換による通常の
    畳み込みで実装。平方分割による O(N^2) 合成ではない。
    逆関数は power projection + Lagrange 反転 + FPS log/exp。
    畳み込み配列の定数倍は通常の FPS 積より大きい。
    基底ライブラリの NTT / CRT が扱える畳み込み長を超えないこと。

  [参照（アルゴリズム）]
    https://noshi91.hatenablog.com/entry/2024/03/16/224034
    https://maspypy.com/fps-composition-and-compositional-inverse-part-1-compositional-inverse-and-power-projection
    https://maspypy.com/fps-composition-and-compositional-inverse-part-2
*/
#ifndef KATU2OU_FPS_COMPOSITION_CPP
#define KATU2OU_FPS_COMPOSITION_CPP

namespace fps_composition_detail {
// 行 j は y^j の係数、多項式 x の長さは h。
// x の積の桁上がりを防ぐため、畳み込み時は行幅を 2h-1 にする。
template <class Mint>
FormalPowerSeries<Mint> pack(const FormalPowerSeries<Mint>& a, int h) {
  using F = FormalPowerSeries<Mint>;
  const int rows = int(a.size()) / h, stride = 2 * h - 1;
  F b((rows - 1) * stride + h);
  for (int j = 0; j < rows; ++j)
    for (int i = 0; i < h; ++i) b[j * stride + i] = a[j * h + i];
  return b;
}

template <class Mint>
FormalPowerSeries<Mint> denominator(const FormalPowerSeries<Mint>& g, int n) {
  FormalPowerSeries<Mint> q(2 * n);
  q[0] = Mint(1);
  for (int i = 0; i < std::min(n, int(g.size())); ++i) q[n + i] = -g[i];
  return q;
}

template <class Mint>
FormalPowerSeries<Mint> negate_x(FormalPowerSeries<Mint> q, int h) {
  for (int j = 0; j < int(q.size()); j += h)
    for (int i = 1; i < h; i += 2) q[j + i] = -q[j + i];
  return pack(q, h);
}

// 積から x の指定 parity の項だけを取り出し x^2 を x に置換する。
template <class Mint>
FormalPowerSeries<Mint> select(const FormalPowerSeries<Mint>& a,
                              int h, int rows, int parity) {
  const int nh = (h + 1) / 2, stride = 2 * h - 1;
  FormalPowerSeries<Mint> b(rows * nh);
  for (int j = 0; j < rows; ++j)
    for (int i = 0; i < nh; ++i) b[j * nh + i] = a[j * stride + 2 * i + parity];
  return b;
}

template <class Mint>
FormalPowerSeries<Mint> compose_transpose(
    const FormalPowerSeries<Mint>& q, int h, int d,
    const FormalPowerSeries<Mint>& f) {
  using F = FormalPowerSeries<Mint>;
  if (h == 1) return f.pre(d);
  const int stride = 2 * h - 1, nh = (h + 1) / 2;
  const int parity = (h - 1) & 1;
  F qneg = negate_x(q, h);
  F next_q = select(pack(q, h) * qneg, h, 2 * d + 1, 0);
  F next = compose_transpose(next_q, nh, 2 * d, f);

  // select の転置: 選ばれた座標への散布。
  F scattered(2 * d * stride);
  for (int j = 0; j < 2 * d; ++j)
    for (int i = 0; i < nh; ++i)
      scattered[j * stride + 2 * i + parity] = next[j * nh + i];

  // p -> p*qneg の転置: reverse(qneg) との畳み込みの中央部分。
  const int offset = int(qneg.size()) - 1;
  F product = scattered * qneg.rev();
  F result(d * h);
  for (int j = 0; j < d; ++j)
    for (int i = 0; i < h; ++i)
      result[j * h + i] = product[offset + j * stride + i];
  return result;
}
}  // namespace fps_composition_detail

template <class Mint>
FormalPowerSeries<Mint> fps_compose(const FormalPowerSeries<Mint>& f,
                                   const FormalPowerSeries<Mint>& g,
                                   int deg = -1) {
  using F = FormalPowerSeries<Mint>;
  if (deg == -1) deg = int(f.size());
  assert(deg >= 0);
  if (deg == 0) return {};
  assert(g.empty() || g[0] == Mint(0));
  if (f.empty()) return F(deg);
  F q = fps_composition_detail::denominator(g, deg);
  // P_0 = reverse(w)。転置後に反転して w に対する係数を得る。
  return fps_composition_detail::compose_transpose(q, deg, 1, f.pre(deg)).rev();
}

template <class Mint>
FormalPowerSeries<Mint> fps_power_projection(const FormalPowerSeries<Mint>& f,
                                            int deg = -1) {
  using F = FormalPowerSeries<Mint>;
  if (deg == -1) deg = int(f.size());
  assert(deg >= 0);
  if (deg == 0) return {};
  assert(f.empty() || f[0] == Mint(0));
  F q = fps_composition_detail::denominator(f, deg), p(deg);
  p[0] = Mint(1);
  int h = deg, d = 1;
  while (h > 1) {
    F qneg = fps_composition_detail::negate_x(q, h);
    F next_p = fps_composition_detail::select(
        fps_composition_detail::pack(p, h) * qneg, h, 2 * d, (h - 1) & 1);
    F next_q = fps_composition_detail::select(
        fps_composition_detail::pack(q, h) * qneg, h, 2 * d + 1, 0);
    p = std::move(next_p);
    q = std::move(next_q);
    h = (h + 1) / 2;
    d *= 2;
  }
  // f(0)=0 なので最終分母 Q(0,y)=1。
  return p.pre(deg);
}

template <class Mint>
FormalPowerSeries<Mint> fps_compositional_inverse(
    const FormalPowerSeries<Mint>& f, int deg = -1) {
  using F = FormalPowerSeries<Mint>;
  if (deg == -1) deg = int(f.size());
  assert(deg >= 0);
  if (deg == 0) return {};
  assert(f.size() >= 2 && f[0] == Mint(0) && f[1] != Mint(0));
  assert(static_cast<uint64_t>(deg) < Mint::get_mod());
  if (deg == 1) return F(1);
  const int n = deg - 1;
  const Mint inv_linear = f[1].inverse();
  F normalized = f.pre(deg) * inv_linear;
  F projection = fps_power_projection(normalized, deg);

  // A(x)=(g(x)/x)^(-n), A[n-i] = n/i * [x^n] normalized(x)^i。
  F inv_int(n + 1), a(n);
  inv_int[1] = Mint(1);
  const auto mod = Mint::get_mod();
  for (int i = 2; i <= n; ++i)
    inv_int[i] = -inv_int[mod % i] * Mint(mod / i);
  for (int i = 1; i <= n; ++i) a[n - i] = Mint(n) * inv_int[i] * projection[i];
  F result = ((a.log(n) * (-inv_int[n])).exp(n) << 1);
  Mint scale = Mint(1);
  for (int i = 1; i < deg; ++i) {
    scale *= inv_linear;
    result[i] *= scale;
  }
  return result;
}
#endif  // KATU2OU_FPS_COMPOSITION_CPP

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
#ifndef KATU2OU_POLYNOMIAL_GCD_CPP
#define KATU2OU_POLYNOMIAL_GCD_CPP
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

/*
<多点評価 / 積木>
  [使い方]
    #include "multipoint-evaluation.cpp"
    vector<mint> xs = {0, 1, 2};
    auto ys = MultipointEvaluation(fps{1, 2, 3}, xs); // {1, 6, 17}
    ProductTree<mint> tree(xs);
    auto zs = tree.evaluate(f); // 同じ評価点で繰り返し使える
    multipoint_evaluation(f, xs) も同じ関数。

  [各関数の入力と返り値]
    MultipointEvaluation(f,xs), multipoint_evaluation(f,xs)
      入力: 多項式 f の係数列（fps）と、評価する点の列 xs（vector<mint>）。
      返り値: vector<mint>{f(xs[0]),...,f(xs[m-1])}（m=xs.size()）。
        点の入力順を保ち、同じ点を複数指定してもよい。
        f が零多項式なら m 個の 0、xs が空なら空列を返す。
    ProductTree<mint>(xs)
      入力: 評価点の列 xs（vector<mint>）。
      結果: 各区間の積 prod(x-xs[i]) を保持する積木を構築する。
        xs をコピーして所有する。同じ点集合での評価や補間に再利用できる。
    tree.evaluate(f)
      入力: 評価する多項式 f の係数列（fps）。
      返り値: tree に保存された点の順に並ぶ評価値の列（vector<mint>）。
        MultipointEvaluation(f,tree.xs) と同じ値を、積木を再利用して求める。
    InnerMultipointEvaluation(f,xs,tree)
      入力: 多項式 f、評価点 xs、構築済みの積木 tree。xs==tree.xs が必要。
      返り値: tree.evaluate(f) と同じ評価値の列（vector<mint>）。
    tree.build(), tree.build_ntt()
      入力: 追加の引数なし。保存済みの評価点を使って積木を再構築する。
      返り値: なし。コンストラクタで構築されるため、通常は直接呼ぶ必要はない。
        build_ntt() も、この実装では build() と同じ処理。

  [仕様 / 前提]
    係数は体（素数 mod）。評価点の重複を許す。空の f は零多項式。
    戻り値は入力と同じ順序の {f(xs[0]), ..., f(xs[m-1])}。
    tree は評価点を所有するため、一時 vector を渡してもよい。
    buf[i] は区間 [l[i], r[i]) の積 prod(x-xs[j])（通常の係数列）。

  [計算量]
    D = f.size(), m = xs.size(), M(n) = n 項の畳み込みの計算量。
    構築 O(M(m) log m)、評価 O(M(D) + M(m) log m)、空間 O(m log m + D)。
    D,m が同程度なら O(m log^2 m)。
*/
#ifndef KATU2OU_MULTIPOINT_EVALUATION_CPP
#define KATU2OU_MULTIPOINT_EVALUATION_CPP

template <typename mint>
struct ProductTree {
  using fps = FormalPowerSeries<mint>;
  vector<mint> xs;
  vector<fps> buf;
  int N, xsz;
  vector<int> l, r;

  explicit ProductTree(const vector<mint>& points)
      : xs(points), N(1), xsz((int)points.size()) {
    while (N < xsz) N *= 2;
    build();
  }

  void build() {
    buf.assign(2 * N, fps{mint(1)});
    l.assign(2 * N, xsz);
    r.assign(2 * N, xsz);
    for (int i = 0; i < xsz; ++i) {
      buf[N + i] = {-xs[i], mint(1)};
      l[N + i] = i;
      r[N + i] = i + 1;
    }
    for (int i = N - 1; i; --i) {
      l[i] = l[2 * i];
      r[i] = r[2 * i + 1];
      buf[i] = buf[2 * i] * buf[2 * i + 1];
    }
  }

  // 旧 API 互換。現在の任意 mod FPS では係数表示の積木を構築する。
  void build_ntt() { build(); }

  vector<mint> evaluate(const fps& f) const {
    vector<mint> values(xsz);
    if (!xsz || f.empty()) return values;
    auto visit = [&](auto&& self, const fps& input, int node) -> void {
      if (l[node] == r[node]) return;
      fps remainder = input % buf[node];
      if (remainder.size() <= 32 || node >= N) {
        for (int i = l[node]; i < r[node]; ++i)
          values[i] = remainder.eval(xs[i]);
        return;
      }
      self(self, remainder, 2 * node);
      self(self, remainder, 2 * node + 1);
    };
    visit(visit, f, 1);
    return values;
  }
};

template <typename mint>
vector<mint> InnerMultipointEvaluation(const FormalPowerSeries<mint>& f,
                                       const vector<mint>& xs,
                                       const ProductTree<mint>& tree) {
  assert(xs == tree.xs);
  return tree.evaluate(f);
}

template <typename mint>
vector<mint> MultipointEvaluation(const FormalPowerSeries<mint>& f,
                                  const vector<mint>& xs) {
  if (f.empty() || xs.empty()) return vector<mint>(xs.size());
  return ProductTree<mint>(xs).evaluate(f);
}

template <typename mint>
vector<mint> multipoint_evaluation(const FormalPowerSeries<mint>& f,
                                  const vector<mint>& xs) {
  return MultipointEvaluation(f, xs);
}
#endif

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
#ifndef KATU2OU_POLYNOMIAL_INTERPOLATION_CPP
#define KATU2OU_POLYNOMIAL_INTERPOLATION_CPP

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

/*
<等間隔の評価値からの補間・評価点シフト>
  [使い方]
    #include "sampling-point-shift.cpp"
    vector<mint> y = {f(0), f(1), ..., f(n-1)}; // deg(f) < n
    mint v = interpolate_consecutive(y, mint(a));       // f(a)
    auto z = sampling_point_shift(y, mint(a), m);       // f(a),...,f(a+m-1)
    // FormalPowerSeries<mint> も y として渡せる。
  [各関数の入力と返り値]
    interpolate_consecutive(y,a)
      入力: 次数 n 未満の多項式 f の評価値 y={f(0),...,f(n-1)} と、評価点 a。
        y は vector<mint>、a は mint。f の係数列を入力する関数ではない。
      返り値: 点 a における値 f(a) を mint で返す。y が空なら 0。
    sampling_point_shift(y,a,m)
      入力: 評価値 y={f(0),...,f(n-1)}、移動先の先頭の評価点 a（mint）、
        求める評価点の個数 m（非負整数）。
      返り値: vector<mint>{f(a),f(a+1),...,f(a+m-1)}（長さ m）。
        評価点は mod p で解釈する。y が空なら m 個の 0、m=0 なら空列。
        返すのは評価値の列であり、f(x+a) の係数列ではない。
  [条件・返り値]
    法 p は素数、n < p、m >= 0。空の y は零多項式として扱う。
    a が既知の評価点と重なる場合、負の a、p をまたぐ区間にも対応。
    係数列の Taylor shift ではなく、評価値の列を変換する。
  [計算量]
    1 点: O(n) 時間・空間。m 点: O((n+m) log(n+m)) 時間、O(n+m) 空間。
    畳み込みの長さは formal-power-series.cpp の上限以内であること。
  [原理]
    Lagrange 補間の重み y[i] / (i! (n-1-i)!) と 1/(a+j-i) を畳み込む。
    分母が 0 の場所は除外して一括逆元を求め、既知の評価値を直接返す。
*/
#ifndef KATU2OU_SAMPLING_POINT_SHIFT_CPP
#define KATU2OU_SAMPLING_POINT_SHIFT_CPP
#include <limits>

template <typename mint>
mint interpolate_consecutive(const vector<mint> &y, mint a) {
  const int n = (int)y.size();
  assert((uint64_t)n < mint::get_mod());
  if (n == 0) return mint(0);
  if (a.get() < (uint64_t)n) return y[a.get()];
  vector<mint> ifact(n, mint(1)), pref(n + 1, mint(1)), suff(n + 1, mint(1));
  mint fact = 1;
  for (int i = 1; i < n; ++i) fact *= mint(i);
  ifact[n - 1] = fact.inverse();
  for (int i = n - 1; i > 0; --i) ifact[i - 1] = ifact[i] * mint(i);
  for (int i = 0; i < n; ++i) pref[i + 1] = pref[i] * (a - mint(i));
  for (int i = n - 1; i >= 0; --i) suff[i] = suff[i + 1] * (a - mint(i));
  mint answer = 0;
  for (int i = 0; i < n; ++i) {
    mint term = y[i] * ifact[i] * ifact[n - 1 - i] * pref[i] * suff[i + 1];
    answer += ((n - 1 - i) & 1) ? -term : term;
  }
  return answer;
}

template <typename mint>
vector<mint> sampling_point_shift(const vector<mint> &y, mint a, int m) {
  using FPS = FormalPowerSeries<mint>;
  const int n = (int)y.size();
  assert(m >= 0 && (uint64_t)n < mint::get_mod());
  if (n == 0 || m == 0) return vector<mint>(m);
  if (n == 1) return vector<mint>(m, y[0]);
  assert((int64_t)n + m - 1 <= numeric_limits<int>::max());
  vector<mint> ifact(n, mint(1));
  mint fact = 1;
  for (int i = 1; i < n; ++i) fact *= mint(i);
  ifact[n - 1] = fact.inverse();
  for (int i = n - 1; i > 0; --i) ifact[i - 1] = ifact[i] * mint(i);
  FPS weights(n);
  for (int i = 0; i < n; ++i) {
    weights[i] = y[i] * ifact[i] * ifact[n - 1 - i];
    if ((n - 1 - i) & 1) weights[i] = -weights[i];
  }

  const int len = n + m - 1;
  FPS inverse(len);
  vector<mint> factors(len), prod(len + 1, mint(1));
  mint x = a - mint(n - 1);
  for (int i = 0; i < len; ++i, x += mint(1)) {
    factors[i] = x;
    prod[i + 1] = prod[i] * (x == mint(0) ? mint(1) : x);
  }
  mint inv_prod = prod.back().inverse();
  for (int i = len - 1; i >= 0; --i) {
    if (factors[i] == mint(0)) continue;
    inverse[i] = inv_prod * prod[i];
    inv_prod *= factors[i];
  }
  FPS conv = weights * inverse;
  mint window = 1;
  for (int i = 0; i < n; ++i)
    if (factors[i] != mint(0)) window *= factors[i];
  vector<mint> result(m);
  x = a;
  for (int i = 0; i < m; ++i, x += mint(1)) {
    result[i] = x.get() < (uint64_t)n ? y[x.get()] : window * conv[n - 1 + i];
    if (i + 1 < m) {
      if (factors[i] != mint(0)) window *= inverse[i];
      if (factors[n + i] != mint(0)) window *= factors[n + i];
    }
  }
  return result;
}
#endif

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
#ifndef KATU2OU_FAULHABER_CPP
#define KATU2OU_FAULHABER_CPP

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
#ifndef KATU2OU_PREFIX_SUM_POLYNOMIAL_CPP
#define KATU2OU_PREFIX_SUM_POLYNOMIAL_CPP

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

/*
<Relaxed Multiplication: オンライン畳み込み>
  [使い方]
    #include "relaxed-multiplication.cpp"
    RelaxedMultiplication<mint> mul;
    for (int i=0; i<n; ++i) {
      // a_i, b_i は直前までに返された c_0,...,c_{i-1} から決めてよい。
      mint ci = mul.append(a_i, b_i); // ci = sum_{j=0}^i a_j*b_{i-j}
    }
    mul.size(); // 追加済みの項数
    mul.clear(); // 初期状態に戻す
  [各関数の入力と返り値]
    RelaxedMultiplication<mint>()
      入力: なし。結果: 係数がまだ一つも与えられていないオンライン畳み込み器を作る。
    mul.append(a_i,b_i)
      入力: i 番目の係数 a_i,b_i（ともに mint）。i は呼出前の mul.size()。
        係数は 0 番目から順に 1 組ずつ渡す。i 自体を引数に渡す必要はない。
      返り値: c_i=sum_{j=0}^i a_j*b_{i-j} を mint で返す。
        つまり 2 つの FPS の積の x^i の係数。この呼出で状態を更新し、項数を 1 増やす。
        次の a_{i+1},b_{i+1} は、返された c_i を利用して決めてよい。
    mul.size()
      入力: なし。返り値: これまでに append した係数の組数を int で返す。
    mul.clear()
      入力: なし。結果: 保存した入力係数と計算結果を破棄し、項数を 0 に戻す。
      返り値: なし。次の append は再び 0 番目の係数として扱う。
  [条件]
    係数は i=0,1,... の順で 1 組ずつ渡す。未来の係数は不要。
    畳み込みの長さは formal-power-series.cpp の上限以内であること。
  [計算量]
    n 回の append 全体で O(n log^2 n) 時間、O(n) 空間。
    1 回が最悪 O(n log n) かかる場合がある（償却計算量）。
  [原理]
    min(i,j) が [L-1,2L-2] に入る積を長さ L=1,2,4,... のブロックで処理。
    ブロックの最後の入力が到着した時点で、寄与の最小次数が現在の次数になる。
*/
#ifndef KATU2OU_RELAXED_MULTIPLICATION_CPP
#define KATU2OU_RELAXED_MULTIPLICATION_CPP
#include <limits>

template <typename mint>
class RelaxedMultiplication {
  using FPS = FormalPowerSeries<mint>;
  FPS a_, b_, c_;

 public:
  int size() const { return (int)a_.size(); }
  void clear() { a_.clear(); b_.clear(); c_.clear(); }

  mint append(mint a, mint b) {
    const int n = size();
    assert(n <= (numeric_limits<int>::max() - 2) / 2);
    a_.push_back(a);
    b_.push_back(b);
    if ((int)c_.size() <= n) c_.resize(n + 1);
    for (int len = 1; len <= (n + 2) / 2 && (n + 2) % len == 0; len <<= 1) {
      const int left = n + 1 - len, fixed = len - 1;
      FPS x(a_.begin() + left, a_.begin() + n + 1);
      FPS y(b_.begin() + fixed, b_.begin() + fixed + len);
      FPS product = x * y;
      if (left != fixed) {
        x = FPS(a_.begin() + fixed, a_.begin() + fixed + len);
        y = FPS(b_.begin() + left, b_.begin() + n + 1);
        product += x * y;
      }
      if ((int)c_.size() < n + (int)product.size()) c_.resize(n + product.size());
      for (int j = 0; j < (int)product.size(); ++j) c_[n + j] += product[j];
    }
    return c_[n];
  }
};
#endif

#endif

using mint=LazyMontgomeryModInt<998244353>;
using fps=FormalPowerSeries<mint>;
fps read_poly(int n) { fps f(n); for(auto& x:f) cin>>x; return f; }
template<class V> void print(const V& v) {
  for(size_t i=0;i<v.size();++i) { if(i) cout<<' '; cout<<v[i]; } cout<<'\n';
}
int main() { ios::sync_with_stdio(false); cin.tie(nullptr);
int n; cin >> n; auto f=read_poly(n); int v=0; while(v<n && f[v]==mint(0)) ++v; if(v==n) { print(fps(n)); return 0; } int64_t r=mod_sqrt(f[v].get(),mint::get_mod()); if((v&1)||r<0) { cout<<"-1\n"; return 0; } auto unit=(f>>v)*f[v].inverse(); auto g=fps_pow_unit(unit,mint(1)/mint(2),n-v)*mint(r); print((g<<(v/2)).pre(n));
}
