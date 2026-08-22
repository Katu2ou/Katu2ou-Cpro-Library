/*
(https://algo-logic.info/trie-tree/)
<Trie木>
    - 一つの頂点が一つの文字を表す
    - 文字列の先頭部分(prefix)が共通する場合は，なるべく無駄がないように共有する
    - 各頂点に，いろいろな情報(子/その頂点で終わる文字列が存在するか/その頂点を通る文字列の個数...)を持たせておく

    [できること]
        文字種類数を char_size，文字の基準を base とする．
        例えば，大文字英字なら Trie<26, 'A'>，
        小文字英字なら Trie<26, 'a'> のように使う．

        ※ 同じ文字列を複数回insertできるので，setというよりmultisetに近い
        ※ search / start_with / word_count は存在するかどうかではなく，個数を返す
        ※ 返り値が1以上なら存在する，0なら存在しない，と判定できる
        - 根の頂点番号は0

        - Trie() : コンストラクタ
            根だけを持つTrie木を作る
            計算量 O(1)

        - void insert(const string &word, int word_id) :
            文字列wordをTrie木に挿入する
            word_idは，その文字列に対応するIDとして末端ノードに保存される
            同じwordを複数回挿入した場合，その回数分だけ保存される
            計算量 O(|word|)

        - void insert(const string &word) :
            文字列wordをTrie木に挿入する
            word_idを指定しない場合，自動でIDを付けて挿入する
            計算量 O(|word|)

        - bool erase(const string &word) :
            wordに完全一致する文字列を1個削除する
            削除できた場合はtrue，wordが存在しない場合はfalseを返す
            同じwordが複数回挿入されている場合は，そのうち1個だけ削除する
            計算量 O(|word|) ただし，不要ノードの切り離しを含めると O(|word| * char_size)

        - bool erase(const string &word, int word_id) :
            wordに完全一致し，かつ末端にword_idを持つものを1個削除する
            削除できた場合はtrue，存在しない場合はfalseを返す
            計算量 O(|word|) ただし，不要ノードの切り離しを含めると O(|word| * char_size)

        - int search(const string &word, bool prefix = false) :
            prefix=false の場合，
                wordに完全一致する文字列が何個挿入されているかを返す
            prefix=true の場合，
                wordをprefixとして持つ文字列が何個挿入されているかを返す
            計算量 O(|word|)

        - int start_with(const string &prefix) :
            prefixをprefixとして持つ文字列が何個挿入されているかを返す
            search(prefix, true) と同じ
            計算量 O(|prefix|)

        - int word_count(const string &word) :
            wordに完全一致する文字列が何個挿入されているかを返す
            search(word, false) と同じ
            計算量 O(|word|)

        - int count() :
            Trie木に挿入されている文字列の総数を返す
            同じ文字列を複数回insertした場合，その回数分だけ数える
            削除済みの文字列は数えない
            計算量 O(1)

        - int size() :
            現在Trie木で使われている頂点数を返す
            削除によって不要になった頂点は数えない
            計算量 O(1)

        - int allocated_size() :
            vector<Node> nodes に確保された頂点数を返す
            削除で切り離された頂点も含む
            デバッグ用
            計算量 O(1)

    [構造]
        - nodes : Trie木の全頂点を持つ配列
        - root : 根の頂点番号．基本的に0

        - Node::next :
            子の頂点番号を格納する配列
            next[c] は，文字 c に対応する子頂点の番号
            子が存在しない場合は -1

        - Node::accept :
            末端がこの頂点になる単語の word_id を保存する配列
            例えば，同じ単語を3回insertした場合，acceptのサイズは3になる

        - Node::c :
            この頂点が表す文字
            実際の文字そのものではなく，baseからの差として持つ
            例えば Trie<26, 'A'> で文字 'C' なら c = 2

        - Node::common :
            この頂点を通る単語の個数
            つまり，この頂点に対応するprefixを持つ単語の個数
            重複してinsertされた単語は重複分だけ数える

    [注意]
        - この実装では，各文字 word[i] について
              0 <= word[i] - base < char_size
          が成り立つ必要がある
          例えば Trie<26, 'A'> に小文字 'a' を入れると範囲外アクセスになる

        - search(word) は bool ではなく個数を返す
          存在判定をしたい場合は
              trie.search(word) > 0
          のように使う

        - 削除では，vector<Node> nodes から頂点自体をeraseするわけではない
          ただし，不要になった頂点への辺は切り離す
          そのため size() は現在使われている頂点数，
          allocated_size() は過去に確保された頂点を含む数を返す
*/

template <int char_size, int base>
struct Trie {
    struct Node {
        vector<int> next;    // 子の頂点番号を格納
        vector<int> accept;  // 末端がこの頂点になる単語の word_id を保存
        int c;               // base からの間隔をint型で表現したもの
        int common;          // いくつの単語がこの頂点を共有しているか

        Node(int c_) : c(c_), common(0) {
            next.assign(char_size, -1);
        }
    };

