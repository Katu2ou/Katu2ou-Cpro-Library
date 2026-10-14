/*
  <Mo's Algorithm / Mo 法>
    - 区間クエリ [l,r) をオフラインで処理する
    - クエリ順を並べ替え，現在区間の左右端を少しずつ動かすことで高速化する
    - 「要素を1個追加・削除したときの状態更新」が高速な問題に有効

    [概要]
        - 現在区間 [l,r) を保持し，各クエリ [ql,qr) まで l,r を1ずつ動かす
        - 左右端の移動に応じて add_left，add_right，erase_left，erase_right を呼ぶ
        - 左右で同じ処理を行う場合は add，erase の2関数だけでよい
        - クエリは left をブロック分割し，同一ブロック内では right を snake order で並べる
        - 初期状態は空区間 [0,0)

    [実装/関数]
        - Mo mo(N, width=-1)
            長さ N の対象に対する Mo を作る
            width<=0 ならブロック幅を自動決定する

        - int add_query(int l,int r), int add(int l,int r)
            クエリ [l,r) を登録し，クエリ番号を返す

        - int size()
            登録済みクエリ数を返す

        - bool empty()
            登録済みクエリが0個なら true

        - void clear()
            登録済みクエリをすべて削除する

        - run(add,erase,query)
            左右で追加・削除処理が同じ場合

        - run(add_left,add_right,erase_left,erase_right,query)
            左右で処理が異なる場合

    [コールバック]
        - add_left(i)
            現在区間 [l,r) を [l-1,r) に拡張するときに呼ばれる
            i は新しく追加された位置

        - add_right(i)
            現在区間 [l,r) を [l,r+1) に拡張するときに呼ばれる
            i は新しく追加された位置

        - erase_left(i)
            現在区間 [l,r) を [l+1,r) に縮めるときに呼ばれる
            i は削除された位置

        - erase_right(i)
            現在区間 [l,r) を [l,r-1) に縮めるときに呼ばれる
            i は削除された位置

        - 各更新関数は f(i) の代わりに f(l,r) と書くこともできる
            このとき [l,r) は「操作後」の現在区間
            変化した位置は
                add_left   : l
                add_right  : r-1
                erase_left : l-1
                erase_right: r

        - query(q) または query(q,l,r)
            q 番目のクエリに対する答えを現在状態から保存する
            3引数版の [l,r) はそのクエリ区間

    [計算時間]
        - 1回の add / erase を O(T) とする
        - ブロック幅を w とすると，端点移動回数は概ね
              O(N^2/w + Qw)
        - 自動設定 w ≈ N/sqrt(Q) では概ね
              O(N sqrt(Q) T + Q log Q)
        - Q≈N の典型的な場合は
              O((N+Q)sqrt(N) T)
          程度

    [適用条件]
        - すべてのクエリを事前に取得できるオフライン問題であること
        - 対象となる列・要素がクエリ途中で変更されないこと
        - 区間 [l,r) の状態から
              [l-1,r), [l+1,r), [l,r-1), [l,r+1)
          への更新を高速に行えること
        - 特に，区間同士を高速にマージできない問題で有効なことが多い

    [備考]
        - add/erase が左右で可換なら run(add,erase,query) で十分
        - 順序依存の状態では add_left/add_right/erase_left/erase_right を分ける
        - run() 開始時には，外部で管理する状態も空区間 [0,0) に対応させておく
        - query は原則として現在状態を変更しないこと
        - クエリは内部で並べ替えられるが，query に渡る q は元の登録順の番号
        - 空区間 [l,l) にも対応する

    [使用例]
        Mo mo(N);
        for(int q=0;q<Q;q++) mo.add_query(L[q],R[q]);

        auto add = [&](int i) { ... };
        auto erase = [&](int i) { ... };
        auto query = [&](int q) { ans[q]=...; };

        mo.run(add,erase,query);

        // 左右で処理が異なる場合
        // mo.run(add_left,add_right,erase_left,erase_right,query);
*/

struct Mo {
private:
    int N, specified_width;
    vector<pair<int,int>> qs;

    template<class F>
    static void upd(F &f, int i, int l, int r) {
        if constexpr (is_invocable_v<F,int,int>) f(l,r);
        else {
            static_assert(is_invocable_v<F,int>);
            f(i);
        }
    }

    template<class F>
    static void ask(F &f, int q, int l, int r) {
        if constexpr (is_invocable_v<F,int,int,int>) f(q,l,r);
        else {
            static_assert(is_invocable_v<F,int>);
            f(q);
        }
    }

public:
    explicit Mo(int N, int width=-1): N(N), specified_width(width) {}

    int add_query(int l, int r) {
        assert(0<=l && l<=r && r<=N);
        qs.emplace_back(l,r);
        return (int)qs.size()-1;
    }

    int add(int l, int r) { return add_query(l,r); }
    int size() const { return (int)qs.size(); }
    bool empty() const { return qs.empty(); }
    void clear() { qs.clear(); }

    template<class AL, class AR, class EL, class ER, class Q>
    void run(AL add_left, AR add_right, EL erase_left, ER erase_right, Q query) {
        int M=qs.size();
        if(!M) return;

        int w = specified_width>0 ? specified_width
                                  : max(1, N/max(1,(int)sqrt(M)));

        vector<int> ord(M);
        iota(ord.begin(),ord.end(),0);

        sort(ord.begin(),ord.end(),[&](int a,int b){
            int ba=qs[a].first/w, bb=qs[b].first/w;
            if(ba!=bb) return ba<bb;
            return (ba&1) ? qs[a].second>qs[b].second
                          : qs[a].second<qs[b].second;
        });

        int l=0,r=0;
        for(int q:ord){
            auto [ql,qr]=qs[q];

            while(l>ql){ --l; upd(add_left,l,l,r); }
            while(r<qr){ int i=r++; upd(add_right,i,l,r); }
            while(l<ql){ int i=l++; upd(erase_left,i,l,r); }
            while(r>qr){ int i=--r; upd(erase_right,i,l,r); }

            ask(query,q,l,r);
        }
    }

    template<class A, class E, class Q>
    void run(A add, E erase, Q query) {
        run(add,add,erase,erase,query);
    }
};