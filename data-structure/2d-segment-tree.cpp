/*
  <2次元セグメント木>
    - 二次元配列上において非可換なモノイドに関する演算を行う
        要素の一点更新と矩形型の漱石の取得ができる

    - ACLのsegtreeと同じように扱えるはず buildが必要な点に注意

    [実装/関数]
        - SegmentTree2D<T,op,e> seg(int h,int w) 
            op,eはそれぞれ二つの要素Tの積，単位元を表す関数
        - void init(int h,int w) : サイズをh*w, 要素を単位元に初期化する
        - void set(int i, int j, int x) : 配列をAとしてA_{i,j}の値を更新(buildの前)
        - void build() : データの構築
        - T get(int i, int j) : A_{i,j}の値を取得
        - void update(int i, int j, int x) : A_{i,j}の値をxに更新
        - void query(int i1, int j1, int i2, int j2) : 区間[A_{i1,j1}, A_{i2,j2})
            の総積を取得

    [計算時間]
        - build : O(hw)
        - set,get : O(1)
        - update,query : O(logh * logw)
    
    [備考]
        -
    
    [参照]
        - https://nyaannyaan.github.io/library/data-structure-2d/2d-segment-tree.hpp.html
        - ChatGPT(atcoder library と同様の使い方ができるように修正)

    [verified at]
        - 
        
*/


template <typename T>
struct SegmentTree2D {
 private:
  // 2次元セグメント木内部でのインデックス計算
  int id(int h, int w) const { return h * 2 * W + w; }

 public:
  int H, W;
  vector<T> seg;
  // 区間演算子 f と、単位元を返す関数 identity を保持
  const function<T(const T&, const T&)> f;
  const function<T()> identity;

  // コンストラクタ：行数 h, 列数 w, 演算子 _f, 単位元を返す関数 _identity を受け取る
  SegmentTree2D(int h, int w, function<T(const T&, const T&)> _f, function<T()> _identity)
      : f(_f), identity(_identity) {
    init(h, w);
  }

  // H, W を2の冪乗に拡大し、セグメント木の内部配列を初期化
  void init(int h, int w) {
    H = W = 1;
    while (H < h) H <<= 1;
    while (W < w) W <<= 1;
    seg.assign(4 * H * W, identity());
  }

  // リーフノードの値を設定する（build前に呼ぶ）
  void set(int h, int w, const T& x) { seg[id(h + H, w + W)] = x; }

  // セグメント木の構築
  void build() {
    // 縦方向（h方向）の構築
    for (int w = W; w < 2 * W; w++) {
      for (int h = H - 1; h > 0; h--) {
        seg[id(h, w)] = f(seg[id(2 * h, w)], seg[id(2 * h + 1, w)]);
      }
    }
    // 横方向（w方向）の構築
    for (int h = 0; h < 2 * H; h++) {
      for (int w = W - 1; w > 0; w--) {
        seg[id(h, w)] = f(seg[id(h, 2 * w)], seg[id(h, 2 * w + 1)]);
      }
    }
  }

  // 指定位置の値を取得
  T get(int h, int w) const { return seg[id(h + H, w + W)]; }
  T operator()(int h, int w) const { return get(h, w); }

  // 指定位置の値を更新し、上位の節点も再計算
  void update(int h, int w, const T& x) {
    h += H, w += W;
    seg[id(h, w)] = x;
    for (int i = h >> 1; i > 0; i >>= 1) {
      seg[id(i, w)] = f(seg[id(2 * i, w)], seg[id(2 * i + 1, w)]);
    }
    for (; h > 0; h >>= 1) {
      for (int j = w >> 1; j > 0; j >>= 1) {
        seg[id(h, j)] = f(seg[id(h, 2 * j)], seg[id(h, 2 * j + 1)]);
      }
    }
  }

  // 横方向の区間クエリの補助関数
  T _inner_query(int h, int w1, int w2) {
    T res = identity();
    for (; w1 < w2; w1 >>= 1, w2 >>= 1) {
      if (w1 & 1) { res = f(res, seg[id(h, w1)]); w1++; }
      if (w2 & 1) { w2--; res = f(res, seg[id(h, w2)]); }
    }
    return res;
  }

  // 半開区間 [h1, w1) から [h2, w2) のクエリ
  T query(int h1, int w1, int h2, int w2) {
    if (h1 >= h2 || w1 >= w2) return identity();
    T res = identity();
    h1 += H, h2 += H, w1 += W, w2 += W;
    for (; h1 < h2; h1 >>= 1, h2 >>= 1) {
      if (h1 & 1) { res = f(res, _inner_query(h1, w1, w2)); h1++; }
      if (h2 & 1) { h2--; res = f(res, _inner_query(h2, w1, w2)); }
    }
    return res;
  }
};

