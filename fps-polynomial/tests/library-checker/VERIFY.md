# FPS ライブラリの検証結果

確認日時: 2026-10-29T16:29:05+09:00（日本時間）

公式 Library Checker の 24 問題に、28 種類のアダプタを匿名提出しました。判定: AC 28件。
公式ケース判定数: AC 806件。

提出ソースをサーバーから取得し、SHA-256 が手元の提出コードと一致することを確認しました。各依存ライブラリの現在のファイルも manifest.json のハッシュと照合済みです。

## 修正した問題

- **自己の係数を使うスカラー倍**: `fps f={2,3}; f*=f[0];` が `[4,12]` を返していました。倍率を更新前にコピーすることで、正しい `[4,6]` を返すように修正しました。`edge-cases-test.cpp` で各係数の参照を渡すケースを検査します。
- **NTT の最大変換長での範囲外参照**: fft/ifft の最後に不要な回転係数の更新を行い、`dw[level]` / `dy[level]` を参照していました。mod 257、長さ 256、スタック上の `NTT` で ASan の stack-buffer-overflow として再現。次の反復がない場合の更新を省略し、最大長の積・自乗と、最大長を超えて CRT に切り替わるケースを回帰テストに追加しました。
- **定数多項式を法とする逆元**: `polynomial_mod_inverse(f,fps{7})` は assert で停止していました。Library Checker `inv_of_polynomials` の公式サンプル `example_01` に合わせ、非零の定数を法とするとき `{true,{}}`（逆元の代表元は零多項式）を返すよう対応しました。零多項式を法とする入力は引き続き許しません。

## 公式ジャッジの判定

