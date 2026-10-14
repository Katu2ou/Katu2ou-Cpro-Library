# ランダムチェッカーの使い方

## 早見表
    - 通常のmain.cpp brute.cpp 比較

    python3 stress.py
        - -n 200 : 200個のテストを行う
        - -n : いつまでも行う
        - --detail : ACの場合でも結果を出力
        - --stop :WAが出たらそこで止まる
        - --run-only : main.cppだけを実行
        - --time : main.cppにかかった時間も出力する
        - --manual : ターミナルから入力を与えてmain.cppとbrute.cppを比較
            (入力を与えたら，Ctrl+Dで確定)
        - --force-compile : 変更の有無にかかわらず必要なファイルを再ビルド

        - --interactive : インタラクティブ用 標準エラー出力がある時そこで終了する
        ( cerr << "WA: expected = " << expected
         << ", actual = " << actual << '\n';)
         - agent.cppが必要


## 構成

- `main.cpp`: 確認したい解答
- `brute.cpp`: 愚直解
- `gen.cpp`: seedをコマンドライン引数で受け取り、ランダム入力を標準出力する生成器
- `stress.py`: コンパイル、入力生成、実行、出力比較を行うスクリプト

## ビルド

通常は、各ソースファイルに対応する実行ファイルが存在しない場合、または
ソースファイルの更新日時が実行ファイルより新しい場合だけビルドする。
前回のビルド後に変更されていないファイルのビルドは省略される。
複数のファイルにビルドが必要な場合、それらは並列にコンパイルされる。
また、コンパイラには`-pipe`を指定し、一時ファイルへの読み書きを減らす。

```text
Build skipped (up to date): main.cpp
Building: brute.cpp
Build skipped (up to date): gen.cpp
```

使用するファイルはモードによって異なる。

- 通常：`main.cpp`、`brute.cpp`、`gen.cpp`
- `--manual`：`main.cpp`、`brute.cpp`
- `--run-only`：`main.cpp`、`gen.cpp`
- `--interactive`：`main.cpp`、`gen.cpp`、`agent.cpp`

更新日時にかかわらず必要なファイルをすべて再ビルドする場合：

```bash
python3 stress.py --force-compile
```

ビルド処理を完全に省略して既存の実行ファイルを使用する場合：

```bash
python3 stress.py --no-compile
```

`--no-compile`と`--force-compile`は同時に指定できない。

## 基本的な使い方

```bash
python3 stress.py -n 200
```

`0001`ではなく、テスト番号は1から始まる整数として表示される。出力が一致した場合は次のように表示される。

```text
1 AC
2 AC
3 AC
```

出力が異なった場合は、WAと入力・両方の出力が表示される。

```text
4 WA
[input]
該当する入力
[main.cpp output]
main.cppの出力
[brute.cpp output]
brute.cppの出力
```

非常に長い行や多数の行がある場合、ターミナル表示は途中で省略される。保存ファイルには省略せず記録される。

## ターミナルから入力して比較

`--manual`を付けると、`gen.cpp`を使わず、標準入力から受け取った1ケースを
`main.cpp`と`brute.cpp`の両方へ与えて出力を比較する。

```bash
python3 stress.py --manual
```

コマンドを起動したらテストケースを入力または貼り付け、最後にCtrl+Dを押して
入力を確定する（macOS/Linuxの場合）。結果が一致すれば入力と一致した出力を
伴う`AC`、異なれば入力と両方の出力を伴う`WA`が表示される。

パイプやリダイレクトでも利用できる。

```bash
printf '3\n1 2 3\n' | python3 stress.py --manual
python3 stress.py --manual < input.txt
```

このモードでは`gen.cpp`をコンパイル・実行しない。`--no-compile`、`--time`、
`--detail`とは併用できるが、`--run-only`、`--interactive`とは併用できない。

## 回数を指定せず実行し続ける

`-n` の値を省略すると、Ctrl+Cなどで終了するまで実行し続ける。

```bash
python3 stress.py -n
```

回数を指定する場合は従来どおり数値を続ける。

```bash
python3 stress.py -n 1000
```

`-n` 自体を指定しない場合の既定値は1000回である。

## 詳細表示

`--detail` を付けると、ACの場合にも入力と一致した出力を表示する。

```bash
python3 stress.py -n 10 --detail
```

表示例：

```text
1 AC
[input]
該当する入力
[matching output]
一致した出力
```

## `main.cpp`の実行時間を表示

`--time`を付けると、各テストについて`main.cpp`の実行時間を表示する。
計測対象は`main.cpp`の起動から終了までだけであり、`gen.cpp`と
`brute.cpp`の実行時間は含まれない。

```bash
python3 stress.py --time
```

表示例：

