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
        return pd.read_csv(filename, index_col='commit').drop(columns = 'avg', errors = 'ignore')
    else:
        cost_file = pd.DataFrame()
        cost_file.index.rename('commit', inplace=True)
        return cost_file

def write_cost(folder, benchmark, cost_file):
    cost_file.to_csv(f'{folder}/{benchmark}.csv', index_label='commit',na_rep='nan')


# parse arguments
if len(sys.argv) != 2:
    sys.exit("compute-score.py <benchmark dir>")

BENCHMARK_DIR = os.path.abspath(sys.argv[1])

# get benchmark & tests
with os.scandir(BENCHMARK_DIR) as entries:
    # [benchmark name]
    benchmarks = [f.name for f in entries if f.is_dir() and f.name != '.git']


# read cost file
print("Reading costs...")

# {benchmark name : score DataFrame}
# cost DataFrame will be converted to score DataFrame
benchmark_scores = {}
for benchmark in benchmarks:
    benchmark_scores[benchmark] = read_cost('results', benchmark)


# {benchmark name : sum benchmark score}
sum_score = {}

# compute avg & ratio
for benchmark, benchmark_score in benchmark_scores.items():
    base_score = benchmark_score.iloc[0].apply(math.log2)
    weight = base_score / base_score.max()
    print(f"{benchmark} base_score:\n{base_score}\n")
    print(f"{benchmark} weight:\n{weight}\n")

    benchmark_scores[benchmark] = benchmark_score.apply(
        lambda x:
            weight * (base_score - x.apply(math.log2)) + 1
        ,axis = 1, result_type = 'broadcast')

    benchmark_scores[benchmark].insert(0, 'sum', benchmark_scores[benchmark].sum(axis = 1, skipna = False))
    sum_score[benchmark] = benchmark_scores[benchmark]['sum']
summary_score = pd.DataFrame(sum_score).sort_index(axis = 1)
summary_score.index.rename('commit', inplace=True)
summary_score.insert(0, 'sum', summary_score.sum(axis = 1, skipna = False))


# write to cost file
print("Writing scores...")

os.makedirs('scores', exist_ok=True)
write_cost('scores', 'SUMMARY', summary_score)
for benchmark, benchmark_score in benchmark_scores.items():
    write_cost('scores', benchmark, benchmark_score)

print("Success!")
