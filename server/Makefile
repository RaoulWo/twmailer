# Compiler
CXX := g++
CXXFLAGS := -Wall -Wextra -pthread -std=c++17 -g
LIBS := -lldap -llber

# Directories
SRC_DIR := src
BUILD_DIR := bin

# Files
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
EXECUTABLE := $(BUILD_DIR)/twmailer-server

all:	build $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)	
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(OBJECTS): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp	
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(EXECUTABLE) $(OBJECTS) $(BUILD_DIR)