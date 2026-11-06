# ============================================================
#  Makefile: onn (neural network from scratch)
#
#  make                 build the debug library (default)
#  make debug           -O0 -g         -> build/debug/libonn.a
#  make release         -O2 -DNDEBUG   -> build/release/libonn.a
#  make test T=tests/test_matrix.cpp   build and run one test file
#  make tests           build and run every tests/test_*.cpp
#  make clean | re | help
#
#  Add SAN=1 to test / tests to enable the address + undefined-behaviour
#  sanitizers (great for catching out-of-bounds bugs in matrix code).
# ============================================================

CXX      := g++
STD      := -std=c++17
WARN     := -Wall -Wextra

SRC_DIR  := srcs
INC_DIR  := includes
TEST_DIR := tests
BUILD    := build

# Library sources. Swap for an explicit list if you prefer:
#   SRCS := srcs/matrix.cpp srcs/matrix_ops.cpp
# A future srcs/main.cpp is excluded so it can't clash with the tests' own main().
SRCS := $(filter-out $(SRC_DIR)/main.cpp,$(wildcard $(SRC_DIR)/*.cpp))

# -MMD -MP make the compiler write .d files so that editing a header
# rebuilds every .cpp that includes it.
COMMON        := $(STD) $(WARN) -I$(INC_DIR) -MMD -MP
DEBUG_FLAGS   := $(COMMON) -g -O0
RELEASE_FLAGS := $(COMMON) -O2 -DNDEBUG

ifeq ($(SAN),1)
SAN_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
endif

DEBUG_OBJS   := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD)/debug/%.o)
RELEASE_OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD)/release/%.o)
DEBUG_LIB    := $(BUILD)/debug/libonn.a
RELEASE_LIB  := $(BUILD)/release/libonn.a

TEST_BIN_DIR := $(BUILD)/tests
TEST_BIN      = $(TEST_BIN_DIR)/$(basename $(notdir $(T)))

.PHONY: all debug release test tests clean re help

all: debug

debug: $(DEBUG_LIB)

release: $(RELEASE_LIB)

$(DEBUG_LIB): $(DEBUG_OBJS)
	ar rcs $@ $^

$(RELEASE_LIB): $(RELEASE_OBJS)
	ar rcs $@ $^

$(BUILD)/debug/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(DEBUG_FLAGS) -c $< -o $@

$(BUILD)/release/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(RELEASE_FLAGS) -c $< -o $@

# Tests are compiled straight from the sources (no .o reuse) so that the
# sanitizer flags are applied consistently to every file.
test:
ifndef T
	$(error usage: make test T=tests/test_matrix.cpp [SAN=1])
endif
	@mkdir -p $(TEST_BIN_DIR)
	$(CXX) $(STD) $(WARN) -g -O0 -I$(INC_DIR) $(SAN_FLAGS) $(T) $(SRCS) -o $(TEST_BIN)
	./$(TEST_BIN)

tests:
	@for t in $(wildcard $(TEST_DIR)/test_*.cpp); do \
		echo "=== $$t ==="; \
		$(MAKE) --no-print-directory test T=$$t SAN=$(SAN) || exit 1; \
	done

clean:
	rm -rf $(BUILD)

re: clean debug

help:
	@echo "make [debug]                      debug library (-O0 -g)"
	@echo "make release                      optimised library (-O2)"
	@echo "make test T=tests/test_x.cpp      build and run one test file"
	@echo "make tests                        build and run all tests/test_*.cpp"
	@echo "SAN=1                             add sanitizers to test / tests"
	@echo "make clean | make re              remove build/ | clean + rebuild"

-include $(DEBUG_OBJS:.o=.d) $(RELEASE_OBJS:.o=.d)
