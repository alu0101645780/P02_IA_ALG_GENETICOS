CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3

OBJS = PFSP.o Algoritmos.o main.o
TARGET = pfsp_ga

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

PFSP.o: PFSP.cpp PFSP.h
	$(CXX) $(CXXFLAGS) -c PFSP.cpp

Algoritmos.o: Algoritmos.cpp Algoritmos.h PFSP.h
	$(CXX) $(CXXFLAGS) -c Algoritmos.cpp

main.o: main.cpp PFSP.h Algoritmos.h
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f $(OBJS) $(TARGET)