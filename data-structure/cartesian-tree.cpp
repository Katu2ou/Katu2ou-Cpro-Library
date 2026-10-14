/*
  <Cartesian Tree>
    - 数列から誘導されるCartesian Treeとは，次の条件を満たす二分木のこと
    - 根は数列全体の最小要素に対応
    - 根の左側頂点は，「数列全体の最小要素より左側の区間」における最小要素
    - 根の右側頂点は，「数列全体の最小要素より右側の区間」における最小要素
    - 以降は再帰的に頂点数nの二分木を構成する
    (ただし，最小要素が複数ある場合は，そのうちindex最小のものを選ぶ)

    - stackを用いてO(n)で構築できる

    - 頂点(u,v)のLCAが区間[u,v]の最小値に対応することから，前計算O(n)クエリO(1)のRMQが作れる

    [実装/関数]
        - CartesianTree(vector<T> v)
            : vに対するCartesian Treeを作成する．各要素は，親，左の子，右の子のindexを持つ(存在しない場合は-1)

    [計算時間]

    [備考]
        - 上から大きい順にしたいときは-1倍して入れる

    
    [参照]
        - https://drken1215.hatenablog.com/entry/2023/10/19/025800

    [verified at]
        - 
    
    [使用例]
        
        
*/



template<class T> struct CartesianTree {
    int root;  // root
    vector<int> par, left, right;

    CartesianTree() {}
    CartesianTree(const vector<T>& v) :
    par(v.size(), -1), left(v.size(), -1), right(v.size(), -1) {
        vector<int> st(v.size(), 0);
        root = 0;
        int top = 0;
        for (int i = 1; i < v.size(); ++i) {
            if (v[st[top]] > v[i]) {
                while (top >= 1 && v[st[top - 1]] > v[i]) --top;
                left[i] = st[top];
                par[left[i]] = i;
                if (top == 0) {
                    root = i;
                } else {
                    par[i] = st[top - 1];
                    right[par[i]] = i;
                }
                st[top] = i;
            } else {
                par[i] = st[top];
                right[par[i]] = i;
                st[++top] = i;
            }
        }
    }
};