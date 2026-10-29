#define FPS_NO_DEFAULT_ALIASES
#include "../../../fps-newton.cpp"
using mint=LazyMontgomeryModInt<998244353>;
using fps=FormalPowerSeries<mint>;
fps read_poly(int n) { fps f(n); for(auto& x:f) cin>>x; return f; }
template<class V> void print(const V& v) {
  for(size_t i=0;i<v.size();++i) { if(i) cout<<' '; cout<<v[i]; } cout<<'\n';
}
int main() { ios::sync_with_stdio(false); cin.tie(nullptr);
int n; cin >> n; auto f=read_poly(n); print(fps_newton(fps{f[0].inverse()},n,[&](const fps& g,int d) { auto a=f.pre(d); return make_pair((a*g-mint(1)).pre(d),a); }));
}
