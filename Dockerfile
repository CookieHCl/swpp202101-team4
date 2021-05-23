# build interpreter
FROM sunghwanlee/swpp202101-ci AS interpreter
RUN apt-get update && apt-get install -y \
    cmake \
    subversion \
 && rm -rf /var/lib/apt/lists/*
# clone w/o version control files
# https://stackoverflow.com/questions/9609835/git-export-from-github-remote-repository
RUN svn export https://github.com/snu-sf-class/swpp202101-interpreter/trunk interpreter \
 && cd interpreter \
 && ./build.sh

# get benchmarks
FROM sunghwanlee/swpp202101-ci AS benchmarks
RUN apt-get update && apt-get install -y \
    subversion \
 && rm -rf /var/lib/apt/lists/*
# clone w/o version control files
# https://stackoverflow.com/questions/9609835/git-export-from-github-remote-repository
RUN svn export https://github.com/snu-sf-class/swpp202101-benchmarks/trunk benchmarks \
    # removes files that are not directories
    # https://stackoverflow.com/questions/7714900/remove-only-files-in-directory-on-linux-not-directories
 && find benchmarks -maxdepth 1 -type f -delete

# actual image
FROM sunghwanlee/swpp202101-ci
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
 && rm -rf /var/lib/apt/lists/*
RUN pip install --no-cache-dir GitPython pandas
COPY --from=interpreter /interpreter/sf-interpreter /sf-interpreter
COPY --from=benchmarks /benchmarks /benchmarks
ENV LLVM_DIR="/llvmscript/llvm-12.0-releaseassert/bin" \
    INTERPRETER="/sf-interpreter" \
    BENCHMARK_DIR="/benchmarks"
CMD ["/bin/bash"]