    vector<Node> nodes;
    int root;

private:
    int active_nodes;
    int next_word_id;

    int char_id(char ch) const {
        int c = (int)(ch - base);
        assert(0 <= c && c < char_size);
        return c;
    }

    int subtree_node_count(int node_id) const {
        if (node_id == -1) return 0;

        int res = 1;
        for (int nxt : nodes[node_id].next) {
            if (nxt != -1) {
                res += subtree_node_count(nxt);
            }
        }
        return res;
    }

    void erase_path_common(const vector<int> &path) {
        for (int node_id : path) {
            --nodes[node_id].common;
        }
    }

    void prune_useless_nodes(const vector<int> &path, const vector<int> &edge_chars) {
        // path[0] は root
        // path[i] は edge_chars[i - 1] を通って到達した頂点
        for (int i = (int)path.size() - 1; i >= 1; --i) {
            int node_id = path[i];

            if (nodes[node_id].common > 0) break;

            int parent = path[i - 1];
            int c = edge_chars[i - 1];

            active_nodes -= subtree_node_count(node_id);
            nodes[parent].next[c] = -1;
        }
    }

    bool erase_accept_index(const string &word, int erase_index) {
        vector<int> path;
        vector<int> edge_chars;

        int node_id = root;
        path.push_back(node_id);

        for (int i = 0; i < (int)word.size(); i++) {
            int c = char_id(word[i]);
            int next_id = nodes[node_id].next[c];

            if (next_id == -1) {
                return false;
            }

            edge_chars.push_back(c);
            node_id = next_id;
            path.push_back(node_id);
        }

        if (erase_index < 0 || erase_index >= (int)nodes[node_id].accept.size()) {
            return false;
        }

        // accept から1個削除する
        nodes[node_id].accept[erase_index] = nodes[node_id].accept.back();
        nodes[node_id].accept.pop_back();

        // 通った頂点の common を1ずつ減らす
        erase_path_common(path);

        // 使われなくなった頂点への辺を切る
        prune_useless_nodes(path, edge_chars);

        return true;
    }

public:
    Trie() : root(0), active_nodes(1), next_word_id(0) {
        nodes.push_back(Node(root));
    }

    // 単語の挿入
    void insert(const string &word, int word_id) {
        int node_id = root;

        for (int i = 0; i < (int)word.size(); i++) {
            int c = char_id(word[i]);
            int &next_id = nodes[node_id].next[c];

            if (next_id == -1) {
                next_id = (int)nodes.size();
                nodes.push_back(Node(c));
                ++active_nodes;
            }

            ++nodes[node_id].common;
            node_id = next_id;
        }

        ++nodes[node_id].common;
        nodes[node_id].accept.push_back(word_id);

        if (word_id >= next_word_id) {
            next_word_id = word_id + 1;
        }
    }

    void insert(const string &word) {
        insert(word, next_word_id++);
    }

    // word に完全一致する文字列を1個削除する
    bool erase(const string &word) {
        int node_id = root;

        for (int i = 0; i < (int)word.size(); i++) {
            int c = char_id(word[i]);
            int next_id = nodes[node_id].next[c];

            if (next_id == -1) {
                return false;
            }

            node_id = next_id;
        }

        if (nodes[node_id].accept.empty()) {
            return false;
        }

        int erase_index = (int)nodes[node_id].accept.size() - 1;
        return erase_accept_index(word, erase_index);
    }

    // word に完全一致し，かつ word_id を持つものを1個削除する
    bool erase(const string &word, int word_id) {
        int node_id = root;

        for (int i = 0; i < (int)word.size(); i++) {
            int c = char_id(word[i]);
            int next_id = nodes[node_id].next[c];

            if (next_id == -1) {
                return false;
            }

            node_id = next_id;
        }

        for (int i = 0; i < (int)nodes[node_id].accept.size(); i++) {
            if (nodes[node_id].accept[i] == word_id) {
                return erase_accept_index(word, i);
            }
        }

        return false;
    }

    // 単語とprefixの検索
    int search(const string &word, bool prefix = false) const {
        int node_id = root;

        for (int i = 0; i < (int)word.size(); i++) {
            int c = char_id(word[i]);
            int next_id = nodes[node_id].next[c];

            if (next_id == -1) {
                return 0;
            }

            node_id = next_id;
        }

        if (prefix) {
            return nodes[node_id].common;
        } else {
            return (int)nodes[node_id].accept.size();
        }
    }

    // prefix を持つ単語が何個存在するかの検索
    int start_with(const string &prefix) const {
        return search(prefix, true);
    }

    // word に一致する文字列が何個存在するか
    int word_count(const string &word) const {
        return search(word, false);
    }

    // 挿入されている単語の数
    int count() const {
        return nodes[root].common;
    }

    // 現在使われているTrie木のノード数
    int size() const {
        return active_nodes;
    }

    // vectorに確保済みのノード数
    int allocated_size() const {
        return (int)nodes.size();
    }
};