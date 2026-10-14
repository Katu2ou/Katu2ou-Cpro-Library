# 競プロ環境でのC++デバッグ方法

この環境では、C++コードをHomebrew GCCの`g++`でデバッグビルドし、
VS CodeのCodeLLDBから実行する。

## 使用するもの

- C++コンパイラ：`/opt/homebrew/bin/g++`
- C++規格：GNU C++20
- デバッガ：CodeLLDB
- VS Code設定：
  - `/Users/kaiwa/competitive-programming/.vscode/tasks.json`
  - `/Users/kaiwa/competitive-programming/.vscode/launch.json`

`.cpp`ファイルのコンパイルには`gcc`ではなく`g++`を使う。
`g++`はC++標準ライブラリも自動的にリンクする。

## 基本的な使い方

### 1. デバッグするC++ファイルを開く

VS Code上で、デバッグしたい`.cpp`ファイルをアクティブにする。
デバッグ開始時には、現在アクティブなファイルが自動的にコンパイルされる。

生成される実行ファイル名は次のようになる。

```text
a.cpp  ->  a_dbg
main.cpp  ->  main_dbg
```

デバッグビルドでは、おおむね次のオプションが使用される。

```bash
/opt/homebrew/bin/g++ \
  -std=gnu++20 \
  -O0 \
  -g3 \
  -fno-omit-frame-pointer \
  -DLOCAL \
  -Wall \
  -Wextra \
  -I/Users/kaiwa/ac-library \
  a.cpp -o a_dbg
```

- `-O0`：最適化を切り、ソースコードの順番どおり追いやすくする
- `-g3`：デバッグ情報を埋め込む
- `-fno-omit-frame-pointer`：コールスタックを追いやすくする
- `-DLOCAL`：ローカルデバッグ用コードを有効にする

### 2. ブレークポイントを設定する

停止させたい行の行番号の左側をクリックする。
赤い丸が表示されればブレークポイントが設定されている。

ショートカットでは`F9`で設定・解除できる。
Macのキーボード設定によっては`fn + F9`が必要になる。

最初は`solve()`の先頭や、調べたいループの直前に設定するとよい。

### 3. デバッグ構成を選ぶ

VS Code左側の「実行とデバッグ」を開き、次のいずれかを選択する。

#### Debug Active File (CodeLLDB, in.txt)

アクティブなC++ファイルと同じフォルダにある`in.txt`を標準入力として使う。
標準出力と標準エラーはDebug Consoleに表示される。

```text
problem/
├── a.cpp
└── in.txt
```

通常の競プロ問題や、ランダムチェッカーが保存した反例を調べるときに使う。

#### Debug Active File (CodeLLDB, terminal input)

VS Codeの統合ターミナルから手動で入力する。
入力が短い場合や、実行中に入力したい場合に使う。

#### Debug Existing Binary (CodeLLDB)

すでに通常コンパイル済みの実行ファイルをデバッグする。
この構成ではデバッグ開始前の自動コンパイルを行わない。

通常は、デバッグ情報を確実に含む前二つの構成を使う。

### 4. デバッグを開始する

構成を選択したら`F5`を押す。Macでは`fn + F5`が必要な場合がある。

次の処理が自動的に行われる。

1. アクティブなC++ファイルを`g++`でデバッグビルド
2. CodeLLDBを起動
3. 指定した入力方法でプログラムを実行
4. ブレークポイントの行で停止

## 実行中の基本操作

| 操作 | ショートカット | 内容 |
|---|---:|---|
| 続行 | `F5` | 次のブレークポイントまで実行 |
| ステップオーバー | `F10` | 現在行を実行し、次の行へ進む |
| ステップイン | `F11` | 呼び出した関数の中へ入る |
| ステップアウト | `Shift + F11` | 現在の関数を抜ける |
| 一時停止 | `F6`など | 実行中のプログラムを停止させる |
| デバッグ終了 | `Shift + F5` | プログラムを終了する |

Macの設定によっては各ファンクションキーに`fn`を加える。

## 変数を確認する

プログラムが停止すると、「実行とデバッグ」欄に次が表示される。

- Variables：現在のスコープにある変数
- Watch：指定した式を継続して監視
- Call Stack：現在の関数がどこから呼ばれたか
- Breakpoints：設定中のブレークポイント

### Variables

変数の左にある三角を押すと、`vector`、`map`、構造体などの中身を展開できる。

### Watch

Watch欄の`+`を押し、確認したい式を入力する。

```cpp
i
dp[i]
a.size()
gr[v]
dist(gr[i], gr[j])
```

変数がスコープ外にある場合、その式は評価できない。

### マウスオーバー

停止中にソースコード上の変数へマウスを重ねると、その値を確認できる。

## 条件付きブレークポイント

何度も通るループでは、通常のブレークポイントだと停止回数が多くなる。
ブレークポイントを右クリックして「式」を選び、条件を指定する。

```cpp
i == 54
dp[i] > ans
v == target
a.size() == 0
```

条件が真になったときだけ停止する。

「ヒットカウント」を使うと、指定回数だけその行を通過したときに停止できる。

## 値が変化した瞬間に停止する

停止中にVariables欄の変数を右クリックし、
「Break When Value Changes」を選ぶとウォッチポイントを設定できる。

例えば、意図しない場所で`ans`や配列要素が変更されている場合に有効。
ハードウェア上の制限があるため、同時に設定できる個数は多くない。

## Debug Console

停止中はDebug ConsoleからLLDBコマンドを実行できる。

よく使うコマンド：

```text
frame variable
frame variable dp
bt
breakpoint list
continue
```

