/*
  <削除可能 Union-Find / Dynamic Connectivity>
    - 無向単純グラフに対して，辺の追加・削除を行いながら連結性を管理する
    - Euler Tour Tree + level 構造による完全動的連結性
    - Holm-de Lichtenberg-Thorup 型の方針

    [機能]
        - 辺の追加
        - 辺の削除
        - 2 頂点の連結判定
        - 頂点 s を含む連結成分のサイズ
        - 頂点 s を含む連結成分の頂点列挙
        - 頂点値への加算
        - 頂点 s を含む連結成分の値の総和
        - 連結成分数の取得

    [実装/関数]
        - DynamicConnectivity<T> dc(int n)
            n 頂点の空グラフを作る

        - bool dc.link(int s, int t)
            無向辺 {s,t} を追加する
            返り値は「この辺によって連結成分が 1 つ減ったか」
            すでに s,t が連結なら false を返すが，辺自体は非木辺として追加される

        - bool dc.cut(int s, int t)
            無向辺 {s,t} を削除する
            返り値は「この削除によって s,t が非連結になったか」
            削除しても別経路で連結なら false

        - bool dc.same(int s, int t)
            s,t が連結かを返す

        - int dc.size(int s)
            s を含む連結成分の頂点数を返す

        - vector<int> dc.get_vertex(int s)
            s を含む連結成分の頂点集合を返す
            順序は保証しない

        - void dc.update(int s, T x)
            頂点 s の値に x を加える
            val[s] <- val[s] + x

        - T dc.get_sum(int s)
            s を含む連結成分の頂点値の総和を返す

        - int dc.count_components()
            現在の連結成分数を返す

    [計算時間]
        - link, cut
            amortized O(log^2 N)

        - same, size, get_sum
            O(log N)

        - get_vertex
            O(K log N) 程度
            K は連結成分サイズ

    [要件]
        - グラフは単純無向グラフとして扱う
        - 同じ辺を複数回追加しないこと
        - T は default constructor と operator+ を持つこと
            例: int, long long, modint など

    [備考]
        - update(s,x) は代入ではなく加算
        - 初期値はすべて T()
        - 辺が木辺か非木辺かは内部で管理される
        - link の返り値は「辺追加に成功したか」ではなく「連結成分が併合されたか」
        - cut の返り値は「辺削除に成功したか」ではなく「連結成分が分裂したか」
        - 重複辺を扱いたい場合は別途 multiplicity 管理が必要

    [verified]
        - g++ 14.2.0, -std=gnu++23
        - ランダムテストで愚直 BFS と比較
*/

template <typename T>
class DynamicConnectivity {
private:
    class EulerTourTree {
    public:
        struct Node;
        using np = Node*;

        struct Node {
            np ch[2] = {nullptr, nullptr};
            np p = nullptr;

            int l = -1, r = -1;
            int sz = 0;

            T val = T();
            T sum = T();

            bool exact = false;
            bool child_exact = false;

            bool edge_connected = false;
            bool child_edge_connected = false;

            Node() {}

            Node(int l, int r)
                : l(l), r(r),
                  sz(l == r),
                  val(T()),
                  sum(T()),
                  exact(l < r),
                  child_exact(l < r) {}

            bool is_root() const {
                return p == nullptr;
            }
        };

    private:
        vector<unordered_map<int, np>> ptr;

        static T op(const T& a, const T& b) {
            return a + b;
        }

        static T e() {
            return T();
        }

        np get_node(int l, int r) {
            auto it = ptr[l].find(r);
            if (it == ptr[l].end()) {
                it = ptr[l].emplace(r, new Node(l, r)).first;
            }
            return it->second;
        }

        int size(np t) const {
            return t ? t->sz : 0;
        }

        np update(np t) {
            if (!t) return t;

            t->sum = e();

            if (t->ch[0]) t->sum = op(t->sum, t->ch[0]->sum);
            if (t->l == t->r) t->sum = op(t->sum, t->val);
            if (t->ch[1]) t->sum = op(t->sum, t->ch[1]->sum);

            t->sz = size(t->ch[0]) + (t->l == t->r) + size(t->ch[1]);

            t->child_edge_connected =
                (t->ch[0] ? t->ch[0]->child_edge_connected : false) |
                t->edge_connected |
                (t->ch[1] ? t->ch[1]->child_edge_connected : false);

            t->child_exact =
                (t->ch[0] ? t->ch[0]->child_exact : false) |
                t->exact |
                (t->ch[1] ? t->ch[1]->child_exact : false);

            return t;
        }

        void push(np) {
            // 遅延伝播を入れる場合はここに書く
        }

