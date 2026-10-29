#define FPS_NO_DEFAULT_ALIASES
#include "../../../fps-all.cpp"
using mint=LazyMontgomeryModInt<998244353>;
using fps=FormalPowerSeries<mint>;
fps read_poly(int n) { fps f(n); for(auto& x:f) cin>>x; return f; }
template<class V> void print(const V& v) {
  for(size_t i=0;i<v.size();++i) { if(i) cout<<' '; cout<<v[i]; } cout<<'\n';
}
int main() { ios::sync_with_stdio(false); cin.tie(nullptr);
int n; cin >> n; auto f=read_poly(n); int v=0; while(v<n && f[v]==mint(0)) ++v; if(v==n) { print(fps(n)); return 0; } int64_t r=mod_sqrt(f[v].get(),mint::get_mod()); if((v&1)||r<0) { cout<<"-1\n"; return 0; } auto unit=(f>>v)*f[v].inverse(); auto g=fps_pow_unit(unit,mint(1)/mint(2),n-v)*mint(r); print((g<<(v/2)).pre(n));
}
