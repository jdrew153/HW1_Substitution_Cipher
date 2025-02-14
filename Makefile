# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -Wall -g -Ithird-party/FTXUI/include -Iinclude

# Target executable
TARGET = main

# Source files
SRCS = main.cpp $(wildcard src/*.cpp)

# Object files
OBJS = $(SRCS:.cpp=.o)

# FTXUI Required Libraries
LDFLAGS = -Lthird-party/FTXUI/build -lftxui-dom -lftxui-screen -lftxui-component -pthread

# Default target
all: $(TARGET)

# Rule to link object files and create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)

# Run the program
run: $(TARGET)
	./$(TARGET)
