/*
  <Convex-hull-trick>
    - 直線集合を管理するデータ構造で、次の2種類のクエリをオンラインで処理する。
        1. 直線の追加クエリ: f(x)=ax+bを追加
        2. 最大/最小値の取得クエリ: ある与えられたxに対して、最大/最小のf(x)を求める

    [実装/関数]
        - convex_hull_trick<T,id> conv(vec); (idは型Tにおけるconst値 -inf) 
        - insert(T a, T b) : 直線 ax + b を加える
        - get(T x) : max(ax+b) を返す (minを得たい場合は，追加時に-a,-bを渡し，getで得られた値を-1倍する)
    
    [計算時間]
        - insertとgetともにO(log N) (Nは追加した直線の本数)
    
    [備考]
        - 中国ではLi Chao Segment Tree と呼ばれているらしい
        - 
    
    [参照]
        - https://kazuma8128.hatenablog.com/entry/2018/02/28/102130

    [verified at]
        - https://atcoder.jp/contests/dp/submissions/60653609
    
    [使用例]
        
        
*/


template <typename T, const T id>
class convex_hull_trick {
    struct line {
        T a, b;
        line(T a_ = 0, T b_ = 0) : a(a_), b(b_) {}
        T get(T x) { return a * x + b; }
    };
    struct node {
        line l;
        node *lch, *rch;
        node(line l_) : l(l_), lch(nullptr), rch(nullptr) {}
        ~node() {
            if (lch) delete lch;
            if (rch) delete rch;
        }
    };

private:
    const int n;
    const vector<T> pos;
    node *root;

public:
    convex_hull_trick(const vector<T>& pos_) : n(pos_.size()), pos(pos_), root(nullptr) {}
    ~convex_hull_trick() {
        if (root) delete root;
    }
    void insert(T a, T b) {
        line l(a, b);
        root = modify(root, 0, n - 1, l);
    }
    T get(T x) const {
        int t = lower_bound(pos.begin(), pos.end(), x) - pos.begin();
        assert(t < n && pos[t] == x);
        return sub(root, 0, n - 1, t);
    }

private:
    node* modify(node *p, int lb, int ub, line& l) {
        if (!p) return new node(l);
        if (p->l.get(pos[lb]) >= l.get(pos[lb]) && p->l.get(pos[ub]) >= l.get(pos[ub])) return p;
        if (p->l.get(pos[lb]) <= l.get(pos[lb]) && p->l.get(pos[ub]) <= l.get(pos[ub])) {
            p->l = l;
            return p;
        }
        int c = (lb + ub) / 2;
        if (p->l.get(pos[c]) < l.get(pos[c])) swap(p->l, l);
        if (p->l.get(pos[lb]) <= l.get(pos[lb]))
            p->lch = modify(p->lch, lb, c, l);
        else
            p->rch = modify(p->rch, c + 1, ub, l);
        return p;
    }
    T sub(node *p, int lb, int ub, int t) const {
        if (!p) return id;
        if (ub - lb == 0) return p->l.get(pos[t]);
        int c = (lb + ub) / 2;
        if (t <= c) return max(p->l.get(pos[t]), sub(p->lch, lb, c, t));
        return max(p->l.get(pos[t]), sub(p->rch, c + 1, ub, t));
    }
};