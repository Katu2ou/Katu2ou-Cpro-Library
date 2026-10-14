struct rolling_hash{
    using ull=unsigned long long; using u128=__uint128_t;
    static constexpr ull M=(1ULL<<61)-1;
    static ull mul(ull a,ull b){
        u128 x=(u128)a*b;
        ull y=(x>>61)+(x&M);
        return y>=M?y-M:y;
    }
    inline static ull B=
        chrono::steady_clock::now().time_since_epoch().count()%(M-256)+256;
    vector<ull>h,p;
    rolling_hash(const string&s):h(s.size()+1),p(s.size()+1,1){
        for(int i=0;i<(int)s.size();i++)
            p[i+1]=mul(p[i],B),
            h[i+1]=(mul(h[i],B)+(unsigned char)s[i]+1)%M;
    }
    ull get(int l,int r)const{
        ull x=h[r]+M-mul(h[l],p[r-l]);
        return x>=M?x-M:x;
    }
};