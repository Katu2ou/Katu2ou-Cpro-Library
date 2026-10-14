/*
  <両端 priority queue>
    - 最小値・最大値の両方を効率的に取得・削除できるデータ構造
    - multiset によって実装する
    - 重複要素を扱うことができる

    [実装/関数]
        - DoubleEndedPriorityQueue<T> pq
            型 T の両端 priority queue を作る

        - void push(T x)
            x を追加する

        - const T& top_min()
            最小値を返す
            空の場合は assert で落ちる

        - const T& top_max()
            最大値を返す
            空の場合は assert で落ちる

        - void pop_min()
            最小値を 1 個削除する
            空の場合は assert で落ちる

        - void pop_max()
            最大値を 1 個削除する
            空の場合は assert で落ちる

        - bool erase(T x)
            x を 1 個削除する
            削除できたら true, 存在しなければ false を返す

        - int count(T x)
            x の個数を返す

        - bool empty()
            空かどうかを返す

        - int size()
            要素数を返す

        - void clear()
            全要素を削除する

    [計算時間]
        - push       : O(log N)
        - top_min    : O(1)
        - top_max    : O(1)
        - pop_min    : O(log N)
        - pop_max    : O(log N)
        - erase      : O(log N)
        - count      : O(log N + 個数)
            ※ multiset::count の仕様上，重複数ぶんかかることがある

    [備考]
        - 内部では multiset<T> を用いる
        - T は比較演算子 < が定義されている必要がある
        - top_min(), top_max() が返す参照は，対応する要素を削除すると無効になる
        - 値を長く保持したい場合は，T x = pq.top_min(); のようにコピーして使う
        - priority_queue 2 個で lazy deletion する実装もあるが，
          multiset 実装は簡潔でバグりにくい

    [参照]
        -

    [verified at]
        -

    [使用例]
        DoubleEndedPriorityQueue<int> pq;
        pq.push(3);
        pq.push(1);
        pq.push(5);
        pq.push(1);

        cout << pq.top_min() << endl; // 1
        cout << pq.top_max() << endl; // 5

        pq.pop_min(); // 1 を 1 個削除
        pq.pop_max(); // 5 を削除

        cout << pq.top_min() << endl; // 1
        cout << pq.top_max() << endl; // 3
*/

template <typename T>
class DoubleEndedPriorityQueue {
private:
    multiset<T> data;

public:
    DoubleEndedPriorityQueue() = default;

    void push(const T& value) {
        data.insert(value);
    }

    const T& top_min() const {
        assert(!data.empty());
        return *data.begin();
    }

    const T& top_max() const {
        assert(!data.empty());
        return *prev(data.end());
    }

    void pop_min() {
        assert(!data.empty());
        data.erase(data.begin());
    }

    void pop_max() {
        assert(!data.empty());
        data.erase(prev(data.end()));
    }

    bool erase(const T& value) {
        auto it = data.find(value);
        if (it == data.end()) return false;
        data.erase(it);
        return true;
    }

    int count(const T& value) const {
        return (int)data.count(value);
    }

    bool contains(const T& value) const {
        return data.find(value) != data.end();
    }

    bool empty() const {
        return data.empty();
    }

    int size() const {
        return (int)data.size();
    }

    void clear() {
        data.clear();
    }
};