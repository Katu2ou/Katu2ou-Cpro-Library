//目次
/*
(約数列挙)
(素因数分解)
(乱数生成)
原始根の求値
Tonelli-Shanks(平方根を高速に求める)
Tonelli-Shanksの一般化(n乗根を高速に求める)
*/

// 準備 /////////////////////////////////////////////////////////////////

//約数列挙(昇順に並べたvectorで返す)
vector<ll> divisor(ll n) {
	vector<ll> ret;
	for (ll i = 1; i * i <= n; i++) {
		if (n % i != 0) continue;
		ret.push_back(i);
		//重複しないならばpush_back(n/i)をする
		if (i * i != n) ret.push_back(n / i);
	}
	sort(ret.begin(), ret.end());
	return ret;
}

//素因数分解 ((素因数,指数)のペアのvectorを返す)
vector<pair<ll, ll>> prime_factor(ll n) {
    vector<pair<ll, ll>> ret;
    for (ll i = 2; i * i <= n; i++) {
        if (n % i != 0) continue;
        ll tmp = 0;
        while (n % i == 0) {
            tmp++;
            n /= i;
        }
        ret.push_back(make_pair(i, tmp));
    }
    if (n != 1) ret.push_back(make_pair(n, 1));
    return ret;
}

//乱数生成([0,n]から一様)
long long random_uniform(long long n) {
    random_device rnd; 
    mt19937_64 mt(rnd());
    uniform_int_distribution<long long> dist(0, n); //[0,n]の一様分布から生成
    return dist(mt);
}

//////////////////////////////////////////////////////////////////

//原始根
//小さい方の整数(2)から判定していく
ll Primitive_Root(ll p){
    vector<pair<ll,ll>> pr = prime_factor(p-1);
    int a=2;
    while(true){
        int fin=1;
        for(auto u : pr){
            if(pow_mod(a,(p-1)/u.fi,p)==1){
                fin = 0;
                break;
            }
        }
        if(fin==0) ++a;
        else break;
    }
    return a;
}

// 

//(注)998244353の原始根は3、1000000007の原始根は5が最小

//エラトステネスの篩(O(NloglogN)のやつ 添字1--nに対してisprime[i]は素数なら1を返し、
//合成数なら最小の素因数を返すvector) 0 には0を返す
//これを使った後ならN以下の素因数分解を最悪計算量O(logN)でできる。
vector<int> Eratosthenes(int N) {
    vector<int> isprime(N+1, 1);
    isprime[0] = 0;
    isprime[1] = 0;
    for (ll p = 2; p <= N; ++p) {
        if (isprime[p] != 1) continue;
        for (ll q = p * p; q <= N; q += p) {
            if(isprime[q]==1) isprime[q] = p;
        }
    }
    return isprime;
}

	//2からnまでの素因数分解を一括で行う(O(NlogN))
	vector<vector<pii>> prime_factor(int n){
		vector<int> isprime(n+1, 1);
		isprime[1] = 0;
		for (int p = 2; p <= n; ++p) {
			if (isprime[p] != 1) continue;
			for (int q = p * 2; q <= n; q += p) {
				if(isprime[q]==1) {isprime[q] = p;}
			}
		}
		vector<vector<pii>> pr(n+1);
		for(int i=2; i<=n; i++){
			if(isprime[i]==1){
				pr[i].pb({i,1});
				continue;
			}
			int tmp=i;
			while(tmp>=2){
				int u=isprime[tmp];
				if(u==1){
					pr[i].pb({tmp,1});
					break;
				}
				int cnt=0;
				while(tmp%u==0){
					cnt++;
					tmp/=u;
				}
				pr[i].pb({u,cnt});
			}
		}
		return pr;
	}

//線形時間でできる素数篩(最小素因数が必要でないときは普通のものの方が高スペック )
vector<ll> LinearSieve(ll n){
    vector<ll> pr(n+1,0);
    vector<ll> prime_list;
    for(int d=2; d<=n;d++){
        if(pr[d]==0){
            pr[d]=d;
            prime_list.push_back(d);
        }
        for(auto p : prime_list){
            if(p > pr[d] || d * p > n) break;
            pr[d*p] = p;
        }
    }
    return  pr;
}
//これを用いると、mを法とする冪数1^k,2^k,...,n^kの列挙がO(nlogk)よりはやくできたりする
//逆元の列挙もO(n)でできる
//乗法的関数についても同様のことができる
//n=10^7では線形篩で0.42secほどかかる

//modの値としてll全体(正)をとれるmodpow
ll Modpow(__int128_t a, ll n, ll mo) {
	__int128_t r=1;
	a%=mo;
	while(n) r=r*((n%2)?a:1)%mo,a=a*a%mo,n>>=1;
	return r;
}

//modの値としてll全体(正)をとれる掛け算
ll Multiple(__int128_t a, __int128_t b, ll mo) {
    __int128_t r=0;
    a%=mo;
    b%=mo;
    r=a*b;
    r%=mo;
    return r;
}


//Tonelli-Shanks
/*
mod P での平方根をO(log(p)^2)で求める (離散対数問題の特殊な場合)

<方針> (p - 1 = q * 2^Q　とする)
1. x^2 = a (mod p)の解の第一近似として、
    ind(y^2*a^(-1)) = 0 (mod q)なるyをとる。(ここでy=a^(q+1)/2とすればよい)
2. ind(b) = 0 (mod q), ind(b) = 1 (mod 2)なるbをとる。
3. yをbの冪乗を乗算した値で更新することで、ind(y^2 * a^(-1)) = 0 (mod 2^Q)とする。
    このとき、y^2 = a (mod p) となる。

2では、指数がmod(p-1)で奇数である数をランダムで取り、それをq乗するとよい。
(指数の偶奇はind(x)=0 (mod 2) <-> x^((p-1)/2) = 1 (mod p)でわかる)
3は詳しくはhttps://37zigen.com/primitive-root/
wを誤差項とする。bを、gでの指数がmod2^Qで1桁目1であるような数とすると、q*bは
modqで0,mod2^Qで1桁目1であるから、wの1である最小桁をn桁目として、
w-((q*b)<<n)とすればwの最小桁数は真に大きくなる
(ここでq*bのkbit左シフト(2^kをかける)は簡単にできることが本質で、右シフトは難しい)

*/

//Toneli-Shanksの一般化
//(k乗根についてもより高速にできる)
/*

*/

int64_t mod_log(int64_t a, int64_t b, int64_t p) {
  int64_t g = 1;

  for(int64_t i = p; i; i /= 2) (g *= a) %= p;
  g = __gcd(g, p);

  int64_t t = 1, c = 0;
  for(; t % g; c++) {
    if(t == b) return c;
    (t *= a) %= p;
  }
  if(b % g) return -1;

  t /= g;
  b /= g;

  int64_t n = p / g, h = 0, gs = 1;

  for(; h * h < n; h++) (gs *= a) %= n;

  unordered_map< int64_t, int64_t > bs;
  for(int64_t s = 0, e = b; s < h; bs[e] = ++s) {
    (e *= a) %= n;
  }

  for(int64_t s = 0, e = t; s < n;) {
    (e *= gs) %= n;
    s += h;
    if(bs.count(e)) return c + s - bs[e];
  }
  return -1;
}



//Cohnの既約判定法(Cohn's irreducibility criterion)
/*
多項式f(x)に対して、整数N(>=2)があり、fの各項の係数が0以上N未満(この部分はもっと緩められる)であり、さらにf(N)が素数になる
とき、f(x)は既約となる

*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
