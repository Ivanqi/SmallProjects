CXX := g++
CFLAGS := -std=c++11 -g -pg
INCLUDE := -I ./


TARGET1 := prehmm
# TARGET2 := download2
# TARGET3 := download3


$(TARGET1): prehmm.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^

# $(TARGET2): $(SRC) download2.cpp
# 	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

# $(TARGET3): $(SRC) download3.cpp
# 	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

# all:
# 	${CC} naive_bayes.cpp main.cpp -o ${TARGET}


clean:
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET1) | xargs rm -f