        np root(np t) {
            if (!t) return t;
            while (t->p) t = t->p;
            return t;
        }

        void rot(np t, bool b) {
            np x = t->p;
            np y = x->p;

            if ((x->ch[1 - b] = t->ch[b])) {
                t->ch[b]->p = x;
            }

            t->ch[b] = x;
            x->p = t;

            update(x);
            update(t);

            if ((t->p = y)) {
                if (y->ch[0] == x) y->ch[0] = t;
                if (y->ch[1] == x) y->ch[1] = t;
                update(y);
            }
        }

        void splay(np t) {
            if (!t) return;

            push(t);

            while (!t->is_root()) {
                np q = t->p;

                if (q->is_root()) {
                    push(q);
                    push(t);
                    rot(t, q->ch[0] == t);
                } else {
                    np r = q->p;

                    push(r);
                    push(q);
                    push(t);

                    bool b = (r->ch[0] == q);

                    if (q->ch[1 - b] == t) {
                        rot(q, b);
                        rot(t, b);
                    } else {
                        rot(t, 1 - b);
                        rot(t, b);
                    }
                }
            }
        }

        bool same(np s, np t) {
            if (s) splay(s);
            if (t) splay(t);
            return root(s) == root(t);
        }

        pair<np, np> split(np s) {
            splay(s);

            np t = s->ch[0];

            if (t) t->p = nullptr;
            s->ch[0] = nullptr;

            return {t, update(s)};
        }

        pair<np, np> split2(np s) {
            splay(s);

            np t = s->ch[0];
            np u = s->ch[1];

            if (t) t->p = nullptr;
            if (u) u->p = nullptr;

            s->ch[0] = nullptr;
            s->ch[1] = nullptr;

            return {t, u};
        }

        tuple<np, np, np> split(np s, np t) {
            auto u = split2(s);

            if (same(u.first, t)) {
                auto r = split2(t);
                return make_tuple(r.first, r.second, u.second);
            } else {
                auto r = split2(t);
                return make_tuple(u.first, r.first, r.second);
            }
        }

        np merge(np s, np t) {
            if (!s) return t;
            if (!t) return s;

            while (s->ch[1]) s = s->ch[1];

            splay(s);

            s->ch[1] = t;
            t->p = s;

            return update(s);
        }

        template <typename First, typename... Rest>
        np merge(First s, Rest... t) {
            return merge(s, merge(t...));
        }

        np reroot(np t) {
            auto s = split(t);
            return merge(s.second, s.first);
        }

        void collect_vertices(np t, vector<int>& res) {
            if (!t) return;

            push(t);

            collect_vertices(t->ch[0], res);

            if (t->l == t->r) {
                res.push_back(t->l);
            }

            collect_vertices(t->ch[1], res);
        }

    public:
        EulerTourTree() {}

        EulerTourTree(int n) {
            ptr.resize(n);
            for (int i = 0; i < n; i++) {
                ptr[i][i] = new Node(i, i);
            }
        }

        void set_size(int n) {
            ptr.assign(n, unordered_map<int, np>());
            for (int i = 0; i < n; i++) {
                ptr[i][i] = new Node(i, i);
            }
        }

        bool same(int s, int t) {
            return same(get_node(s, s), get_node(t, t));
        }

        int size(int s) {
            np t = get_node(s, s);
            splay(t);
            return t->sz;
        }

        vector<int> vertex_list(int s) {
            np t = get_node(s, s);
            splay(t);

            vector<int> res;
            collect_vertices(t, res);
            return res;
        }

        void update(int s, T x) {
            np t = get_node(s, s);
            splay(t);

            t->val = op(t->val, x);

            update(t);
        }

        T get_sum(int s) {
            np t = get_node(s, s);
            splay(t);
            return t->sum;
        }

        void edge_connected_update(int s, bool b) {
            np t = get_node(s, s);
            splay(t);

            t->edge_connected = b;

            update(t);
        }

        template <class F>
        void edge_update(int s, F g) {
            np t = get_node(s, s);
            splay(t);

            function<void(np)> dfs = [&](np t) {
                assert(t);

                if (t->l < t->r && t->exact) {
                    splay(t);

                    t->exact = false;

                    update(t);

                    g(t->l, t->r);
                    return;
                }

                if (t->ch[0] && t->ch[0]->child_exact) {
                    dfs(t->ch[0]);
                } else {
                    dfs(t->ch[1]);
                }
            };

            while (t && t->child_exact) {
                dfs(t);
                splay(t);
            }
        }

