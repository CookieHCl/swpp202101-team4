; ModuleID = '/tmp/a.ll'
source_filename = "/home/psjlds/swpp202101-team4/filechecks/PhierasePass-Test4.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @get_three(i32 %x, i32 %y, i32 %z) #0 {
; CHECK: start get_three 3:
; CHECK: ret
; CHECK: ret
; CHECK: ret
entry:
  %cmp = icmp slt i32 %x, %y
  br i1 %cmp, label %if.then, label %if.else3

if.then:                                          ; preds = %entry
  %cmp1 = icmp slt i32 %y, %z
  br i1 %cmp1, label %if.then2, label %if.else

if.then2:                                         ; preds = %if.then
  br label %if.end

if.else:                                          ; preds = %if.then
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then2
  %res.0 = phi i32 [ %z, %if.then2 ], [ %y, %if.else ]
  br label %if.end4

if.else3:                                         ; preds = %entry
  br label %if.end4

if.end4:                                          ; preds = %if.else3, %if.end
  %res.1 = phi i32 [ %res.0, %if.end ], [ %x, %if.else3 ]
  ret i32 %res.1
}
; CHECK: end get_three

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK: start main 0:
entry:
  %call = call i32 @get_three(i32 8, i32 10, i32 11)
  %call1 = call i32 @get_three(i32 100, i32 99, i32 8)
  %call2 = call i32 @get_three(i32 99, i32 101, i32 89)
  ret i32 0
}
; CHECK: end main

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
