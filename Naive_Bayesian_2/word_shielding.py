from numpy import *
import feedparser

'''
贝叶斯公式
p(xy)=p(x|y)p(y)=p(y|x)p(x)
p(x|y)=p(y|x)p(x)/p(y)
'''

'''-----------项目案例1: 屏蔽社区留言板的侮辱性言论----------------'''
'''创建数据集：单词列表postingList, 所属类别classVec'''
def loadDataSet():
    postingList = [
        ['my', 'dog', 'has', 'flea', 'porblems', 'help', 'please'],
        ['maybe', 'not', 'take', 'him', 'to', 'dog', 'park', 'stupid'],
        ['my', 'dalmation', 'is', 'so', 'cute', 'I', 'love', 'him'],
        ['stop', 'posting', 'stupid', 'worthless', 'garbage'],
        ['mr', 'licks', 'ate', 'my', 'steak', 'how', 'to', 'stop', 'him'],
        ['quit', 'buying', 'worthless', 'dog', 'food', 'stupid']
    ]
    classVec = [0, 1, 0, 1, 0, 1]   # 1代表侮辱性文字，0代表非侮辱文字
    return postingList, classVec

'''获取所有单词的集合:返回不含重复元素的单词列表'''
def createVocabList(dataSet):
    vocabSet = set([])
    for document in dataSet:
        vocabSet = vocabSet | set(document)     # 操作符 | 用于求两个集合的并集
    return list(vocabSet)

'''词集模型构建数据矩阵'''
def setOfWords2Vec(vocabList, inputSet):
    # 创建一个和词汇表等长的向量，并将其元素都设置为0
    returnVec = [0] * len(vocabList)
    # 遍历文档中的所有单词，如果出现了词汇表中的单词，则将输出的文档向量中的对应值设为1
    for word in inputSet:
        if word in vocabList:
            returnVec[vocabList.index(word)] = 1
        else:
            print("单词: %s 不在词汇表之中!" % word)
    return returnVec

'''训练数据优化版本'''
def trainNB0(trainMatrix, trainCategory):

    numTrainDocs = len(trainMatrix) # 总文件数
    numWords = len(trainMatrix[0])  # 总单词数

    pAbusive = sum(trainCategory) / float(numTrainDocs) # 侮辱性文件的出现概率

    # 构造单词出现次数列表,p0Num 正常的统计,p1Num 侮辱的统计
    # 避免单词列表中的任何一个单词为0，而导致最后的乘积为0，所以将每个单词的出现次数初始化为 1
    p0Num = ones(numWords)  # [0,0......]->[1,1,1,1,1.....],ones初始化1的矩阵
    p1Num = ones(numWords)

    # 整个数据集单词出现总数，2.0根据样本实际调查结果调整分母的值（2主要是避免分母为0，当然值可以调整）
    # p0Denom 正常的统计
    # p1Denom 侮辱的统计
    p0Denom = 2.0
    p1Denom = 2.0

    for i in range(numTrainDocs):
        if trainCategory[i] == 1:
            p1Num += trainMatrix[i]         # 累加辱骂词的频率
            p1Denom += sum(trainMatrix[i])  # 对每篇文章的辱骂的频次，进行统计汇总
        else:
            p0Num += trainMatrix[i]
            p0Denom += sum(trainMatrix[i])
    
    '''
    类别1，即侮辱性文档的[log(P(F1|C1)),log(P(F2|C1)),log(P(F3|C1)),log(P(F4|C1)),log(P(F5|C1))....]列表
    取对数避免下溢出或浮点舍入出错
    '''
    p1Vect = log(p1Num / p1Denom)

    '''
    类别0，即正常文档的[log(P(F1|C0)),log(P(F2|C0)),log(P(F3|C0)),log(P(F4|C0)),log(P(F5|C0))....]列表
    '''
    p0Vect = log(p0Num / p0Denom)
    return p0Vect, p1Vect, pAbusive


'''
朴素贝叶斯分类函数

将乘法转换为加法
乘法：P(C|F1F2...Fn) = P(F1F2...Fn|C)P(C)/P(F1F2...Fn)
加法：P(F1|C)*P(F2|C)....P(Fn|C)P(C) -> log(P(F1|C))+log(P(F2|C))+....+log(P(Fn|C))+log(P(C))

vec2Classify: 
    待测数据[0,1,1,1,1...]，即要分类的向量
p0Vec: 
    类别0，即正常文档的[log(P(F1|C0)),log(P(F2|C0)),log(P(F3|C0)),log(P(
    F4|C0)),log(P(F5|C0))....]列表

p1Vec: 
    类别1，即侮辱性文档的[log(P(F1|C1)),log(P(F2|C1)),log(P(F3|C1)),log(P(
    F4|C1)),log(P(F5|C1))....]列表

pClass1: 
    类别1，侮辱性文件的出现概率

返回：类别1 or 0
'''
def classifyNB(vec2Classify, p0Vec, p1Vec, pClass1):
    # 计算公式 log(P(F1|C))+log(P(F2|C))+....+log(P(Fn|C))+log(P(C))
    # 使用 NumPy 数组来计算两个向量相乘的结果，这里的相乘是指对应元素相乘，即先将两个向量中的第一个元素相乘，然后将第2个元素相乘，以此类推。这里的 vec2Classify * p1Vec 的意思就是将每个词与其
    p1 = sum(vec2Classify * p1Vec) + log(pClass1)
    p0 = sum(vec2Classify * p0Vec) + log(1.0 - pClass1)
    if p1 > p0:
        return 1
    else:
        return 0

'''朴素贝叶斯算法屏蔽社区留言板的侮辱性言论的应用'''
def testingNB():
    # 1. 加载数据集
    dataSet, Classlabels = loadDataSet()

    # 2. 创建单词集合
    myVocabList = createVocabList(dataSet)

    # 3. 计算单词是否出现并创建数据矩阵
    trainMat = []
    for postinDoc in dataSet:
        # 返回m*len(myVocabList)的矩阵， 记录的都是0，1信息
        trainMat.append(setOfWords2Vec(myVocabList, postinDoc))
    
    # 4. 训练数据
    p0V, p1V, pAb = trainNB0(array(trainMat), array(Classlabels))

    # # 5. 测试数据
    testEntry = ['love', 'my', 'dalmation']
    thisDoc = array(setOfWords2Vec(myVocabList, testEntry))
    print(testEntry, '分类结果是: ', classifyNB(thisDoc, p0V, p1V, pAb))

    testEntry = ['stupid', 'garbage']
    thisDoc = array(setOfWords2Vec(myVocabList, testEntry))
    print(testEntry, '分类结果是: ', classifyNB(thisDoc, p0V, p1V, pAb))

if __name__ == "__main__":
    testingNB()