```text
1 AC [main.cpp: 1.234 ms]
2 AC [main.cpp: 0.987 ms]
3 WA [main.cpp: 1.105 ms]
```

AC時にも入力と`main.cpp`の出力を表示したい場合は、`--detail`と併用する。

```bash
python3 stress.py -n 100 --time --detail
```

`main.cpp`だけを実行し、入力・出力・時間を表示する場合：

```bash
python3 stress.py -n 10 --run-only --time
```

`--run-only --output-only --time`では、`main.cpp`の出力は標準出力、
実行時間は標準エラーへ表示される。

`--interactive`では、`main.cpp`が`agent.cpp`の応答を待つ時間と処理時間を
分離できないため、`--time`は同時に指定できない。

## 最初のWAで停止

通常はWAを発見しても次のテストを続行する。最初のWAを表示した直後に停止したい場合は `--stop` を付ける。

```bash
python3 stress.py -n --stop
```

`--detail` と同時に指定することもできる。

```bash
python3 stress.py -n 200 --detail --stop
```

## その他のオプション

```bash
python3 stress.py -n 1000 --no-compile
python3 stress.py -n 200 --compiler g++-23
python3 stress.py -n 10 --run-only
python3 stress.py -n 10 --run-only --output-only
python3 stress.py -n 100 --time
```

- `--no-compile`: 前回作成した実行ファイルを再利用する
- `--compiler`: 使用するC++コンパイラを指定する
- `--run-only`: `brute.cpp`と比較せず、`main.cpp`だけを実行する
- `--output-only`: `--run-only`時に入力を表示せず、出力だけ表示する
- `--time`: 各テストで`main.cpp`だけにかかった実時間を表示する

## インタラクティブテスト

インタラクティブ形式では、次の4ファイルを用意する。

- `main.cpp`: テストする解答。誤りを検出した場合は標準エラーへ出力する
- `agent.cpp`: インタラクティブ相手の動作を実装する
- `gen.cpp`: `agent.cpp`が使用するテストケースを生成する
- `stress.py`

次のコマンドで実行する。

```bash
python3 stress.py -n 100 --interactive
```

回数無制限で実行する場合：

```bash
python3 stress.py -n --interactive
```

### 入出力の接続

各テストでは、次の順番で接続される。

1. `gen.cpp`の出力が、最初に`main.cpp`と`agent.cpp`両方の標準入力へ送られる
2. 両プログラムが生成入力を読み終えた後、`main.cpp`の標準出力が`agent.cpp`の標準入力へ中継される
3. `agent.cpp`の標準出力が`main.cpp`の標準入力へ中継される

概念的には次の接続になる。

```text
gen.cpp  ──最初のテストデータ──> main.cpp
        └─最初のテストデータ──> agent.cpp
main.cpp ─────問い合わせ等─────> agent.cpp
main.cpp <──────応答等──────── agent.cpp
```

`main.cpp`と`agent.cpp`は、どちらも最初に`gen.cpp`が生成した同一の入力を通常の入力形式に従って読み、その後に対話を開始する。生成入力の終端をEOFで判断することはできないため、個数などから読み終わりを判断できる形式にする必要がある。

`main.cpp`と`agent.cpp`は、相手からの入力を待つ前に必ず出力をflushする。

```cpp
cout << query << endl;
// または
cout << query << '\n' << flush;
```

### AC・WAの判定

次の条件をすべて満たした場合にACとなる。

- `main.cpp`が終了コード0で終了
- `agent.cpp`が終了コード0で終了
- 制限時間内に対話が終了
- `main.cpp`が標準エラーへ何も出力していない

`main.cpp`が標準エラーへ1文字でも出力すると、そのケースをWAとして表示し、以降のテストを直ちに停止する。したがって、このモードでは`main.cpp`のデバッグ出力に標準エラーを使用しないこと。

TLEやどちらかのプログラムの異常終了でも、その場で停止する。`--interactive`では`--stop`を付けなくても失敗時に必ず停止する。

`--detail`を付けると、AC時にも生成入力と双方向の通信内容を表示する。

```bash
python3 stress.py -n 10 --interactive --detail
```

失敗時には次のファイルへ省略前の内容が保存される。

- `in.txt`
- `transcript_main_to_agent.txt`
- `transcript_agent_to_main.txt`
- `err_main.txt`
- `err_agent.txt`

## WA時に保存されるファイル

WAを発見すると、次のファイルが更新される。

- `in.txt`
- `out_main.txt`
- `out_brute.txt`
- `err_main.txt`
- `err_brute.txt`

`--stop`を付けず複数のWAが発生した場合、これらには最後に発見したWAが保存される。

保存された入力は次のように再確認できる。

```bash
./main < in.txt
./brute < in.txt
```
