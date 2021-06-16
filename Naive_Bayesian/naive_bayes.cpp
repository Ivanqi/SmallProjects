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
