/*
  <Merge Sort Tree / マージソート木>
- 静的な配列 A に対して，区間 [l,r) に含まれる値の個数を
  値の大小関係を条件として高速に数える
- 各セグメント木ノードが，担当区間の要素を昇順に並べた配列を持つ
- 要素の更新には対応しない

    [問題]
- 配列 A = {A[0], A[1], ..., A[N-1]} に対して，次のクエリを扱う

    1. 区間 [l,r) にある x 以下の要素数
    2. 区間 [l,r) にある x 未満の要素数
    3. 区間 [l,r) にある low 以上 high 以下の要素数

    [原理]
- 通常のセグメント木と同様に，配列の区間を二分木で管理する

- 葉には対応する A[i] だけを入れ，内部ノードには
  左右の子が持つソート済み配列をマージしたものを保存する

- クエリ区間 [l,r) は O(log N) 個のノードに分解できる
  各ノードのソート済み配列に対して二分探索を行うことで，
  条件を満たす要素数を数える

- x 以下の要素数は upper_bound(x) の位置，
  x 未満の要素数は lower_bound(x) の位置に等しい

- low <= A[i] <= high の要素数は

      (high 以下の要素数) - (low 未満の要素数)

  として求める

    [テンプレート引数]
- template<class T> struct MergeSortTree

    T は次の操作に対応している必要がある

    - コピーまたはムーブによって vector<T> に格納できる
    - operator< によって大小比較できる

  std::merge，std::lower_bound，std::upper_bound が
  デフォルトの比較方法を使用するため，operator< は
  strict weak orderingを満たす必要がある

    [構築・再構築]
- MergeSortTree()
    デフォルト構築する
    使用前に build(a) を呼ぶ必要がある

- explicit MergeSortTree(const vector<T>& a)
    配列 a からマージソート木を構築する

- void build(const vector<T>& a)
    現在の内容を破棄し，配列 a から再構築する
    空配列からの構築も可能

    [クエリ]
- int count_leq(int l, int r, const T& x) const
    区間 [l,r) にある x 以下の要素数を返す

        |{i | l <= i < r, A[i] <= x}|

- int count_less(int l, int r, const T& x) const
    区間 [l,r) にある x 未満の要素数を返す

        |{i | l <= i < r, A[i] < x}|

- int count_between(
      int l,
      int r,
      const T& low,
      const T& high
  ) const

    区間 [l,r) にある low 以上 high 以下の要素数を返す

        |{i | l <= i < r, low <= A[i] <= high}|

    high < low の場合は0を返す

    [区間の規約]
- すべてのクエリ区間は半開区間 [l,r) である

- 通常は次を満たす範囲で呼び出す

      0 <= l <= r <= A.size()

- 内部変数 n は，A.size() 以上の最小の2冪であり，
  元配列の要素数そのものではない

- 実装中のassertは r <= n を検査しているため，
  A.size() < r <= n の呼び出しもassertを通る
  この範囲は空の補助葉として扱われるが，配列 A の通常の
  区間クエリとしては r <= A.size() を呼び出し側で守ること

    [計算時間]
- N = A.size() とする

- 構築:
        O(N log N)

- count_leq:
        O(log^2 N)

- count_less:
        O(log^2 N)

- count_between:
        O(log^2 N)

  クエリ区間を O(log N) 個のノードに分解し，
  各ノードで O(log N) の二分探索を行う

    [メモリ]
- O(N log N)

  各階層に全要素が高々1回ずつ現れ，
  階層数が O(log N) である

    [要件・注意]
- 構築後に元配列を変更しても，木の内容は変化しない
  新しい内容を反映するには build(a) で再構築すること

- 1点更新や区間更新には対応していない

- 添字と返り値はintで管理している
  配列長およびクエリ結果がintの範囲に収まることを前提とする

- assertを有効にした状態では，不正な区間に対して停止する
  ただし，上記の通り元配列長は保存していないため，
  r <= A.size() は呼び出し側で保証する

- このファイル単体にはinclude文とusing宣言がない
  使用側で少なくとも次に相当する宣言が必要である

      #include <algorithm>
      #include <cassert>
      #include <vector>
      using namespace std;

    [使用例]
        vector<int> a = {5, 1, 4, 2, 3, 2};
        MergeSortTree<int> tree(a);

        // A[1..5) = {1,4,2,3} にある3以下の要素数
        cout << tree.count_leq(1, 5, 3) << '\n';       // 3

        // A[0..6)にある2未満の要素数
        cout << tree.count_less(0, 6, 2) << '\n';      // 1

        // A[0..6)にある2以上4以下の要素数
        cout << tree.count_between(0, 6, 2, 4) << '\n'; // 4

    [verified at]
- 未検証
*/

template<class T>
struct MergeSortTree {
    int n;
    vector<vector<T>> seg;

    MergeSortTree() = default;

    explicit MergeSortTree(const vector<T>& a) {
        build(a);
    }

    void build(const vector<T>& a) {
        int sz = static_cast<int>(a.size());

        n = 1;
        while (n < sz) n <<= 1;

        seg.assign(2 * n, {});

        for (int i = 0; i < sz; ++i) {
            seg[n + i] = {a[i]};
        }

        for (int k = n - 1; k >= 1; --k) {
            const auto& left = seg[2 * k];
            const auto& right = seg[2 * k + 1];

            seg[k].resize(left.size() + right.size());

            merge(
                left.begin(), left.end(),
                right.begin(), right.end(),
                seg[k].begin()
            );
        }
    }

    // [l, r) にある x 以下の要素数
    int count_leq(int l, int r, const T& x) const {
        assert(0 <= l && l <= r && r <= n);

        int answer = 0;

        l += n;
        r += n;

        while (l < r) {
            if (l & 1) {
                answer += upper_bound(
                    seg[l].begin(), seg[l].end(), x
                ) - seg[l].begin();
                ++l;
            }

            if (r & 1) {
                --r;
                answer += upper_bound(
                    seg[r].begin(), seg[r].end(), x
                ) - seg[r].begin();
            }

            l >>= 1;
            r >>= 1;
        }

        return answer;
    }

    // [l, r) にある x 未満の要素数
    int count_less(int l, int r, const T& x) const {
        assert(0 <= l && l <= r && r <= n);

        int answer = 0;

        l += n;
        r += n;

        while (l < r) {
            if (l & 1) {
                answer += lower_bound(
                    seg[l].begin(), seg[l].end(), x
                ) - seg[l].begin();
                ++l;
            }

            if (r & 1) {
                --r;
                answer += lower_bound(
                    seg[r].begin(), seg[r].end(), x
                ) - seg[r].begin();
            }

            l >>= 1;
            r >>= 1;
        }

        return answer;
    }

    // [l, r) にある low <= A[i] <= high の要素数
    int count_between(
        int l,
        int r,
        const T& low,
        const T& high
    ) const {
        if (high < low) return 0;

        return count_leq(l, r, high)
             - count_less(l, r, low);
    }
};
