/*
<Randomized Binary Search Tree / RBST>
    - 平衡二分探索木の一種
    - merge/split を基本操作として，乱択により木の高さを期待 O(log N) に保つ
    - 値の昇順に要素を管理する multiset 的な用途を想定する
    - 重複要素も保持できる

[実装/関数]
    - RBST<VAL> st
        空の RBST を作る

    - RBST<VAL> st(VAL sum_unity)
        sum の単位元を指定して RBST を作る

    - int size()
        現在の要素数を返す

    - bool empty()
        空かどうかを返す

    - VAL sum()
        木全体に含まれる値の総和を返す

    - int lowerBound(VAL val)
        val 以上である最初の要素の位置を返す
        すなわち，val 未満の要素数を返す

    - int upperBound(VAL val)
        val より大きい最初の要素の位置を返す
        すなわち，val 以下の要素数を返す

    - int count(VAL val)
        val と等しい要素の個数を返す

    - VAL get(int k)
        0-indexed で k 番目に小さい値を返す

    - pair<NODE*, NODE*> split(NODE* node, int k)
        node を根とする木を [0,k), [k,n) に分割する

    - RBST split(int k)
        現在の木を [0,k), [k,n) に分割する
        自身には [0,k) 側が残り，返り値として [k,n) 側を返す

    - NODE* merge(NODE* left, NODE* right)
        2つの木を併合する
        left の全要素 <= right の全要素 が必要

    - void merge(RBST& add)
        現在の木と add を併合する
        add 側は空になる

    - void insert(VAL val)
        val を1つ挿入する

    - bool erase(VAL val)
        val を1つ削除する
        削除できたら true，存在しなければ false を返す

    - void print()
        現在の要素を昇順に出力する

[計算量]
    - insert, erase
        期待 O(log N)

    - lowerBound, upperBound, count, get
        期待 O(log N)

    - split, merge
        期待 O(log N)

    - size, empty, sum
        O(1)

[備考]
    - VAL には比較演算子 <=, >= と，加算 + が必要
    - sum を使わない場合でも update 内で + を使っているので注意
    - merge(left, right) は，一般の2つのBSTを自動でソートして混ぜる操作ではない
      必ず left 側の全要素 <= right 側の全要素 が必要
    - new でノードを確保しているが delete はしていない
      競プロ用途では問題になりにくいが，長時間実行では注意

[使用例]
    RBST<long long> st;
    st.insert(5);
    st.insert(2);
    st.insert(5);

    st.count(5);       // 2
    st.get(0);         // 2
    st.get(1);         // 5
    st.lowerBound(5);  // 1
    st.upperBound(5);  // 3

    st.erase(5);       // 5 を1つ削除する
*/

template <class VAL>
struct RBST {
    struct NODE {
        NODE *left, *right;
        VAL val;
        int size;
        VAL sum;

        NODE(const VAL& v) : left(nullptr), right(nullptr), val(v), size(1), sum(v) {}

        inline void update() {
            // 追加情報を持たせたい場合はここに書く
        }

        inline void push() {
            // 遅延伝播を入れたい場合はここに書く
        }
    };

    NODE* root;
    VAL SUM_UNITY;

    RBST(VAL sum_unity = VAL()) : root(nullptr), SUM_UNITY(sum_unity) {}
    RBST(NODE* node, VAL sum_unity = VAL()) : root(node), SUM_UNITY(sum_unity) {}

    unsigned int randInt() {
        static unsigned int tx = 123456789;
        static unsigned int ty = 362436069;
        static unsigned int tz = 521288629;
        static unsigned int tw = 88675123;

        unsigned int tt = tx ^ (tx << 11);
        tx = ty;
        ty = tz;
        tz = tw;

        return tw = (tw ^ (tw >> 19)) ^ (tt ^ (tt >> 8));
    }

    int size(NODE* node) const {
        return node ? node->size : 0;
    }

    int size() const {
        return size(root);
    }

    bool empty() const {
        return size() == 0;
    }

    VAL sum(NODE* node) const {
        return node ? node->sum : SUM_UNITY;
    }

    VAL sum() const {
        return sum(root);
    }

    NODE* update(NODE* node) {
        if (!node) return node;

        node->size = size(node->left) + size(node->right) + 1;
        node->sum = sum(node->left) + node->val + sum(node->right);

        node->update();
        return node;
    }

    void push(NODE* node) {
        if (!node) return;
        node->push();
    }

    int lowerBound(NODE* node, const VAL& val) {
        if (!node) return 0;
        push(node);

        if (val <= node->val) {
            return lowerBound(node->left, val);
        } else {
            return size(node->left) + 1 + lowerBound(node->right, val);
        }
    }

    int lowerBound(const VAL& val) {
        return lowerBound(root, val);
    }

    int upperBound(NODE* node, const VAL& val) {
        if (!node) return 0;
        push(node);

        if (val >= node->val) {
            return size(node->left) + 1 + upperBound(node->right, val);
        } else {
            return upperBound(node->left, val);
        }
    }

    int upperBound(const VAL& val) {
        return upperBound(root, val);
    }

    int count(const VAL& val) {
        return upperBound(val) - lowerBound(val);
    }

    VAL get(NODE* node, int k) {
        assert(node);
        push(node);

        int lsize = size(node->left);

        if (k == lsize) return node->val;
        if (k < lsize) return get(node->left, k);
        return get(node->right, k - lsize - 1);
    }

    VAL get(int k) {
        assert(0 <= k && k < size());
        return get(root, k);
    }

    NODE* merge(NODE* left, NODE* right) {
        push(left);
        push(right);

        if (!left) return right;
        if (!right) return left;

        if (randInt() % (left->size + right->size) < (unsigned int)left->size) {
            left->right = merge(left->right, right);
            return update(left);
        } else {
            right->left = merge(left, right->left);
            return update(right);
        }
    }

    void merge(RBST& add) {
        root = merge(root, add.root);
        add.root = nullptr;
    }

    pair<NODE*, NODE*> split(NODE* node, int k) {
        // [0,k), [k,n)
        if (!node) return {nullptr, nullptr};
        push(node);

        if (k <= size(node->left)) {
            auto sub = split(node->left, k);
            node->left = sub.second;
            return {sub.first, update(node)};
        } else {
            auto sub = split(node->right, k - size(node->left) - 1);
            node->right = sub.first;
            return {update(node), sub.second};
        }
    }

    RBST split(int k) {
        assert(0 <= k && k <= size());

        auto sub = split(root, k);
        root = sub.first;

        return RBST(sub.second, SUM_UNITY);
    }

    void insert(const VAL& val) {
        int k = lowerBound(val);
        auto sub = split(root, k);

        NODE* node = new NODE(val);
        root = merge(merge(sub.first, node), sub.second);
    }

    bool erase(const VAL& val) {
        if (count(val) == 0) return false;

        int k = lowerBound(val);
        auto sub1 = split(root, k);
        auto sub2 = split(sub1.second, 1);

        // sub2.first が削除されるノード
        // delete はしない

        root = merge(sub1.first, sub2.second);
        return true;
    }

    void print(NODE* node) {
        if (!node) return;
        push(node);

        print(node->left);
        cout << node->val << " ";
        print(node->right);
    }

    void print() {
        cout << "{";
        print(root);
        cout << "}" << '\n';
    }
};