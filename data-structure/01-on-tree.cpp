/*
  <01-on-tree>
    - 次のような問題を考える
        - N 頂点の根付き木と，各頂点 i に 01 列 V_i が与えられる．
        - 親が子よりも先に現れるような頂点順列 q のうち，
          連結列 V_{q_1}, V_{q_2}, ..., V_{q_N} の転倒数を最小化するものを求める．

    - 特に，V_i が 0 または 1 の 1 文字である場合，
      これは「親が子より先に現れる順序のうち，ラベル列の転倒数を最小化する問題」である．

    - 各 V_i が 00...01 のような列であっても同様に解ける．
      より一般に，各頂点 i を
          zero[i] = V_i に含まれる 0 の個数
          one[i]  = V_i に含まれる 1 の個数
      を持つブロックとして扱えばよい．
      ただし，V_i 内部の転倒数は定数なので，必要なら最後に足せばよい．
      00...01 の形なら内部転倒数は 0 である．

    [原理]
        - 2 つのブロック A, B を並べるとき，
              A -> B の追加転倒数 = one[A] * zero[B]
              B -> A の追加転倒数 = one[B] * zero[A]
          である．

        - よって，制約がなければ
              one[A] / zero[A] <= one[B] / zero[B]
          の順に並べるのがよい．

        - 木の親子制約がある場合は，
          現在の各連結成分をブロックと見て，
          root 以外の成分のうち one / zero が最小のものを親成分の直後にマージする，
          という操作を繰り返せばよい．

    [実装/関数]
        - ZeroOneOnTree(int n, int root = 0)
            N 頂点，根 root の問題を作る

        - void set_parent(int v, int p)
            v の親を p に設定する
            root には設定しない

        - void set_value(int v, int x)
            V_v を 1 文字 x とする
            x は 0 または 1

        - void set_block(int v, long long z, long long o)
            V_v を 0 が z 個，1 が o 個のブロックとして設定する

        - pair<long long, vector<int>> solve()
            最小転倒数と，それを達成する頂点順序を返す

    [計算時間]
        - O(N log N)

    [要件]
        - 親配列は根付き木を表していること
        - root の parent は設定しない
        - zero, one の値は非負
        - 各頂点のブロックは空でないことを想定

    [備考]
        - 戻り値の順列は 0-indexed
        - 1-indexed で使いたい場合は，入力時または出力時に ±1 する
        - zero = 0 のブロックは one / zero = ∞ として扱う
        - 内部転倒数を持つ一般の 01 列 V_i にも使えるが，
          その場合，答えには各 V_i 内部の転倒数を別途足す必要がある

    [参照]
        - AtCoder Grand Contest 023 F - 01 on Tree

    [verified at]
        -
*/

struct ZeroOneOnTree {
    using ll = long long;

    struct HeapNode {
        int v;
        ll zero, one;
        int ver;
    };

    struct Compare {
        // priority_queue 用．one / zero が小さいものを優先する
        bool operator()(const HeapNode &a, const HeapNode &b) const {
            __int128 lhs = (__int128)a.one * b.zero;
            __int128 rhs = (__int128)b.one * a.zero;

            if (lhs != rhs) return lhs > rhs;

            // 比率が同じなら頂点番号が小さいものを優先
            return a.v > b.v;
        }
    };

    int n, root;
    vector<int> par;

    vector<ll> zero, one;

    vector<int> uf;
    vector<int> head, tail, nxt;
    vector<int> ver;
    vector<char> alive;

    ZeroOneOnTree(int n, int root = 0)
        : n(n), root(root),
          par(n, -1),
          zero(n, 0), one(n, 0),
          uf(n), head(n), tail(n), nxt(n, -1),
          ver(n, 0), alive(n, 1) {
        iota(uf.begin(), uf.end(), 0);
        iota(head.begin(), head.end(), 0);
        iota(tail.begin(), tail.end(), 0);
    }

    void set_parent(int v, int p) {
        par[v] = p;
    }

    void set_value(int v, int x) {
        assert(x == 0 || x == 1);
        zero[v] = (x == 0);
        one[v] = (x == 1);
    }

    void set_block(int v, ll z, ll o) {
        assert(z >= 0 && o >= 0);
        zero[v] = z;
        one[v] = o;
    }

    int find(int v) {
        if (uf[v] == v) return v;
        return uf[v] = find(uf[v]);
    }

    pair<ll, vector<int>> solve() {
        priority_queue<HeapNode, vector<HeapNode>, Compare> pq;

        for (int v = 0; v < n; v++) {
            assert(v == root || par[v] != -1);
            assert(zero[v] >= 0 && one[v] >= 0);
            assert(zero[v] + one[v] > 0);

            if (v != root) {
                pq.push({v, zero[v], one[v], ver[v]});
            }
        }

        ll ans = 0;

        while (!pq.empty()) {
            auto cur = pq.top();
            pq.pop();

            int v = cur.v;

            // 古い情報なら捨てる
            if (!alive[v]) continue;
            if (cur.ver != ver[v]) continue;
            if (cur.zero != zero[v] || cur.one != one[v]) continue;

            // v 成分の現在の親成分
            int p = find(par[v]);

            // p のブロックの後ろに v のブロックを置く
            ans += one[p] * zero[v];

            // 順序の復元用に連結リストを結合する
            nxt[tail[p]] = head[v];
            tail[p] = tail[v];

            // 個数をマージする
            zero[p] += zero[v];
            one[p] += one[v];

            // v 成分を p 成分へ統合
            alive[v] = 0;
            uf[v] = p;

            // p の比率が変わったので，root でなければ入れ直す
            ver[p]++;
            if (p != root) {
                pq.push({p, zero[p], one[p], ver[p]});
            }
        }

        vector<int> order;
        for (int v = head[root]; v != -1; v = nxt[v]) {
            order.push_back(v);
        }

        assert((int)order.size() == n);
        return {ans, order};
    }
};