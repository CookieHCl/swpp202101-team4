; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @min(i64 %0, i64 %1) #0 {
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  store i64 %0, i64* %3, align 8
  store i64 %1, i64* %4, align 8
  %5 = load i64, i64* %3, align 8
  %6 = load i64, i64* %4, align 8
  %7 = icmp sgt i64 %5, %6
  br i1 %7, label %8, label %10

8:                                                ; preds = %2
  %9 = load i64, i64* %4, align 8
  br label %12

10:                                               ; preds = %2
  %11 = load i64, i64* %3, align 8
  br label %12

12:                                               ; preds = %10, %8
  %13 = phi i64 [ %9, %8 ], [ %11, %10 ]
  ret i64 %13
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @g() #0 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i64, align 8
  %14 = alloca i64, align 8
  %15 = alloca i64, align 8
  %16 = alloca i64, align 8
  %17 = alloca i64, align 8
  %18 = alloca i64, align 8
  %19 = alloca i64, align 8
  %20 = alloca i64, align 8
  %21 = alloca i64, align 8
  %22 = alloca i64, align 8
  %23 = alloca i64, align 8
  %24 = alloca i64, align 8
  %25 = alloca i64, align 8
  %26 = alloca i64, align 8
  %27 = alloca i64, align 8
  %28 = alloca i64, align 8
  %29 = alloca i64, align 8
  %30 = alloca i64, align 8
  %31 = alloca i64, align 8
  %32 = alloca i64, align 8
  %33 = alloca i64, align 8
  %34 = alloca i64, align 8
  %35 = alloca i64, align 8
  %36 = alloca i64, align 8
  %37 = alloca i64, align 8
  %38 = alloca i64, align 8
  %39 = alloca i64, align 8
  %40 = alloca i64, align 8
  %41 = load i64, i64* %1, align 8
  %42 = load i64, i64* %2, align 8
  %43 = call i64 @min(i64 %41, i64 %42)
  %44 = load i64, i64* %3, align 8
  %45 = load i64, i64* %4, align 8
  %46 = call i64 @min(i64 %44, i64 %45)
  %47 = load i64, i64* %5, align 8
  %48 = load i64, i64* %6, align 8
  %49 = call i64 @min(i64 %47, i64 %48)
  %50 = load i64, i64* %7, align 8
  %51 = load i64, i64* %8, align 8
  %52 = call i64 @min(i64 %50, i64 %51)
  %53 = load i64, i64* %9, align 8
  %54 = load i64, i64* %10, align 8
  %55 = call i64 @min(i64 %53, i64 %54)
  %56 = load i64, i64* %11, align 8
  %57 = load i64, i64* %12, align 8
  %58 = call i64 @min(i64 %56, i64 %57)
  %59 = load i64, i64* %13, align 8
  %60 = load i64, i64* %14, align 8
  %61 = call i64 @min(i64 %59, i64 %60)
  %62 = load i64, i64* %15, align 8
  %63 = load i64, i64* %16, align 8
  %64 = call i64 @min(i64 %62, i64 %63)
  %65 = load i64, i64* %17, align 8
  %66 = load i64, i64* %18, align 8
  %67 = call i64 @min(i64 %65, i64 %66)
  %68 = load i64, i64* %19, align 8
  %69 = load i64, i64* %20, align 8
  %70 = call i64 @min(i64 %68, i64 %69)
  %71 = load i64, i64* %21, align 8
  %72 = load i64, i64* %22, align 8
  %73 = call i64 @min(i64 %71, i64 %72)
  %74 = load i64, i64* %23, align 8
  %75 = load i64, i64* %24, align 8
  %76 = call i64 @min(i64 %74, i64 %75)
  %77 = load i64, i64* %25, align 8
  %78 = load i64, i64* %26, align 8
  %79 = call i64 @min(i64 %77, i64 %78)
  %80 = load i64, i64* %27, align 8
  %81 = load i64, i64* %28, align 8
  %82 = call i64 @min(i64 %80, i64 %81)
  %83 = load i64, i64* %29, align 8
  %84 = load i64, i64* %30, align 8
  %85 = call i64 @min(i64 %83, i64 %84)
  %86 = load i64, i64* %31, align 8
  %87 = load i64, i64* %32, align 8
  %88 = call i64 @min(i64 %86, i64 %87)
  %89 = load i64, i64* %33, align 8
  %90 = load i64, i64* %34, align 8
  %91 = call i64 @min(i64 %89, i64 %90)
  %92 = load i64, i64* %35, align 8
  %93 = load i64, i64* %36, align 8
  %94 = call i64 @min(i64 %92, i64 %93)
  %95 = load i64, i64* %37, align 8
  %96 = load i64, i64* %38, align 8
  %97 = call i64 @min(i64 %95, i64 %96)
  %98 = load i64, i64* %39, align 8
  %99 = load i64, i64* %40, align 8
  %100 = call i64 @min(i64 %98, i64 %99)
  ret i64 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1 "}
