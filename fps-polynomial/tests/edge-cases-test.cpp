/*
  境界条件・別名参照・小標数の全列挙・動的法の回帰テスト。
  tests/run-tests.sh に含まれる。独立した係数計算と比較する。
*/
#define FPS_NO_DEFAULT_ALIASES
#include "../fps-all.cpp"
#include <random>

template<class M> using F=FormalPowerSeries<M>;
template<class M>
F<M> slow_product(const F<M>& a,const F<M>& b,int n) {
  F<M> c(n);
  for(int i=0;i<(int)a.size();++i)
    for(int j=0;j<(int)b.size() && i+j<n;++j) c[i+j]+=a[i]*b[j];
  return c;
}

template<class M>
void alias_checks() {
  F<M> original{2,3,4,5};
  for(int i=0;i<4;++i) {
    auto f=original;
    M scale=f[i];
    f*=f[i];
    for(int j=0;j<4;++j) assert(f[j]==original[j]*scale);
  }
  auto f=original; f+=f;
  for(int i=0;i<4;++i) assert(f[i]==original[i]*M(2));
  f=original; f-=f; assert(f==F<M>(4));
  f=original; f*=f; assert(f==slow_product(original,original,7));
  f=original; f/=f; assert(f==F<M>{M(1)});
  f=original; f%=f; assert(f.empty());
  // Nonzero constant modulus, including a=0 and trailing-zero representations.
  for(auto a : {F<M>{},F<M>{0},original}) {
    auto [ok,h]=polynomial_mod_inverse(a,F<M>{7,0,0});
    assert(ok && h.empty());
  }
}

template<class M>
void exhaustive_sqrt() {
  const int p=M::get_mod();
  for(int n=1;n<=std::min(p,4);++n) {
    int count=1; for(int i=0;i<n;++i) count*=p;
    auto decode=[&](int value) { F<M> f(n); for(auto& c:f) { c=M(value%p); value/=p; } return f; };
    auto encode=[&](const F<M>& f) { int v=0; for(int i=n-1;i>=0;--i) v=v*p+f[i].get(); return v; };
    std::vector<bool> has_sqrt(count);
    for(int id=0;id<count;++id) {
      auto g=decode(id); has_sqrt[encode(slow_product(g,g,n))]=true;
    }
    for(int id=0;id<count;++id) {
      auto f=decode(id), a=fps_sqrt(f,n), b=fps_sqrt_sparse(f,n);
      assert(!a.empty()==has_sqrt[id]);
      assert(!b.empty()==has_sqrt[id]);
      if(has_sqrt[id]) {
        assert(a.size()==size_t(n) && b.size()==size_t(n));
        assert(slow_product(a,a,n)==f && slow_product(b,b,n)==f);
      }
    }
  }
}

template<class M>
void wrapped_sampling() {
  const int p=M::get_mod(), n=p-1;
  F<M> f(n); for(int i=0;i<n;++i) f[i]=M(i*i+3*i+7);
  std::vector<M> values(n); for(int i=0;i<n;++i) values[i]=f.eval(M(i));
  for(M start:{M(0),M(-1),M(p-2),M(p/2)}) {
    auto out=sampling_point_shift(values,start,3*p+7);
    for(int i=0;i<(int)out.size();++i) assert(out[i]==f.eval(start+M(i)));
  }
}

void dynamic_moduli() {
  using M=ArbitraryLazyMontgomeryModInt<20261029>;
  std::mt19937 rng(332901);
  for(uint32_t p:{97u,257u,998244353u,1000000007u,1073741789u,97u}) {
    M::set_mod(p);
    int n=std::min<uint32_t>(p-1,257);
    F<M> f(n),g(n);
    for(auto& c:f) c=M(rng());
    for(auto& c:g) c=M(rng());
    assert(f*g==slow_product(f,g,2*n-1));
    f[0]=M(1);
    assert(slow_product(f,f.inv(n),n)==F<M>{M(1)}.pre(n));
    assert(f.log(n).exp(n)==f);
    assert(fps_inv_sparse(f,n)==f.inv(n));
    alias_checks<M>();
  }
}

void maximum_ntt_length() {
  using M=LazyMontgomeryModInt<257>;
  // 257-1=256。変換長が法の最大長と一致する場合を、スタック上の NTT でも検査。
  // 静的オブジェクトだけでは配列末尾の不正参照を sanitizer が見逃しうる。
  NTT<M> engine;
  for(int n:{41,64,65,127,128}) {
    F<M> a(n), b(n);
    for(int i=0;i<n;++i) a[i]=M(i+1),b[i]=M(2*i+1);
    auto expected=slow_product(a,b,2*n-1);
    assert(engine.multiply(a,b)==expected);
    assert(a*b==expected);
    assert(engine.multiply(a,a)==slow_product(a,a,2*n-1));
  }
  F<M> a(129,M(1)),b(129,M(2)); // 最大長を超えると CRT に切り替わる。
  assert(a*b==slow_product(a,b,257));
}

int main() {
  alias_checks<LazyMontgomeryModInt<998244353>>();
  alias_checks<LazyMontgomeryModInt<1000000007>>();
  exhaustive_sqrt<LazyMontgomeryModInt<3>>();
  exhaustive_sqrt<LazyMontgomeryModInt<5>>();
  wrapped_sampling<LazyMontgomeryModInt<17>>();
  wrapped_sampling<LazyMontgomeryModInt<97>>();
  wrapped_sampling<LazyMontgomeryModInt<257>>();
  dynamic_moduli();
  maximum_ntt_length();
  std::cout<<"alias / constant modulus / exhaustive sqrt / modular wrap / dynamic mod / NTT limit: OK\n";
}
