#define FPS_NO_DEFAULT_ALIASES
#include "../../../fps-sparse.cpp"
using mint=LazyMontgomeryModInt<998244353>;
using fps=FormalPowerSeries<mint>;
fps read_poly(int n) { fps f(n); for(auto& x:f) cin>>x; return f; }
template<class V> void print(const V& v) {
  for(size_t i=0;i<v.size();++i) { if(i) cout<<' '; cout<<v[i]; } cout<<'\n';
}
int main() { ios::sync_with_stdio(false); cin.tie(nullptr);
int n,k; cin >> n >> k; SparseFPS<mint> f(k); for(auto& [i,c]:f) cin >> i >> c; auto g=fps_sqrt_sparse(f,n); if (g.empty()) cout << "-1\n"; else print(g);
}
