# @file run_experiments.sh
# @author Ahmed Osama Ibrahim
# @brief builds the project and runs all experiments automatically, making it easy to get consistent results.

#!/bin/bash

# HPC Project: Numerical Integration
# Reproducibility Script

echo "--- Building project ---"
make clean
make

echo "--- Running experiments and plotting ---"
# Check if python3 and required libraries are installed
if command -v python3 &> /dev/null
then
    python3 experiments/run_experiments.py
else
    echo "Error: python3 is not installed. Please install python3 to run the experiments."
    exit 1
fi

echo "--- Done ---"
echo "Results are in experiments/results/"
