# SWPP Compiler

This compiler converts LLVM IR to SWPP assembly.

NOTE: Please don't just fork this and use it as your team repository!
It will make your team repository public (visible to other teams).
Instead, create an empty private repository, copy the contents of
this repo to yours.

Whenever there is a change in this repo, you can cherry-pick the new commits.
Relevant links:
[here](https://coderwall.com/p/sgpksw/git-cherry-pick-from-another-repository),
[here](https://stackoverflow.com/questions/5120038/is-it-possible-to-cherry-pick-a-commit-from-another-git-repository).

We have taken a powerful register allocation algorithm from the last competition winner team (special thanks to **swpp202001 team number 1**) and patched it to support vector instructions(`vload` and `vstore`). Our purpose was to give you a solid backend to focus more on the IR level optimizations.


## How to compile

To compile this project, you'll need to clone & build LLVM 12.0 first.
Please follow README.md from https://github.com/aqjune/llvmscript using
llvm-12.0.json. If you did the assignments, you will already have built this.

After LLVM 12.0 is successfully built, please run:

```
./configure.sh <LLVM bin dir (ex: ~/llvm-12.0-releaseassert/bin)>
make
```


## How to test

You should implement `test` part inside `Makefile.template` to support the underlying test command:

```
make test
```


## How to run

Compile LLVM IR `input.ll` into an assembly `a.s` using this command:

```
bin/sf-compiler input.ll a.s
```