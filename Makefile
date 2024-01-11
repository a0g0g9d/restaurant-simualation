CXX = g++
CXXFLAGS = -std=c++17 -pthread

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files and target executable
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
TARGET = $(BIN_DIR)/myprogram

# Include directories
INC_FLAGS = -I$(INCLUDE_DIR)

# Input file
INPUT_FILE = input.txt
OUTPUT_FILE = output.txt
# Build rules
all: directories $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INC_FLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_FLAGS) -c $< -o $@

directories:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

run: $(TARGET) $(INPUT_FILE)
	./$(TARGET) < $(INPUT_FILE) > $(OUTPUT_FILE)

.PHONY: all clean run
