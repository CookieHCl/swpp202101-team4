import git
import pandas as pd

import math
import os
import shutil
import subprocess
import sys
import threading

def read_cost(folder, benchmark):
    filename = f'{folder}/{benchmark}.csv'
    if os.path.exists(filename):
        # remove current commit's log
        # remove avg to calculate later
        return pd.read_csv(filename, index_col='commit').drop(index = REPO_COMMIT, columns = 'avg', errors = 'ignore')
    else:
        cost_file = pd.DataFrame()
        cost_file.index.rename('commit', inplace=True)
        return cost_file

def write_cost(folder, benchmark, cost_file):
    cost_file.to_csv(f'{folder}/{benchmark}.csv', index_label='commit',na_rep='nan')

# for thread
def run_benchmark(benchmark, tests):
    cwd = f'temp/{benchmark}'
    os.mkdir(cwd)

    # {input file name : cost}
    benchmark_cost = {}
    subprocess.run([COMPILER, f'{BENCHMARK_DIR}/{benchmark}/src/{benchmark}.ll', 'tmp.s'], cwd=cwd)
    for input_file in tests:
        output_file = input_file.replace('input', 'output')
        with open(f'{BENCHMARK_DIR}/{benchmark}/test/{input_file}', 'r') as input, open(f'{BENCHMARK_DIR}/{benchmark}/test/{output_file}', 'r') as output:
            proc = subprocess.run([INTERPRETER, 'tmp.s'], stdin=input, stdout=subprocess.PIPE, cwd=cwd)
            if proc.returncode != 0:
                print(f"WARNING: benchmark {benchmark} test {input_file} returned nonzero")
                benchmark_cost[input_file] = math.nan
            elif proc.stdout.decode() != output.read():
                print(f"WARNING: benchmark {benchmark} test {input_file} have different output")
                benchmark_cost[input_file] = math.nan
            else:
                with open(os.path.join(cwd, 'sf-interpreter.log')) as log:
                    content = log.readlines()
                    cost = float(content[1][6:])
                    mem = float(content[2][24:])
                    benchmark_cost[input_file] = cost + mem
                    print(f'benchmark {benchmark} test {input_file}: cost {cost}, memcost {mem}')

    benchmark_costs[benchmark] = benchmark_costs[benchmark].append(pd.DataFrame(benchmark_cost, index = [REPO_COMMIT]))


# parse arguments
if len(sys.argv) != 3:
    sys.exit("compute-benchmarks.py <sf-interpreter path> <benchmark dir>")

COMPILER = os.path.abspath('bin/sf-compiler')
INTERPRETER = os.path.abspath(sys.argv[1])
BENCHMARK_DIR = os.path.abspath(sys.argv[2])

# get git commit
with git.Repo('.') as repo:
    repo_summary = repo.commit().summary.partition(']')
    REPO_COMMIT = repo_summary[2].strip() if repo_summary[1] else repo_summary[0].strip()

# {benchmark name : [input file name]}
benchmarks = {}

# get benchmark & tests
with os.scandir(BENCHMARK_DIR) as entries:
    for benchmark in [f.name for f in entries if f.is_dir() and f.name != '.git']:
        benchmarks[benchmark] = [f for f in os.listdir(f'{BENCHMARK_DIR}/{benchmark}/test') if f.startswith('input')]

# make dir
os.makedirs('results', exist_ok=True)
os.makedirs('ratio_results', exist_ok=True)
os.makedirs('temp', exist_ok=True)


# read cost file
print("Reading costs...")

summary_cost = read_cost('results', 'SUMMARY')
# {benchmark name : cost DataFrame}
benchmark_costs = {}
for benchmark in benchmarks:
    benchmark_costs[benchmark] = read_cost('results', benchmark)

summary_cost_ratio = read_cost('ratio_results', 'SUMMARY')
# {benchmark name : cost ratio DataFrame}
benchmark_cost_ratios = {}
for benchmark in benchmarks:
    benchmark_cost_ratios[benchmark] = read_cost('ratio_results', benchmark)


# run benchmarks
print("Running benchmarks...")

benchmark_threads = []
for benchmark in benchmarks:
    thread = threading.Thread(target = run_benchmark, args = (benchmark, benchmarks[benchmark]))
    thread.start()
    benchmark_threads.append(thread)
for thread in benchmark_threads:
    thread.join()

shutil.rmtree('temp')


# {benchmark name : average benchmark cost}
avg_cost = {}

# compute avg & ratio
for benchmark, benchmark_cost in benchmark_costs.items():
    benchmark_cost_ratios[benchmark] = benchmark_cost_ratios[benchmark].append(
        benchmark_cost.iloc[-1].div(benchmark_cost.iloc[0]).rename(REPO_COMMIT)
    )
    benchmark_cost.sort_index(axis = 1, inplace = True)
    benchmark_cost.insert(0, 'avg', benchmark_cost.mean(axis = 1, skipna = False))
    avg_cost[benchmark] = benchmark_cost.at[REPO_COMMIT, 'avg']
summary_cost = summary_cost.append(pd.DataFrame(avg_cost, index = [REPO_COMMIT])).sort_index(axis = 1)
summary_cost.insert(0, 'avg', summary_cost.mean(axis = 1, skipna = False))

# {benchmark name : average benchmark cost ratio}
avg_cost_ratio = {}

# compute ratio avg
for benchmark, benchmark_cost_ratio in benchmark_cost_ratios.items():
    benchmark_cost_ratio.sort_index(axis = 1, inplace = True)
    benchmark_cost_ratio.insert(0, 'avg', benchmark_cost_ratio.mean(axis = 1, skipna = False))
    avg_cost_ratio[benchmark] = benchmark_cost_ratio.at[REPO_COMMIT, 'avg']
summary_cost_ratio = summary_cost_ratio.append(pd.DataFrame(avg_cost_ratio, index = [REPO_COMMIT])).sort_index(axis = 1)
summary_cost_ratio.insert(0, 'avg', summary_cost_ratio.mean(axis = 1, skipna = False))


# write to cost file
print("Writing costs...")

write_cost('results', 'SUMMARY', summary_cost)
for benchmark, benchmark_cost in benchmark_costs.items():
    write_cost('results', benchmark, benchmark_cost)

write_cost('ratio_results', 'SUMMARY', summary_cost_ratio)
for benchmark, benchmark_cost_ratio in benchmark_cost_ratios.items():
    write_cost('ratio_results', benchmark, benchmark_cost_ratio)

print("Success!")
