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
#ifndef MULTIPOINT_EVALUATION_CPP
#define MULTIPOINT_EVALUATION_CPP
#include "formal-power-series.cpp"

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
