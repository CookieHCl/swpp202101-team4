# Contributer's Guide

프로젝트 작업을 위한 사항들을 정리한다.

## 프로젝트 구조

### main.cpp

- `enum Opts`에 자신이 구현한 Pass 이름을 알파벳 순으로 추가해야 한다.  
  여기서 추가한 enumerator가 `--passes` 컴파일러 옵션에서 사용하는 Pass 이름이 된다.
- `cl::bits<Opts, unsigned> optimizations`에서 `cl::values`에 방금 추가한
  enumerator와 이에 대한 설명을 추가해야 한다.  
  **여기서 enumerator를 지정할 때 `Opts::`를 사용해서는 안 된다.**
- `IFSET` 매크로를 사용하고 Pass를 적절한 곳에 추가해야 한다.  
  단, Pass 구현에 여러 statement가 필요한 경우에는 매크로를 풀어서 사용해도 된다.
- (선택사항) 생성자로 `optPrintProgress`를 받고 `optPrintProgress == true`인 경우
  자세한 과정을 `outs()`로 출력하면 `-v` 컴파일러 옵션을 지원할 수 있다.

### Team4Header.h

- 모든 Pass는 해당하는 `.cpp` 파일과 `.h` 파일이 있어야 한다.
- 모든 `.h`는 Header Guard를 꼭 넣어야 한다. (#ifndef, #define, #endif)  
  이때 Header Guard 이름은 파일명을 대문자로 바꾼 것을 사용한다.
- `Team4Header.h`는 `.h`만 알파벳 순으로 추가해야 한다.

### 컴파일러 옵션

모든 컴파일러 옵션들은 `-`와 `--`를 구분하지 않는다.

- `-h` 또는 `--help`를 사용할 경우 컴파일러 옵션들을 볼 수 있다.
- `-v` 또는 `--verbose`를 사용할 경우 자세한 과정을 출력한다.
- `--emit-llvm`을 사용할 경우 backend를 생략하고 구현한 Pass만 사용한 LLVM IR를 출력한다.
- `--passes=<value>`를 사용할 경우 지정한 Pass들만 사용할 수 있다.  
  이때 `<value>`는 Pass 이름들을 쉼표로 구분한 형식이다.  
  Pass 이름은 `enum Opts`에서 지정한 이름들이고, `-h`로 볼 수 있다.
- 이 외에는 두 개의 옵션 `<input file>`과 `<output file>`을 받는다.  
  이때 `<output file>`을 생략하거나 `-`로 지정하면 stdout으로 출력한다.

### FileCheck

- FileCheck는 [benchmark의](https://github.com/snu-sf-class/swpp202101-benchmarks)
  `c-to-ll.sh`를 이용해 `.c`를 변환한 `.ll`을 사용해야 한다.
- FileCheck의 이름이 `(Pass 이름)Pass-Test(test 번호).ll` 꼴이라면 지정한 Pass만 사용한다.  
  이때 Pass 이름은 `enum Opts`에서 지정한 이름들이고, `-h`로 볼 수 있다.
- FileCheck의 이름이 위의 꼴이 아니라면 모든 Pass를 사용한다.

## Code style

### **Golden rule: Use the style that is already being used so that the source is uniform and easy to follow.**

### Read https://llvm.org/docs/CodingStandards.html

### 공백 관련

- 인덴팅은 space 2칸을 사용한다.
- **모든 파일의 마지막은 빈 줄로 끝나야 한다.**
- trailing space는 없어야 한다.
- 포인터 변수를 여러 개 생성할 경우 변수에 붙여쓰고 `int *a, *b, *c`,
  이 외에는 타입에 붙여쓴다. `int* f(int *p)`
- 함수의 경우 괄호를 붙여쓰고 `f()` 이 외에는 괄호를 띄어쓴다. `while ()`
- `p->n`은 띄어쓰지 않는다.

### 중괄호 관련

- 인덴팅은 되도록이면 줄인다.
  - https://llvm.org/docs/CodingStandards.html#use-early-exits-and-continue-to-simplify-code
  - https://llvm.org/docs/CodingStandards.html#don-t-use-else-after-a-return
  - https://llvm.org/docs/CodingStandards.html#turn-predicate-loops-into-predicate-functions
- 명백한 경우 if/else나 for에서 중괄호를 생략할 수 있지만, 인덴팅은 해야 한다.
  자세한 정보는 https://llvm.org/docs/CodingStandards.html#don-t-use-braces-on-simple-single-statement-bodies-of-if-else-loop-statements 참고

### C++ 관련

- 가능하다면 `i++` 대신 `++i`를 사용한다.
- `const`를 최대한 사용한다. 최적화해주는 역할도 있지만, 변하지 않아야 할 값이 변하는지 확인할 수 있다.
- 정말 `if (x = 3)`같은 코드를 쓰고 싶다면 warning을 없애기 위해 `if ((x = 3))`처럼 쓴다.
- 레퍼런스인 경우 `auto &`, 포인터인 경우 `auto *`를 사용한다.

## git

### commit 메시지

https://blog.ull.im/engineering/2019/03/10/logs-on-git.html

commit을 길게 설명할 필요가 있는 경우 첫 줄에 제목을 적고, 두 번째 줄은 빈 줄,
세 번째 줄부터 본문을 적는다.

### 팁

되도록이면 cherry-pick은 쓰지 않는다.
cherry-pick을 쓰면 나중에 merge할 때 같은 change를 한 commit이 여러 개 생기게 된다.

cherry-pick 안 쓰고 다른 repository의 commit으로 업데이트 하는 법

1. `git remote -vv`로 내 repository의 remote 이름을 확인한다. (기본값 origin)
1. `git remote add base {다른 repository 주소}`로 다른 repository를 base로 추가한다.
1. (선택사항) `git remote set-url --push base NO_PUSH`로 base에 push하는 것을 막는다.
1. `git pull base main` 등으로 필요한 commit을 직접 받아온다.

## 리포지토리

### Project

issue와 PR는 해당하는 Project에 할당해야 한다.  
Sprint 1, Sprint 2, Sprint 3이 있다.

- To do: 새로운 이슈는 여기에 들어간다.
- In progress: 새로운 PR는 여기에 들어간다.
- Review in progress: 리뷰가 진행중인 PR는 여기에 들어간다.
- Reviewer approved: 리뷰가 완료된 PR는 여기에 들어간다.
- Done: 끝난 이슈와 PR는 여기에 들어간다.

### Label

레이블은 이슈에서 사용한다.

- optimization: 최적화에 대한 이슈다.
- bug: 버그에 대한 이슈다.
- draft: 아직 완성되지 않는 이슈다.
- general: 버그나 최적화는 아닌 이슈다. 주로 NFC가 해당된다.
- notice: 공지를 위한 이슈로, 모두가 알아야 할 정보가 있을 때 사용한다.
