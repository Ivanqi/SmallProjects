#include "Dictionary.h"

#define MaxWordLength 15
#define Separator "/"   // 词界标记

/**
 * 参考资料: 
 *  https://www.52nlp.cn/maximum-matching-method-of-chinese-word-segmentation
 *  https://blog.csdn.net/u010189459/article/details/37774003
 */

CDictionary WordDic;    // 初始化一个词典

// 对字符串用最大匹配法(正向) 处理
string SegmentSentence_1(string s1) {
    
    string s2 = "";     // 用s2存放分词结果

    while (!s1.empty()) {
        int len = (int) s1.length();    // 取输入串长度
        if (len > MaxWordLength) {      // 如果输入串长度大于最大词长
            len = MaxWordLength;        // 只在最大词长范围内进行处理
        }

        string w = s1.substr(0, len);                   // (正向用)将输入串左边等于最大词长长度取出作为候选词
        int n = WordDic.FindWord(w);                    // 在词典中查找相应的词

        while (len > 3 && n == 0) {                     // 如果不是词
            len -= 3;                                   // 从候选词右边减去一个汉字，将剩下的部分作为候选词
            w = w.substr(0, len);                       // 正向用
            n = WordDic.FindWord(w);
        }

        s2 += w + Separator;                                          // (正向用) 将匹配得到的词连同词界标记加到输出串末尾

        s1 = s1.substr(w.length(), s1.length() - w.length());        // (正向用)从s1-w处开始
    }

    return s2;
}

// 对字符串用最大匹配法(逆向) 处理
string SegmentSentence_2(string s1) {
    
    string s2 = ""; // 用s2存放分词结果

    while (!s1.empty()) {
        int len = (int) s1.length();    // 取输入串长度
        if (len > MaxWordLength) {      // 如果输入串长度大于最大词长
            len = MaxWordLength;        // 只在最大词长范围内进行处理
        }

        string w = s1.substr(s1.length() - len, len);   // 逆向用
        int n = WordDic.FindWord(w);                    // 在词典中查找相应的词

        while (len > 3 && n == 0) {                     // 如果不是词
            len -= 3;                                   // 从候选词右边减去一个汉字，将剩下的部分作为候选词
            w = s1.substr(s1.length() - len, len);      // 逆向用
            n = WordDic.FindWord(w);
        }

        w = w + Separator;          // (逆向用)
        s2 = w + s2;                // (逆向用)

        s1 = s1.substr(0, s1.length() - len);   // (逆向用)
    }

    return s2;
}



// 对句子进行最大匹配法处理，包含对特殊字符的处理
string SegmentSentenceMM(string s1, int mode)
{
    string s2 = ""; // 用s2存放分词结果
    int i;
    int dd;

    while (!s1.empty()) {
        unsigned char ch = (unsigned char)s1[0];
        if (ch < 128) {     // 处理西文字符
            i = 1;
            dd = (int)s1.length();

            while (i < dd && ((unsigned char)s1[i] < 128) && (s1[i] != 10) && (s1[i] != 13)) {   // s1[i]不能是换行符或回车符
                i++;
            }

            if ((ch != 32) && (ch != 10) && (ch != 13)) {   // 如果不是西文空格或换行或回车符
                s2 += s1.substr(0, i) + Separator;
            } else {
                //if (ch == 10 || ch == 13) // 如果是换行或回车符，将它拷贝给s2输出
                if (ch == 10 || ch == 13 || ch == 32) {
                    s2 += s1.substr(0, i);
                }
            }

            s1 = s1.substr(i, dd);
            continue;
        } else {
            if (ch < 176) { // 中文标点等非汉字字符
                i = 0;
                dd = (int)s1.length();
                while (i < dd && ((unsigned char)s1[i] < 176) && ((unsigned char)s1[i] >= 161)
                    && (!((unsigned char)s1[i] == 161 && ((unsigned char)s1[i + 1] >= 162 
                    && (unsigned char)s1[i + 1] <= 168))) && (!((unsigned char)s1[i] == 161 
                    && ((unsigned char)s1[i + 1] >= 171 && (unsigned char)s1[i + 1] <= 191))) 
                    && (!((unsigned char)s1[i] == 163 && ((unsigned char)s1[i + 1] == 172 
                    || (unsigned char)s1[i + 1] == 161) || (unsigned char)s1[i + 1] == 168 
                    || (unsigned char)s1[i + 1] == 169 || (unsigned char)s1[i + 1] == 186 || (unsigned char)s1[i+1] == 187 
                    || (unsigned char)s1[i + 1] == 191))) {
                    i = i + 2;  // 假定没有半个汉字
                }

                if (i == 0) {
                    i = i + 2;
                }

                if (!(ch == 161 && (unsigned char)s1[1] == 161)) {  // 不处理中文空格 
                    s2+=s1.substr(0, i) + Separator;                // 其他的非汉字双字节字符可能连续输出 
                } 

                s1 = s1.substr(i, dd); continue; 
            }
        }

        i = 3; 
        dd = (int)s1.length(); 

        // 非标点
        while(i < dd && (unsigned char)s1[i] >= 176) {
            i += 3;
        }

        if (mode == 1) {
            s2 += SegmentSentence_1(s1.substr(0, i));
        } else {
            s2 += SegmentSentence_2(s1.substr(0, i));
        }

        s1 = s1.substr(i, dd);
    }

    return s2;
}


