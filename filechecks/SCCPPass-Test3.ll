; ModuleID = '/tmp/a.ll'
source_filename = "/home/psjlds/swpp202101-team4/filechecks/SccpPass-Test3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @getans3(i32 %x, i32 %y) #0 {
; CHECK: start getans3 2:
; CHECK: icmp
entry:
  %add = add nsw i32 %x, 10
  %add1 = add nsw i32 %x, 10
  %cmp = icmp eq i32 %add, %add1
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %cleanup

if.else:                                          ; preds = %entry
  br label %cleanup

cleanup:                                          ; preds = %if.else, %if.then
  %retval.0 = phi i32 [ %y, %if.then ], [ %x, %if.else ]
  ret i32 %retval.0
}
; CHECK: end getans3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK: start main 0:
entry:
  %call = call i32 @getans3(i32 10, i32 10)
  ret i32 0
}
; CHECK: end main

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
