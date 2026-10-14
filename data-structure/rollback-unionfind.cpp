/*
  <Rollback Union-Find>
    - rollback 可能な Union-Find
    - unite による変更を undo / rollback によって元に戻せる

    [概要]
        - 通常の Union-Find では find の高速化のために経路圧縮を行うが，
          経路圧縮を行うと多くの parent が変更されるため rollback が難しくなる．

        - この実装では経路圧縮を行わず，union by size のみを行う．
          これにより，1 回の unite で変更される箇所は高々 2 箇所になる．

        - unite のたびに変更前の情報を history に保存しておき，
          undo によって直前の unite 前の状態へ戻すことができる．

    [実装/関数]
        - RollbackUnionFind uf(int n)
            n 頂点の Rollback Union-Find を作る

        - bool unite(int x, int y)
            x, y を含む集合を併合する
            すでに同じ集合なら false，そうでなければ true を返す
            false の場合でも undo / rollback との整合性のために履歴は積む

        - int find(int x)
            x の属する集合の代表元を返す
            経路圧縮は行わない

        - bool same(int x, int y)
            x, y が同じ集合に属するかを返す

        - int size(int x)
            x を含む集合のサイズを返す

        - void undo()
            直前の unite を 1 回取り消す

        - void snapshot()
            現在の状態を保存する
            rollback() のデフォルト戻り先になる

        - int get_state()
            現在の状態を表す値を返す
            具体的には，これまでに呼ばれた unite の回数に対応する

        - void rollback(int state = -1)
            state が指定された場合，その状態まで戻す
            state == -1 の場合，最後に snapshot() した状態まで戻す

    [計算時間]
        - find, same, size
            O(log N)

        - unite
            O(log N)

        - undo
            O(1)

        - rollback(state)
            O(戻す unite の回数)

    [備考]
        - 経路圧縮はしてはいけない
        - union by size により木の高さは O(log N) に抑えられる
        - undo() は直前の unite を 1 回取り消す
        - unite(x,y) が false を返す場合でも履歴を積むため，undo() との対応が崩れない
        - get_state() の値は「成功した unite の回数」ではなく「unite が呼ばれた回数」
        - rollback(s) には get_state() で取得した値を渡すとよい

    [使用例]
        RollbackUnionFind uf(5);

        uf.unite(0, 1);
        int state = uf.get_state();

        uf.unite(1, 2);
        cout << uf.same(0, 2) << endl; // 1

        uf.rollback(state);
        cout << uf.same(0, 2) << endl; // 0

    [参照]
        - https://nyaannyaan.github.io/library/data-structure/rollback-union-find.hpp.html

    [verified at]
        -
*/

struct RollbackUnionFind {
    vector<int> data;
    stack<pair<int, int> > history;
    int inner_snap;
  
    RollbackUnionFind(int sz) : inner_snap(0) { data.assign(sz, -1); }
  
    bool unite(int x, int y) {
      x = find(x), y = find(y);
      history.emplace(x, data[x]);
      history.emplace(y, data[y]);
      if (x == y) return false;
      if (data[x] > data[y]) swap(x, y);
      data[x] += data[y];
      data[y] = x;
      return true;
    }
  
    int find(int k) {
      if (data[k] < 0) return k;
      return find(data[k]);
    }
  
    int same(int x, int y) { return find(x) == find(y); }
  
    int size(int k) { return (-data[find(k)]); }
  
    void undo() {
      data[history.top().first] = history.top().second;
      history.pop();
      data[history.top().first] = history.top().second;
      history.pop();
    }
  
    void snapshot() { inner_snap = int(history.size() >> 1); }
  
    int get_state() { return int(history.size() >> 1); }
  
    void rollback(int state = -1) {
      if (state == -1) state = inner_snap;
      state <<= 1;
      assert(state <= (int)history.size());
      while (state < (int)history.size()) undo();
    }
};


// モノイドを持てるもの(chatgpt)
template<typename Monoid, Monoid (*Op)(Monoid, Monoid), Monoid (*E)()>
struct RollbackUnionFind {
    vector<int> parent;            // parent[k] < 0 のとき k は根で，その絶対値が集合のサイズ
    vector<Monoid> value;          // 各集合のモノイド値（根にのみ意味がある）
    // 履歴スタックには (ノード番号, 変更前の parent 値, 変更前の value) を記録
    stack<tuple<int, int, Monoid>> history;
    int inner_snap;                // snapshot 時の履歴のサイズ（ペア数）

    // コンストラクタ: 各ノードの初期モノイド値は init から与える
    RollbackUnionFind(int sz, const vector<Monoid>& init) : inner_snap(0) {
        parent.assign(sz, -1);
        value = init;
    }

    // 経路圧縮は行わず，rollback に対応
    int find(int k) {
        if (parent[k] < 0) return k;
        return find(parent[k]);
    }

    // 2 つの集合を併合する
    bool unite(int x, int y) {
        x = find(x), y = find(y);
        history.emplace(x, parent[x], value[x]);
        history.emplace(y, parent[y], value[y]);
        if (x == y) return false;
        if (parent[x] > parent[y]) swap(x, y);
        parent[x] += parent[y];
        parent[y] = x;
        // 併合時に，2 つのモノイドの値を結合関数 Op で合成
        value[x] = Op(value[x], value[y]);
        return true;
    }

    bool same(int x, int y) { return find(x) == find(y); }
    int size(int k) { return -parent[find(k)]; }
    // ノード k が属する集合全体のモノイド値を返す
    Monoid get_value(int k) { return value[find(k)]; }

    // 最新の union 操作で行った変更（2 回分）を undo
    void undo() {
        auto [y, p_y, v_y] = history.top(); history.pop();
        auto [x, p_x, v_x] = history.top(); history.pop();
        parent[y] = p_y; value[y] = v_y;
        parent[x] = p_x; value[x] = v_x;
    }

    // 現在の状態をスナップショットとして記録
    void snapshot() { inner_snap = history.size() / 2; }
    int get_state() { return history.size() / 2; }

    // rollback: 指定した状態まで変更を取り消す（state を省略した場合は snapshot まで戻す）
    void rollback(int state = -1) {
        if(state == -1) state = inner_snap;
        while(history.size() > (size_t)(state * 2)) {
            undo();
        }
    }
};