| 検証対象 | 問題 | 判定 | 最大時間 | ケース数 |
| --- | --- | --- | ---: | ---: |
| [inv](https://judge.yosupo.jp/submission/413911) | [inv_of_formal_power_series](https://judge.yosupo.jp/problem/inv_of_formal_power_series) | AC | 0.268 秒 | 25 |
| [inv_sparse](https://judge.yosupo.jp/submission/413912) | [inv_of_formal_power_series_sparse](https://judge.yosupo.jp/problem/inv_of_formal_power_series_sparse) | AC | 0.071 秒 | 24 |
| [log](https://judge.yosupo.jp/submission/413913) | [log_of_formal_power_series](https://judge.yosupo.jp/problem/log_of_formal_power_series) | AC | 0.342 秒 | 25 |
| [log_sparse](https://judge.yosupo.jp/submission/413914) | [log_of_formal_power_series_sparse](https://judge.yosupo.jp/problem/log_of_formal_power_series_sparse) | AC | 0.079 秒 | 24 |
| [exp](https://judge.yosupo.jp/submission/413915) | [exp_of_formal_power_series](https://judge.yosupo.jp/problem/exp_of_formal_power_series) | AC | 0.719 秒 | 26 |
| [exp_sparse](https://judge.yosupo.jp/submission/413916) | [exp_of_formal_power_series_sparse](https://judge.yosupo.jp/problem/exp_of_formal_power_series_sparse) | AC | 0.089 秒 | 25 |
| [pow](https://judge.yosupo.jp/submission/413917) | [pow_of_formal_power_series](https://judge.yosupo.jp/problem/pow_of_formal_power_series) | AC | 0.970 秒 | 37 |
| [pow_sparse](https://judge.yosupo.jp/submission/413918) | [pow_of_formal_power_series_sparse](https://judge.yosupo.jp/problem/pow_of_formal_power_series_sparse) | AC | 0.102 秒 | 35 |
| [sqrt](https://judge.yosupo.jp/submission/413919) | [sqrt_of_formal_power_series](https://judge.yosupo.jp/problem/sqrt_of_formal_power_series) | AC | 0.582 秒 | 35 |
| [sqrt_sparse](https://judge.yosupo.jp/submission/413920) | [sqrt_of_formal_power_series_sparse](https://judge.yosupo.jp/problem/sqrt_of_formal_power_series_sparse) | AC | 0.128 秒 | 45 |
| [composition](https://judge.yosupo.jp/submission/413921) | [composition_of_formal_power_series](https://judge.yosupo.jp/problem/composition_of_formal_power_series) | AC | 0.075 秒 | 27 |
| [compositional_inverse](https://judge.yosupo.jp/submission/413922) | [compositional_inverse_of_formal_power_series](https://judge.yosupo.jp/problem/compositional_inverse_of_formal_power_series) | AC | 0.056 秒 | 23 |
| [composition_large](https://judge.yosupo.jp/submission/413923) | [composition_of_formal_power_series_large](https://judge.yosupo.jp/problem/composition_of_formal_power_series_large) | AC | 1.778 秒 | 32 |
| [compositional_inverse_large](https://judge.yosupo.jp/submission/413924) | [compositional_inverse_of_formal_power_series_large](https://judge.yosupo.jp/problem/compositional_inverse_of_formal_power_series_large) | AC | 1.418 秒 | 28 |
| [convolution_998244353](https://judge.yosupo.jp/submission/413925) | [convolution_mod](https://judge.yosupo.jp/problem/convolution_mod) | AC | 0.161 秒 | 53 |
| [convolution_1000000007](https://judge.yosupo.jp/submission/413926) | [convolution_mod_1000000007](https://judge.yosupo.jp/problem/convolution_mod_1000000007) | AC | 0.294 秒 | 48 |
| [division](https://judge.yosupo.jp/submission/413928) | [division_of_polynomials](https://judge.yosupo.jp/problem/division_of_polynomials) | AC | 0.643 秒 | 35 |
| [polynomial_inverse](https://judge.yosupo.jp/submission/413930) | [inv_of_polynomials](https://judge.yosupo.jp/problem/inv_of_polynomials) | AC | 0.638 秒 | 18 |
| [multipoint](https://judge.yosupo.jp/submission/413932) | [multipoint_evaluation](https://judge.yosupo.jp/problem/multipoint_evaluation) | AC | 0.764 秒 | 11 |
| [interpolation](https://judge.yosupo.jp/submission/413933) | [polynomial_interpolation](https://judge.yosupo.jp/problem/polynomial_interpolation) | AC | 0.918 秒 | 9 |
| [sampling_shift](https://judge.yosupo.jp/submission/413934) | [shift_of_sampling_points_of_polynomial](https://judge.yosupo.jp/problem/shift_of_sampling_points_of_polynomial) | AC | 0.209 秒 | 32 |
| [bernoulli](https://judge.yosupo.jp/submission/413936) | [bernoulli_number](https://judge.yosupo.jp/problem/bernoulli_number) | AC | 0.248 秒 | 11 |
| [prefix_exclusive](https://judge.yosupo.jp/submission/413937) | [prefix_sum_of_polynomial](https://judge.yosupo.jp/problem/prefix_sum_of_polynomial) | AC | 0.341 秒 | 26 |
| [prefix_inclusive](https://judge.yosupo.jp/submission/413938) | [prefix_sum_of_polynomial](https://judge.yosupo.jp/problem/prefix_sum_of_polynomial) | AC | 0.350 秒 | 26 |
| [mod_sqrt](https://judge.yosupo.jp/submission/413939) | [sqrt_mod](https://judge.yosupo.jp/problem/sqrt_mod) | AC | 0.065 秒 | 13 |
| [relaxed](https://judge.yosupo.jp/submission/413940) | [convolution_mod](https://judge.yosupo.jp/problem/convolution_mod) | AC | 2.621 秒 | 53 |
| [newton](https://judge.yosupo.jp/submission/413941) | [inv_of_formal_power_series](https://judge.yosupo.jp/problem/inv_of_formal_power_series) | AC | 0.697 秒 | 25 |
| [pow_unit](https://judge.yosupo.jp/submission/413942) | [sqrt_of_formal_power_series](https://judge.yosupo.jp/problem/sqrt_of_formal_power_series) | AC | 0.975 秒 | 35 |

## 対応範囲

- dense/sparse の inv・log・exp・pow・sqrt、合成・合成逆関数、積・商・剰余、多点評価・補間、評価点シフト、Bernoulli 数、平方剰余は対応する公式問題で直接検証。
- `polynomial_mod_inverse` の公式問題で拡張 GCD と half-GCD も実行。gcd の正規化、Bézout 等式、half-GCD の次数条件と行列式は独立したローカル検査で確認。
- `fps_power_projection` は合成逆関数内で使用。単体の出力は愚直なべき乗列挙と照合。
- `newton` は汎用 Newton 法で `f*g-1=0` を解き、FPS 逆元の公式問題で検証。別途、Catalan 数の方程式 `g=1+x*g^2` も検査。
- `pow_unit` は先頭次数と定数を正規化し、指数 `1/2` の結果を平方根の公式問題で検証。一般の有理数べき・疎な有理数べきはローカルで比較。
- Prefix Sum の公式問題は上端を含まない和。`prefix_exclusive` は直接出力し、`prefix_inclusive` は上端を含む多項式から入力 f を引いて公式の形式へ変換。
- `relaxed` は各係数を append し、その都度の戻り値を公式 Convolution の出力に使用。未来の係数に依存しないことは、直前の出力から次の入力を作るローカルテストでも確認。
- Faulhaber の多項式・指数別のべき乗和・多項式の総和、疎な積と除算、単一点の等間隔補間、補助演算は独立した愚直計算・恒等式で検証。これら単体の公式 AC を得たという意味ではありません。

## ローカル検証

- 公式サンプル 54 ケース: 公式正解プログラムと公式チェッカーで全件 PASS。
- 公式ソースのコミット: `73876f1cbcf082d2b22722924b924d3d65a2f00e`。
- GCC (`g++-14 -std=c++17 -O2`) と Clang ASan/UBSan (`clang++ -std=c++17 -O1 -g -fsanitize=address,undefined`) の両方で全テスト通過。
- 固定シードのランダム入力、空列、全零、先頭・末尾の零、長さ 0/1、非 2 冪長、巨大指数 `LLONG_MAX`、入力より長い精度、重複評価点を検査。
- mod 3/5 の小規模平方根は全入力・全候補を列挙して存在判定も比較。mod 17/97/257 で法を複数回またぐ評価点シフトを検査。
- 動的法 97, 257, 998244353, 1000000007, 1073741789 と法の再設定を検査。固定法 998244353 と 1000000007 でも独立した係数計算と比較。
- 各 .cpp の単独 include と一括 include をコンパイル。

## 再現方法

FPS フォルダで実行します。

```sh
./tests/run-tests.sh
CXX=clang++ CXXFLAGS='-O1 -g -fsanitize=address,undefined' ./tests/run-tests.sh
python3 tests/library-checker/build.py
git clone --depth 1 https://github.com/yosupo06/library-checker-problems.git /tmp/lc-problems
python3 tests/library-checker/run-local.py /tmp/lc-problems
# 全公式テストをローカルでも生成・実行する場合:
python3 tests/library-checker/run-local.py /tmp/lc-problems --full
# 公式サイトへ匿名提出する場合（同一コードは再提出せず判定取得を再開）:
python3 tests/library-checker/submit.py
python3 tests/library-checker/report.py
```

## 保存した記録

- `adapters/`: 入出力形式を合わせる短い main。
- `submissions/`: ローカルの .cpp 依存を展開した提出コード。
- `manifest.json`: 対象関数・提出コード・依存ファイルのハッシュ。
- `online-results.json`, `judge/`: 提出 URL、判定、ケース別結果、サーバー上のソース。
- `local-samples.json`, `local-samples.log`: 公式サンプルの入力ハッシュと実行結果。
- `gcc.log`, `sanitizer.log`: 修正後のローカルテスト結果。

テストは記載した入力条件・問題制約の範囲での検証であり、全入力に対する証明ではありません。
