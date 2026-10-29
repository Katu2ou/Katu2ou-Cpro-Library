#define FPS_NO_DEFAULT_ALIASES
#include "../../../polynomial-matrix-product.cpp"
using mint=LazyMontgomeryModInt<998244353>;
using fps=FormalPowerSeries<mint>;
fps read_poly(int n) { fps f(n); for(auto& x:f) cin>>x; return f; }
template<class V> void print(const V& v) {
  for(size_t i=0;i<v.size();++i) { if(i) cout<<' '; cout<<v[i]; } cout<<'\n';
}
int main() { ios::sync_with_stdio(false); cin.tie(nullptr);
int t; cin>>t; while(t--) { int64_t n; cin>>n; auto a=p_recursive_nth(vector<fps>{{1,1},{-3,-1},{1}},vector<mint>{1,1},n); assert(a); cout<<*a<<"\n"; }
}