/*
 * 函数功能：删除分词标记（即去掉字符串中的/）
 * 函数输入：含有分词标记的字符串
 * 函数输出：不含分词标记的字符串
 */
string removeSeparator(string str_in){
	char s[10000];
	int j = 0;
	for(int i = 0; i < str_in.length(); i++){
		if(!(str_in[i] == '/')){
			s[j] = str_in[i];
			j++;
		}
	}
	s[j] = '\0';
	string str_out = s;
	return str_out;

}

/**
 * 函数功能：计算切分标记的位置
 * 函数输入:
 * 
 *  1. strline_right 进行切分后的汉字字符串
 *  2. strline_in 未进行切分的汉字字符串
 * 
 * 函数输出: vector, 其中存放了strline_in中哪些位置放置了分词标记。注意：vector中不包含最后标记的位置，但是包含位置0
 */
vector<int> getPos(string strline_right, string strline_in) {
    int pos_1 = 0;
    int pos_2 = -1;
    int pos_3 = 0;
    string word = "";
    vector<int> vec;

    int length = strline_right.length();
    while (pos_2 < length) {
        // 前面的分词标记
        pos_1 = pos_2;

        // 后面的分词标记
        pos_2 = strline_right.find('/', pos_1 + 1);

        if (pos_2 > pos_1) {
            // 将两个分词标记之间的单词取出
            word = strline_right.substr(pos_1 + 1, pos_2 - pos_1 - 1);
            // 根据单词去输入序列中查出出现的位置
            pos_3 = strline_in.find(word, pos_3);

            // 将位置存入数组
            vec.push_back(pos_3);
            pos_3 = pos_3 + word.size();
        } else {
            break;
        }
    }

    return vec;
}

/**
 * 函数功能：获取单个句子切分的结果统计
 * 函数输入:
 *      1. vec_right 正确的分词标记位置集合
 *      2. vec_out 函数切分得到的分词标记位置集合
 * 函数输出: 返回一个vector，含有两个元素
 *      1. 不该切分而切分的数量
 *      2. 该切分而未切分的数量
 */
vector<int> getCount(vector<int> vec_right, vector<int> vec_out) {
    vector<int> vec;                // 存放计算结果
    map<int, int> map_result;

    int length_1 = 0;               // map改变前的长度
    int length_2 = 0;               // map改变后的长度
    int count_1 = 0;                // 不该切分而切分的数量
    int count_2 = 0;                // 该切分而未切分的数量

    for (int i = 0; i < vec_right.size(); i++) {
        map_result[vec_right[i]] = 0;
    }

    length_1 = map_result.size();

    for (int i = 0; i < vec_out.size(); i++) {
        ++map_result[vec_out[i]];
    }

    length_2 = map_result.size();

    count_1 = length_2 - length_1;

    for (int i = 0; i < vec_right.size(); i++) {
        if (map_result[vec_right[i]] == 0) {
            ++count_2;
        }
    }

    vec.push_back(count_1);
    vec.push_back(count_2);

    return vec;
}