- `frame variable`：現在の変数一覧
- `frame variable dp`：指定した変数を表示
- `bt`：コールスタックを表示
- `breakpoint list`：ブレークポイント一覧
- `continue`：実行を続行

CodeLLDBのDebug ConsoleでC++式を評価する場合は、先頭に`?`を付ける。

```text
?i
?dp[i]
?a.size()
```

## ランダムチェッカーで見つけた反例を調べる

ランダムチェッカーを最初のWAで止める。

```bash
python3 stress.py -n --stop
```

WAになった入力が`in.txt`へ保存されたら、次の手順で調べる。

1. `a.cpp`をアクティブにする
2. 怪しい遷移やループへブレークポイントを置く
3. `Debug Active File (CodeLLDB, in.txt)`を選ぶ
4. `F5`で開始する
5. VariablesやWatchで状態を確認する

`a.cpp`と`in.txt`が同じフォルダにあることを確認する。

## `LOCAL`を使ったデバッグ出力

デバッグビルドでは`-DLOCAL`が指定されるため、次のようなコードを使える。

```cpp
#ifdef LOCAL
cerr << "i = " << i << ", dp[i] = " << dp[i] << '\n';
#endif
```

通常の提出用コンパイルで`-DLOCAL`を付けなければ、この出力は無効になる。

`LOCAL`の有無によってアルゴリズム自体が変化しないよう注意する。

## コマンドラインからLLDBを使う

VS Codeを使わず、ターミナルから操作することもできる。

```bash
/opt/homebrew/bin/g++ \
  -std=gnu++20 -O0 -g \
  -fno-omit-frame-pointer \
  -I/Users/kaiwa/ac-library \
  a.cpp -o a_dbg

xcrun lldb ./a_dbg
```

LLDB起動後：

```text
(lldb) breakpoint set --name solve
(lldb) process launch -i in.txt
(lldb) next
(lldb) step
(lldb) frame variable
(lldb) bt
(lldb) continue
(lldb) quit
```

短縮形も使える。

```text
b solve
n
s
bt
c
q
```

## よくある問題

### ブレークポイントで停止しない

次を確認する。

- デバッグしたい`.cpp`がアクティブになっているか
- CodeLLDB用の構成を選んでいるか
- `_dbg`の付いたバイナリを実行しているか
- コンパイルオプションに`-g`または`-g3`があるか
- 強い最適化が有効になっていないか

ソースに次のような指定がある場合、デバッグ中は無効にする。

```cpp
#pragma GCC optimize("O3")
```

必要なら次のようにローカルデバッグ時だけ無効化する。

```cpp
#ifndef LOCAL
#pragma GCC optimize("O3")
#endif
```

### `in.txt`を開けない

`a.cpp`と同じフォルダに`in.txt`を作るか、
`Debug Active File (CodeLLDB, terminal input)`を使用する。

`in.txt`用の構成では、統合ターミナルのterminal agentを使用せず、
入力を`in.txt`から読み、出力をDebug Consoleへ直接表示する。

### 変数が`optimized out`になる

デバッグビルドが`-O0`になっていることを確認する。
通常ビルドの実行ファイルではなく、`_dbg`の付いたものを使う。

### CodeLLDBが起動できない

以下を確認する。

1. VS Codeの拡張機能からCodeLLDBが有効になっているか
2. VS Codeを再起動したか
3. macOSから表示されたデバッグ権限の確認を許可したか
4. 「システム設定 → プライバシーとセキュリティ → デベロッパツール」で
   Visual Studio Codeが許可されているか

通常の自作プログラムをデバッグするためにSIPを無効化する必要はない。

### `terminal-agent`で`ConnectionRefused`と表示される

次のようなメッセージがターミナルに出ることがある。

```text
codelldb terminal-agent --connect=...
Error: Os { code: 61, kind: ConnectionRefused, ... }
```

これは解答プログラムのエラーではなく、CodeLLDBの統合ターミナル用プロセスが、
すでに終了したデバッグアダプターへ接続しようとしたときに発生する。

`in.txt`を使う場合は、次の順番で再実行する。

1. `Shift + F5`ですべてのデバッグセッションを終了する
2. エラーが表示された古い統合ターミナルを閉じる
3. `a.cpp`と同じフォルダに`in.txt`が存在することを確認する
4. `Debug Active File (CodeLLDB, in.txt)`を選び直す
5. `F5`で開始する

まだ古い設定が使われる場合は、コマンドパレットから
`Developer: Reload Window`を実行してVS Codeを再読み込みする。

`in.txt`用構成は`"terminal": "console"`に設定されているため、
正常ならterminal agentは起動せず、解答の出力はDebug Consoleに表示される。

それでも同じエラーが新しいセッションごとに発生する場合は、
VS Codeの拡張機能画面からCodeLLDBを最新版へ更新する。

### デバッグ中だけ答えが変わる

次の可能性を調べる。

- 未初期化変数
- 配列外アクセス
- 符号付き整数オーバーフロー
- イテレータの無効化
- `LOCAL`内で状態を書き換えている
- 最適化の有無で表面化する未定義動作

この場合はLLDBに加えて、`_GLIBCXX_DEBUG`やSanitizerの利用も検討する。

## 使い分け

- 値を素早く見る：`cerr`、`dbg(...)`
- 特定条件で止める：条件付きブレークポイント
- 値を書き換えた場所を探す：ウォッチポイント
- 関数の呼び出し経路を見る：Call Stack、`bt`
- REやメモリ破壊を調べる：Sanitizer
- 小さい反例を探す：ランダムチェッカー
- 反例を1行ずつ追う：CodeLLDB
