/*
  <Rollback Mo's Algorithm / 巻き戻し Mo>
    - 区間クエリ [l,r) をオフラインで処理する
    - 通常の Mo と異なり erase を使わず，add と rollback によって状態を戻す
    - 「追加は簡単だが削除が難しい」データ構造（Rollback UnionFind，XOR basis 等）に有効

    [概要]
        - 短い区間は空状態から直接構築して query 後に rollback する
        - 長い区間は left のブロックごとに処理する
        - 同一ブロックでは右側を add_right で単調に伸ばす
        - 左側は snapshot 後に add_left で一時的に追加し，query 後に rollback する
        - rollback に必要な変更履歴の保存自体は，使用するデータ構造側で実装する

    [実装/関数]
        - MoRollback mo(N, width=-1)
            長さ N の対象に対する Rollback Mo を作る
            width<=0 ならブロック幅を自動決定する

        - int add_query(int l,int r), int add(int l,int r)
            クエリ [l,r) を登録し，クエリ番号を返す //0indexed, 閉開区間

        - int size(), bool empty(), void clear()
            登録クエリ数，空判定，全クエリ削除

        - run(add,query,reset,snapshot,rollback)
            左右で追加処理が同じ場合

        - run(add_left,add_right,query,reset,snapshot,rollback)
            左右で追加処理が異なる場合

    [コールバック]
        - add_left(i), add_right(i)
            今回追加された位置 i を受け取る

        - add_left(l,r), add_right(l,r)
            操作後の現在区間 [l,r) を受け取る
            add_left で追加された位置は l，add_right では r-1

        - query(q) または query(q,l,r)
            q 番目のクエリの答えを現在状態から保存する

        - reset()
            データ構造を空状態に戻し，rollback 用履歴等も初期化する

        - snapshot()
            現在状態を rollback 先として記録する
            通常は history.size() を保存すればよい

        - rollback()
            直前の snapshot() 時点まで変更を巻き戻す

    [計算時間]
        - add 1回を O(T)，ブロック幅を w とすると
              O((N^2/w + Qw)T + Q log Q)
        - 自動設定 w ≈ N/sqrt(Q) では概ね O(N sqrt(Q) T + Q log Q)
        - Q≈N なら通常 O(N sqrt(N) T)

    [備考]
        - 同一ブロック内では右端を単調増加させる必要があるため snake order は使わない
        - rollback は LIFO で変更を戻せる必要がある
        - snapshot は通常データ構造全体をコピーせず，変更履歴の位置だけを保存する
        - query は原則として状態を変更しないこと
        - 空区間 [l,l) にも対応する

    [使用例]
        MoRollback mo(N);
        for(int q=0;q<Q;q++) mo.add_query(L[q],R[q]);
        mo.run(add,query,reset,snapshot,rollback);
        // 左右で追加処理が違うなら
        // mo.run(add_left,add_right,query,reset,snapshot,rollback);

    [verified at]
        - g++ 14.2.0, -std=gnu++23
        - ランダム区間和 5000 ケース
        - 左右非可換な deque 構築 2000 ケース

        - https://www.luogu.com.cn/record/295166535 (指定区間に含まれる，異なる二要素間の距離の最大値)
*/

struct MoRollback {
private:
    int N, specified_width;
    vector<pair<int,int>> qs;

    template<class F>
    static void upd(F &f,int i,int l,int r) {
        if constexpr(is_invocable_v<F,int,int>) f(l,r);
        else { static_assert(is_invocable_v<F,int>); f(i); }
    }

    template<class F>
    static void ask(F &f,int q,int l,int r) {
        if constexpr(is_invocable_v<F,int,int,int>) f(q,l,r);
        else { static_assert(is_invocable_v<F,int>); f(q); }
    }

public:
    explicit MoRollback(int n,int width=-1): N(n), specified_width(width) {}

    int add_query(int l,int r) {
        assert(0<=l && l<=r && r<=N);
        qs.emplace_back(l,r);
        return (int)qs.size()-1;
    }

    int add(int l,int r) { return add_query(l,r); }
    int size() const { return (int)qs.size(); }
    bool empty() const { return qs.empty(); }
    void clear() { qs.clear(); }

    template<class AL,class AR,class Q,class RESET,class SNAPSHOT,class ROLLBACK>
    void run(AL add_left,AR add_right,Q query,RESET reset,SNAPSHOT snapshot,ROLLBACK rollback) {
        int M=qs.size();
        if(!M) return;

        int w=specified_width>0 ? specified_width : max(1,N/max(1,(int)sqrt(M)));
        vector<int> ord(M);
        iota(ord.begin(),ord.end(),0);

        sort(ord.begin(),ord.end(),[&](int a,int b){
            int ba=qs[a].first/w,bb=qs[b].first/w;
            if(ba!=bb) return ba<bb;
            return qs[a].second<qs[b].second;
        });

        // 短い区間
        reset();
        for(int q:ord) if(qs[q].second-qs[q].first<w) {
            auto [ql,qr]=qs[q];
            snapshot();
            int l=ql,r=ql;
            while(r<qr) { int i=r++; upd(add_right,i,l,r); }
            ask(query,q,l,r);
            rollback();
        }

        // 長い区間
        int last_block=-1,r=0;
        for(int q:ord) {
            auto [ql,qr]=qs[q];
            if(qr-ql<w) continue;

            int block=ql/w,br=min(N,(block+1)*w);
            if(block!=last_block) { reset(); last_block=block; r=br; }

            while(r<qr) { int i=r++; upd(add_right,i,br,r); }

            snapshot();
            int l=br;
            while(l>ql) { --l; upd(add_left,l,l,r); }
            ask(query,q,l,r);
            rollback();
        }
    }

    template<class A,class Q,class RESET,class SNAPSHOT,class ROLLBACK>
    void run(A add,Q query,RESET reset,SNAPSHOT snapshot,ROLLBACK rollback) {
        run(add,add,query,reset,snapshot,rollback);
    }
};