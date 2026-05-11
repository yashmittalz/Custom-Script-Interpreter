CXX = g++
CXXFLAGS = -Wall -std=c++11

SRCS = $(filter-out src/demo.cc, $(wildcard src/*.cc))
OBJS = $(SRCS:.cc=.o)
TARGET = a.out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

src/%.o: src/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean
