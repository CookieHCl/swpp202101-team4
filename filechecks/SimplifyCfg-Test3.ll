; ModuleID = '/tmp/a.ll'
source_filename = "simplify-cfg.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK: start main 0:
; COM: Check if br appears only three times
; CHECK-NOT: add
; CHECK: br
; CHECK-NOT: add
; CHECK: br
; CHECK-NOT: add
; CHECK: br
; CHECK-NOT: add
; CHECK-NOT: br
; CHECK: end main
entry:
  %call = call i64 (...) @read()
  %call1 = call i64 (...) @read()
  %cmp = icmp sgt i64 %call, %call1
  br i1 %cmp, label %if.then, label %if.else4

if.then:                                          ; preds = %entry
  %cmp2 = icmp sgt i64 %call, %call1
  br i1 %cmp2, label %if.then3, label %if.else

if.then3:                                         ; preds = %if.then
  %div = sdiv i64 %call, %call1
  call void @write(i64 %div)
  br label %if.end

if.else:                                          ; preds = %if.then
  %add = add nsw i64 %call, %call1
  call void @write(i64 %add)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then3
  br label %if.end5

if.else4:                                         ; preds = %entry
  %mul = mul nsw i64 %call, %call1
  call void @write(i64 %mul)
  br label %if.end5

if.end5:                                          ; preds = %if.else4, %if.end
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local i64 @read(...) #2

declare dso_local void @write(i64) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4ed9f17e9390a6845cfd8a235f2078cb9b0e4719)"}
