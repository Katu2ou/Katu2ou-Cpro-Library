/*
  BSGS 行列積・P-recursive 第 N 項の検証。
  独立した逐次計算との比較、行列積の順序、法をまたぐ区間、分母の零、
  固定/動的 mod、2^63 に近い添字、疎な FPS 累乗との係数比較を行う。
*/
#define FPS_NO_DEFAULT_ALIASES
#include "../polynomial-matrix-product.cpp"
#include <random>
#include <limits>

template <class M> using F = FormalPowerSeries<M>;

template <class M>
BSGSMatrix<M> slow_product(const BSGSPolynomialMatrix<M>& a, int n, M start) {
  const int r = (int)a.size();
  BSGSMatrix<M> out(r, vector<M>(r));
  for (int i = 0; i < r; ++i) out[i][i] = 1;
  for (int step = 0; step < n; ++step) {
    BSGSMatrix<M> next(r, vector<M>(r));
    for (int i = 0; i < r; ++i) for (int j = 0; j < r; ++j) {
      M v = 0;
      const auto& f = a[i][j];
      for (int k = (int)f.size()-1; k >= 0; --k) v = v*(start+M(step))+f[k];
      for (int k = 0; k < r; ++k) next[i][k] += v*out[j][k];
    }
    out.swap(next);
  }
  return out;
}

template <class M>
optional<M> slow_recurrence(const vector<F<M>>& p, vector<M> a, int n) {
  const int r = (int)a.size();
  for (int i = 0; i+r <= n; ++i) {
    M lead = p[r].eval(M(i)), value = 0;
    if (lead == M(0)) return nullopt;
    for (int j = 0; j < r; ++j) value -= p[j].eval(M(i))*a[i+j];
    a.push_back(value/lead);
  }
  return a[n];
}

template <class M>
void random_cases() {
  std::mt19937 rng(7406821);
  for (int trial = 0; trial < 65; ++trial) {
    int r = 1+rng()%4, d = rng()%6, n = trial < 10 ? trial : 65+rng()%480;
    M start = M(int(rng()%51)-25);
    BSGSPolynomialMatrix<M> a(r, vector<F<M>>(r));
    for (auto& row : a) for (auto& f : row) {
      f.resize(rng()%(d+2));
      for (auto& c : f) c = M(rng()%31);
      if (trial%4 == 0) f.resize(f.size()+3); // trailing zeros
    }
    assert(polynomial_matrix_product_bsgs(a,n,start) == slow_product(a,n,start));
    vector<F<M>> p(r+1);
    for (auto& f : p) {
      f.resize(d+1);
      for (auto& c : f) c = M(rng()%31);
    }
    // 零分母も含め、初期値からの逐次計算と optional を比較。
    vector<M> initial(r);
    for (auto& c : initial) c = M(rng()%31);
    assert(p_recursive_nth(p,initial,n) == slow_recurrence(p,initial,n));
  }
}

