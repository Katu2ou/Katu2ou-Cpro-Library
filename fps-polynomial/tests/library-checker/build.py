#!/usr/bin/env python3
"""Library Checker 用アダプタと単一ファイルの提出コードを生成する。

ライブラリ本体を依存順に展開するだけで、アルゴリズムの置換は行わない。
manifest.json に提出コードと依存ファイルの SHA-256 を記録する。
"""
import hashlib
import json
from pathlib import Path
import re

HERE = Path(__file__).resolve().parent
LIB = HERE.parent.parent


def digest(data):
    return hashlib.sha256(data).hexdigest()


TASKS = []


def task(key, problem, header, functions, main, mod=998244353):
    TASKS.append(dict(key=key, problem=problem, header=header,
                      functions=functions, main=main, mod=mod))


for name in ['inv', 'log', 'exp', 'pow', 'sqrt']:
    problem = name + '_of_formal_power_series'
    header = 'fps-sqrt.cpp' if name == 'sqrt' else 'formal-power-series.cpp'
    read = 'int n; cin >> n; '
    if name == 'pow':
        read += 'int64_t k; cin >> k; '
    call = 'fps_sqrt(f,n)' if name == 'sqrt' else 'f.' + name + ('(k,n)' if name == 'pow' else '(n)')
    output = 'if (g.empty()) cout << "-1\\n"; else print(g);' if name == 'sqrt' else 'print(g);'
    task(name, problem, header, [call], read + 'auto f=read_poly(n); auto g=' + call + '; ' + output)
    read = 'int n,k; cin >> n >> k; '
    if name == 'pow':
        read += 'int64_t exponent; cin >> exponent; '
    read += 'SparseFPS<mint> f(k); for(auto& [i,c]:f) cin >> i >> c; '
    call = 'fps_' + name + '_sparse(f,' + ('exponent,' if name == 'pow' else '') + 'n)'
    task(name + '_sparse', problem + '_sparse', 'fps-sparse.cpp', [call],
         read + 'auto g=' + call + '; ' + output)

for suffix in ['', '_large']:
    task('composition' + suffix, 'composition_of_formal_power_series' + suffix,
         'fps-composition.cpp', ['fps_compose'],
         'int n; cin >> n; auto f=read_poly(n),g=read_poly(n); print(fps_compose(f,g,n));')
    task('compositional_inverse' + suffix, 'compositional_inverse_of_formal_power_series' + suffix,
         'fps-composition.cpp', ['fps_compositional_inverse', 'fps_power_projection'],
         'int n; cin >> n; auto f=read_poly(n); print(fps_compositional_inverse(f,n));')

for mod in [998244353, 1000000007]:
    task('convolution_' + str(mod), 'convolution_mod' + ('_1000000007' if mod == 1000000007 else ''),
         'formal-power-series.cpp', ['operator*'],
         'int n,m; cin >> n >> m; auto f=read_poly(n),g=read_poly(m); print(f*g);', mod)

task('division', 'division_of_polynomials', 'formal-power-series.cpp', ['operator/', 'operator%'],
     'int n,m; cin >> n >> m; auto f=read_poly(n),g=read_poly(m); '
     'auto q=f/g,r=f%g; q.shrink(); r.shrink(); cout<<q.size()<<" "<<r.size()<<"\\n"; print(q); print(r);')
task('polynomial_inverse', 'inv_of_polynomials', 'polynomial-gcd.cpp',
     ['polynomial_mod_inverse', 'polynomial_extgcd', 'half-GCD'],
     'int n,m; cin >> n >> m; auto f=read_poly(n),g=read_poly(m); '
     'auto [ok,h]=polynomial_mod_inverse(f,g); if(!ok) cout<<"-1\\n"; '
     'else { cout<<h.size()<<"\\n"; print(h); }')
task('multipoint', 'multipoint_evaluation', 'multipoint-evaluation.cpp', ['MultipointEvaluation'],
     'int n,m; cin >> n >> m; auto f=read_poly(n); vector<mint> xs=read_poly(m); '
     'print(MultipointEvaluation(f,xs));')
task('interpolation', 'polynomial_interpolation', 'polynomial-interpolation.cpp', ['polynomial_interpolation'],
     'int n; cin >> n; vector<mint> xs=read_poly(n),ys=read_poly(n); '
     'print(polynomial_interpolation(xs,ys));')
task('sampling_shift', 'shift_of_sampling_points_of_polynomial', 'sampling-point-shift.cpp',
     ['sampling_point_shift'],
     'int n,m; mint c; cin >> n >> m >> c; vector<mint> y=read_poly(n); '
     'print(sampling_point_shift(y,c,m));')
