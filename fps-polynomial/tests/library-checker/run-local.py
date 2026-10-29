#!/usr/bin/env python3
"""公式ソースの正解プログラム・チェッカーでローカル検証する。

python3 run-local.py /path/to/library-checker-problems [--full] [--only key ...]
既定は公式サンプル。--full は公式 generate.py で全テストを生成する。
ビルド・大きな入出力は --cache（既定: /tmp/katu2ou-fps-lc-build）に保存。
"""
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent


def run(command, **kwargs):
    return subprocess.run(command, check=True, **kwargs)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('upstream', type=Path)
    parser.add_argument('--full', action='store_true')
    parser.add_argument('--only', nargs='+')
    parser.add_argument('--cache', type=Path, default=Path('/tmp/katu2ou-fps-lc-build'))
    args = parser.parse_args()
    upstream = args.upstream.resolve()
    cache = args.cache.resolve()
    cache.mkdir(exist_ok=True, parents=True)
    logs = HERE/'logs'
    logs.mkdir(exist_ok=True)
    sys.path.insert(0, str(upstream))
    spec = importlib.util.spec_from_file_location('lc_generate', upstream/'generate.py')
    generator = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(generator)
    tasks = json.loads((HERE/'manifest.json').read_text())
    if args.only:
        tasks = [t for t in tasks if t['key'] in args.only]
    cxx = os.environ.get('CXX', 'g++-14')
    flags = ['-std=c++17', '-O2', '-I', str(upstream/'common')]
    commit = subprocess.check_output(['git','-C',str(upstream),'rev-parse','HEAD'], text=True).strip()
    problems = {}
    for task in tasks:
        problem = task['problem']
        if problem in problems:
            continue
        paths = list(upstream.glob('**/'+problem+'/info.toml'))
        assert len(paths) == 1, problem
        obj = generator.Problem(upstream, paths[0].parent)
        obj.generate_params_h()
        problems[problem] = obj
    commands = []
    for task in tasks:
        source = HERE/task['source']
        assert hashlib.sha256(source.read_bytes()).hexdigest() == task['source_sha256']
        for name, sha in task['libraries'].items():
            assert hashlib.sha256((HERE.parent.parent/name).read_bytes()).hexdigest() == sha, name
        commands.append((task['key'], [cxx, *flags, str(source), '-o', str(cache/task['key'])]))
    for problem, obj in problems.items():
        for kind, source in [('correct', obj.basedir/'sol/correct.cpp'), ('checker', obj.checker)]:
            commands.append((problem+'-'+kind, [cxx, *flags, str(source), '-o', str(cache/(problem+'-'+kind))]))

    def compile_one(item):
        name, command = item
        signature = hashlib.sha256(Path(command[-3]).read_bytes()
                    + json.dumps(command).encode() + commit.encode()).hexdigest()
        stamp = cache/(name+'.compile.sha256')
        if Path(command[-1]).exists() and stamp.exists() and stamp.read_text() == signature:
            return name+' (cached)'
        with (logs/(name+'.compile.log')).open('w') as log:
            run(command, stdout=log, stderr=subprocess.STDOUT)
        stamp.write_text(signature)
        return name

    with ThreadPoolExecutor(max_workers=2) as pool:
        for completed in as_completed([pool.submit(compile_one, item) for item in commands]):
            print('COMPILED', completed.result(), flush=True)
    if args.full:
        env = dict(os.environ, CXX=cxx)
        for problem in problems:
            with (logs/(problem+'.generate.log')).open('w') as log:
                run([sys.executable, str(upstream/'generate.py'), '-p', problem],
                    env=env, stdout=log, stderr=subprocess.STDOUT)
            print('GENERATED', problem, flush=True)
    mode = 'full' if args.full else 'samples'
    records = []
    result_path = HERE/('local-'+mode+'.json')
    for task in tasks:
        obj = problems[task['problem']]
        cases = sorted((obj.basedir/'in').glob('*.in')) if args.full else sorted((obj.basedir/'gen').glob('example_*.in'))
        assert cases, task['key']
        record = dict(key=task['key'], problem=task['problem'], source_sha256=task['source_sha256'],
                      upstream_commit=commit, mode=mode, cases=[])
        for case in cases:
            expected = cache/(task['problem']+'-'+case.stem+'.expected')
            if args.full:
                expected = obj.basedir/'out'/(case.stem+'.out')
            else:
                with case.open('rb') as inp, expected.open('wb') as out:
                    run([str(cache/(task['problem']+'-correct'))], stdin=inp, stdout=out, timeout=120)
            actual = cache/(task['key']+'-'+case.stem+'.actual')
            started = time.monotonic()
            with case.open('rb') as inp, actual.open('wb') as out:
                execution = subprocess.run([str(cache/task['key'])], stdin=inp, stdout=out,
                                           stderr=subprocess.PIPE, timeout=120)
            seconds = time.monotonic()-started
            checked = subprocess.run([str(cache/(task['problem']+'-checker')), str(case), str(actual), str(expected)],
                                     capture_output=True, text=True, timeout=120)
            status = 'PASS' if execution.returncode == 0 and checked.returncode == 0 else 'FAIL'
            record['cases'].append(dict(name=case.name, status=status, seconds=seconds,
                                        input_sha256=hashlib.sha256(case.read_bytes()).hexdigest(),
                                        returncode=execution.returncode,
                                        stderr=execution.stderr.decode(errors='replace'), checker=checked.stderr))
            print(task['key'], case.name, status, f'{seconds:.3f}s', flush=True)
        records.append(record)
        result_path.write_text(json.dumps(records, ensure_ascii=False, indent=2)+'\n')
    if any(c['status'] != 'PASS' for r in records for c in r['cases']):
        raise SystemExit(1)
    print('PASS',len(tasks),'adapters,',sum(len(r['cases']) for r in records),'official cases.', flush=True)


if __name__ == '__main__':
    main()
