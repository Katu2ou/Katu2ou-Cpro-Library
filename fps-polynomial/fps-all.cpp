/*
<FPS 拡張ライブラリ 全機能 / C++17>
  [使い方]
    #define FPS_NO_DEFAULT_ALIASES
    #include "fps-all.cpp"
    using mint = LazyMontgomeryModInt<998244353>;
    using fps = FormalPowerSeries<mint>;
    int main() {
      fps f = {0, 1, 1};
      auto g = fps_compositional_inverse(f, 16);
      auto x = fps_compose(f, g, 16); // {0,1,0,...}
    }
    デフォルトの mint / fps は mod 1000000007。上の define/using を省略しても使える。
    必要な機能の .cpp だけを include してもよい（依存先は自動で読み込む）。
    貼り付け運用では依存先を先に貼り、ローカル .cpp の include 行を取り除く。

  [機能一覧: 関数名 / ファイル / 計算量]
    f.inv(N), f.log(N), f.exp(N), f.pow(k,N)
      formal-power-series.cpp / O(N log N)、整数 pow は追加 O(log k)
    fps_newton(initial,N,equation), fps_pow_unit(f,alpha,N)
      fps-newton.cpp / 評価が O(N log N) なら Newton 全体も O(N log N)
    fps_sqrt(f,N)
      fps-sqrt.cpp / O(N log N + log^2 p)
    fps_mul_sparse, fps_div_sparse, fps_inv_sparse, fps_log_sparse,
    fps_exp_sparse, fps_pow_sparse, fps_pow_unit_sparse, fps_sqrt_sparse
      fps-sparse.cpp / O(N(K+1)+K)、K=非零項数（定数の pow/sqrt は別途）
    fps_compose(f,g,N), fps_compositional_inverse(f,N), fps_power_projection(f,N)
      fps-composition.cpp / O(N log^2 N)
    half_gcd(a,b), polynomial_gcd(a,b), polynomial_extgcd(a,b),
    polynomial_mod_inverse(a,b)
      polynomial-gcd.cpp / O(N log^2 N)
    MultipointEvaluation(f,xs), ProductTree<mint>(xs)
      multipoint-evaluation.cpp / O(N log^2 N) (次数と点数が同程度の場合)
    polynomial_interpolation(xs,ys)
      polynomial-interpolation.cpp / O(N log^2 N)
    interpolate_consecutive(ys,a), sampling_point_shift(ys,a,M)
      sampling-point-shift.cpp / O(N), O((N+M) log(N+M))
    bernoulli_numbers<mint>(k), faulhaber_polynomial<mint>(k),
    power_sum_polynomial<mint>(k), power_sums(mint(N),k), polynomial_sum(f,mint(N))
      faulhaber.cpp / O(k log k)
    prefix_sum_polynomial(f), prefix_sum_polynomial_exclusive(f)
      prefix-sum-polynomial.cpp / O(N log N)
    RelaxedMultiplication<mint>::append(a_i,b_i)
      relaxed-multiplication.cpp / N 回合計 O(N log^2 N)
    polynomial_matrix_product_bsgs(A,N,start), polynomial_product_bsgs(f,N,start),
    p_recursive_nth(p,initial,N), fps_pow_coefficient_bsgs(f,k,N)
      polynomial-matrix-product.cpp / 行列サイズ・係数次数が固定なら O(sqrt(N) log(N+2))
      遠くの 1 項のみを計算。定数行列は O(log(N+1))、N>=p は周期を利用。

  [各関数の入力と返り値の早見表]
    fps は通常の係数列、SparseFPS<mint> は (次数,非零係数) の列。
    以下の「N 項の係数列」は、次数 0 以上 N 未満を並べた長さ N の fps を指す。
    省略できる引数・入力条件・失敗時の扱いの詳細は各ファイル先頭を参照。

    f.inv(N): 入力は定数項が非零の f と項数 N。
      返り値は 1/f の N 項の係数列。
    f.log(N): 入力は定数項が 1 の f と項数 N。
      返り値は log(f) の N 項の係数列。
    f.exp(N): 入力は定数項が 0 の f と項数 N。
      返り値は exp(f) の N 項の係数列。
    f.pow(k,N): 入力は f、非負整数の指数 k、項数 N。
      返り値は f^k の N 項の係数列。
    fps_newton(initial,N,equation): 入力は初期近似、項数、方程式と導関数を返す関数。
      返り値は方程式を mod x^N で満たす解の N 項の係数列。
    fps_pow_unit(f,alpha,N): 入力は定数項 1 の f、mint 型の指数 alpha、項数 N。
      返り値は exp(alpha*log(f)) の N 項の係数列。
    fps_sqrt(f,N): 入力は f と項数 N。
      返り値は g*g=f mod x^N を満たす N 項の係数列 g。存在しなければ空列。
    mod_sqrt(a,p): 入力は整数 a と素数 p。
      返り値は r*r=a mod p を満たす整数 r。存在しなければ -1。

    fps_to_sparse(f): 入力は通常の係数列 f。
      返り値は非零項を次数の昇順に並べた SparseFPS<mint>。
    fps_mul_sparse(f,s,N): 入力は通常の f、疎な s、項数 N。
      返り値は f*s の N 項の係数列。
    fps_div_sparse(f,s,N): 入力は通常の f、定数項が非零の疎な s、項数 N。
      返り値は f/s を FPS として計算した N 項の係数列。
    fps_inv_sparse(s,N): 入力は疎な s と項数 N。返り値は 1/s の N 項の係数列。
    fps_log_sparse(s,N): 入力は疎な s と項数 N。返り値は log(s) の N 項の係数列。
    fps_exp_sparse(s,N): 入力は疎な s と項数 N。返り値は exp(s) の N 項の係数列。
    fps_pow_sparse(s,k,N): 入力は疎な s、非負整数 k、項数 N。
      返り値は s^k の N 項の係数列。
    fps_pow_unit_sparse(s,alpha,N): 入力は定数項 1 の疎な s、mint 型の指数、項数 N。
      返り値は exp(alpha*log(s)) の N 項の係数列。
    fps_sqrt_sparse(s,N): 入力は疎な s と項数 N。
      返り値は g*g=s mod x^N を満たす N 項の係数列 g。存在しなければ空列。

    fps_compose(f,g,N): 入力は外側の f、代入する g、項数 N。
      返り値は f(g(x)) の N 項の係数列。
    fps_compositional_inverse(f,N): 入力は逆関数を求める f と項数 N。
      返り値は f(g(x))=g(f(x))=x mod x^N を満たす N 項の係数列 g。
    fps_power_projection(f,N): 入力は f と項数 N。
      返り値は f^0,f^1,...,f^(N-1) の x^(N-1) の係数を順に並べた長さ N の fps。
    half_gcd(a,b): 入力は 2 つの多項式 a,b。
      返り値は互除法を途中まで進める 2 行 2 列の多項式行列。apply(a,b) で作用させる。
    polynomial_gcd(a,b): 入力は 2 つの多項式。返り値は最高次係数 1 の gcd の係数列。
    polynomial_extgcd(a,b): 入力は 2 つの多項式。
      返り値は {g,s,t} の tuple。g は gcd、s*a+t*b=g。
    polynomial_mod_inverse(a,b): 入力は逆元を求める a と、法とする多項式 b。
      返り値は {true,h} の pair（a*h=1 mod b）。逆元がなければ {false,{}}。

    MultipointEvaluation(f,xs): 入力は多項式の係数列 f と評価点の列 xs。
      返り値は xs と同じ順序の評価値の列 vector<mint>。
    ProductTree<mint>(xs): 評価点の列を受け取って積木を構築する。
      tree.evaluate(f) は多項式 f を受け取り、保存した各点での値を vector<mint> で返す。
    polynomial_interpolation(xs,ys): 入力は評価点 xs と各点での値 ys。
      返り値はそれらを満たす多項式の係数列 fps（長さ xs.size()）。
    polynomial_interpolation(tree,ys): 入力は構築済みの積木と評価値の列。
      返り値は保存した評価点と ys を満たす多項式の係数列 fps。
    interpolate_consecutive(ys,a): 入力は f(0),...,f(n-1) の値の列と評価点 a。
      返り値は f(a) を表す mint。
    sampling_point_shift(ys,a,M): 入力は f(0),...,f(n-1)、先頭の点 a、個数 M。
      返り値は f(a),...,f(a+M-1) の値を並べた長さ M の vector<mint>。

    bernoulli_numbers<mint>(k): 入力は最大添字 k。
      返り値は B[0],...,B[k] を並べた vector<mint>。
    faulhaber_polynomial<mint>(k): 入力は指数 k。
      返り値は S(t)=sum_{i=0}^t i^k を満たす多項式の係数列 fps（長さ k+2）。
    power_sum_polynomial<mint>(k): 入力は指数 k。
      返り値は T(t)=sum_{i=0}^{t-1} i^k を満たす多項式の係数列 fps（長さ k+2）。
    power_sums(mint(N),k): 入力は総和の上端 N と最大指数 k。
      返り値は sum_{i=0}^{N-1} i^j を j=0,...,k について並べた vector<mint>。
    polynomial_sum(f,mint(N)): 入力は多項式の係数列 f と総和の上端 N。
      返り値は sum_{i=0}^{N-1} f(i) を表す mint。
    prefix_sum_polynomial(f): 入力は多項式の係数列 f。
      返り値は F(t)=sum_{i=0}^t f(i) を満たす多項式の係数列 fps。
    prefix_sum_polynomial_exclusive(f): 入力は多項式の係数列 f。
      返り値は G(t)=sum_{i=0}^{t-1} f(i) を満たす多項式の係数列 fps。

    RelaxedMultiplication<mint> mul: 入力係数がまだない畳み込み器を作る。
    mul.append(a_i,b_i): 入力は次の係数の組。返り値は積の x^i の係数 c_i（mint）。
    mul.size(): 入力なし。追加済みの係数の組数（int）を返す。
    mul.clear(): 入力なし。保存した係数と結果を消去する。返り値なし。

    polynomial_matrix_product_bsgs(A,N,start=mint(0))
      入力: 正方行列 A の各成分を fps とした BSGSPolynomialMatrix<mint>、
        非負 int64_t の積の個数 N、最初の評価点 start。
      返り値: A(start+N-1)*...*A(start) の値を BSGSMatrix<mint> で返す。N=0 は単位行列。
    polynomial_product_bsgs(f,N,start=mint(0))
      入力: 多項式 f、積の個数 N、最初の評価点 start。
      返り値: prod(i=0..N-1) f(start+i) を mint で返す。N=0 は 1。
    p_recursive_nth(p,initial,N)
      入力: sum(j=0..r) p[j](i)*a[i+j]=0 の係数多項式 p（vector<fps>）、
        初期値 {a[0],...,a[r-1]}（vector<mint>）、求める添字 N（非負 int64_t）。
      返り値: optional<mint> に格納した a[N]。必要な p[r](i) が 0 なら nullopt。
    fps_pow_coefficient_bsgs(f,k,N)
      入力: 次数の小さい多項式 f、非負 int64_t の指数 k と次数 N（N<p）。
      返り値: [x^N] f(x)^k を mint で返す。先頭から N 項を作る関数ではない。

  [共通の約束]
    FPS を次数で打ち切る関数の N は「項数」で、計算は mod x^N。
    既定値 -1 は通常入力の長さ。総和を求める関数の N は総和の上端を表す。
    係数は昇べき順 {定数項,x の係数,x^2 の係数,...}。
    法は 2<p<2^30 の素数。微積分・階乗を使う関数は次数と法の条件に注意。
    f/g は多項式の商。FPS の除算は (f*g.inv(N)).pre(N)。
    合成は g[0]=0、合成逆関数は f[0]=0,f[1]!=0。
    補間では評価点が相異なること。多点評価では重複可。
    Prefix Sum / Faulhaber は inclusive と half-open を別関数で提供。
    返り値・空列・失敗時の仕様など、詳細は各ファイル先頭を参照。

  [検証]
    tests/run-tests.sh で愚直解との比較・恒等式・境界ケースを再検証できる。
    Library Checker の提出結果・対応関数・再現手順は tests/library-checker/VERIFY.md。
*/
#ifndef FPS_ALL_CPP
#define FPS_ALL_CPP
#include "formal-power-series.cpp"
#include "fps-newton.cpp"
#include "fps-sqrt.cpp"
#include "fps-sparse.cpp"
#include "fps-composition.cpp"
#include "polynomial-gcd.cpp"
#include "multipoint-evaluation.cpp"
#include "polynomial-interpolation.cpp"
#include "sampling-point-shift.cpp"
#include "faulhaber.cpp"
#include "prefix-sum-polynomial.cpp"
#include "relaxed-multiplication.cpp"
#include "polynomial-matrix-product.cpp"
#endif
