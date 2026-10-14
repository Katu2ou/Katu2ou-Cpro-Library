# 典型的なクエリ




## 配列に対するクエリ
- 連続部分列と値rが指定され，その中のr以下の要素の個数を求める
    - マージソートツリー
    - Wavelet Matrix
    - オフライン + BIT ? 

- iが与えられて，a[i]より右側にa[i]以上の値が存在するか判定する
    - cartesian tree でO(N)/O(1)

- 区間chmax + 区間和
    - segment tree beats ! 

- 区間chmax
    - beatsがなくてもいける(普通にlazy segでoperationをchmaxとすればよい)

- 値vとindex iが与えられて，a[i+1],...,a[n-1]にv以上の値があるか判定し，存在するなら
  最も左のindexを返す
    - max segtree上の二分探索でO(log n)
    - 