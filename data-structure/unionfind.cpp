/*
  <UnionFind>
    - 

    [実装/関数]
        - UnionFind uf(n) : 頂点数を指定
        - void merge(int a, int b) : 結合
        - int find(int i) : iが所属する木の親のindexを返す
        - int size(int i) : iが所属する木のサイズを返す
        - bool connected(int a, int b) : aとbが連結であるかを返す
    [計算時間]
        - 
    
    [備考]
        - 
    
    [参照]
        - 

    [verified at]
        - 
    
    [使用例]
        
        
*/

class UnionFind
{
public:
	UnionFind() = default;
	explicit UnionFind(size_t n)
		: m_parentsOrSize(n, -1) {}
	int find(int i)
	{	if (m_parentsOrSize[i] < 0) return i;
		return (m_parentsOrSize[i] = find(m_parentsOrSize[i]));}
	void merge(int a, int b)
	{	a = find(a);
		b = find(b);
		if (a != b){
			if (-m_parentsOrSize[a] < -m_parentsOrSize[b]) std::swap(a, b);
			m_parentsOrSize[a] += m_parentsOrSize[b];
			m_parentsOrSize[b] = a;
		}
	}
	bool connected(int a, int b)
	{	return (find(a) == find(b));}
	int size(int i)
	{	return -m_parentsOrSize[find(i)];}
private:
	std::vector<int> m_parentsOrSize;
};