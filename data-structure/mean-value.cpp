/*
  <中央値を保持する>
    - multiset を 2 つ持ち，動的集合の中央値を管理する
    - 要素の追加，削除，中央値取得ができる
    - 重複要素も扱える

    [概要]
        - lower  : 小さい側の要素を持つ
        - higher : 大きい側の要素を持つ

        常に次の条件を保つ:
            - lower.size() == higher.size()
              または
              lower.size() == higher.size() + 1

            - lower の任意の要素 <= higher の任意の要素

        したがって，lower の最大値が lower median になる．

    [実装/関数]
        - MedianValue<T> med
            中央値管理構造を作る

        - void push(T x)
            x を追加する

        - bool erase(T x)
            x を 1 個削除する
            削除できたら true，存在しなければ false を返す

        - T get_median()
            現在の中央値を返す
            要素数が奇数なら通常の中央値
            要素数が偶数なら lower median と upper median の平均を返す

        - T get_lower_median()
            lower median を返す
            要素数が偶数のときは小さい方の中央値

        - T get_upper_median()
            upper median を返す
            要素数が偶数のときは大きい方の中央値

        - pair<int,int> get_size()
            {lower.size(), higher.size()} を返す

        - int size()
            全体の要素数を返す

        - bool empty()
            空かどうかを返す

        - void clear()
            全要素を削除する

    [計算時間]
        - push              : O(log N)
        - erase             : O(log N)
        - get_median        : O(1)
        - get_lower_median  : O(1)
        - get_upper_median  : O(1)
        - size, empty       : O(1)

    [備考]
        - T は比較演算子 < を持つ必要がある
        - get_median() では T に + と / が定義されている必要がある
        - int 型などで偶数個の中央値を平均すると，小数部分は切り捨てになる
        - 平均が不要な場合は get_lower_median() または get_upper_median() を使うとよい
        - 空の状態で中央値を取得すると assert で落ちる

    [verified at]
        -

    [使用例]
        MedianValue<int> med;
        med.push(3);
        med.push(1);
        med.push(5);

        cout << med.get_lower_median() << endl; // 3
        cout << med.get_upper_median() << endl; // 3
        cout << med.get_median() << endl;       // 3

        med.push(10);
        cout << med.get_lower_median() << endl; // 3
        cout << med.get_upper_median() << endl; // 5
        cout << med.get_median() << endl;       // 4

        med.erase(3);
*/

template <typename T>
struct MedianValue {
    multiset<T> lower, higher;

    MedianValue() {}

private:
    void balance() {
        // lower のサイズが higher 以上になるようにする
        while (lower.size() < higher.size()) {
            auto it = higher.begin();
            lower.insert(*it);
            higher.erase(it);
        }

        // lower が大きすぎる場合，最大値を higher に移す
        while (lower.size() > higher.size() + 1) {
            auto it = prev(lower.end());
            higher.insert(*it);
            lower.erase(it);
        }
    }

public:
    void push(const T& x) {
        if (lower.empty() || x <= *prev(lower.end())) {
            lower.insert(x);
        } else {
            higher.insert(x);
        }

        balance();
    }

    bool erase(const T& x) {
        auto it_low = lower.find(x);
        if (it_low != lower.end()) {
            lower.erase(it_low);
            balance();
            return true;
        }

        auto it_high = higher.find(x);
        if (it_high != higher.end()) {
            higher.erase(it_high);
            balance();
            return true;
        }

        return false;
    }

    T get_lower_median() const {
        assert(!empty());
        return *prev(lower.end());
    }

    T get_upper_median() const {
        assert(!empty());

        if (lower.size() > higher.size()) {
            return *prev(lower.end());
        } else {
            return *higher.begin();
        }
    }

    T get_median() const {
        assert(!empty());

        if (lower.size() > higher.size()) {
            return *prev(lower.end());
        } else {
            return (*prev(lower.end()) + *higher.begin()) / 2;
        }
    }

    pair<int, int> get_size() const {
        return {(int)lower.size(), (int)higher.size()};
    }

    int size() const {
        return (int)lower.size() + (int)higher.size();
    }

    bool empty() const {
        return lower.empty() && higher.empty();
    }

    void clear() {
        lower.clear();
        higher.clear();
    }
};