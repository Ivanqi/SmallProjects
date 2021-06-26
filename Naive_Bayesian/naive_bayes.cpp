#include "naive_bayes.h"
#include <cmath>

naive_bayes::naive_bayes()
    : m_estimate_(1)
{

}

bool naive_bayes::clear()
{
    headers_.clear();
    attr_to_int_.clear();
    int_to_attr_.clear();
    attrs_size_.clear();
    is_numeric_.clear();
    datas_.clear();
    p_datas_.clear();
    target_to_label_.clear();
    p_target_.clear();
    return true;
}

/**
 * function: naive_bayes::set_data 设置数据集，产生辅助数据
 * d: 数据集
 * h: 属性, 需保证最后一列为目标属性，且为离散值
 * b: 属性是离散值(false), 还是数值型(true)
 */
bool naive_bayes::set_data(vvs& d, vs& h, vb b)
{
    bool f = clear();
    if (!f) return f;

    assert(d.size() > 0);   // 数据集不能为空
    datas_ = d;
    headers_ = h;
    num_attr_ = (int) headers_.size();  // 列
    num_data_ = (int) d.size();         // 行
    std::cout << "num_attr_: " << num_attr_ << " | num_data_: " << num_data_ << std::endl;

    is_numeric_ = b;
    is_numeric_.resize(num_attr_, false);
    
    assert(is_numeric_.back() == false);    // 目标属性必须为离散值

    target_attr_ = headers_.back();         // header的最后一个元素

    attr_to_int_.resize(num_attr_);         // 按列(header)计算每个 header 下不重复的key的类别序号
    int_to_attr_.resize(num_attr_);         // 按列(header)记录每个不重复key
    attrs_size_.resize(num_attr_);          // 按列(header)得到不重复key的数量

    // 类型分类
    for (int i = 0; i < num_data_; ++i) {
        auto& e = d[i];
        for (int j = 0; j < num_attr_; ++j) {
            if (is_numeric_[j]) {
                continue;                   // 数值型数据不需要映射
            }

            // std::cout << j << " | " << e[j] << std::endl;
            auto it = attr_to_int_[j].find(e[j]);
            if (it == attr_to_int_[j].end()) {
                attr_to_int_[j][e[j]] = (int) int_to_attr_[j].size();   // 每次都是更新 attr_to_int_[j]中 e[j] 数量
                int_to_attr_[j].push_back(e[j]);                        // 相同下标的e[j] 存入 int_to_attr_[j] 中
                // std::cout << "j: " << j << ", e[j]: " << e[j] << " | size: " << int_to_attr_[j].size() << std::endl;
            }
        }
    }

    for (int i = 0; i < num_attr_; ++i) {
        attrs_size_[i] = (int) int_to_attr_[i].size();
    }

    // back 返回vector中最后一个元素
    num_targ_ = attrs_size_.back();

    // 目标属性值下标
    for (int i = 0; i < num_data_; ++i) {
        // 取attr_to_int_最后一个桶里的map
        // 取d[i]中最后一个元素
        // 然后得到d[i]中最后一个元素在attr_to_int_中记录的分类ID
        // std::cout << "xxx: " << d[i][num_attr_ - 1] << " | " << attr_to_int_[num_attr_ - 1][d[i][num_attr_ - 1]] << std::endl;
        target_to_label_.push_back(attr_to_int_[num_attr_ - 1][d[i][num_attr_ - 1]]);
    }

    return true;
}

/**
 * function: naive_bayes::run  获取各部分概率值，为分类做好准备
 */
