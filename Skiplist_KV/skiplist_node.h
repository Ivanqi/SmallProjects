#ifndef SKIPLIST_NODE_H
#define SKIPLIST_NODE_H

// 实现节点的类模板
template<typename K, typename V>
class Node
{
    private:
        K key_;
        V value_;

    public:
        Node()
        {

        }

        Node(K k, V v, int);

        ~Node();

        K get_key() const;

        V get_value() const;

        void set_value(V);

        // 用于保存指向不同级别的下一个节点的指针的线性数组
        Node<K, V> **forward_;

        int node_level_;
};


template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level)
    :key_(k), value_(v), node_level_(level), forward_(new Node<K, V>*[level + 1])
{
    /**
     * level + 1，因为数组索引是从 0 - level
     * 
     * 用 0(NULL) 填充前向数组
     */
    memset(forward_, 0, sizeof(Node<K, V>*) * (level + 1));
}

template<typename K, typename V>
Node<K, V>::~Node()
{
    delete [] forward_;
}

template<typename K, typename V>
K Node<K, V>::get_key() const
{
    return key_;
}

template<typename K, typename V> 
V Node<K, V>::get_value() const {
    return value_;
};

template<typename K, typename V>
void Node<K, V>::set_value(V value)
{
    value_ = value;
}


#endif