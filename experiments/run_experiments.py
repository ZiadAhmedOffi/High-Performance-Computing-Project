# @file run_experiments.py
# @author [Ahmed Osama Ibrahim]
# @brief automates running performance tests on the integrator and collects the results.

import subprocess
import csv
import os
import time
import pandas as pd
import matplotlib.pyplot as plt

def run_cmd(cmd):
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.stdout

def parse_output(output):
    lines = output.split('\n')
    res = {}
    for line in lines:
        if "Result:" in line: res['value'] = float(line.split(':')[1].strip())
        if "Avg Time:" in line: res['time'] = float(line.split(':')[1].strip().split()[0])
    return res

def main():
    binary = "./hpc_integrator"
    function = "x^2 + x*sin(x) + 2*x"
    a, b = 0, 10
    n_values = [10**6, 5*10**6, 10**7]
    methods = ["riemann"]
    impls = ["serial", "omp", "cuda"]
    omp_threads = [1, 2, 4, 6, 8, 16]
    cuda_blocks = [128, 256, 512]
    repeats = 5
    
    results = []
    
    print("Starting experiments...")
    
    for n in n_values:
        for method in methods:
            # Serial
            print(f"Running Serial: n={n}, method={method}")
            cmd = [binary, "--function", function, "--a", str(a), "--b", str(b), "--n", str(n), "--method", method, "--impl", "serial", "--repeats", str(repeats)]
            out = run_cmd(cmd)
            data = parse_output(out)
            results.append({"n": n, "method": method, "impl": "serial", "threads": 1, "block_size": 0, "time": data['time'], "value": data['value']})
            
            # OpenMP
            for t in omp_threads:
                print(f"Running OpenMP: n={n}, threads={t}")
                cmd = [binary, "--function", function, "--a", str(a), "--b", str(b), "--n", str(n), "--method", method, "--impl", "omp", "--threads", str(t), "--repeats", str(repeats)]
                out = run_cmd(cmd)
                data = parse_output(out)
                results.append({"n": n, "method": method, "impl": "omp", "threads": t, "block_size": 0, "time": data['time'], "value": data['value']})
            
            # CUDA
            for bs in cuda_blocks:
                print(f"Running CUDA (/Mock): n={n}, block_size={bs}")
                cmd = [binary, "--function", function, "--a", str(a), "--b", str(b), "--n", str(n), "--method", method, "--impl", "cuda", "--block-size", str(bs), "--repeats", str(repeats)]
                out = run_cmd(cmd)
                data = parse_output(out)
                results.append({"n": n, "method": method, "impl": "cuda", "threads": 1, "block_size": bs, "time": data['time'], "value": data['value']})

    # Save to CSV
    os.makedirs("experiments/results", exist_ok=True)
    df = pd.DataFrame(results)
    df.to_csv("experiments/results/benchmarks.csv", index=False)
    print("Results saved to experiments/results/benchmarks.csv")

    # Plotting
    plot_results(df)

def plot_results(df):
    # Time vs n for serial
    serial_df = df[df['impl'] == 'serial']
    plt.figure(figsize=(10, 6))
    plt.plot(serial_df['n'], serial_df['time'], marker='o')
    plt.title('Serial Execution Time vs Problem Size (n)')
    plt.xlabel('n')
    plt.ylabel('Time (ms)')
    plt.grid(True)
    plt.savefig('experiments/results/time_vs_n_serial.png')

    # Speedup vs threads for n=10^7
    n_large = 10**7
    omp_df = df[(df['impl'] == 'omp') & (df['n'] == n_large)]
    serial_time = df[(df['impl'] == 'serial') & (df['n'] == n_large)]['time'].values[0]
    omp_df['speedup'] = serial_time / omp_df['time']
    
    plt.figure(figsize=(10, 6))
    plt.plot(omp_df['threads'], omp_df['speedup'], marker='o')
    plt.title(f'OpenMP Speedup vs Threads (n={n_large})')
    plt.xlabel('Threads')
    plt.ylabel('Speedup')
    plt.grid(True)
    plt.savefig('experiments/results/speedup_vs_threads.png')
    
    print("Plots saved to experiments/results/")

if __name__ == "__main__":
    main()
