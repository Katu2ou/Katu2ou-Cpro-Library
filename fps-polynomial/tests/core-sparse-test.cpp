// g++-14 -std=c++17 -O2 core-sparse-test.cpp -o /tmp/fps-core-test && /tmp/fps-core-test
#include "../fps-newton.cpp"
#include "../fps-sparse.cpp"
#include <limits>
#include <random>

template <typename M>
FormalPowerSeries<M> naive_product(const FormalPowerSeries<M>& a,
                                  const FormalPowerSeries<M>& b, int n) {
  FormalPowerSeries<M> c(n);
  for (int i=0; i<(int)a.size() && i<n; ++i)
    for (int j=0; j<(int)b.size() && i+j<n; ++j) c[i+j] += a[i]*b[j];
  return c;
}
template <typename M>
FormalPowerSeries<M> naive_power(FormalPowerSeries<M> f, uint64_t k, int n) {
  auto ret = FormalPowerSeries<M>{1}.pre(n);
  while (k) {
    if (k&1) ret = naive_product(ret, f, n);
    f = naive_product(f, f, n);
    k >>= 1;
  }
  return ret;
}
template <typename M>
void verify_core(int limit) {
  using F=FormalPowerSeries<M>;
  mt19937_64 rng(20261029);
  assert(F{}.inv(0).empty() && F{}.log(0).empty() && F{}.exp(0).empty());
  assert(F{}.pow(0,0).empty());
  assert(F{7}.exp(0).empty());
  assert(fps_sqrt(F{},0).empty());
  for (int n=1; n<=limit; ++n) {
    for (int iteration=0; iteration<5; ++iteration) {
      F f(n), g(n);
      for(int j=0;j<n;++j) {
        if(rng()%5==0) f[j]=M(rng()%M::get_mod());
        g[j]=M(rng()%M::get_mod());
      }
      auto s=fps_to_sparse(f);
      assert(fps_mul_sparse(g,s,n)==naive_product(g,f,n));
      int64_t k=iteration==0?numeric_limits<int64_t>::max():iteration;
      auto expected=naive_power(f,k,n);
      assert(f.pow(k,n)==expected);
      assert(fps_pow_sparse(s,k,n)==expected);
      f[0]=M(1); s=fps_to_sparse(f);
      F inverse(n); inverse[0]=1;
      for(int i=1;i<n;++i)
        for(int j=1;j<=i;++j) inverse[i]-=f[j]*inverse[i-j];
      assert(f.inv(n)==inverse);
      assert(fps_inv_sparse(s,n)==inverse);
      assert(fps_div_sparse(g,s,n)==naive_product(g,inverse,n));
      assert(fps_log_sparse(s,n)==f.log(n));
      auto log=naive_product(f.diff(),inverse,n-1).integral();
      assert(log==f.log(n));
      assert(fps_pow_unit_sparse(s,M(7)/M(3),n)==fps_pow_unit(f,M(7)/M(3),n));
      f[0]=M(0); s=fps_to_sparse(f);
      F exp(n); exp[0]=1;
      for(int i=1;i<n;++i) {
        for(int j=1;j<=i;++j) exp[i]+=M(j)*f[j]*exp[i-j];
        exp[i]/=M(i);
      }
      assert(f.exp(n)==exp && fps_exp_sparse(s,n)==exp);
      if(iteration==0) g=F(n);
      if(iteration==1) g[0]=M(0);
      F sq=naive_product(g,g,n);
      auto root=fps_sqrt(sq,n), sparse_root=fps_sqrt_sparse(sq,n);
      assert(root.size()==size_t(n) && sparse_root.size()==size_t(n));
      assert(naive_product(root,root,n)==sq);
      assert(naive_product(sparse_root,sparse_root,n)==sq);
    }
  }
  assert(fps_sqrt(F{0,1},4).empty());
  assert(fps_sqrt_sparse(F{0,1},4).empty());
  assert(fps_sqrt(F{0,0,0,1},2)==F(2));
  F a={1,2,3,0,0}, b={1,1,0,0};
  assert((a/b==F{-1,3}));
  assert(a%b==F{2});
  assert((a/a)==F{1});
  assert((a%a).empty());
  // Newton: Catalan series solves g=1+x*g^2.
  int n=min(limit,65);
  auto catalan=fps_newton(F{1},n,[](const F& g,int n) {
    return make_pair((g-F{1}-((g*g)<<1)).pre(n), (F{1}-(g<<1)*M(2)).pre(n));
  });
  assert(catalan==(F{1}+((catalan*catalan)<<1)).pre(n));
  cerr<<"core/sparse checks passed, mod="<<M::get_mod()<<'\n';
}

int main() {
  verify_core<LazyMontgomeryModInt<998244353>>(90);
  verify_core<LazyMontgomeryModInt<1000000007>>(90);
  verify_core<LazyMontgomeryModInt<17>>(17);
  using M=LazyMontgomeryModInt<998244353>;
  using F=FormalPowerSeries<M>;
  // Enough coefficients to run the NTT path, including non-power-of-two lengths.
  F f(513),g(300);
  for(int i=0;i<(int)f.size();++i) f[i]=M(i*i+7);
  for(int i=0;i<(int)g.size();++i) g[i]=M(3*i+9);
  assert(f*g==naive_product(f,g,812));
  f[0]=1;
  assert(naive_product(f,f.inv(513),513)==F{1}.pre(513));
  assert(f.log(513).exp(513)==f);
}
