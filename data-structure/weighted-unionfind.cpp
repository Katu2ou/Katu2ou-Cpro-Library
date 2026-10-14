/*
  <重み付きUnion-Find>
    - 

    [実装/関数]
        - WeightedUnionFind<T> uf(n) : Tはポテンシャルの値の型
        - int merge(int a, int b T c) : aとbを連結にして，bの重みはaの重みよりcだけ大きくなるようにする
        - int find(int a) : aの属する連結成分のrootのindexを返す
        - T diff(int a, int b) : aとbが同じ連結成分に属する場合に，(bの重み) - (aの重み) を返す
        - int size(int a) : aの属する連結成分のサイズを返す
        - bool connected(int a, int b) : aとbが同じ連結成分に属するか判定する

    [計算時間]
        - 
    
    [備考]
        - 指定した重みの差を満たしているかどうかは，mergeを全て終えた後に各(a,b)に対してdiff(a,b)の値が
        適切であるかを確認すればよい．(操作によっては，指定した重みの差を満たすような割り当てが存在しないかもしれない)
    
    [参照]
        - 

    [verified at]
        - https://atcoder.jp/contests/abc328/submissions/47613386
    
    [使用例]
        
*/

template <class Type>
class WeightedUnionFind{
public:
	WeightedUnionFind() = default;
	explicit WeightedUnionFind(size_t n)
		: m_parentsOrSize(n, -1)
		, m_diffWeights(n) {}
	int find(int i){
		if (m_parentsOrSize[i] < 0){
			return i;
		}
		const int root = find(m_parentsOrSize[i]);
		m_diffWeights[i] += m_diffWeights[m_parentsOrSize[i]];
		return (m_parentsOrSize[i] = root);
	}
	void merge(int a, int b, Type w){
		w += weight(a);
		w -= weight(b);
		a = find(a);
		b = find(b);
		if (a != b){
			if (-m_parentsOrSize[a] < -m_parentsOrSize[b]){
				std::swap(a, b);
				w = -w;
			}
			m_parentsOrSize[a] += m_parentsOrSize[b];
			m_parentsOrSize[b] = a;
			m_diffWeights[b] = w;
		}
	}
	Type diff(int a, int b){
		return (weight(b) - weight(a));
	}
	bool connected(int a, int b){
		return (find(a) == find(b));
	}
	int size(int i){
		return -m_parentsOrSize[find(i)];
	}
private:
	std::vector<int> m_parentsOrSize;
	std::vector<Type> m_diffWeights;
	Type weight(int i){
		find(i); 
		return m_diffWeights[i];
	}
};