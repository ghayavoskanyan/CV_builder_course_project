CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Ibackend/include
SRC_DIR := backend/src
OBJ_DIR := obj
BIN := cv_builder

MAIN_SRC := backend/main.cpp
SRC_SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
SOURCES := $(MAIN_SRC) $(SRC_SOURCES)

MAIN_OBJ := $(OBJ_DIR)/main.o
SRC_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/src/%.o,$(SRC_SOURCES))
OBJECTS := $(MAIN_OBJ) $(SRC_OBJS)

# Link Winsock on Windows (MinGW)
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32
endif

.PHONY: all clean run server cli

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(MAIN_OBJ): backend/main.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/src/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)/src
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/src:
	mkdir -p $(OBJ_DIR)/src

run: all
	./$(BIN)

server: all
	./$(BIN) --server

cli: all
	./$(BIN) --cli

clean:
	rm -rf $(OBJ_DIR) $(BIN) *.html *.pdf __cv_export_temp.pdf __cv_export_temp.html

# Windows (MinGW):
#   mingw32-make clean
#   mingw32-make
#   cv_builder.exe --server
#
# Frontend (open separately in browser):
#   Open frontend/index.html or serve the frontend/ folder with any static file server.
