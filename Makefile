# Compiler settings
CXX = g++
NVCC = $(shell which nvcc)
CXXFLAGS = -O3 -Wall -Wextra -fopenmp -Iinclude
LDFLAGS = -lm

# Detection logic
ifeq ($(NVCC),)
    $(warning "nvcc not found, using mock CUDA implementation")
    CUDA_SRCS = src/integrator_mock.cpp
    OBJS = src/main.o src/parser.o src/utils.o src/integrator_mock.o
else
    $(info "nvcc found, compiling with true CUDA support")
    NVCC_BIN = $(NVCC)
    NVCCFLAGS = -O3 -Iinclude -arch=sm_70
    CUDA_SRCS = src/integrator.cu
    OBJS = src/main.o src/parser.o src/utils.o src/integrator.o
    # Link with CUDA libraries
    LDFLAGS += -L/usr/local/cuda/lib64 -lcudart
endif

TARGET = hpc_integrator
TEST_TARGET = run_tests

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Special rule for CUDA object
src/integrator.o: src/integrator.cu
	$(NVCC_BIN) $(NVCCFLAGS) -Xcompiler "-fopenmp $(CXXFLAGS)" -c $< -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): tests/run_tests.o src/parser.o src/integrator_mock.o src/utils.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

tests/run_tests.o: tests/run_tests.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o tests/*.o $(TARGET) $(TEST_TARGET)
