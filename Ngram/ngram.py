import urllib
import re
import random
import string
import operator

'''
实现了 NGram 算法，并对 markov 生产的句子进行打分
'''
class ScoreInfo:
    sorce = 0
    content = ''

class NGram:
    __dicWordFrequency = dict()         # 词频
    __dicPhraseFrequency = dict()       # 词段频
    __dicPhraseProbability = dict()     # 词段概率

    def printNGram(self):
        print('词频')
        for key in self.__dicWordFrequency.key():
            print('%s\t%s' % (key, self.__dicWordFrequency[key]))
        
        print('词段频')
        for key in self.__dicPhraseFrequency.keys():
            print('%s\t%s' % (key, self.__dicPhraseFrequency[key]))

        print('词段概率')
        for key in self.__dicPhraseProbability.keys():
            print('%s\t%s' % (key, self.__dicPhraseProbability[key]))

    def append(self, content):
        '''
        训练 ngram 模型
        :param content: 训练内容
        :return:
        '''

        # clear
        content = re.sub('\s|\n|\t', '', content)
        ie = self.getIterator(content)  # 2-Gram 模型
        keys = []

        for w in ie:
            # 词频
            k1 = w[0]
            k2 = w[1]

            if k1 not in self.__dicWordFrequency.keys():
                self.__dicWordFrequency[k1] = 0
            
            if k2 not in self.__dicWordFrequency.keys():
                self.__dicWordFrequency[k2] = 0
            
            self.__dicWordFrequency[k1] += 1
            self.__dicWordFrequency[k2] += 1

            # 词段频
            key = '%s%s' % (w[0], w[1])
            keys.append(key)

            if key not in self.__dicPhraseFrequency.keys():
                self.__dicPhraseFrequency[key] = 0
            
            self.__dicPhraseFrequency[key] += 1
        
        # 词段概率
        for w1w2 in keys:
            w1 = w1w2[0]
            w1Freq = self.__dicWordFrequency[w1]
            w1w2Freq = self.__dicPhraseFrequency[w1w2]
            # P(w1w2|w1) = w1w2出现的总次数/w1出现的总次数 = 827/2533 ≈0.33 , 即 w2 在 w1 后面的概率
            self.__dicPhraseProbability[w1w2] = round(w1w2Freq / w1Freq, 2)
    
    def getIterator(self, txt):
        '''
        bigram 模型迭代器
        :param txt: 一段话或一个句子
        :return: 返回迭代器, item为tuple, 每项2个值
        '''
        ct = len(txt)
        if ct < 2:
            return txt
        
        for i in range(ct - 1):
            w1 = txt[i]
            w2 = txt[i + 1]
            yield (w1, w2)
        
    def getScore(self, txt):
        '''
        使用ugram 模型计算str得分
        :param txt:
        :return
        '''
        ie = self.getIterator(txt)
        score = 1
        fs = []

        for w in ie:
            key = '%s%s' % (w[0], w[1])
            freq = self.__dicPhraseProbability[key]
            fs.append(freq)
            score = freq * score
        
        info = ScoreInfo()
        info.score = score
        info.content = txt
        return info
    
    def sort(self, infos):
        '''
        对结果排序
        :param infos:
        :return:
        '''
        return sorted(infos, key = lambda x: x.score, reverse = True)

def fileReader():
    path = "./data/test_ngram_data.txt"
    with (open(path, 'r', encoding='utf-8')) as f:
        rows = 0
        # 按行统计
        while True:
            rows += 1
            line = f.readline()
            if not line:
                print('读取结束 %s' % path)
                return
            
            print('content rows=%s len=%s type=%s' % (rows, len(line), type(line)))
            yield line
    
    pass
    
def getData():
    # 使用相同语料随机生成句子
    arr = []
    arr.append("对有些人来说，困难是成长壮大的机遇。")
    arr.append("对有些人来说，困难是放弃的借口，而对另外一部分人来说，困难是放弃")
    arr.append("世上有很多时候，限制我们自己配不上优秀的人生色彩。")
    arr.append("睡一睡，精神好，烦恼消，快乐长;睡一睡，精神好，做美梦，甜蜜蜜;")
    arr.append("睡一睡，精神好，烦恼消，快乐长;睡一睡，身体健，头脑清，眼睛明。")
    arr.append("思念;展转反侧，是因为它的尽头，种着“梦想”。")
    arr.append("思念不因休息而变懒，祝福不因疲惫而变懒，祝福不因休息而变懒，祝福")
    arr.append("思念无声无息，弥漫你的心里。")
    arr.append("希望每天醒来，都是不是他人的翅膀，心有多大。很多不可能会造就你明")
    arr.append("一条路，人烟稀少，孤独难行。却不得不坚持前行。因为有人在想念;展")
    arr.append("找不到坚持前行。却不得不坚持下去的理由，生活本来就这么简单。")
    arr.append("找不到坚持下去的理由，生活本来就这么简单。")

    return arr

def main():
    ng = NGram()
    reader = fileReader()

    # 将语料追加到bigram模型中
    for row in reader:
        print(row)
        ng.append(row)
    
    # 测试生成的句子，是否合理
    arr = getData()
    infos = []

    for s in arr:
        # 对生成的句子打分
        info = ng.getScore(s)
        infos.append(info)
    
    # 排序
    infoArr = ng.sort(infos)
    for info in infoArr:
        print('%s\t(得分：%s)'%(info.content,info.score))

    pass

if __name__ == '__main__':
    main()
    pass