bool naive_bayes::run()
{
    p_target_.resize(num_targ_);
    p_datas_.resize(num_targ_);             // 每行表示一个目标属性值

    for (int i = 0; i < num_targ_; ++i) {
        p_datas_[i].resize(num_attr_ - 1);  // 每列表示一个非目标属性
    }


    for (int k = 0; k < num_targ_; ++k) {
        // 对每个目标属性值
        vi data_k;                          // 目标属性值下标为k的数据集
        for (int i = 0; i < num_data_; ++i) {
            if (target_to_label_[i] == k) {
                data_k.push_back(i);
            }
        }

        p_target_[k] = (double) data_k.size() / num_data_;  // 计算每个属性值的概率

        for (int j = 0; j < num_attr_ - 1; ++j) {
            // 对每个非目标属性
            int k_size = (int) data_k.size();
            auto& p = p_datas_[k][j];                       // 当前需要计算的结点

            if (is_numeric_[j]) {
                // 计算均值和方差
                double mean_value = 0, variance = 0, sum = 0;
                vd tmp;
                tmp.resize(k_size);

                for (int i = 0; i < k_size; ++i) {
                    // std::stod 将字符串转换为双精度
                    tmp[i] = std::stod(datas_[data_k[i]][j]), sum += tmp[i];
                }

                mean_value = sum / k_size;

                for (int i = 0; i < k_size; ++i) {
                    variance += (tmp[i] - mean_value) * (tmp[i] - mean_value);
                }

                variance /= k_size;

                // 保存结果
                p.is_num_ = true;
                p.mean_value_ = mean_value;
                p.variance_ = variance;
                continue;
            }

            // else
            p.is_num_ = false;
            p.p_attr_.clear();
            p.p_attr_.resize(attrs_size_[j], 0.0);

            // 计算每个属性值的数量，即有多少条数据具有该属性值
            for (int i = 0; i < k_size; ++i) {
                // data_k[i] 基于 num_targ_ 从 target_to_label_ 得到的下标值
                // datas_[x][y]: 样本数据
                // datas_[data_k[i]][j]: 通过 data_k[i] 取出行的下标，j作为列的下标。然后这样的组合，取出样本数据
                // attr_to_int_[j]: 得到每列不重复key对应每列的不重复的列字段的数量
                int tmp = attr_to_int_[j][datas_[data_k[i]][j]];
                std::cout << "k:" << k << " | xxx: " << datas_[data_k[i]][j] << " | tmp: " << tmp << std::endl;
                p.p_attr_[tmp]++;
            }

            // 计算概率
            double pp = 1.0 / attrs_size_[j];   // 用于 m-估计的先验概率
            for (int i = 0; i < attrs_size_[j]; ++i) {
                p.p_attr_[i] = (p.p_attr_[i] + m_estimate_ * pp) / (k_size + m_estimate_);
            }
        }
    }
    
    return true;
}

/**
 * function: naive_bayes::classification 对数据进行分类
 * return: 该数据最可能的目标属性值
 */
std::string naive_bayes::classification(vs& data)
{
    assert((int) data.size() == num_attr_ - 1);

    // 为了防止溢出，以下对概率值取了对数
    int max_index = -1;
    double p_max = -1e300;  // 最大概率
    
    vd p_targ_val;          // 每个目标值对该数据的概率 P(data | target_attr[i])
    p_targ_val.resize(num_targ_, 0.0);

    auto f = [&](double x, double u, double d) {
        // 求正态分布概率密度
        // exp: 计算指数函数
        // acos: 计算反余弦
        return std::exp(-(x - u) * (x - u) / (2 * d)) / sqrt(4 * std::acos(-1) * d);
    };

    for (int i = 0; i < num_targ_; ++i) {
        auto& t = p_targ_val[i];    // 值引用
        t = std::log(p_target_[i]); // 取对数
        
        for (int j = 0; j < num_attr_ - 1; ++j) {
            auto& p = p_datas_[i][j];
            if (is_numeric_[j]) {
                // stod: 将字符串转换为双精度
                t += std::log(f(std::stod(data[j]), p.mean_value_, p.variance_));
            } else {
                auto it = attr_to_int_[j].find(data[j]);
                if (it == attr_to_int_[j].end()) {
                    std::cerr << "No such attribute value. " << std::endl;
                    exit(1);
                }
                t += std::log(p.p_attr_[it->second]);
            }
        }
    }

    // 找到最大概率值
    for (int i = 0; i < num_targ_; ++i) {
        if (p_max < p_targ_val[i]) {
            p_max = p_targ_val[i], max_index = i;
        }
    }

    return int_to_attr_[num_attr_ - 1][max_index];
}

naive_bayes::~naive_bayes()
{
    clear();
}