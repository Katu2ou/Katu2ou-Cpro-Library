#!/usr/bin/env python3
"""BSGS の Library Checker 検証。既存 FPS の検証記録とは別に保存する。

python3 verify.py build
python3 verify.py local /path/to/library-checker-problems
python3 verify.py submit

Factorial をスカラー積と 2 階の P-recursive 漸化式で検証する。
local は公式全ケース・公式チェッカー、submit は匿名のオンライン提出。
既存 tests/library-checker の提出・検証処理を共有する。
"""
import importlib.util
from pathlib import Path
import sys

HERE = Path(__file__).resolve().parent
LIB = HERE.parent.parent


def module(name):
    path = HERE.parent/'library-checker'/(name+'.py')
    spec = importlib.util.spec_from_file_location('bsgs_'+name, path)
    obj = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(obj)
    obj.HERE, obj.LIB = HERE, LIB
    return obj


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in ('build','local','submit'):
        raise SystemExit(__doc__)
    mode = sys.argv[1]
    if mode == 'build':
        obj = module('build')
        obj.TASKS = []
        obj.task('polynomial_factorial','factorial','polynomial-matrix-product.cpp',
                 ['polynomial_product_bsgs','polynomial_matrix_product_bsgs'],
                 'int t; cin>>t; while(t--) { int64_t n; cin>>n; '
                 'cout<<polynomial_product_bsgs(fps{0,1},n,mint(1))<<"\\n"; }')
        # n! は a_(i+2)-(i+3)*a_(i+1)+(i+1)*a_i=0, a_0=a_1=1 を満たす。
        obj.task('p_recursive_factorial','factorial','polynomial-matrix-product.cpp',
                 ['p_recursive_nth','polynomial_matrix_product_bsgs'],
                 'int t; cin>>t; while(t--) { int64_t n; cin>>n; '
                 'auto a=p_recursive_nth(vector<fps>{{1,1},{-3,-1},{1}},vector<mint>{1,1},n); '
                 'assert(a); cout<<*a<<"\\n"; }')
        obj.main()
    elif mode == 'local':
        if len(sys.argv) != 3:
            raise SystemExit('Usage: verify.py local /path/to/library-checker-problems')
        obj = module('run-local')
        sys.argv = [sys.argv[0],sys.argv[2],'--full','--cache','/tmp/katu2ou-bsgs-lc-build']
        obj.main()
    else:
        obj = module('submit')
        obj.RESULT = HERE/'online-results.json'
        sys.argv = [sys.argv[0]]
        obj.main()


if __name__ == '__main__':
    main()
