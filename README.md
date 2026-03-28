# HPC Numerical Integration Project

## Overview
This project implements a numerical integrator for user-provided mathematical expressions using Riemann and Simpson sums. It provides three implementations:
1.  **Serial**: Baseline implementation for correctness and performance comparison.
2.  **OpenMP**: Parallelized implementation using OpenMP for multi-core CPUs.
3.  **CUDA**: Parallelized implementation for NVIDIA GPUs using CUDA.

The application includes an expression parser (Shunting-Yard algorithm) that converts mathematical strings into Reverse Polish Notation (RPN) for efficient evaluation on both CPU and GPU.

## Program Flow & Design Decisions

### 1. Expression Parsing (Shunting-Yard & RPN)
**Decision:** Instead of hard-coding functions or using a heavy JIT compiler, I implemented a **Shunting-Yard parser**.
**Reason:** This converts infix expressions (e.g., `x^2 + 1`) into Reverse Polish Notation (RPN) bytecode. This bytecode is a simple array of "tokens" that can be easily copied to the GPU and evaluated in a stack-based loop. This allows for dynamic user input while maintaining high performance.

### 2. CUDA Execution (Grid-Stride Loops & Shared Memory)
**Decision:** The CUDA kernels use **Grid-Stride loops**.
**Reason:** This ensures the program can handle integration intervals ($n$) much larger than the maximum number of hardware threads. It also improves data locality and occupancy.
**Decision:** I used **Shared Memory Reduction**.
**Reason:** Performing thousands of `atomicAdd` operations to global memory is slow. Reducing results within a block using shared memory before performing a single atomic add per block significantly boosts performance.

### 3. Cross-Platform Compatibility (Mock Fallback)
**Decision:** The `Makefile` and source code use conditional compilation.
**Reason:** To ensure the project is "deliverable" even on systems without NVIDIA GPUs. If `nvcc` is missing, the system transparently falls back to a mock implementation that uses the serial CPU logic, allowing the CLI and experiment scripts to still function.

### 4. Memory Efficiency
**Decision:** Use of POD (Plain Old Data) structs for tokens.
**Reason:** To ensure bitwise compatibility between the Host (CPU) and Device (GPU) memory spaces, avoiding complex serialization overhead.

## Deliverables
- `hpc_integrator`: The compiled binary.
- `src/`: Source code including Serial, OpenMP, and CUDA implementations.
- `include/`: Header files.
- `experiments/`: Automation scripts and benchmark results.
- `README.md`: This file.

## Build Instructions
### Prerequisites
- GCC (with OpenMP support)
- CUDA Toolkit (for GPU implementation)
- Python 3 (for experiment automation and plotting)

### Compilation
To compile the project, run:
```bash
make
```
If CUDA is not available, the project will compile a mock CUDA implementation using the serial logic for demonstration.

## Usage
Run the binary with the following options:
```bash
./hpc_integrator --function "x^2 + sin(x)" --a 0 --b 3.14159 --n 1000000 --method riemann --impl serial
```

### Options
- `--function <str>`: Mathematical expression (supports `+`, `-`, `*`, `/`, `^`, `sin`, `cos`, `tan`, `exp`, `log`, `x`).
- `--a <float>`: Lower integration limit.
- `--b <float>`: Upper integration limit.
- `--n <int>`: Number of intervals.
- `--method <str>`: `riemann` or `simpson`.
- `--impl <str>`: `serial`, `omp`, or `cuda`.
- `--threads <int>`: Number of threads for OpenMP (default: 1).
- `--block-size <int>`: CUDA block size (default: 256).
- `--grid-size <int>`: CUDA grid size (default: 1024).
- `--repeats <int>`: Number of repeats for averaging (default: 1).
- `--help`: Show help message.

## Reproducibility
To run all benchmarks and generate plots, use the provided script:
```bash
python3 experiments/run_experiments.py
```
This script will produce a CSV file and plots in the `experiments/results/` directory.

## System Requirements
- OS: Linux
- CPU: Multi-core with OpenMP support
- GPU: NVIDIA GPU with CUDA support (Compute Capability 7.0+)

## Authors
Ahmed Osama Ibrahim\
Ziad Ahmed Mohamed\
Mahmoud Atef Mahmoud\
Yassin Ahmed Nasr\
**Benha University - Shoubra College of Engineering**\
HPC Project Assignment
