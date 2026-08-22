/*
<binary trie> (https://kazuma8128.hatenablog.com/entry/2018/05/06/022654)
    - 整数をbit列と見做してtrie木の要領でsetみたいなことができるデータ構造


    - 各nodeはcnt,ch[0],ch[1]を持つ
        - cnt : そのノード以下の部分木に存在する要素数
        - ch[0] : 次のnodeへの0側のポインタ
        - ch[1] : 次のnodeへの1側のポインタ


[できること] ビット長(固定)をBとすると，次の操作ができる
        ※ 同じ値を複数個入れられるので，setというよりmultisetに近い
        ※ xor_all(x)
後は，各関数は「全要素にxがxorされた後の集合」に対して動作する

        - binary_trie() : コンストラクタ
            binary_trie<unsigned, 32> などとする

        - int size() : 集合に入っている要素数を返す
        - bool empty() : 集合が空かどうかを返す

        - void insert(U x) : 値xを集合に一つ挿入する
            計算量 O(B)

        - void erase(U x) : 値xを集合から一つ削除する
            計算量 O(B)
            ただし，xが存在しない場合はassertで落ちる

        - bool erase_one(U x) : 値xを集合から一つ削除する
            計算量 O(B)
            xが存在して削除できた場合はtrue，存在しない場合はfalseを返す

        - void xor_all(U x) : 集合内の全ての要素yを y xor x に置き換える
            遅延評価により計算量 O(1)

        - U min_element() : 集合内の最小値を返す
            計算量 O(B)

        - U max_element() : 集合内の最大値を返す
            計算量 O(B)

        - U min_element(U x) : 集合内の値yの中で，y xor x が最小となるyを返す
            y xor x の値ではなく，集合の元yを返す
            計算量 O(B)

        - U max_element(U x) : 集合内の値yの中で，y xor x が最大となるyを返す
            y xor x の値ではなく，集合の元yを返す
            計算量 O(B)

        - int lower_bound(U x) : 集合内でx以上となる最小の要素の番号を返す
            つまり，x未満の要素数を返す
            要素を小さい順に0-indexedで並べたときの，最初にx以上となる位置
            該当する要素がない場合はsize()を返す
            計算量 O(B)

        - int upper_bound(U x) : 集合内でxより大きい最小の要素の番号を返す
            つまり，x以下の要素数を返す
            要素を小さい順に0-indexedで並べたときの，最初にxより大きくなる位置
            該当する要素がない場合はsize()を返す
            計算量 O(B)

        - U operator[](int k) : k番目に小さい要素を返す
            0-indexed
            計算量 O(B)

        - U kth_element(U x, int k) :
            集合内の各要素yを y xor x に変換した値で小さい順に並べたとき，
            k番目に来る要素の，xorする前の値yを返す
            0-indexed
            計算量 O(B)

        - int count(U x) : 値xが集合内に何個入っているかを返す
            count(x, B) と同じ
            計算量 O(B)

        - int count(U x, int D) :
            深さDのノードに対応する要素数を返す
            すなわち，xの上位Dビットと一致する要素が何個入っているかを返す
            D=0なら全要素数，D=Bなら値xそのものの個数を返す
            0 <= D <= B
            計算量 O(D)

        - void dfs(F f) :
            集合内の相異なる要素を小さい順に列挙する
            各要素valueについて f(value, cnt) を呼び出す
            cntはvalueの重複個数
            計算量 O(B * 相異なる要素数)

        - void dfs(Go go, F f) :
            条件付きでDFSする
            各ノードで go(prefix, depth, cnt) を呼び，
            falseが返った場合はその部分木を探索しない
            葉では f(value, cnt) を呼ぶ

            prefix : 現在のノードまでで確定した上位ビット
            depth  : 現在の深さ
            cnt    : その部分木に含まれる要素数

            条件を満たす範囲だけを探索したいときに使える
            計算量 O(B * 訪問した葉の数 + 訪問した内部ノード数)

            bt.dfs([&](unsigned prefix, int depth, int cnt){~~},
                [&](unsigned value, int cnt){~~}
            );


    [構造]
        - ダミーノードは最初のinsertで生成(それまではnullptr)
        - 木の根元にダミーノードが入っており，その後ろが二分の構造になっている
        - B : 格納する値のビット長 (初期値は32)
        - value : 1->1->0としたら110が入っている 根のvalueは-1
        - depth : 1->1としたら深さは2 (実際にはダミーノード * -> 1 -> 1)
        - 木の根側に上位ビットが入っている
*/

