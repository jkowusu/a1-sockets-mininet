# Define the executable name
EXECUTABLE = iPerfer

# Designate which compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++14 -Wall -Wextra -pedantic

# Source files
SOURCES = iPerfer.cpp

# Object files (generated from source files)
OBJECTS = $(SOURCES:.cpp=.o)

# The rule to build the executable
iPerfer: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o iPerfer $(OBJECTS)

# Rule to clean up object files and the executable
clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

# Phony targets
.PHONY: clean
