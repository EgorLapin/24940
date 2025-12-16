CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11
TARGET = Lab1
SRC = Lab1.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean