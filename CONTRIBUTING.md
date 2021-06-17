# Contributer's Guide

프로젝트 작업을 위한 사항들을 정리한다.

## Code style

### **Golden rule: Use the style that is already being used so that the source is uniform and easy to follow.**

### Read https://llvm.org/docs/CodingStandards.html

### 공백 관련

- 인덴팅은 space 2칸을 사용한다.
- **모든 파일의 마지막은 빈 줄로 끝나야 한다.**
- trailing space는 없어야 한다.
- 포인터 변수를 여러 개 생성할 경우 변수에 붙여쓰고 `int *a, *b, *c`, 이 외에는 타입에 붙여쓴다. `int* f(int *p)`
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
- header guard를 꼭 넣는다. (#ifndef, #define, #endif)
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

- enhancement: 최적화에 대한 이슈다.
- bug: 버그에 대한 이슈다.
- draft: 아직 완성되지 않는 이슈다.
- general: 구현과 관련없는 이슈다.
