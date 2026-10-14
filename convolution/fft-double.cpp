/*
  <double型の畳み込み>
    - 

    [実装/関数]
        - 関数として実装
        - vector<double> convolution_double(vector<double> a, vector<double> b)
            はaとbの畳み込み結果を返す
        - 出力されるベクトルのサイズは a.size()+b.size()-1
    
    [計算時間]
        - O(NlogN)
    
    [参照]
        - ChatGPTが作成

    [verified at]
        - 
    
    [使用例]
        
        
*/


typedef complex<double> cd;

// FFT の実装（反転処理を含む）
void fft(vector<cd>& a, bool invert) {
    int n = a.size();
    // ビット逆順並び替え
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j)
            swap(a[i], a[j]);
    }

    // 長さ len ごとに DFT を行う
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * M_PI / len * (invert ? -1 : 1);
        cd wlen(cos(ang), sin(ang));
        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; j++) {
                cd u = a[i + j];
                cd v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    // 逆変換の際は各要素を n で割る
    if (invert) {
        for (cd &x : a)
            x /= n;
    }
}

// 畳み込み（畳み込み結果のサイズは a.size() + b.size() - 1）
vector<double> convolution_double(const vector<double>& a, const vector<double>& b) {
    int n = 1;
    while (n < (int)a.size() + (int)b.size() - 1)
        n <<= 1;
    vector<cd> fa(n), fb(n);
    
    // 入力配列を複素数配列に変換（実部に値を入れる）
    for (size_t i = 0; i < a.size(); i++)
        fa[i] = a[i];
    for (size_t i = 0; i < b.size(); i++)
        fb[i] = b[i];
    
    // FFT を実施
    fft(fa, false);
    fft(fb, false);
    
    // 点ごとの積
    for (int i = 0; i < n; i++)
        fa[i] *= fb[i];
    
    // 逆 FFT で元の空間に戻す
    fft(fa, true);
    
    // 実部を結果として取得（丸め誤差がある場合は適宜丸める）
    vector<double> result(n);
    for (int i = 0; i < n; i++)
        result[i] = fa[i].real();
    
    // 実際の畳み込み結果はサイズ a.size() + b.size() - 1
    result.resize(a.size() + b.size() - 1);
    return result;
}