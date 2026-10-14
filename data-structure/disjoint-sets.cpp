/*
 素集合の要素の列挙と併合 (https://noshi91.hatenablog.com/entry/2019/07/19/180606 を参照)
    - get(x) : xと同じ集合の要素を全て列挙
    - unite(x,y) : x,yがそれぞれ含まれる集合同士を併合する (x,yが同じ集合に含まれているとバグる)

    - 集合に加わる各要素xに対して，next[x] (xの次の要素)を持っておく(1要素しかない時は，自分自身をnextとする)
        uniteのときは，next[x]とnext[y]をスワップするだけで良い

    - ChatGpt 5.5 plus が作成
*/


struct EnumerableDSU {
    int n;
    vector<int> par, siz;
    vector<int> nxt;

    EnumerableDSU(int n) : n(n), par(n), siz(n, 1), nxt(n) {
        iota(par.begin(), par.end(), 0);
        iota(nxt.begin(), nxt.end(), 0);
    }

    int leader(int x) {
        if (par[x] == x) return x;
        return par[x] = leader(par[x]);
    }

    bool same(int x, int y) {
        return leader(x) == leader(y);
    }

    int size(int x) {
        return siz[leader(x)];
    }

    // x を含む集合と y を含む集合を併合する
    // ただし，x, y が同じ集合にある場合は呼んではいけない
    bool unite(int x, int y) {
        int rx = leader(x);
        int ry = leader(y);

        if (rx == ry) {
            // 同じ集合に対して呼ぶと巡回リストが壊れるので false を返す
            return false;
        }

        // 巡回リストの併合
        swap(nxt[x], nxt[y]);

        // DSU の併合
        if (siz[rx] < siz[ry]) swap(rx, ry);
        par[ry] = rx;
        siz[rx] += siz[ry];

        return true;
    }

    // x と同じ集合の要素をすべて列挙する
    vector<int> get(int x) const {
        vector<int> res;
        int cur = x;
        do {
            res.push_back(cur);
            cur = nxt[cur];
        } while (cur != x);
        return res;
    }
};