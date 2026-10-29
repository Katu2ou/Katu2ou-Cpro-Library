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
#ifndef FPS_COMPOSITION_CPP
#define FPS_COMPOSITION_CPP
#include "formal-power-series.cpp"

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
#endif  // FPS_COMPOSITION_CPP
