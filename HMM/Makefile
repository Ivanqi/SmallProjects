CXX := g++
CFLAGS := -std=c++11 -g -pg
INCLUDE := -I ./


TARGET1 := prehmm
TARGET2 := matrix
# TARGET3 := download3


$(TARGET1): prehmm.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^

$(TARGET2): matrix.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^

# $(TARGET3): $(SRC) download3.cpp
# 	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

# all:
# 	${CC} naive_bayes.cpp main.cpp -o ${TARGET}


clean:
	find . -name '*.o' | xargs rm -f
	ind . -name '*.out' | xargs rm -f
	find . -name $(TARGET1) | xargs rm -f
	find . -name $(TARGET2) | xargs rm -f