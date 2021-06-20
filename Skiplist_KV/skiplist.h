#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "dumpFile"

std::mutex mtx;                 // 临界的互斥锁
std::string delimiter = ";";

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

// skiplist 的类模板
template<typename K, typename V>
class SkipList
{
    private:
        int max_level_;             // skiplist 的最大层级
        int skip_list_level_;       // 当前层级
        Node<K, V>* header_;        // 指向头节点的指针
        
        std::ofstream file_writer_; // 文件操作符
        std::ifstream file_reader_; // 文件操作符

        int element_count_;         // skiplist 当前元素
    
    public:
        SkipList(int);
        ~SkipList();
        int get_random_level();
        Node<K, V>* create_node(K, V, int);
        int insert_element(K, V);
        void display_list();
        bool search_element(K);
        void delete_element(K);
        void dump_file();
        void load_file();
        int size();
    
    private:
        void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
        bool is_valid_string(const std::string& str);
};

// 创建新的节点
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level)
{
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}


/* 

在跳过列表中插入给定的键和值
返回 1 表示元素存在
return 0 表示插入成功

                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+
*/
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value)
{
    mtx.lock();
    Node<K, V> *current = header_;

    // 创建更新数组并初始化它
    // update 是放置节点的数组，node->forward[i] 应该稍后操作
    Node<K, V> *update[max_level_ + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (max_level_ + 1));

    // 从skiplist 的最大层级开始
    for (int i = skip_list_level_; i >= 0; i--) {
        while (current->forward_[i] != NULL && current->forward_[i]->get_key() < key) {
            current = current->forward_[i];
        }
        update[i] = current;
    }

    current = current->forward_[0];

    // 如果当前节点的键等于搜索到的键，就可以返回信息
    if (current != NULL && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // 如果 current 为 NULL，则表示我们已到达该级别的末尾
    // 如果当前的键不等于键，这意味着我们必须在 update[0] 和当前节点之间插入节点
    if (current == NULL || current->get_key() != key) {
        // 为节点生成一个随机级别
        int random_level = get_random_level();

        // 如果随机级别大于跳过列表的当前级别，则使用指向标头的指针初始化更新值
        if (random_level > skip_list_level_) {
            for (int i = skip_list_level_ + 1; i < random_level + 1; i++) {
                update[i] = header_;
            }
            skip_list_level_ = random_level;
        }

        // 创建具有随机级别的新节点
        Node<K, V>* inserted_node = create_node(key, value, random_level);

        // insert node
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward_[i] = update[i]->forward_[i];
            update[i]->forward_[i] = inserted_node;
        }

        std::cout << "Successfully inserted key: " << key << ", value: " << std::endl;
        element_count_++;
    }

    mtx.unlock();
    return 0;
}

// 显示skiplist
template<typename K, typename V> 
void SkipList<K, V>::display_list()
{
    std::cout << "\n*****Skip List*****"<<"\n";
    for (int i = 0; i <= skip_list_level_; i++) {
        Node<K, V> *node = header_->forward_[i];
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward_[i];
        }
        std::cout << std::endl;
    }
}

// 将内存中的数据转储到文件
template<typename K, typename V> 
void SkipList<K, V>::dump_file()
{
    std::cout << "dump_file-----------------" << std::endl;
    file_writer_.open(STORE_FILE);

    Node<K, V> *node = header_->forward_[0];

    while (node != NULL) {
        file_writer_ << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ":\n";
        node = node->forward_[0];
    }

    file_writer_.flush();
    file_writer_.close();
    
    return;
}

// 从磁盘加载数据
template<typename K, typename V>
void SkipList<K, V>::load_file()
{
    file_reader_.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;

    std::string line;
    std::string *key = new std::string();
    std::string *value = new std::string();

    while (getline(file_reader_, line)) {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty()) {
            continue;
        }

        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }

    file_reader_.close();
}

// 获取当前的 SkipList 大小
template<typename K, typename V>
int SkipList<K, V>::size()
{
    return element_count_;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value)
{
    if (!is_valid_string(str)) {
        return;
    }

    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

// 从skiplist 删除数据
template<typename K, typename V>
void SkipList<K, V>::delete_element(K key)
{
    mtx.lock();
    Node<K, V> *current = header_;
    Node<K, V> *update[max_level_ + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (max_level_ + 1));

    // 从skiplist 的最大层级开始
    for (int i = skip_list_level_; i >= 0; i--) {
        while (current->forward_[i] != NULL && current->forward_[i]->get_key() < key) {
            current = current->forward_[i];
        }
        update[i] = current;
    }

    current = current->forward_[0];

    if (current != NULL && current->get_key() == key) {
        // 从最低层开始，删除每一层的当前节点
        for (int i = 0; i <= skip_list_level_; i++) {
            // 如果在第 i 层，下一个节点不是目标节点，则中断循环
            if (update[i]->forward_[i] != current) {
                break;
            }

            update[i]->forward[i] = current->forward_[i];
        }

        // 删除没有数据的层级 
        while (skip_list_level_ > 0 && header_->forward_[skip_list_level_] == 0) {
            skip_list_level_--;
        }

        std::cout << "Successfully deleted key" << key << std::endl;
        element_count_--;
    }

    mtx.unlock();
    return ;
}

/*
在skiplist 中搜索元素

                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K, typename V> 
bool SkipList<K, V>::search_element(K key)
{
    std::cout << "search_element-----------------" << std::endl;
    Node<K, V> *current = header_;

    // 从skiplist 的最大层级开始
    for (int i = skip_list_level_; i >= 0; i--) {
        while (current->forward_[i] && current->forward_[i]->get_key() < key) {
            current = current->forward_[i];
        }
    }

    // 到达第 0 级并将指针前进到我们搜索的右节点
    current = current->forward_[0];

    // 如果当前节点的键等于搜索到的键，我们得到它
    if (current && current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// 构造函数
template<typename K, typename V> 
SkipList<K, V>::SkipList(int max_level)
    :max_level_(max_level), skip_list_level_(0), element_count_(0)
{
    // 创建头节点并将键和值初始化为空
    K k;
    V v;
    header_ = new Node<K, V>(k, v, max_level_);
}

template<typename K, typename V> 
SkipList<K, V>::~SkipList()
{
    if (file_writer_.is_open()) {
        file_writer_.close();
    }

    if (file_reader_.is_open()) {
        file_reader_.close();
    }

    delete header_;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level()
{
    int k = 1;
    while (rand() % 2) {
        k++;
    }

    k = (k < max_level_) ? k : max_level_;
    return k;
}

#endif