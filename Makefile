# Nome del target finale
TARGET = test_ossched_pipe

# Directory dei sorgenti e di output
SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

# Compilatore e flag
CXX = g++
CXXFLAGS = -std=c++20 -Wall -I/home/bindi/fastflow

# Lista dei file sorgenti e degli oggetti
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

# Regola di default
all: $(BIN_DIR)/$(TARGET)

# Regola per generare il file binario finale
$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@

# Regola per compilare i file sorgenti in oggetti
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Pulizia dei file generati
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Regola per visualizzare le dipendenze
deps:
	$(CXX) -MM $(SOURCES)

.PHONY: all clean deps
