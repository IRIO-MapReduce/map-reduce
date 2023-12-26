CXX = g++
CXXFLAGS = -std=c++20 -Wall
SRC_FILES = main.cpp mapreduce.cpp
OBJ_FILES = $(SRC_FILES:.cpp=.o)
EXECUTABLE = main

$(EXECUTABLE): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ_FILES) $(EXECUTABLE) input-intermediate.txt output.txt