template<typename U = unsigned, int B = 32>
class binary_trie {
    static_assert(std::is_unsigned<U>::value, "U must be an unsigned integer type");
    static_assert(0 < B && B <= std::numeric_limits<U>::digits, "invalid bit length B");

    struct node {
        int cnt;
        node *ch[2];
        node() : cnt(0), ch{nullptr, nullptr} {}
    };

    node *root;
    U lazy; // 実際の値 = trie内の値 ^ lazy

    static constexpr int W = std::numeric_limits<U>::digits;

    static constexpr U mask_value() {
        if constexpr (B == W) return ~U(0);
        else return (U(1) << B) - U(1);
    }

    static constexpr U norm(U x) {
        return x & mask_value();
    }

    static int size_of(node *t) {
        return t ? t->cnt : 0;
    }

    static void delete_tree(node *t) {
        if (!t) return;
        delete_tree(t->ch[0]);
        delete_tree(t->ch[1]);
        delete t;
    }

    static node* clone(node *t) {
        if (!t) return nullptr;
        node *r = new node;
        r->cnt = t->cnt;
        r->ch[0] = clone(t->ch[0]);
        r->ch[1] = clone(t->ch[1]);
        return r;
    }

    node* add(node *t, U val, int b = B - 1) {
        if (!t) t = new node;
        t->cnt += 1;
        if (b < 0) return t;

        int f = (val >> b) & U(1);
        t->ch[f] = add(t->ch[f], val, b - 1);
        return t;
    }

    node* sub(node *t, U val, int b = B - 1) {
        assert(t && t->cnt > 0);

        t->cnt -= 1;

        if (t->cnt == 0) {
            delete_tree(t);
            return nullptr;
        }

        if (b < 0) return t;

        int f = (val >> b) & U(1);
        t->ch[f] = sub(t->ch[f], val, b - 1);
        return t;
    }

    int count_stored(node *t, U val, int b = B - 1) const {
        if (!t) return 0;
        if (b < 0) return t->cnt;

        int f = (val >> b) & U(1);
        return count_stored(t->ch[f], val, b - 1);
    }

    U get_best_stored(node *t, U bias, bool maximize, int b = B - 1) const {
        assert(t);
        if (b < 0) return U(0);

        int bit = (bias >> b) & U(1);

        // minimize の場合: stored_bit ^ bias_bit を 0 にしたい
        // maximize の場合: stored_bit ^ bias_bit を 1 にしたい
        int first = bit ^ (maximize ? 1 : 0);

        if (!t->ch[first]) first ^= 1;

        return get_best_stored(t->ch[first], bias, maximize, b - 1)
             | (U(first) << b);
    }

    U kth_by_xor_stored(node *t, int k, U bias, int b = B - 1) const {
        assert(t && 0 <= k && k < t->cnt);

        if (b < 0) return U(0);

        // stored ^ bias の値で昇順に見たい。
        // そのbitを0にするstored_bitは bias_bit。
        int first = (bias >> b) & U(1);

        int m = size_of(t->ch[first]);

        if (k < m) {
            return kth_by_xor_stored(t->ch[first], k, bias, b - 1)
                 | (U(first) << b);
        } else {
            first ^= 1;
            return kth_by_xor_stored(t->ch[first], k - m, bias, b - 1)
                 | (U(first) << b);
        }
    }

    int count_less(node *t, U val, int b = B - 1) const {
        if (!t || b < 0) return 0;

        int v = (val >> b) & U(1);
        int l = (lazy >> b) & U(1);

        // logical_bit = stored_bit ^ lazy_bit
        if (v == 0) {
            // logical_bit = 0 の方だけ見る
            return count_less(t->ch[l], val, b - 1);
        } else {
            // logical_bit = 0 は全部小さい
            // logical_bit = 1 は再帰
            return size_of(t->ch[l]) + count_less(t->ch[l ^ 1], val, b - 1);
        }
    }

    int count_leq(node *t, U val, int b = B - 1) const {
        if (!t) return 0;
        if (b < 0) return t->cnt;

        int v = (val >> b) & U(1);
        int l = (lazy >> b) & U(1);

        if (v == 0) {
            return count_leq(t->ch[l], val, b - 1);
        } else {
            return size_of(t->ch[l]) + count_leq(t->ch[l ^ 1], val, b - 1);
        }
    }

    template<class Go, class F>
    void dfs_impl(node *t, U prefix, int depth, Go &go, F &f) const {
        if (!t) return;

        // prefix: 上位 depth bit が確定した値。下位bitは0。
        // depth : 根からの深さ。0 <= depth <= B
        // cnt   : この部分木に含まれる要素数。
        if (!go(prefix, depth, t->cnt)) return;

        if (depth == B) {
            f(prefix, t->cnt);
            return;
        }

        int b = B - 1 - depth;
        int l = (lazy >> b) & U(1);

        // logicalな昇順で見る
        for (int logical_bit = 0; logical_bit < 2; logical_bit++) {
            int stored_bit = logical_bit ^ l;
            U nxt = prefix | (U(logical_bit) << b);
            dfs_impl(t->ch[stored_bit], nxt, depth + 1, go, f);
        }
    }

public:
    binary_trie() : root(nullptr), lazy(0) {}

    ~binary_trie() {
        delete_tree(root);
    }

    binary_trie(const binary_trie &other)
        : root(clone(other.root)), lazy(other.lazy) {}

    binary_trie(binary_trie &&other) noexcept
        : root(other.root), lazy(other.lazy) {
        other.root = nullptr;
        other.lazy = 0;
    }

    binary_trie& operator=(binary_trie other) noexcept {
        std::swap(root, other.root);
        std::swap(lazy, other.lazy);
        return *this;
    }

    int size() const {
        return size_of(root);
    }

    bool empty() const {
        return size() == 0;
    }

    void insert(U val) {
        root = add(root, norm(val ^ lazy));
    }

    bool erase_one(U val) {
        U stored = norm(val ^ lazy);

        if (count_stored(root, stored) == 0) return false;

        root = sub(root, stored);
        return true;
    }

    void erase(U val) {
        bool ok = erase_one(val);
        assert(ok);
    }

    // 全要素に x を xor する。O(1)
    void xor_all(U x) {
        lazy = norm(lazy ^ x);
    }

    // 集合内の値 y のうち、y ^ bias が最小になる y を返す。
    U min_element(U bias = 0) const {
        assert(root);
        U stored = get_best_stored(root, norm(lazy ^ bias), false);
        return norm(stored ^ lazy);
    }

    // 集合内の値 y のうち、y ^ bias が最大になる y を返す。
    U max_element(U bias = 0) const {
        assert(root);
        U stored = get_best_stored(root, norm(lazy ^ bias), true);
        return norm(stored ^ lazy);
    }

    // x 未満の要素数。
    // つまり、lower_bound の index を返す。
    int lower_bound(U val) const {
        if constexpr (B < W) {
            if (val > mask_value()) return size();
        }
        return count_less(root, norm(val));
    }

    // x 以下の要素数。
    // つまり、upper_bound の index を返す。
    int upper_bound(U val) const {
        if constexpr (B < W) {
            if (val > mask_value()) return size();
        }
        return count_leq(root, norm(val));
    }

    // k番目に小さい要素を返す。
    U operator[](int k) const {
        assert(0 <= k && k < size());
        return kth_element(0, k);
    }

    // 全要素 y を y ^ x にしたとき、
    // 小さい方から k 番目になる「元の y」を返す。
    //
    // つまり、返り値 ans は集合内の元で、
    // ans ^ x が k 番目に小さい。
    U kth_element(U x, int k) const {
        assert(0 <= k && k < size());

        U stored = kth_by_xor_stored(root, k, norm(lazy ^ x));
        return norm(stored ^ lazy);
    }

    // 深さDのprefixに対応する部分木の個数を返す。
    //
    // D=0 : 全体の個数
    // D=B : valそのものの個数
    //
    // 例: B=4, val=1011, D=2 なら、
    // 上位2bitが10である要素数を返す。
    int count(U val, int D = B) const {
        assert(0 <= D && D <= B);

        node *t = root;

        for (int i = B - 1; i >= B - D; --i) {
            if (!t) return 0;

            int logical_bit = (val >> i) & U(1);
            int l = (lazy >> i) & U(1);

            t = t->ch[logical_bit ^ l];
        }

        return size_of(t);
    }

    // 全要素を昇順に列挙する。
    // f(value, multiplicity) が呼ばれる。
    template<class F>
    void dfs(F f) const {
        auto go = [](U, int, int) { return true; };
        dfs_impl(root, U(0), 0, go, f);
    }

    // 条件付きDFS。
    //
    // go(prefix, depth, cnt) が false を返すと、その部分木を探索しない。
    // f(value, multiplicity) が葉で呼ばれる。
    template<class Go, class F>
    void dfs(Go go, F f) const {
        dfs_impl(root, U(0), 0, go, f);
    }
};

// template<typename U = unsigned, int B = 32>
// class binary_trie {
//     struct node {
//         int cnt;
//         node *ch[2];
//         node() : cnt(0), ch{ nullptr, nullptr } {}
//     };
//     node* add(node* t, U val, int b = B - 1) {
//         if (!t) t = new node;
//         t->cnt += 1;
//         if (b < 0) return t;
//         bool f = (val >> (U)b) & (U)1;
//         t->ch[f] = add(t->ch[f], val, b - 1);
//         return t;
//     }
//     node* sub(node* t, U val, int b = B - 1) {
//         assert(t);
//         t->cnt -= 1;
//         if (t->cnt == 0) return nullptr;
//         if (b < 0) return t;
//         bool f = (val >> (U)b) & (U)1;
//         t->ch[f] = sub(t->ch[f], val, b - 1);
//         return t;
//     }
//     U get_min(node* t, U val, int b = B - 1) const {
//         assert(t);
//         if (b < 0) return 0;
//         bool f = (val >> (U)b) & (U)1; f ^= !t->ch[f];
//         return get_min(t->ch[f], val, b - 1) | ((U)f << (U)b);
//     }
//     U get(node* t, int k, int b = B - 1) const {
//         if (b < 0) return 0;
//         int m = t->ch[0] ? t->ch[0]->cnt : 0;
//         return k < m ? get(t->ch[0], k, b - 1) : get(t->ch[1], k - m, b - 1) | ((U)1 << (U)b);
//     }
//     int count_lower(node* t, U val, int b = B - 1) {
//         if (!t || b < 0) return 0;
//         bool f = (val >> (U)b) & (U)1;
//         return (f && t->ch[0] ? t->ch[0]->cnt : 0) + count_lower(t->ch[f], val, b - 1);
//     }
//     node *root;
// public:
//     binary_trie() : root(nullptr) {}
//     int size() const {
//         return root ? root->cnt : 0;
//     }
//     bool empty() const {
//         return !root;
//     }
//     void insert(U val) {
//         root = add(root, val);
//     }
//     void erase(U val) {
//         root = sub(root, val);
//     }
//     U max_element(U bias = 0) const {
//         return get_min(root, ~bias);
//     }
//     U min_element(U bias = 0) const {
//         return get_min(root, bias);
//     }
//     int lower_bound(U val) { // return id
//         return count_lower(root, val);
//     }
//     int upper_bound(U val) { // return id
//         return count_lower(root, val + 1);
//     }
//     U operator[](int k) const {
//         assert(0 <= k && k < size());
//         return get(root, k);
//     }
//     int count(U val, int D) const {
//         if (!root) return 0;
//         node *t = root;
//         for (int i = D - 1; i >= 0; i--) {
//             t = t->ch[(val >> (U)i) & (U)1];
//             if (!t) return 0;
//         }
//         return t->cnt;
//     }
// };