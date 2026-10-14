/*
  <ランダム入力生成ライブラリ>
    - ランテス用の gen.cpp で使う乱数生成関数群
    - seed を指定することで，同じランダム入力を再現できる
    - グラフ・木の頂点番号は基本的に 0-index で返す
        - 問題が 1-index 入力を要求する場合は，出力時に +1 する

    [使い方]
        int main(int argc, char* argv[]) {
            unsigned long long seed =
                chrono::steady_clock::now().time_since_epoch().count();

            if (argc >= 2) {
                seed = stoull(argv[1]);
            }

            RNG rng(seed);

            // ここに入力生成を書く
        }

    [整数・実数]
        - long long randint(long long a, long long b)
            : a 以上 b 以下の整数を一様ランダムに返す

        - double randdouble(double a, double b)
            : a 以上 b 以下の double 値をランダムに返す
            : 厳密には uniform_real_distribution に従うため，実用上 [a,b) と考えてよい

    [一次元配列]
        - vector<int> int_vector(int n, int a, int b)
            : 長さ n の vector<int> を返す
            : 各要素は a 以上 b 以下のランダムな整数

        - vector<long long> ll_vector(int n, long long a, long long b)
            : 長さ n の vector<long long> を返す
            : 各要素は a 以上 b 以下のランダムな整数

        - template <typename T>
          vector<T> sequence_from_values(int n, const vector<T> &values)
            : 長さ n の vector<T> を返す
            : 各要素は values の中からランダムに選ばれる
            : 例 sequence_from_values<int>(n, {1, 3, 5})

    [順列]
        - vector<int> permutation(int n, int start = 0)
            : 長さ n のランダムな順列を返す
            : start = 0 なら {0,1,...,n-1} の順列
            : start = 1 なら {1,2,...,n} の順列

    [グラフ]
        - vector<pair<int,int>> graph(
              int n,
              int m,
              bool directed = false,
              bool self_loop = false,
              bool multi = false
          )
            : n 頂点 m 辺のランダムグラフを返す
            : 返り値は vector<pair<int,int>>
            : 各辺は {u,v} で表される
            : 頂点番号は 0-index

            : directed = false のとき無向グラフ
            : directed = true  のとき有向グラフ

            : self_loop = false のとき自己ループなし
            : self_loop = true  のとき自己ループあり

            : multi = false のとき多重辺なし
            : multi = true  のとき多重辺あり

            : 無向グラフの場合，各辺は u <= v になるように保存される
              ただし，返す直前に辺の順序は shuffle される

        - vector<tuple<int,int,long long>> weighted_graph(
              int n,
              int m,
              long long a,
              long long b,
              bool directed = false,
              bool self_loop = false,
              bool multi = false
          )
            : n 頂点 m 辺のランダム重み付きグラフを返す
            : 返り値は vector<tuple<int,int,long long>>
            : 各辺は {u,v,w} で表される
            : u,v は 0-index の端点
            : w は a 以上 b 以下のランダムな整数重み

        - vector<tuple<int,int,double>> weighted_graph_double(
              int n,
              int m,
              double a,
              double b,
              bool directed = false,
              bool self_loop = false,
              bool multi = false
          )
            : n 頂点 m 辺のランダム double 重み付きグラフを返す
            : 各辺は {u,v,w}
            : w は a 以上 b 以下のランダムな double 値

    [木]
        - vector<pair<int,int>> tree(int n)
            : n 頂点のランダム木を返す
            : 返り値は vector<pair<int,int>>
            : 各辺は {u,v}
            : 頂点番号は 0-index
            : 辺数は n-1
            : Prüfer 列により一様ランダムなラベル付き木を生成する
            : 辺の順序と向きはランダムにされる

        - vector<tuple<int,int,long long>> weighted_tree(
              int n,
              long long a,
              long long b
          )
            : n 頂点のランダム重み付き木を返す
            : 返り値は vector<tuple<int,int,long long>>
            : 各辺は {u,v,w}
            : u,v は 0-index の端点
            : w は a 以上 b 以下のランダムな整数重み
            : 辺数は n-1

        - vector<tuple<int,int,double>> weighted_tree_double(
              int n,
              double a,
              double b
          )
            : n 頂点のランダム double 重み付き木を返す
            : 各辺は {u,v,w}
            : w は a 以上 b 以下のランダムな double 値

    [文字列]
        - string lowercase_string(int n)
            : 長さ n のランダムな英小文字列を返す
            : 各文字は 'a' 以上 'z' 以下

        - string uppercase_string(int n)
            : 長さ n のランダムな英大文字列を返す
            : 各文字は 'A' 以上 'Z' 以下

        - string string_from_chars(int n, const string &chars)
            : 長さ n のランダム文字列を返す
            : 各文字は chars に含まれる文字からランダムに選ばれる
            : 例 string_from_chars(n, "01")
            : 例 string_from_chars(n, ".#")
            : 例 string_from_chars(n, "abc")

    [二次元配列・grid]
        - vector<vector<int>> int_matrix(int h, int w, int a, int b)
            : h 行 w 列の vector<vector<int>> を返す
            : 各要素は a 以上 b 以下のランダムな整数

        - vector<vector<long long>> ll_matrix(
              int h,
              int w,
              long long a,
              long long b
          )
            : h 行 w 列の vector<vector<long long>> を返す
            : 各要素は a 以上 b 以下のランダムな整数

        - vector<string> grid_from_chars(int h, int w, const string &chars)
            : h 行 w 列のランダム grid を返す
            : 返り値は vector<string>
            : 各マスは chars に含まれる文字からランダムに選ばれる
            : 例 grid_from_chars(h, w, ".#")
            : 例 grid_from_chars(h, w, "UDLR")
            : 例 grid_from_chars(h, w, "abc")

    [返り値の型の例]
        auto edges = rng.graph(n, m);
            -> vector<pair<int,int>>

        auto edges = rng.weighted_graph(n, m, 1, 100);
            -> vector<tuple<int,int,long long>>

        auto edges = rng.tree(n);
            -> vector<pair<int,int>>

        auto edges = rng.weighted_tree(n, 1, 100);
            -> vector<tuple<int,int,long long>>

    [出力例]
        // 1-index の無向グラフとして出力
        auto edges = rng.graph(n, m, false);
        cout << n << " " << m << '\n';
        for (auto [u, v] : edges) {
            cout << u + 1 << " " << v + 1 << '\n';
        }

        // 1-index の重み付きグラフとして出力
        auto edges = rng.weighted_graph(n, m, 1, 100);
        cout << n << " " << m << '\n';
        for (auto [u, v, w] : edges) {
            cout << u + 1 << " " << v + 1 << " " << w << '\n';
        }

    [注意]
        - graph で multi = false の場合，指定された m が作れる最大辺数を超えると assert で落ちる
        - self_loop = false かつ n <= 1 のとき，多重辺ありでも正の辺数は作れない
        - 木は n >= 1 を仮定する
        - ランテスで落ちた入力を再現するため，gen.cpp は seed を argv[1] から受け取る形にしておくとよい
*/

