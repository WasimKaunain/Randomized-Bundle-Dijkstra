CXX := g++
CXXFLAGS := -O0 -g  -std=c++17 -march=native -Wall -Wextra
INCLUDES := -Iinclude -I$(HOME)/boost_1_84_0

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# ==========================
# Core sources (NO main)
# ==========================
CORE_SRCS := \
	$(SRC_DIR)/graph.cpp \
	$(SRC_DIR)/bundle_construct.cpp \
	$(SRC_DIR)/bundle_dijkstra.cpp \
	$(SRC_DIR)/bundle_dijkstra_fib.cpp \
	$(SRC_DIR)/dijkstra_ref.cpp \
	$(SRC_DIR)/dijkstra_fibheap.cpp

CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CORE_SRCS))

# ==========================
# Executables
# ==========================
BUNDLE := $(BIN_DIR)/bundle
GEN_ER := $(BIN_DIR)/gen_er
GEN_SPARSE := $(BIN_DIR)/gen_sparse

all: $(BUNDLE) $(GEN_ER) $(GEN_SPARSE)

# ==========================
# bundle executable
# ==========================
$(BUNDLE): $(CORE_OBJS) $(OBJ_DIR)/main.o
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# ==========================
# graph generators
# ==========================
$(GEN_ER): $(OBJ_DIR)/gen_er.o
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(GEN_SPARSE): $(OBJ_DIR)/gen_sparse.o
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# ==========================
# Object compilation rule
# ==========================
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean