/*
<Wavelet Matrix>
    非負整数列 a に対して，値のビットごとに分解して管理するデータ構造．
    区間内の k 番目に小さい値，ある値未満の個数，前後の値などを
    O(log A) で求められる．
    ここで A は列に含まれる最大値程度であり，実装上は最大値のビット長 lg を用いる．

    内部では bit_vector を用いる．

<bit_vector>
    ビット列 B に対して以下の操作を提供する．

    - get(i)
        : B[i] を返す．

    - set(i)
        : B[i] を 1 にする．

    - build()
        : rank を高速に求めるための前計算を行う．

    - rank1(i)
        : B[0, i) に含まれる 1 の個数を返す．

    - rank0(i)
        : B[0, i) に含まれる 0 の個数を返す．

    ※ この実装には select はない．

<WaveletMatrix<T>>
    非負整数列 a に対して以下の操作を提供する．
    区間はすべて半開区間 [l, r) で扱う．

    - WaveletMatrix(n)
        : 長さ n の Wavelet Matrix を作る．
          set(i, x) で値を代入した後，build() を呼ぶ．

    - WaveletMatrix(vector<T> a)
        : 配列 a から Wavelet Matrix を構築する．

    - set(i, x)
        : i 番目の要素に x を代入する．
          x は 0 以上である必要がある．
          build() 前に使う．

    - build()
        : データ構造を構築する．
          基本的に build() 後は set() で値を変更しない．

    - access(k)
        : a[k] を復元して返す．
          計算量 O(log A)．

    - kth_smallest(l, r, k)
        : a[l, r) の中で k 番目に小さい値を返す．
          k は 0-indexed．
          計算量 O(log A)．

    - kth_largest(l, r, k)
        : a[l, r) の中で k 番目に大きい値を返す．
          k は 0-indexed．
          計算量 O(log A)．

    - range_freq(l, r, upper)
        : a[l, r) の中で upper 未満の要素数を返す．
          つまり，
              #{ i | l <= i < r, a[i] < upper }
          を返す．
          計算量 O(log A)．

    - range_freq(l, r, lower, upper)
        : a[l, r) の中で lower <= a[i] < upper を満たす要素数を返す．
          計算量 O(log A)．

    - prev_value(l, r, upper)
        : a[l, r) の中で upper 未満の値のうち最大のものを返す．
          存在しない場合は T(-1) を返す．
          計算量 O(log A)．

    - next_value(l, r, lower)
        : a[l, r) の中で lower 以上の値のうち最小のものを返す．
          存在しない場合は T(-1) を返す．
          計算量 O(log A)．

<計算量>
    - build()
        : O(N log A)

    - access(k)
        : O(log A)

    - kth_smallest(l, r, k)
        : O(log A)

    - kth_largest(l, r, k)
        : O(log A)

    - range_freq(l, r, upper)
        : O(log A)

    - range_freq(l, r, lower, upper)
        : O(log A)

    - prev_value(l, r, upper)
        : O(log A)

    - next_value(l, r, lower)
        : O(log A)

<備考>
    - この実装では，列の要素は 0 以上であることを仮定する．
    - 区間は [l, r) で扱う．
    - k は 0-indexed．
    - rank_c, select_c, topk, sum, rangelist, intersect などは
      このコードには実装されていない．
    - T(-1) を番兵として返す関数があるため，T を unsigned 型にすると扱いに注意が必要．
*/


// 参照:https://miti-7.hatenablog.com/entry/2018/04/28/152259#%E3%81%84%E3%82%8D%E3%81%84%E3%82%8D%E3%81%AA%E4%BA%BA%E3%81%AE%E5%AE%9F%E8%A3%85


struct bit_vector {
  using u32 = uint32_t;
  using i64 = int64_t;
  using u64 = uint64_t;

  static constexpr u32 w = 64;
  vector<u64> block;
  vector<u32> count;
  u32 n, zeros;

  inline u32 get(u32 i) const { return u32(block[i / w] >> (i % w)) & 1u; }
  inline void set(u32 i) { block[i / w] |= 1LL << (i % w); }

  bit_vector() {}
  bit_vector(int _n) { init(_n); }
  __attribute__((optimize("O3", "unroll-loops"))) void init(int _n) {
    n = zeros = _n;
    block.resize(n / w + 1, 0);
    count.resize(block.size(), 0);
  }

  //__attribute__((target("popcnt"))) 
  void build() {
    for (u32 i = 1; i < block.size(); ++i)
      count[i] = count[i - 1] + __builtin_popcountll(block[i - 1]);
    zeros = rank0(n);
  }

  inline u32 rank0(u32 i) const { return i - rank1(i); }
  //__attribute__((target("bmi2,popcnt"))) 
  inline u32 rank1(u32 i) const {
    //return count[i / w] +__builtin_popcountll(__builtin_ia32_bzhi_u64(block[i / w], i % w));
    u64 x = block[i / w] & ((1LL << (i % w)) - 1);
    return count[i / w] + __builtin_popcountll(x);
  }
};

template <typename T>
struct WaveletMatrix {
  using u32 = uint32_t;
  using i64 = int64_t;
  using u64 = uint64_t;

  int n, lg;
  vector<T> a;
  vector<bit_vector> bv;

  WaveletMatrix(u32 _n) : n(max<u32>(_n, 1)), a(n) {}
  WaveletMatrix(const vector<T>& _a) : n(_a.size()), a(_a) { build(); }

  __attribute__((optimize("O3"))) void build() {
    lg = __lg(max<T>(*max_element(begin(a), end(a)), 1)) + 1;
    bv.assign(lg, n);
    vector<T> cur = a, nxt(n);
    for (int h = lg - 1; h >= 0; --h) {
      for (int i = 0; i < n; ++i)
        if ((cur[i] >> h) & 1) bv[h].set(i);
      bv[h].build();
      array<decltype(begin(nxt)), 2> it{begin(nxt), begin(nxt) + bv[h].zeros};
      for (int i = 0; i < n; ++i) *it[bv[h].get(i)]++ = cur[i];
      swap(cur, nxt);
    }
    return;
  }

  void set(u32 i, const T& x) { 
    assert(x >= 0);
    a[i] = x; 
  }

  inline pair<u32, u32> succ0(int l, int r, int h) const {
    return make_pair(bv[h].rank0(l), bv[h].rank0(r));
  }

  inline pair<u32, u32> succ1(int l, int r, int h) const {
    u32 l0 = bv[h].rank0(l);
    u32 r0 = bv[h].rank0(r);
    u32 zeros = bv[h].zeros;
    return make_pair(l + zeros - l0, r + zeros - r0);
  }

  // return a[k]
  T access(u32 k) const {
    T ret = 0;
    for (int h = lg - 1; h >= 0; --h) {
      u32 f = bv[h].get(k);
      ret |= f ? T(1) << h : 0;
      k = f ? bv[h].rank1(k) + bv[h].zeros : bv[h].rank0(k);
    }
    return ret;
  }

  // k-th (0-indexed) smallest number in a[l, r)
  T kth_smallest(u32 l, u32 r, u32 k) const {
    T res = 0;
    for (int h = lg - 1; h >= 0; --h) {
      u32 l0 = bv[h].rank0(l), r0 = bv[h].rank0(r);
      if (k < r0 - l0)
        l = l0, r = r0;
      else {
        k -= r0 - l0;
        res |= (T)1 << h;
        l += bv[h].zeros - l0;
        r += bv[h].zeros - r0;
      }
    }
    return res;
  }

  // k-th (0-indexed) largest number in a[l, r)
  T kth_largest(int l, int r, int k) {
    return kth_smallest(l, r, r - l - k - 1);
  }

  // count i s.t. (l <= i < r) && (v[i] < upper)
  int range_freq(int l, int r, T upper) {
    if (upper >= (T(1) << lg)) return r - l;
    int ret = 0;
    for (int h = lg - 1; h >= 0; --h) {
      bool f = (upper >> h) & 1;
      u32 l0 = bv[h].rank0(l), r0 = bv[h].rank0(r);
      if (f) {
        ret += r0 - l0;
        l += bv[h].zeros - l0;
        r += bv[h].zeros - r0;
      } else {
        l = l0;
        r = r0;
      }
    }
    return ret;
  }

  int range_freq(int l, int r, T lower, T upper) {
    return range_freq(l, r, upper) - range_freq(l, r, lower);
  }

  // max v[i] s.t. (l <= i < r) && (v[i] < upper)
  T prev_value(int l, int r, T upper) {
    int cnt = range_freq(l, r, upper);
    return cnt == 0 ? T(-1) : kth_smallest(l, r, cnt - 1);
  }

  // min v[i] s.t. (l <= i < r) && (lower <= v[i])
  T next_value(int l, int r, T lower) {
    int cnt = range_freq(l, r, lower);
    return cnt == r - l ? T(-1) : kth_smallest(l, r, cnt);
  }
};