template <class M>
void boundaries() {
  for (int n : {0,1,2,63,64,65,127,128,129,255,256,257,1023,1024,1025}) {
    // 非可換で、法をまたいでも零行列に潰れない行列。
    BSGSPolynomialMatrix<M> a = {{{1},{0,1}}, {{1},{1,1}}};
    for (M start : {M(0),M(-5),M(17)})
      assert(polynomial_matrix_product_bsgs(a,n,start) == slow_product(a,n,start));
  }
  assert(polynomial_matrix_product_bsgs(BSGSPolynomialMatrix<M>{},500).empty());
  assert(polynomial_product_bsgs(F<M>{},0) == M(1));
  assert(polynomial_product_bsgs(F<M>{},500) == M(0));
  assert(polynomial_product_bsgs(F<M>{7,0,0},INT64_MAX) == M(7).pow(INT64_MAX));
  BSGSPolynomialMatrix<M> a = {{{1},{0,1}}, {{},{1}}};
  const int64_t n = INT64_MAX;
  auto product = polynomial_matrix_product_bsgs(a,n,M(3));
  assert(product == BSGSMatrix<M>({{M(1),M(n)*(M(3)+(M(n)-M(1))/M(2))},{M(0),M(1)}}));
  // initial の範囲なら未使用の零分母は問題にしない。
  vector<F<M>> p = {{1},{0}};
  assert(p_recursive_nth(p,vector<M>{9},0) == optional<M>(M(9)));
  assert(!p_recursive_nth(p,vector<M>{9},1));
  p = {{1},{-80,1}};
  assert(p_recursive_nth(p,vector<M>{1},80) == slow_recurrence(p,vector<M>{1},80));
  assert(!p_recursive_nth(p,vector<M>{1},81));
  // p 次以上の多項式、小標数による B の制限、定数行列の累乗。
  if (M::get_mod() <= 257) {
    int deg = M::get_mod()+3;
    BSGSPolynomialMatrix<M> high = {{{}}};
    high[0][0].resize(deg+1);
    high[0][0][0] = high[0][0][deg] = 1;
    assert(polynomial_matrix_product_bsgs(high,700) == slow_product(high,700,M(0)));
    BSGSPolynomialMatrix<M> near = {{{}}};
    near[0][0].resize(M::get_mod()/4+1,M(1));
    assert(polynomial_matrix_product_bsgs(near,700) == slow_product(near,700,M(0)));
  }
}

template <class M>
void power_coefficients() {
  std::mt19937 rng(4322341);
  const int len = std::min<int>(M::get_mod(),300);
  for (int trial = 0; trial < 70; ++trial) {
    F<M> f(rng()%7);
    for (auto& c : f) c = M(rng()%11);
    if (trial%4 == 0 && !f.empty()) f[0] = 0;
    int64_t k = trial%3 == 0 ? INT64_MAX-trial : int(rng()%80);
    auto expected = fps_pow_sparse(f,k,len);
    for (int n : {0,std::min(1,len-1),len/2,len-1})
      assert(fps_pow_coefficient_bsgs(f,k,n) == expected[n]);
  }
  for (int n : {0,1,len-1}) {
    assert(fps_pow_coefficient_bsgs(F<M>{},0,n) == M(n == 0));
    assert(fps_pow_coefficient_bsgs(F<M>{},7,n) == M(0));
    assert(fps_pow_coefficient_bsgs(F<M>{0,0,3},4,n) == M(n == 8 ? 81 : 0));
  }
}

int main() {
  random_cases<LazyMontgomeryModInt<998244353>>();
  random_cases<LazyMontgomeryModInt<1000000007>>();
  random_cases<LazyMontgomeryModInt<97>>();
  boundaries<LazyMontgomeryModInt<998244353>>();
  boundaries<LazyMontgomeryModInt<3>>();
  boundaries<LazyMontgomeryModInt<17>>();
  boundaries<LazyMontgomeryModInt<97>>();
  boundaries<LazyMontgomeryModInt<257>>();
  power_coefficients<LazyMontgomeryModInt<998244353>>();
  power_coefficients<LazyMontgomeryModInt<1000000007>>();
  power_coefficients<LazyMontgomeryModInt<17>>();
  using D = ArbitraryLazyMontgomeryModInt<7301>;
  for (int p : {998244353,97,1000000007,257}) {
    D::set_mod(p);
    random_cases<D>();
    power_coefficients<D>();
  }
  // 多点評価による初期化 (d>32) を通す。
  using M = LazyMontgomeryModInt<998244353>;
  BSGSPolynomialMatrix<M> high = {{{}}};
  high[0][0].resize(51,M(1));
  assert(polynomial_matrix_product_bsgs(high,1300) == slow_product(high,1300,M(0)));
  std::cout << "BSGS matrices / P-recursive / FPS coefficient / periods / singularities: OK\n";
}
