#define KATU2OU_FPS_NO_DEFAULT_ALIASES
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

using mint=LazyMontgomeryModInt<998244353>;
using fps=FormalPowerSeries<mint>;
fps read_poly(int n) { fps f(n); for(auto& x:f) cin>>x; return f; }
template<class V> void print(const V& v) {
  for(size_t i=0;i<v.size();++i) { if(i) cout<<' '; cout<<v[i]; } cout<<'\n';
}
int main() { ios::sync_with_stdio(false); cin.tie(nullptr);
int t; cin >> t; while(t--) { int64_t a,p; cin >> a >> p; cout<<mod_sqrt(a,p)<<"\n"; }
}
