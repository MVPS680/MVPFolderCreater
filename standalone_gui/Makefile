# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# FLTK configuration
FLTK_CONFIG = fltk-config
FLTK_CXXFLAGS = $(shell $(FLTK_CONFIG) --cxxflags)
FLTK_LIBS = $(shell $(FLTK_CONFIG) --ldflags)

# Target executable
TARGET = FolderCreator

# Source files
SRCS = main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(FLTK_LIBS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(FLTK_CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)

# Install dependencies (Ubuntu/Debian)
install-deps-ubuntu:
	sudo apt-get update
	sudo apt-get install libfltk1.3-dev

# Install dependencies (CentOS/RHEL/Fedora)
install-deps-fedora:
	sudo dnf install fltk-devel

# Install dependencies (macOS with Homebrew)
install-deps-macos:
	brew install fltk

.PHONY: all clean install-deps-ubuntu install-deps-fedora install-deps-macos