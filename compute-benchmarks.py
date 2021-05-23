import git
import math
import os
import pandas as pd
import subprocess
import sys

def avg(iterable):
    return sum(iterable) / len(iterable)

def strjoin(delimiter, iterable):
    return delimiter.join(map(str, iterable))

def already_logged():
    with open('results/SUMMARY.csv', 'r') as file:
        content = file.readlines()
        for line in content:
            if line.partition(",")[0] == REPO_COMMIT:
                return True
    return False

# parse arguments
if len(sys.argv) != 3:
    sys.exit("compute-benchmarks.py <sf-interpreter path> <benchmark dir>")

INTERPRETER = sys.argv[1]
BENCHMARK_DIR = sys.argv[2]

# get git commit
with git.Repo('.') as repo:
    if repo.is_dirty(untracked_files=True):
        #sys.exit("ERROR: git repo is dirty; commit or stash")
        pass
    repo_summary = repo.commit().summary.partition(']')
    REPO_COMMIT = repo_summary[2].strip() if repo_summary[1] else repo_summary[0].strip()

# {benchmark name : [input file name]}
benchmarks = {}

# get benchmark & tests
with os.scandir(BENCHMARK_DIR) as entries:
    for benchmark in sorted(f.name for f in entries if f.is_dir() and f.name != '.git'):
        benchmarks[benchmark] = sorted(f for f in os.listdir(f'{BENCHMARK_DIR}/{benchmark}/test') if f.startswith('input'))

# create initial cost file
if not os.path.exists('results'):
    os.makedirs('results')
    with open('results/SUMMARY.csv', 'w') as file:
        file.write(f"commit,avg,{strjoin(',', benchmarks)}\n")
    for benchmark, tests in benchmarks.items():
        with open(f'results/{benchmark}.csv', 'w') as file:
            file.write(f"commit,avg,{strjoin(',', tests)}\n")

# check if commit is already logged
if already_logged():
    print("WARNING: already logged this commit; removing previous log")
    with open('results/SUMMARY.csv', 'r+') as file:
        content = file.readlines()
        file.seek(0)
        file.truncate()
        for line in content:
            if line.partition(",")[0] != REPO_COMMIT:
                file.write(line)
    for benchmark, test_num in benchmarks:
        with open(f'results/{benchmark}.csv', 'r+') as file:
            content = file.readlines()
            file.seek(0)
            file.truncate()
            for line in content:
                if line.partition(",")[0] != REPO_COMMIT:
                    file.write(line)

# {benchmark name : {input file name : cost}}
test_cost = {}
# {benchmark name : average benchmark cost}
avg_cost = {}

# run benchmarks
print("Running benchmarks...")
for benchmark, tests in benchmarks.items():
    # {input file name : cost}
    benchmark_cost = {}
    subprocess.run(['bin/sf-compiler', f'{BENCHMARK_DIR}/{benchmark}/src/{benchmark}.ll', 'tmp.s'])
    for input_file in tests:
        output_file = input_file.replace('input', 'output')
        with open(f'{BENCHMARK_DIR}/{benchmark}/test/{input_file}', 'r') as input, open(f'{BENCHMARK_DIR}/{benchmark}/test/{output_file}', 'r') as output:
            proc = subprocess.run([INTERPRETER, 'tmp.s'], stdin=input, stdout=subprocess.PIPE)
            if proc.returncode != 0:
                print(f"WARNING: benchmark {benchmark} test {input_file} returned nonzero")
                benchmark_cost[input_file] = math.nan
            elif proc.stdout.decode() != output.read():
                print(f"WARNING: benchmark {benchmark} test {input_file} have different output")
                benchmark_cost[input_file] = math.nan
            else:
                with open('sf-interpreter.log') as log:
                    content = log.readlines()
                    cost = float(content[1][6:])
                    mem = float(content[2][24:])
                    benchmark_cost[input_file] = cost + mem
                    #print(f'benchmark {benchmark} test {input_file}: cost {cost + mem}')

    test_cost[benchmark] = benchmark_cost
    avg_cost[benchmark] = avg(benchmark_cost.values())

# remove temporary files
os.remove('tmp.s')
os.remove('sf-interpreter.log')
os.remove('sf-interpreter-cost.log')

# write to cost file
print("Writing costs...")
for benchmark, benchmark_cost in test_cost.items():
    with open(f'results/{benchmark}.csv', 'a') as file:
        file.write(f"{REPO_COMMIT},{avg(benchmark_cost.values())},{strjoin(',', benchmark_cost.values())}\n")

with open('results/SUMMARY.csv', 'a') as file:
    file.write(f"{REPO_COMMIT},{avg(avg_cost.values())},{strjoin(',', avg_cost.values())}\n")

print("Success!")
