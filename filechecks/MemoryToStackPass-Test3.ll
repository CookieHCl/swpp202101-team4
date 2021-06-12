; ModuleID = '/tmp/a.ll'
source_filename = "c.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i8* @malloc2(i64 %n) #0 {
entry:
  %call = call noalias i8* @malloc(i64 %n) #3
  ret i8* %call
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK: start main 0:
; CHECK-NOT: call ____free
; CHECK: br
; CHECK-NOT: call ____free
; CHECK: br
; CHECK: call ____free
; CHECK: call malloc2
; CHECK-NOT: call ____free
; CHECK: end main
entry:
  %call = call noalias i8* @malloc(i64 100) #3
  %call1 = call noalias i8* @malloc(i64 200) #3
  %tobool = icmp ne i8* %call1, null
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @free(i8* %call) #3
  call void @free(i8* %call1) #3
  br label %cleanup

if.end:                                           ; preds = %entry
  call void @free(i8* %call) #3
  %call2 = call i8* @malloc2(i64 300)
  %tobool3 = icmp ne i8* %call2, null
  br i1 %tobool3, label %if.then4, label %if.else

if.then4:                                         ; preds = %if.end
  call void @free(i8* %call2) #3
  br label %if.end6

if.else:                                          ; preds = %if.end
  %call5 = call noalias i8* @malloc(i64 400) #3
  call void @free(i8* %call5) #3
  br label %if.end6

if.end6:                                          ; preds = %if.else, %if.then4
  call void @free(i8* %call1) #3
  br label %cleanup

cleanup:                                          ; preds = %if.end6, %if.then
  %cleanup.dest.slot.0 = phi i32 [ 1, %if.then ], [ 0, %if.end6 ]
  switch i32 %cleanup.dest.slot.0, label %unreachable [
    i32 0, label %cleanup.cont
    i32 1, label %cleanup.cont
  ]

cleanup.cont:                                     ; preds = %cleanup, %cleanup
  ret i32 0

unreachable:                                      ; preds = %cleanup
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nounwind
declare dso_local void @free(i8*) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nofree nosync nounwind willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.1 (https://github.com/llvm/llvm-project.git 4973ce53ca8abfc14233a3d8b3045673e0e8543c)"}