#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct RNG {
    mt19937_64 eng;

    RNG() : eng(chrono::steady_clock::now().time_since_epoch().count()) {}
    RNG(unsigned long long seed) : eng(seed) {}

    // a 以上 b 以下の整数
    long long randint(long long a, long long b) {
        assert(a <= b);
        uniform_int_distribution<long long> dist(a, b);
        return dist(eng);
    }

    // a 以上 b 以下の double 値
    // 実質 [a, b) だと思って使えばよいです
    double randdouble(double a, double b) {
        assert(a <= b);
        uniform_real_distribution<double> dist(a, b);
        return dist(eng);
    }

    // 長さ n，各要素が a 以上 b 以下のランダム整数列
    vector<int> int_vector(int n, int a, int b) {
        vector<int> v(n);
        for (int i = 0; i < n; i++) {
            v[i] = (int)randint(a, b);
        }
        return v;
    }

    vector<long long> ll_vector(int n, long long a, long long b) {
        vector<long long> v(n);
        for (int i = 0; i < n; i++) {
            v[i] = randint(a, b);
        }
        return v;
    }

    // 長さ n のランダムな順列
    // start = 0 なら 0,1,...,n-1
    // start = 1 なら 1,2,...,n
    vector<int> permutation(int n, int start = 0) {
        vector<int> p(n);
        iota(p.begin(), p.end(), start);
        shuffle(p.begin(), p.end(), eng);
        return p;
    }

    // n 頂点 m 辺のランダムグラフ
    // directed = false : 無向グラフ
    // directed = true  : 有向グラフ
    // self_loop = false : 自己ループなし
    // multi = false : 多重辺なし
    //
    // 返り値は 0-index の辺集合
    vector<pair<int,int>> graph(
        int n,
        int m,
        bool directed = false,
        bool self_loop = false,
        bool multi = false
    ) {
        assert(n >= 0);
        assert(m >= 0);

        vector<pair<int,int>> edges;

        auto max_edges = [&]() -> long long {
            if (directed) {
                if (self_loop) return 1LL * n * n;
                else return 1LL * n * (n - 1);
            } else {
                if (self_loop) return 1LL * n * (n + 1) / 2;
                else return 1LL * n * (n - 1) / 2;
            }
        };

        if (!multi) {
            assert((long long)m <= max_edges());
        } else {
            if (!self_loop && n <= 1) assert(m == 0);
            if (self_loop && n == 0) assert(m == 0);
        }

        if (multi) {
            while ((int)edges.size() < m) {
                int u = (int)randint(0, n - 1);
                int v = (int)randint(0, n - 1);

                if (!self_loop && u == v) continue;

                if (!directed && u > v) swap(u, v);

                edges.push_back({u, v});
            }

            shuffle(edges.begin(), edges.end(), eng);
            return edges;
        }

        unordered_set<long long> used;
        used.reserve((size_t)m * 2 + 10);

        auto encode = [&](int u, int v) -> long long {
            return 1LL * u * n + v;
        };

        while ((int)edges.size() < m) {
            int u = (int)randint(0, n - 1);
            int v = (int)randint(0, n - 1);

            if (!self_loop && u == v) continue;

            if (!directed && u > v) swap(u, v);

            long long id = encode(u, v);
            if (used.count(id)) continue;

            used.insert(id);
            edges.push_back({u, v});
        }

        shuffle(edges.begin(), edges.end(), eng);
        return edges;
    }

    // n 頂点のランダム木
    // Prüfer 列による一様ランダムなラベル付き木
    // 返り値は 0-index
    vector<pair<int,int>> tree(int n) {
        assert(n >= 1);

        vector<pair<int,int>> edges;

        if (n == 1) return edges;

        vector<int> prufer(n - 2);
        vector<int> deg(n, 1);

        for (int i = 0; i < n - 2; i++) {
            prufer[i] = (int)randint(0, n - 1);
            deg[prufer[i]]++;
        }

        set<int> leaves;
        for (int i = 0; i < n; i++) {
            if (deg[i] == 1) leaves.insert(i);
        }

        for (int x : prufer) {
            int leaf = *leaves.begin();
            leaves.erase(leaves.begin());

            edges.push_back({leaf, x});

            deg[leaf]--;
            deg[x]--;

            if (deg[x] == 1) {
                leaves.insert(x);
            }
        }

        assert((int)leaves.size() == 2);
        int u = *leaves.begin();
        int v = *next(leaves.begin());
        edges.push_back({u, v});

        // 辺の順番と向きをランダムにする
        for (auto &[a, b] : edges) {
            if (randint(0, 1)) swap(a, b);
        }
        shuffle(edges.begin(), edges.end(), eng);

        return edges;
    }

    vector<tuple<int,int,long long>> weighted_graph(
    int n,
    int m,
    long long a,
    long long b,
    bool directed = false,
    bool self_loop = false,
    bool multi = false
    ) {
        auto es = graph(n, m, directed, self_loop, multi);

        vector<tuple<int,int,long long>> res;
        res.reserve(m);

        for (auto [u, v] : es) {
            long long w = randint(a, b);
            res.push_back({u, v, w});
        }

        return res;
    }

    vector<tuple<int,int,long long>> weighted_tree(
        int n,
        long long a,
        long long b
    ) {
        auto es = tree(n);

        vector<tuple<int,int,long long>> res;
        res.reserve(max(0, n - 1));

        for (auto [u, v] : es) {
            long long w = randint(a, b);
            res.push_back({u, v, w});
        }

        return res;
    }

    // 長さ n のランダムな英小文字列
    string lowercase_string(int n) {
        string chars = "abcdefghijklmnopqrstuvwxyz";
        return string_from_chars(n, chars);
    }

    // 長さ n のランダムな英大文字列
    string uppercase_string(int n) {
        string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        return string_from_chars(n, chars);
    }

    // 長さ n の，出現する文字を指定したランダム文字列
    // 例: string_from_chars(n, "abc")
    string string_from_chars(int n, const string &chars) {
        assert(!chars.empty());

        string s;
        s.reserve(n);

        for (int i = 0; i < n; i++) {
            int id = (int)randint(0, (int)chars.size() - 1);
            s.push_back(chars[id]);
        }

        return s;
    }

    // 長さ n の，出現する値を指定したランダム列
    // 例: sequence_from_values<int>(n, {1, 3, 5})
    template <typename T>
    vector<T> sequence_from_values(int n, const vector<T> &values) {
        assert(!values.empty());

        vector<T> v(n);
        for (int i = 0; i < n; i++) {
            int id = (int)randint(0, (int)values.size() - 1);
            v[i] = values[id];
        }

        return v;
    }

    // h * w の各要素が a 以上 b 以下のランダム二次元配列
    vector<vector<int>> int_matrix(int h, int w, int a, int b) {
        vector<vector<int>> mat(h, vector<int>(w));

        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                mat[i][j] = (int)randint(a, b);
            }
        }

        return mat;
    }

    vector<vector<long long>> ll_matrix(int h, int w, long long a, long long b) {
        vector<vector<long long>> mat(h, vector<long long>(w));

        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                mat[i][j] = randint(a, b);
            }
        }

        return mat;
    }

    // h * w の，出現する文字を指定したランダム grid
    // 例: grid_from_chars(h, w, ".#")
    vector<string> grid_from_chars(int h, int w, const string &chars) {
        assert(!chars.empty());

        vector<string> gr(h, string(w, '?'));

        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                int id = (int)randint(0, (int)chars.size() - 1);
                gr[i][j] = chars[id];
            }
        }

        return gr;
    }
};

int main(int argc, char* argv[]) {
    unsigned long long seed = chrono::steady_clock::now().time_since_epoch().count();

    if (argc >= 2) {
        seed = stoull(argv[1]);
    }

    RNG rng(seed);

    // ここから問題ごとに書き換える

    int n = rng.randint(1, 8);
    cout << n << '\n';

    for (int i = 0; i < n; i++) {
        int x = rng.randint(1, 20);
        cout << x << (i + 1 == n ? '\n' : ' ');
    }
}