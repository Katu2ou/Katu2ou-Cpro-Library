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
#ifndef SAMPLING_POINT_SHIFT_CPP
#define SAMPLING_POINT_SHIFT_CPP
#include "formal-power-series.cpp"
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
