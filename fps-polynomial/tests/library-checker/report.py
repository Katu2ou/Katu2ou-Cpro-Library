#!/usr/bin/env python3
"""保存した公式判定とソースハッシュを照合し、日本語の検証報告書を生成する。"""
from collections import Counter
from datetime import datetime, timedelta, timezone
import hashlib
import json
from pathlib import Path

HERE=Path(__file__).resolve().parent
LIB=HERE.parent.parent


def main():
    manifest=json.loads((HERE/'manifest.json').read_text())
    records=json.loads((HERE/'online-results.json').read_text())
    local=json.loads((HERE/'local-samples.json').read_text())
    selected=[]
    for task in manifest:
        raw=(HERE/task['source']).read_bytes()
        assert hashlib.sha256(raw).hexdigest()==task['source_sha256']
        for file,sha in task['libraries'].items():
            assert hashlib.sha256((LIB/file).read_bytes()).hexdigest()==sha, file
        rows=[r for r in records if r['key']==task['key'] and r['source_sha256']==task['source_sha256']]
        assert rows, task['key']
        row=rows[-1]
        assert row.get('source_verified'), task['key']
        judge=json.loads((HERE/'judge'/f"{row['id']}.json").read_text())
        assert hashlib.sha256(judge['source'].encode()).hexdigest()==task['source_sha256']
        selected.append((row,judge))
    counts=Counter(row['status'] for row,_ in selected)
    case_counts=Counter(c['status'] for _,j in selected for c in j['case_results'])
    sample_by_key={r['key']:r for r in local}
    for task in manifest:
        assert sample_by_key[task['key']]['source_sha256']==task['source_sha256']
        assert all(c['status']=='PASS' for c in sample_by_key[task['key']]['cases'])
    sample_count=sum(len(r['cases']) for r in local)
    for log in ['gcc.log','sanitizer.log']:
        output=(HERE/log).read_text()
        assert 'All FPS tests passed.' in output, log
        assert 'runtime error:' not in output and 'ERROR: AddressSanitizer' not in output, log
    checked_at=datetime.now(timezone(timedelta(hours=9))).isoformat(timespec='seconds')
    lines=[
        '# FPS ライブラリの検証結果', '',
        f'確認日時: {checked_at}（日本時間）', '',
        f'公式 Library Checker の {len(set(r["problem"] for r,_ in selected))} 問題に、'
        f'{len(selected)} 種類のアダプタを匿名提出しました。判定: '
        +', '.join(f'{k} {v}件' for k,v in sorted(counts.items()))+'。',
        '公式ケース判定数: '+', '.join(f'{k} {v}件' for k,v in sorted(case_counts.items()))+'。', '',
        '提出ソースをサーバーから取得し、SHA-256 が手元の提出コードと一致することを確認しました。'
        '各依存ライブラリの現在のファイルも manifest.json のハッシュと照合済みです。', '',
        '## 修正した問題', '',
        '- **自己の係数を使うスカラー倍**: `fps f={2,3}; f*=f[0];` が `[4,12]` を返していました。'
        '倍率を更新前にコピーすることで、正しい `[4,6]` を返すように修正しました。'
        '`edge-cases-test.cpp` で各係数の参照を渡すケースを検査します。',
        '- **NTT の最大変換長での範囲外参照**: fft/ifft の最後に不要な回転係数の更新を行い、'
        '`dw[level]` / `dy[level]` を参照していました。mod 257、長さ 256、スタック上の '
        '`NTT` で ASan の stack-buffer-overflow として再現。次の反復がない場合の更新を省略し、'
        '最大長の積・自乗と、最大長を超えて CRT に切り替わるケースを回帰テストに追加しました。',
        '- **定数多項式を法とする逆元**: `polynomial_mod_inverse(f,fps{7})` は assert で停止していました。'
        'Library Checker `inv_of_polynomials` の公式サンプル `example_01` に合わせ、'
        '非零の定数を法とするとき `{true,{}}`（逆元の代表元は零多項式）を返すよう対応しました。'
        '零多項式を法とする入力は引き続き許しません。', '',
        '## 公式ジャッジの判定', '',
        '| 検証対象 | 問題 | 判定 | 最大時間 | ケース数 |',
        '| --- | --- | --- | ---: | ---: |',
    ]
    for row,judge in selected:
        timing=row['overview']['time']
        lines.append(f'| [{row["key"]}]({row["url"]}) | '
                     f'[{row["problem"]}](https://judge.yosupo.jp/problem/{row["problem"]}) | '
                     f'{row["status"]} | {timing:.3f} 秒 | {len(judge["case_results"])} |')
    lines += [
        '', '## 対応範囲', '',
        '- dense/sparse の inv・log・exp・pow・sqrt、合成・合成逆関数、積・商・剰余、'
        '多点評価・補間、評価点シフト、Bernoulli 数、平方剰余は対応する公式問題で直接検証。',
        '- `polynomial_mod_inverse` の公式問題で拡張 GCD と half-GCD も実行。'
        'gcd の正規化、Bézout 等式、half-GCD の次数条件と行列式は独立したローカル検査で確認。',
        '- `fps_power_projection` は合成逆関数内で使用。単体の出力は愚直なべき乗列挙と照合。',
        '- `newton` は汎用 Newton 法で `f*g-1=0` を解き、FPS 逆元の公式問題で検証。'
        '別途、Catalan 数の方程式 `g=1+x*g^2` も検査。',
        '- `pow_unit` は先頭次数と定数を正規化し、指数 `1/2` の結果を平方根の公式問題で検証。'
        '一般の有理数べき・疎な有理数べきはローカルで比較。',
        '- Prefix Sum の公式問題は上端を含まない和。`prefix_exclusive` は直接出力し、'
        '`prefix_inclusive` は上端を含む多項式から入力 f を引いて公式の形式へ変換。',
        '- `relaxed` は各係数を append し、その都度の戻り値を公式 Convolution の出力に使用。'
        '未来の係数に依存しないことは、直前の出力から次の入力を作るローカルテストでも確認。',
        '- Faulhaber の多項式・指数別のべき乗和・多項式の総和、疎な積と除算、'
        '単一点の等間隔補間、補助演算は独立した愚直計算・恒等式で検証。'
        'これら単体の公式 AC を得たという意味ではありません。', '',
        '## ローカル検証', '',
        f'- 公式サンプル {sample_count} ケース: 公式正解プログラムと公式チェッカーで全件 PASS。',
        f'- 公式ソースのコミット: `{local[0]["upstream_commit"]}`。',
        '- GCC (`g++-14 -std=c++17 -O2`) と Clang ASan/UBSan '
        '(`clang++ -std=c++17 -O1 -g -fsanitize=address,undefined`) の両方で全テスト通過。',
        '- 固定シードのランダム入力、空列、全零、先頭・末尾の零、長さ 0/1、'
        '非 2 冪長、巨大指数 `LLONG_MAX`、入力より長い精度、重複評価点を検査。',
        '- mod 3/5 の小規模平方根は全入力・全候補を列挙して存在判定も比較。'
        'mod 17/97/257 で法を複数回またぐ評価点シフトを検査。',
        '- 動的法 97, 257, 998244353, 1000000007, 1073741789 と法の再設定を検査。'
        '固定法 998244353 と 1000000007 でも独立した係数計算と比較。',
        '- 各 .cpp の単独 include と一括 include をコンパイル。', '',
        '## 再現方法', '',
        'FPS フォルダで実行します。', '',
        '```sh',
        './tests/run-tests.sh',
        "CXX=clang++ CXXFLAGS='-O1 -g -fsanitize=address,undefined' ./tests/run-tests.sh",
        'python3 tests/library-checker/build.py',
        'git clone --depth 1 https://github.com/yosupo06/library-checker-problems.git /tmp/lc-problems',
        'python3 tests/library-checker/run-local.py /tmp/lc-problems',
        '# 全公式テストをローカルでも生成・実行する場合:',
        'python3 tests/library-checker/run-local.py /tmp/lc-problems --full',
        '# 公式サイトへ匿名提出する場合（同一コードは再提出せず判定取得を再開）:',
        'python3 tests/library-checker/submit.py',
        'python3 tests/library-checker/report.py',
        '```', '',
        '## 保存した記録', '',
        '- `adapters/`: 入出力形式を合わせる短い main。',
        '- `submissions/`: ローカルの .cpp 依存を展開した提出コード。',
        '- `manifest.json`: 対象関数・提出コード・依存ファイルのハッシュ。',
        '- `online-results.json`, `judge/`: 提出 URL、判定、ケース別結果、サーバー上のソース。',
        '- `local-samples.json`, `local-samples.log`: 公式サンプルの入力ハッシュと実行結果。',
        '- `gcc.log`, `sanitizer.log`: 修正後のローカルテスト結果。', '',
        'テストは記載した入力条件・問題制約の範囲での検証であり、全入力に対する証明ではありません。',
    ]
    (HERE/'VERIFY.md').write_text('\n'.join(lines)+'\n')
    summary=dict(checked_at=checked_at, submissions=len(selected), problems=len(set(r['problem'] for r,_ in selected)),
                 verdicts=dict(counts), case_verdicts=dict(case_counts), official_samples=sample_count,
                 source_verified=True, current_libraries_verified=True)
    (HERE/'summary.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2)+'\n')
    print(json.dumps(summary,ensure_ascii=False,indent=2))


if __name__=='__main__':
    main()
