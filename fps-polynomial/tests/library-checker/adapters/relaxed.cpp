#define FPS_NO_DEFAULT_ALIASES
#include "../../../relaxed-multiplication.cpp"
using mint=LazyMontgomeryModInt<998244353>;
using fps=FormalPowerSeries<mint>;
fps read_poly(int n) { fps f(n); for(auto& x:f) cin>>x; return f; }
template<class V> void print(const V& v) {
  for(size_t i=0;i<v.size();++i) { if(i) cout<<' '; cout<<v[i]; } cout<<'\n';
}
int main() { ios::sync_with_stdio(false); cin.tie(nullptr);
int n,m; cin >> n >> m; auto a=read_poly(n),b=read_poly(m); if(!n||!m) { print(fps{}); return 0; } RelaxedMultiplication<mint> mul; fps c(n+m-1); for(int i=0;i<n+m-1;++i) c[i]=mul.append(i<n?a[i]:mint(0),i<m?b[i]:mint(0)); print(c);
}
