#define FPS_NO_DEFAULT_ALIASES
#include "../../../polynomial-gcd.cpp"
using mint=LazyMontgomeryModInt<998244353>;
using fps=FormalPowerSeries<mint>;
fps read_poly(int n) { fps f(n); for(auto& x:f) cin>>x; return f; }
template<class V> void print(const V& v) {
  for(size_t i=0;i<v.size();++i) { if(i) cout<<' '; cout<<v[i]; } cout<<'\n';
}
int main() { ios::sync_with_stdio(false); cin.tie(nullptr);
int n,m; cin >> n >> m; auto f=read_poly(n),g=read_poly(m); auto [ok,h]=polynomial_mod_inverse(f,g); if(!ok) cout<<"-1\n"; else { cout<<h.size()<<"\n"; print(h); }
}