int main(int argc, char *argv[]) {

    string strline; // 用于保存从语料库中读入的每一行
    string line;    // 用于输出每一行的结果

    ifstream infile(argv[1]);   // 打开输入文件
    if (!infile.is_open()) {     // 打开输入文件失败则退出程序
        cerr << "Unable to open input file: " << argv[1] << " --- bailing out!" << endl;
        exit(-1);
    }

    long count = 0;                 // 句子编号
    long count_right_all = 0;       // 准确的切分总数
    long count_out_1_all = 0;       // 正向匹配切分总数
    long count_out_2_all = 0;       // 逆向匹配切分总数
    long count_out_1_right_all = 0; // 正向匹配切分正确总数
    long count_out_2_right_all = 0; // 逆向匹配切分正确总数

    string strline_out_1;	        // 正向最大匹配分词完毕的语料
	string strline_out_2;	        // 逆向最大匹配分词完毕的语料


    while (getline(infile, strline, '\n')) {         // 读入语料库中的每一行并用最大匹配法处理

        if(strline.length() < 1) {
            break;
        }

        line = removeSeparator(strline);

        
        strline_out_1 = SegmentSentenceMM(line, 1);       // 正向最大匹配分词

        strline_out_2 = SegmentSentenceMM(line, 2);       // 逆向最大匹配分词

        // 输出结果
        count++;
        cout << "----------------------------------------------" << endl;
		cout << "句子编号：" << count << endl;
		cout << endl;

        cout << "待分词的句子长度: " << line.length() << "  句子：" << endl;
		cout << line << endl;

        cout << "标准比对结果长度: " << strline.length() << "  句子：" << endl;
		cout << strline << endl;

        cout << "正向匹配分词长度: " << strline_out_1.length() << "  句子：" << endl;
		cout << strline_out_1 << endl;
		cout << endl;

		cout << "逆向匹配分词长度: " << strline_out_2.length() << "  句子：" << endl;
		cout << strline_out_2 << endl;
		cout << endl;

        // 计算准确率、召回率
        vector<int> vec_right = getPos(strline, line);
        vector<int> vec_out_1 = getPos(strline_out_1, line);
		vector<int> vec_out_2 = getPos(strline_out_2, line);

        if (vec_right.size() > 0) {
            cout << "标准结果：" << endl;
            for(int i = 0; i < vec_right.size(); i++){
                cout << setw(4) << vec_right[i];
            }
		    cout << endl;
        } else {
            cout << "标准结果：[]" <<  endl;
        }


        if (vec_out_1.size() > 0) {
            cout << "标准结果：" << endl;
            for(int i = 0; i < vec_out_1.size(); i++){
                cout << setw(4) << vec_out_1[i];
            }
		    cout << endl;
        } else {
            cout << "标准结果：[]" <<  endl;
        }

        if (vec_out_2.size() > 0) {
            cout << "标准结果：" << endl;
            for(int i = 0; i < vec_out_2.size(); i++){
                cout << setw(4) << vec_out_2[i];
            }
		    cout << endl;
        } else {
            cout << "标准结果：[]" <<  endl;
        }

        vector<int> vec_count_1 = getCount(vec_right, vec_out_1);
		vector<int> vec_count_2 = getCount(vec_right, vec_out_2);

        // 准确的切分数量
		int count_right = vec_right.size();

		// 切分得到的数量
		int count_out_1 = vec_out_1.size();
	    int count_out_2 = vec_out_2.size();

		// 切分正确的数量
		int count_out_1_right = count_out_1 - vec_count_1[0] - vec_count_1[1];
		int count_out_2_right = count_out_2 - vec_count_2[0] - vec_count_2[1];

        cout << "正向最大匹配：" << endl;	
		cout << "  不该切分而切分的数量：" << vec_count_1[0] << endl;
		cout << "  该切分而未切分的数量：" << vec_count_1[1] << endl;

		cout << "逆向最大匹配：" << endl;	
		cout << "  不该切分而切分的数量：" << vec_count_2[0] << endl;
		cout << "  该切分而未切分的数量：" << vec_count_2[1] << endl;

        count_right_all += count_right;
		count_out_1_all += count_out_1;
		count_out_2_all += count_out_2;
		count_out_1_right_all += count_out_1_right;
		count_out_2_right_all += count_out_2_right;
    }

    double kk_1 = (double)count_out_1_right_all / count_out_1_all;	//正向准确率
	double kk_2 = (double)count_out_1_right_all / count_right_all;	//正向召回率
	double kk_3 = (double)count_out_2_right_all / count_out_2_all;	//逆向准确率
	double kk_4 = (double)count_out_2_right_all / count_right_all;	//逆向召回率
	cout << "----------------------------------" << endl;
	cout << endl;
	cout << "统计结果：" << endl;
	cout << "正向准确率：" << kk_1 * 100 << "%    正向召回率：" << kk_2 * 100 << "%" << endl;
	cout << "逆向准确率：" << kk_3 * 100 << "%    逆向召回率：" << kk_4 * 100 << "%" << endl;

    return 0;
}