        template <class F>
        bool try_reconnect(int s, F f) {
            np t = get_node(s, s);
            splay(t);

            function<bool(np)> dfs = [&](np t) -> bool {
                assert(t);

                if (t->edge_connected) {
                    splay(t);
                    return f(t->l);
                }

                if (t->ch[0] && t->ch[0]->child_edge_connected) {
                    return dfs(t->ch[0]);
                }

                return dfs(t->ch[1]);
            };

            while (t->child_edge_connected) {
                if (dfs(t)) return true;
                splay(t);
            }

            return false;
        }

        bool link(int l, int r) {
            if (same(l, r)) return false;

            merge(
                reroot(get_node(l, l)),
                get_node(l, r),
                reroot(get_node(r, r)),
                get_node(r, l)
            );

            return true;
        }

        bool cut(int l, int r) {
            if (ptr[l].find(r) == ptr[l].end()) return false;

            np s, t, u;
            tie(s, t, u) = split(get_node(l, r), get_node(r, l));

            merge(s, u);

            np p = ptr[l][r];
            np q = ptr[r][l];

            ptr[l].erase(r);
            ptr[r].erase(l);

            delete p;
            delete q;

            return true;
        }
    };

    int n;
    int dep = 1;
    int comp_cnt;

    vector<EulerTourTree> ett;
    vector<vector<unordered_set<int>>> edges;

public:
    DynamicConnectivity(int n) : n(n), comp_cnt(n) {
        ett.emplace_back(n);
        edges.emplace_back(n);
    }

    bool link(int s, int t) {
        if (s == t) return false;

        if (ett[0].link(s, t)) {
            comp_cnt--;
            return true;
        }

        // すでに連結なら非木辺として保持する
        edges[0][s].insert(t);
        edges[0][t].insert(s);

        if (edges[0][s].size() == 1) {
            ett[0].edge_connected_update(s, true);
        }

        if (edges[0][t].size() == 1) {
            ett[0].edge_connected_update(t, true);
        }

        return false;
    }

    bool cut(int s, int t) {
        if (s == t) return false;

        for (int i = 0; i < dep; i++) {
            edges[i][s].erase(t);
            edges[i][t].erase(s);

            if (edges[i][s].empty()) {
                ett[i].edge_connected_update(s, false);
            }

            if (edges[i][t].empty()) {
                ett[i].edge_connected_update(t, false);
            }
        }

        for (int i = dep - 1; i >= 0; i--) {
            if (ett[i].cut(s, t)) {
                if (dep - 1 == i) {
                    dep++;
                    ett.emplace_back(n);
                    edges.emplace_back(n);
                }

                bool disconnected = !try_reconnect(s, t, i);

                if (disconnected) {
                    comp_cnt++;
                }

                return disconnected;
            }
        }

        return false;
    }

    bool same(int s, int t) {
        return ett[0].same(s, t);
    }

    int size(int s) {
        return ett[0].size(s);
    }

    int count_components() const {
        return comp_cnt;
    }

    vector<int> get_vertex(int s) {
        return ett[0].vertex_list(s);
    }

    void update(int s, T x) {
        ett[0].update(s, x);
    }

    T get_sum(int s) {
        return ett[0].get_sum(s);
    }

private:
    bool try_reconnect(int s, int t, int k) {
        for (int i = 0; i < k; i++) {
            ett[i].cut(s, t);
        }

        for (int i = k; i >= 0; i--) {
            if (ett[i].size(s) > ett[i].size(t)) {
                swap(s, t);
            }

            auto g = [&](int a, int b) {
                ett[i + 1].link(a, b);
            };

            ett[i].edge_update(s, g);

            auto f = [&](int x) -> bool {
                for (auto it = edges[i][x].begin(); it != edges[i][x].end();) {
                    int y = *it;

                    it = edges[i][x].erase(it);
                    edges[i][y].erase(x);

                    if (edges[i][x].empty()) {
                        ett[i].edge_connected_update(x, false);
                    }

                    if (edges[i][y].empty()) {
                        ett[i].edge_connected_update(y, false);
                    }

                    if (ett[i].same(x, y)) {
                        edges[i + 1][x].insert(y);
                        edges[i + 1][y].insert(x);

                        if (edges[i + 1][x].size() == 1) {
                            ett[i + 1].edge_connected_update(x, true);
                        }

                        if (edges[i + 1][y].size() == 1) {
                            ett[i + 1].edge_connected_update(y, true);
                        }
                    } else {
                        for (int j = 0; j <= i; j++) {
                            ett[j].link(x, y);
                        }

                        return true;
                    }
                }

                return false;
            };

            if (ett[i].try_reconnect(s, f)) {
                return true;
            }
        }

        return false;
    }
};