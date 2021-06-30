from numpy import *
import time, nltk
from decimal import Decimal
import jieba.posseg as pseg
import re

'''
描述: 朴素贝叶斯新闻分类
思路:
    训练数据采集
    数据读取和清洗
    数据分析和特征选择
    数据向量化
    朴素贝叶斯模型训练
    分类算法实现
优点:
    训练简单，分类器行之有效，可以处理多类别问题
缺点:
    强制的独立假设问题，新词比较敏感
关键点:
    训练数据的选择、特征的选择、模型训练方式
改进点:
    准备数据，数据样本类间均衡问题
    分析数据，结合数据分布（结合NLTK等）和特点，对文本词性进行特征选择。
    模型训练，文本分类用主题模型（LDA）来向量化文本，还可以将文本向量用PCA进行一次特征降维，然后再训练模型
'''


# 分词去停用词, 创建数据集
import jieba, itertools

'''创建数据集和类标签'''
def loadDataSet():
    docList = []; classList = []; fullText = [] # 文档列表、类别列表、文本特征
    dirList = ['C3-Art','C4-Literature','C5-Education','C6-Philosophy','C7-History']

    for j in range(5):
        for i in range(1, 11):  # 总共10个文档
            # 切分，解析数据，并归类为1类别
            wordList = textParse(open('./fudan/%s/%d.txt' % (dirList[j], i), "rb").read().decode('UTF-8', 'ignore'))
            docList.append(wordList)
            fullText.extend(wordList)
            classList.append(j)

    return docList, classList, fullText

# 利用jieba对文本进行分词，返回切词后的list
def textParse(src_doc):
    # 正则过滤掉特殊符号、标点、英文、数字
    r1 = '[a-zA-Z0-9’!"#$%&\'()*+,-./:;<=>?@，。?★、…【】《》？“”‘’！[\\]^_`{|}~]+'
    str_doc = re.sub(r1, '', src_doc)

    # 创建停用词列表
    stwlist = set([line.strip() for line in open('./stopwords.txt', 'r', encoding='utf-8').readlines()])
    sent_list = str_doc.split('\n')

    # 带词性分词并去停用词
    word_2dlist = [rm_tokens([word + "/" + flag + " " for word, flag in pseg.cut(part) if flag in ['n', 'v', 'a', 'ns', 'nr', 'nt']], stwlist) for part in sent_list]

    word_list = list(itertools.chain(*word_2dlist)) # 合并列表
    return word_list

# 去掉一些停用词、数字、特殊符号
def rm_tokens(words, stwlist):
    words_list = list(words)
    for i in range(words_list.__len__())[::-1]:
        word = words_list[i]
        if word in stwlist:     # 去除停用词
            word_list.pop(i)
        elif len(word) == 1:    # 去除单个字符
            word_list.pop(i)
        elif word == " ":       # 去除空字符
            words_list.pop(i)
    return words_list

'''-----------朴素贝叶斯文本分类----------------'''
'''获取所有文档单词的集合'''
def createVocabList(dataSet):
    vocabSet = set([])
    for document in dataSet:
        vocabSet = vocabSet | set(document)  # 操作符 | 用于求两个集合的并集
    return list(vocabSet)

'''文档词袋模型，创建矩阵数据'''
def bagOfWords2VecMN(vocabList, inputSet):
    returnVec = [0] * len(vocabList)
    for word in inputSet:
        if word in vocabList:
            returnVec[vocabList.index(word)] += 1
    return returnVec

'''朴素贝叶斯模型训练数据优化'''
def trainNB0(trainMatrix, trainCategory):
    numTrainDocs = len(trainMatrix) # 总文件数
    numWords = len(trainMatrix[0])  # 总单词数

    p1Num = p2Num = p3Num = p4Num = p5Num = ones(numWords)  # 各类为1的矩阵
    p1Denom = p2Denom = p3Denom = p4Denom = p5Denom = 2.0   # 各类特征和
    num1 = num2 = num3 = num4 = num5 = 0                    # 各类文档数目

    pNumlist = [p1Num, p2Num, p3Num, p4Num, p5Num]
    pDenomlist = [p1Denom, p2Denom, p3Denom, p4Denom, p5Denom]
    Numlist = [num1, num2, num3, num4, num5]

    for i in range(numTrainDocs):                       # 遍历每篇训练文档
        for j in range(5):                              # 遍历每个类别
            if trainCategory[i] == j:                   # 如果在类别下的文档
                pNumlist[j] += trainMatrix[i]           # 增加词条计数值
                pDenomlist[j] += sum(trainMatrix[i])    # 增加该类下所有词条计数值s
                Numlist[j] += 1                         # 该类文档数目加1

    pVect, pi = [], []
    
    for index in range(5):
        pVect.append(log(pNumlist[index] / pDenomlist[index]))
        pi.append(Numlist[index] / float(numTrainDocs))
    return pVect, pi

'''朴素贝叶斯分类函数,将乘法转换为加法'''
def classifyNB(vec2Classify, pVect, pi):
    # 计算公式  log(P(F1|C))+log(P(F2|C))+....+log(P(Fn|C))+log(P(C))
    bnpi = []   # 文档分类到各类的概率值列表
    for x in range(5):
        bnpi.append(sum(vec2Classify * pVect[x]) + log(pi[x]))
    
    # 分类集合
    resList = ['Art', 'Literature', 'Education', 'Philosophy', 'History']
    # 根据最大概率，选择索引值
    index = [bnpi.index(res) for res in bnpi if res == max(bnpi)]
    return resList[index[0]]    # 返回分类值

'''-----------特征选择改进方案与朴素贝叶斯分类应用----------------'''

'''高频词去除函数'''
def calcMostFreq(vocabList,fullText):
    import operator
    freqDict = {}
    for token in vocabList:                    # 遍历词汇表中的每个词
        freqDict[token]=fullText.count(token)  # 统计每个词在文本中出现的次数
    sortedFreq = sorted(freqDict.items(),key = operator.itemgetter(1),reverse = True)  # 根据每个词出现的次数从高到底对字典进行排序
    return sortedFreq[:30]                     # 返回出现次数最高的30个单词

'''朴素贝叶斯新闻分类应用'''
def testingNB():
    # 1. 加载数据集
    dataSet, Classlabels, fullText = loadDataSet()
    # 2. 创建单词集合
    myVocabList = createVocabList(dataSet)

    # 3. 计算单词是否出现并创建数据矩阵
    trainMat = []
    for postinDoc in dataSet:
        trainMat.append(bagOfWords2VecMN(myVocabList, postinDoc))
    
    with open('./word-bag.txt','w') as f:
        for i in trainMat:
            f.write(str(i) + '\r\n')

    # 4. 训练数据
    pVect, pi = trainNB0(array(trainMat), array(Classlabels))

    # 5. 测试数据
    testEntry = textParse(open('./fudan/test/C5-1.txt', 'rb').read().decode('UTF-8', 'ignore'))
    # testEntry = textParse('我是一篇艺术类文本，艺术是我的全部，我爱艺术人生。')
    thisDoc = array(bagOfWords2VecMN(myVocabList, testEntry))
    print(testEntry[:10], '分类结果是: ', classifyNB(thisDoc, pVect, pi))

if __name__ == "__main__":
    testingNB()
