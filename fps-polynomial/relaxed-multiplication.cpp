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
#ifndef RELAXED_MULTIPLICATION_CPP
#define RELAXED_MULTIPLICATION_CPP
#include "formal-power-series.cpp"
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
