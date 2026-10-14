#include <bitset>
using namespace std;

bitset<N> b;              // N bit の固定長 bitset。全 bit 0
bitset<N> b(x);           // 整数 x の2進表現で初期化。b[0] は最下位bit（右端）
bitset<N> b("010101");    // 文字列で初期化。右端の文字が b[0]

b[i];                     // i bit目を取得/変更
b.test(i);                // i bit目を取得（範囲外なら例外）

b.set();                  // 全 bit を 1 にする
b.set(i);                 // i bit目を 1 にする
b.set(i, v);              // i bit目を v にする

b.reset();                // 全 bit を 0 にする
b.reset(i);               // i bit目を 0 にする

b.flip();                 // 全 bit を反転
b.flip(i);                // i bit目だけ反転

b.count();                // 1 の個数を返す
b.any();                  // 1 が1つでもあれば true
b.none();                 // 全 bit が 0 なら true
b.all();                  // 全 bit が 1 なら true
b.size();                 // bit 数 N を返す

a & b;                    // bitwise AND
a | b;                    // bitwise OR
a ^ b;                    // bitwise XOR
~a;                       // bitwise NOT

a &= b;                   // a = a & b
a |= b;                   // a = a | b
a ^= b;                   // a = a ^ b

a << k;                   // k bit 左シフト（はみ出したbitは捨てる）
a >> k;                   // k bit 右シフト
a <<= k;                  // a を k bit 左シフト
a >>= k;                  // a を k bit 右シフト

b.to_string();            // "0101..." 形式の string に変換
b.to_ulong();             // unsigned long に変換
b.to_ullong();            // unsigned long long に変換

cin >> b;                 // 0/1文字列を入力
cout << b;                // 0/1文字列として出力

b._Find_first();          // 最初の 1 bit の index。なければ b.size()（GCC/libstdc++拡張）
b._Find_next(i);          // i より後の最初の 1 bit の index。なければ b.size()（GCC/libstdc++拡張）

for (size_t i = b._Find_first(); i < b.size(); i = b._Find_next(i)) {
    // 1 になっている bit の index を順に列挙