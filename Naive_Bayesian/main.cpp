#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include "naive_bayes.h"
using namespace std;

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

bool iris() {

    vector<string> header {
        "sepal length",
        "sepal width",
        "petal length",
        "petal width",
        "class"
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

        cout<<'<';
        for (auto& j : data) {
            cout << j << ", ";
        }
        cout << real << ">\t the answer is" << ans << endl;

    }

    return true;
}

int main() {

    playTennis();
    return 0;
}