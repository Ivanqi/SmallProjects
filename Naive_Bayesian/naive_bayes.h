#ifndef NAIVE_BAYES_H
#define NAIVE_BAYES_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include <cassert>

class naive_bayes;

class _naive_bayes_node 
{
    private:
        bool is_num_;                   // 是否为数值型数据
        double mean_value_;             // 均值
        double variance_;               // 方差
        std::vector<double> p_attr_;    // 非数值型数据保存概率
    
    public:
        _naive_bayes_node()
            : is_num_(false), mean_value_(0.0), variance_(0.0)
        {

        }

        friend class naive_bayes;
};

class naive_bayes
{
    private:
        typedef std::vector<std::string> vs;
        typedef std::vector<vs> vvs;
        typedef std::vector<int> vi;
        typedef std::vector<vi> vvi;
        typedef std::unordered_map<std::string, int> usi;
        typedef std::vector<usi> vusi;
        typedef _naive_bayes_node node;
        typedef std::vector<node> vn;
        typedef std::vector<vn> vvn;
        typedef std::vector<bool> vb;
        typedef std::vector<double> vd;
    
    private:
        double m_estimate_;          // m估计
        std::string target_attr_;    // 目标属性
        vs headers_;                 // 各个属性的名称
        vusi attr_to_int_;           // 属性到整数的映射
        vvs int_to_attr_;            // 整数到属性的映射
        vi attrs_size_;              // 每个属性有多少不同的属性值
        vb is_numeric_;              // 属性是否为数值型
        vvs datas_;                  // 保存原始数据
        vvn p_datas_;                // 保存各概率值
        vd p_target_;                // 每个目标属性值的概率
        vi target_to_label_;         // 目标属性值的下标
        int num_attr_;               // 属性数量, 列
        int num_data_;               // 数据集大小, 行
        int num_targ_;               // 目标属性有多少不同的取值
    
    public:
        naive_bayes();
        bool set_data(vvs&, vs&, vb b = vb());
        bool clear(); // 使用clear释放空间
        bool run();
        std::string classification(vs&);
        ~naive_bayes();
};

#endif