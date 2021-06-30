#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include "naive_bayes.h"
using namespace std;

/**
 * 是否能打网球?
 *  流程
 *      1. 数据准备
 *          1. 遍历data.txt 行列的数据
 *          2. 获得 按列(header)计算每个 header 下不重复的key的类别序号. 代号A
 *          3. 获得 按列(header)记录每个不重复key. 代号B
 *          4. 获得 按列(header)得到不重复key的数量. 代号C，这个数据是从B的数据统计出来的
 *          5. 获得 当前事件下(playTennis)的标识。 NO:0, Yes: 1. 代号D, 这个数据是从A的数据得到的
 *      2. 统计部分概率值
 *          1. 设置当前事件(playTennis)的标识。 NO:0, Yes: 1, 数量为2
 *          2. 分别得到当前事件(playTennis)的列下标
 *          3. 然后计算当前事件不同标识(NO, Yes)的概率. 例P(NO) = NO num / ALL num，代号E
 *          4. 然后计算当前事件不同标识(NO, Yes)下的不同列(Sunny)的概率，例如P(Sunny|No)，代号F
 *          5. 加入m-估计，避免数据过低到时概率归0
 *      3. 数据分类
 *          1. 设置需要求的事件的概率(Sunny、Cool、High、Strong)，代号G
 *          2. 获取当前事件不同标识(NO, Yes)的概率,然后求它的对数值
 *          3. 从代号A中获取对应的事件的下标，然后从代号F中寻找对应的概率
 *              1. 例如 P(X) = P(Sunny|No) * P(Cool|No) * P(Hight|No) * P(Strong|No)
 *              2. 例如 P(Y) = P(Sunny|Yes) * P(Cool|Yes) * P(Hight|Yes) * P(Strong|Yes)
 *          4. 然后求P(X), P(Y)那个概率适合
 */
bool playTennis() {

    vector<string> header;
    vector<vector<string>> examples;

    string data_file_name = "./data/data.txt";

    // 打开文件
    ifstream input(data_file_name.c_str());

    // 读入属性名
    string line, tmp;
    getline(input, line);

    
    istringstream iss(line);

    while (iss >> tmp) {
        header.push_back(tmp);
    }

    // 读入数据集
    while (getline(input, line)) {
        istringstream iss(line);
        vector<string> v;
        
        while (iss >> tmp) {
            v.push_back(tmp);
        }

        examples.push_back(v);
    }

    input.close();

    naive_bayes nb;
    nb.set_data(examples, header);
    nb.run();

    vector<string> v = {
        "Sunny",
        "Cool",
        "High",
        "Strong"
    };

    auto ans = nb.classification(v);

    cout << "<";

    for (auto& s:v) cout << s << " ";

    cout << ans << ">" << endl;

    return true;
}

/**
 * 究竟是那种类别的花?
 *  1. 数据准备
 *      1. 遍历iris.data的数据
 *      2. 获得 iris的类别数据(Iris-setosa:0, Iris-versicolor:1, Iris-virginica: 2),代号A
 *  2. 统计分布概率
 *      1. 根据iris的类别数据A,得到该类别每行所得到的平均值和方差
 *  3. 数据分类
 *      1. 通过传入的数据(例如: 5.1 3.5 1.4 0.2) 求每个类别下的概率
 *      2. 通过整个一列的每个数据和每行的平均和方差做正态分布。然后把正态分布后的概率累加起来
 *      3. P(数据集|Iris-setosa), P(数据集|Iris-versicolor), P(数据集|Iris-virginica)求最大概率
 */
bool iris() {

    vector<string> header {
        "sepal length", // 萼片长度
        "sepal width",  // 萼片宽度
        "petal length", // 花瓣长度
        "petal width",  // 花瓣宽度
        "class"         // 类别
    };

    vector<vector<string>> examples;
    string data_file_name = "./data/iris.data";
    ifstream input(data_file_name.c_str());

    string line, tmp;
    while (getline(input, line)) {

        istringstream iss(line);
        vector<string> v;

        while (iss >> tmp) {
            v.push_back(tmp);
        }
        examples.push_back(v);
    }

    input.close();
    vector<vector<string>> test;    // 测试集

    for (int i = 40; i < 50; ++i) {
        test.push_back(examples[i]);
        test.push_back(examples[i + 50]), test.push_back(examples[i + 100]);
    }

    naive_bayes nb;
    vector<bool> flag{true, true, true, true, false};

    nb.set_data(examples, header, flag);
    nb.run();

    for (auto& data:test) {
        auto real = data.back();
        data.pop_back();
        auto ans = nb.classification(data);

        cout << '<';
        for (auto& j : data) {
            cout << j << ", ";
        }
        cout << real << ">\t the answer is" << ans << endl;
    }

    return true;
}

int main(int argc, char* argv[]) {

    if (argc >= 2 && atoi(argv[1]) == 2) {
        iris();
    } else {
        playTennis();
    }

    return 0;
}