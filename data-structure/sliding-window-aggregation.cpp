/*
(https://motsu-xe.hatenablog.com/entry/2021/05/13/224016)
(https://nyaannyaan.github.io/library/data-structure/slide-window-aggregation.hpp.html)

    - 半群Gに対して，
        - new() : Gの元を要素とする空のqueueを生成
        - push(x) : queueにxを追加
        - pop() : queueの先頭の要素を削除する
        - fold() : queueに入っている要素の総積を求める
    を償却計算量O(1)で処理するデータ構造(SWAG)

    - 半群なので，逆元がとれなくてもよい・単位元がなくてもよい

    - SWAGのstack版は累積積を持っておけば良いので簡単
    - queueはstack2本で再現できる
        -> stack2本と元の値を持つstackでSWAGを作る

    - 渡す関数fは次のように定義して，型はdecltype(f)とする．
    auto f = [](const pair<int,int> &a, const pair<int,int> &b) {
        return p{a.first * b.first, a.second * b.first + b.second};
    };

    - verified with https://atcoder.jp/contests/abc125/submissions/60011788
*/

template <typename T, typename F>
struct SlideWindowAggregation {
  vector<T> a0, a1, r0, r1;
  F f;
  T I, f0, f1;

  SlideWindowAggregation(F _f, T _i) : f(_f), I(_i), f0(_i), f1(_i) {}

 private:
  void push_s0(const T &x) {
    a0.push_back(x);
    r0.push_back(f0 = f(x, f0));
  }
  void push_s1(const T &x) {
    a1.push_back(x);
    r1.push_back(f1 = f(f1, x));
  }
  void transfer() {
    while (!a1.empty()) {
      push_s0(a1.back());
      a1.pop_back();
    }
    while (!r1.empty()) r1.pop_back();
    f1 = I;
  }

 public:
  void push(const T &x) {
    if (a0.empty()) {
      push_s0(x);
      transfer();
    } else {
      push_s1(x);
    }
  }
  void pop() {
    if (a0.empty()) transfer();
    a0.pop_back();
    r0.pop_back();
    f0 = r0.empty() ? I : r0.back();
  }
  T query() { return f(f0, f1); }
};