task('bernoulli', 'bernoulli_number', 'faulhaber.cpp', ['bernoulli_numbers'],
     'int n; cin >> n; print(bernoulli_numbers<mint>(n));')
task('prefix_exclusive', 'prefix_sum_of_polynomial', 'prefix-sum-polynomial.cpp',
     ['prefix_sum_polynomial_exclusive'],
     'int n; cin >> n; auto f=read_poly(n); print(prefix_sum_polynomial_exclusive(f));')
task('prefix_inclusive', 'prefix_sum_of_polynomial', 'prefix-sum-polynomial.cpp',
     ['prefix_sum_polynomial'],
     'int n; cin >> n; auto f=read_poly(n); print(prefix_sum_polynomial(f)-f);')
task('mod_sqrt', 'sqrt_mod', 'fps-sqrt.cpp', ['mod_sqrt'],
     'int t; cin >> t; while(t--) { int64_t a,p; cin >> a >> p; cout<<mod_sqrt(a,p)<<"\\n"; }')
task('relaxed', 'convolution_mod', 'relaxed-multiplication.cpp', ['RelaxedMultiplication::append'],
     'int n,m; cin >> n >> m; auto a=read_poly(n),b=read_poly(m); '
     'if(!n||!m) { print(fps{}); return 0; } '
     'RelaxedMultiplication<mint> mul; fps c(n+m-1); '
     'for(int i=0;i<n+m-1;++i) c[i]=mul.append(i<n?a[i]:mint(0),i<m?b[i]:mint(0)); print(c);')
task('newton', 'inv_of_formal_power_series', 'fps-newton.cpp', ['fps_newton'],
     'int n; cin >> n; auto f=read_poly(n); '
     'print(fps_newton(fps{f[0].inverse()},n,[&](const fps& g,int d) '
     '{ auto a=f.pre(d); return make_pair((a*g-mint(1)).pre(d),a); }));')
task('pow_unit', 'sqrt_of_formal_power_series', 'fps-all.cpp', ['fps_pow_unit'],
     'int n; cin >> n; auto f=read_poly(n); int v=0; while(v<n && f[v]==mint(0)) ++v; '
     'if(v==n) { print(fps(n)); return 0; } '
     'int64_t r=mod_sqrt(f[v].get(),mint::get_mod()); '
     'if((v&1)||r<0) { cout<<"-1\\n"; return 0; } '
     'auto unit=(f>>v)*f[v].inverse(); '
     'auto g=fps_pow_unit(unit,mint(1)/mint(2),n-v)*mint(r); print((g<<(v/2)).pre(n));')


def main():
    for name in ['adapters', 'submissions']:
        (HERE/name).mkdir(exist_ok=True)
    manifest = []
    for t in TASKS:
        preamble = '#define FPS_NO_DEFAULT_ALIASES\n'
        common = f'using mint=LazyMontgomeryModInt<{t["mod"]}>;\nusing fps=FormalPowerSeries<mint>;\n'
        common += '''fps read_poly(int n) { fps f(n); for(auto& x:f) cin>>x; return f; }
template<class V> void print(const V& v) {
  for(size_t i=0;i<v.size();++i) { if(i) cout<<' '; cout<<v[i]; } cout<<'\\n';
}
int main() { ios::sync_with_stdio(false); cin.tie(nullptr);
'''
        common += t['main'] + '\n}\n'
        # Python fragments above use escaped newlines for C++ string literals.
        common = common.replace('\\\\n', '\\n')
        adapter = preamble + '#include "../../../' + t['header'] + '"\n' + common
        (HERE/'adapters'/(t['key']+'.cpp')).write_text(adapter)
        libraries = {}

        def expand(name):
            if name in libraries:
                return ''
            raw = (LIB/name).read_bytes()
            libraries[name] = digest(raw)
            lines = []
            for line in raw.decode().splitlines(keepends=True):
                match = re.fullmatch(r'#include "([^"/]+\.cpp)"\s*', line)
                lines.append(expand(match[1]) if match else line)
            return ''.join(lines) + '\n'

        source = preamble + expand(t['header']) + common
        source_path = 'submissions/' + t['key'] + '.cpp'
        (HERE/source_path).write_text(source)
        manifest.append(dict(key=t['key'], problem=t['problem'], lang='cpp17',
                             functions=t['functions'], source=source_path,
                             source_sha256=digest(source.encode()), libraries=libraries))
    (HERE/'manifest.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=2)+'\n')
    print('Generated',len(manifest),'Library Checker adapters.')


if __name__ == '__main__':
    main()
