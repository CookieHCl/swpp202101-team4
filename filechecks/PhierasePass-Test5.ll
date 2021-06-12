; ModuleID = 'test/swpp202101-benchmarks/binary_tree/src/binary_tree.ll'
source_filename = "binary_tree/src/binary_tree.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@root = external dso_local global i64*, align 8

; Function Attrs: nounwind uwtable
define dso_local i64 @insert(i64 %data) #0 {
; CHECK: start insert 1:
; CHECK: ret
; CHECK: ret
entry:
  %0 = load i64*, i64** @root, align 8
  %cmp = icmp eq i64* %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = call noalias i8* @malloc(i64 24) #4
  %1 = bitcast i8* %call to i64*
  store i64 %data, i64* %1, align 8
  %add.ptr = getelementptr inbounds i64, i64* %1, i64 1
  store i64 0, i64* %add.ptr, align 8
  %add.ptr1 = getelementptr inbounds i64, i64* %1, i64 2
  store i64 0, i64* %add.ptr1, align 8
  store i64* %1, i64** @root, align 8
  br label %return

if.end:                                           ; preds = %entry
  %2 = load i64*, i64** @root, align 8
  br label %while.cond.preheader

while.cond.preheader:                             ; preds = %if.end
  br label %while.cond

while.cond:                                       ; preds = %while.cond.preheader, %cleanup
  %curr.0 = phi i64* [ %curr.1, %cleanup ], [ %2, %while.cond.preheader ]
  %retval.0 = phi i64 [ %retval.1, %cleanup ], [ 0, %while.cond.preheader ]
  br label %while.body

while.body:                                       ; preds = %while.cond
  %3 = load i64, i64* %curr.0, align 8
  %cmp2 = icmp ugt i64 %3, %data
  br i1 %cmp2, label %if.then3, label %if.else

if.then3:                                         ; preds = %while.body
  %add.ptr4 = getelementptr inbounds i64, i64* %curr.0, i64 1
  %4 = load i64, i64* %add.ptr4, align 8
  %5 = inttoptr i64 %4 to i64*
  %cmp5 = icmp eq i64* %5, null
  br i1 %cmp5, label %if.then6, label %if.end12

if.then6:                                         ; preds = %if.then3
  %call8 = call noalias i8* @malloc(i64 24) #4
  %6 = bitcast i8* %call8 to i64*
  store i64 %data, i64* %6, align 8
  %add.ptr9 = getelementptr inbounds i64, i64* %6, i64 1
  store i64 0, i64* %add.ptr9, align 8
  %add.ptr10 = getelementptr inbounds i64, i64* %6, i64 2
  store i64 0, i64* %add.ptr10, align 8
  %7 = ptrtoint i64* %6 to i64
  %add.ptr11 = getelementptr inbounds i64, i64* %curr.0, i64 1
  store i64 %7, i64* %add.ptr11, align 8
  br label %cleanup

if.end12:                                         ; preds = %if.then3
  br label %cleanup, !llvm.loop !2

if.else:                                          ; preds = %while.body
  %cmp13 = icmp ult i64 %3, %data
  br i1 %cmp13, label %if.then14, label %if.else24

if.then14:                                        ; preds = %if.else
  %add.ptr15 = getelementptr inbounds i64, i64* %curr.0, i64 2
  %8 = load i64, i64* %add.ptr15, align 8
  %9 = inttoptr i64 %8 to i64*
  %cmp16 = icmp eq i64* %9, null
  br i1 %cmp16, label %if.then17, label %if.end23

if.then17:                                        ; preds = %if.then14
  %call19 = call noalias i8* @malloc(i64 24) #4
  %10 = bitcast i8* %call19 to i64*
  store i64 %data, i64* %10, align 8
  %add.ptr20 = getelementptr inbounds i64, i64* %10, i64 1
  store i64 0, i64* %add.ptr20, align 8
  %add.ptr21 = getelementptr inbounds i64, i64* %10, i64 2
  store i64 0, i64* %add.ptr21, align 8
  %11 = ptrtoint i64* %10 to i64
  %add.ptr22 = getelementptr inbounds i64, i64* %curr.0, i64 2
  store i64 %11, i64* %add.ptr22, align 8
  br label %cleanup

if.end23:                                         ; preds = %if.then14
  br label %cleanup, !llvm.loop !2

if.else24:                                        ; preds = %if.else
  br label %cleanup

cleanup:                                          ; preds = %if.else24, %if.end23, %if.then17, %if.end12, %if.then6
  %curr.1 = phi i64* [ %curr.0, %if.then6 ], [ %5, %if.end12 ], [ %curr.0, %if.then17 ], [ %9, %if.end23 ], [ %curr.0, %if.else24 ]
  %cleanup.dest.slot.0 = phi i32 [ 1, %if.then6 ], [ 2, %if.end12 ], [ 1, %if.then17 ], [ 2, %if.end23 ], [ 1, %if.else24 ]
  %retval.1 = phi i64 [ 1, %if.then6 ], [ %retval.0, %if.end12 ], [ 1, %if.then17 ], [ %retval.0, %if.end23 ], [ 0, %if.else24 ]
  switch i32 %cleanup.dest.slot.0, label %cleanup26 [
    i32 2, label %while.cond
  ]

cleanup26:                                        ; preds = %cleanup
  %retval.1.lcssa = phi i64 [ %retval.1, %cleanup ]
  br label %return

return:                                           ; preds = %cleanup26, %if.then
  %retval.2 = phi i64 [ %retval.1.lcssa, %cleanup26 ], [ 1, %if.then ]
  ret i64 %retval.2
}
; CHECK: end insert

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local i64 @adjust(i64* %node) #0 {
; CHECK: start adjust 1:
; CHECK: ret
entry:
  %add.ptr = getelementptr inbounds i64, i64* %node, i64 1
  %0 = load i64, i64* %add.ptr, align 8
  %1 = inttoptr i64 %0 to i64*
  br label %while.cond.preheader

while.cond.preheader:                             ; preds = %entry
  br label %while.cond

while.cond:                                       ; preds = %while.cond.preheader, %cleanup.cont
  %curr.0 = phi i64* [ %curr.1, %cleanup.cont ], [ %1, %while.cond.preheader ]
  %parent.0 = phi i64* [ %parent.1, %cleanup.cont ], [ %node, %while.cond.preheader ]
  %retval.0 = phi i64 [ %retval.1, %cleanup.cont ], [ 0, %while.cond.preheader ]
  br label %while.body

while.body:                                       ; preds = %while.cond
  %2 = load i64, i64* %curr.0, align 8
  %add.ptr1 = getelementptr inbounds i64, i64* %curr.0, i64 1
  %3 = load i64, i64* %add.ptr1, align 8
  %4 = inttoptr i64 %3 to i64*
  %add.ptr2 = getelementptr inbounds i64, i64* %curr.0, i64 2
  %5 = load i64, i64* %add.ptr2, align 8
  %6 = inttoptr i64 %5 to i64*
  %cmp = icmp eq i64* %6, null
  br i1 %cmp, label %if.then, label %if.end8

if.then:                                          ; preds = %while.body
  %add.ptr3 = getelementptr inbounds i64, i64* %parent.0, i64 1
  %7 = load i64, i64* %add.ptr3, align 8
  %8 = inttoptr i64 %7 to i64*
  %cmp4 = icmp eq i64* %curr.0, %8
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %9 = ptrtoint i64* %4 to i64
  %add.ptr6 = getelementptr inbounds i64, i64* %parent.0, i64 1
  store i64 %9, i64* %add.ptr6, align 8
  br label %if.end

if.else:                                          ; preds = %if.then
  %10 = ptrtoint i64* %6 to i64
  %add.ptr7 = getelementptr inbounds i64, i64* %parent.0, i64 2
  store i64 %10, i64* %add.ptr7, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then5
  %11 = bitcast i64* %curr.0 to i8*
  call void @free(i8* %11) #4
  br label %cleanup

if.end8:                                          ; preds = %while.body
  br label %cleanup

cleanup:                                          ; preds = %if.end8, %if.end
  %curr.1 = phi i64* [ %curr.0, %if.end ], [ %6, %if.end8 ]
  %parent.1 = phi i64* [ %parent.0, %if.end ], [ %curr.0, %if.end8 ]
  %retval.1 = phi i64 [ %2, %if.end ], [ %retval.0, %if.end8 ]
  %cleanup.dest.slot.0 = phi i32 [ 1, %if.end ], [ 0, %if.end8 ]
  switch i32 %cleanup.dest.slot.0, label %cleanup11 [
    i32 0, label %cleanup.cont
  ]

cleanup.cont:                                     ; preds = %cleanup
  br label %while.cond, !llvm.loop !4

cleanup11:                                        ; preds = %cleanup
  %retval.1.lcssa = phi i64 [ %retval.1, %cleanup ]
  ret i64 %retval.1.lcssa
}
; CHECK: end adjust

; Function Attrs: nounwind
declare dso_local void @free(i8*) #2

; Function Attrs: nounwind uwtable
define dso_local i64 @remove(i64 %data) #0 {
; CHECK: start remove 1:
; CHECK: ret
; CHECK: ret
; CHECK: ret
entry:
  %0 = load i64*, i64** @root, align 8
  %cmp = icmp eq i64* %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %1 = load i64*, i64** @root, align 8
  br label %while.cond.preheader

while.cond.preheader:                             ; preds = %if.end
  br label %while.cond

while.cond:                                       ; preds = %while.cond.preheader, %cleanup
  %curr.0 = phi i64* [ %curr.1, %cleanup ], [ %1, %while.cond.preheader ]
  %parent.0 = phi i64* [ %parent.1, %cleanup ], [ null, %while.cond.preheader ]
  %retval.0 = phi i64 [ %retval.1, %cleanup ], [ 0, %while.cond.preheader ]
  br label %while.body

while.body:                                       ; preds = %while.cond
  %cmp1 = icmp eq i64* %curr.0, null
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %while.body
  br label %cleanup42

if.end3:                                          ; preds = %while.body
  %2 = load i64, i64* %curr.0, align 8
  %add.ptr = getelementptr inbounds i64, i64* %curr.0, i64 1
  %3 = load i64, i64* %add.ptr, align 8
  %4 = inttoptr i64 %3 to i64*
  %add.ptr4 = getelementptr inbounds i64, i64* %curr.0, i64 2
  %5 = load i64, i64* %add.ptr4, align 8
  %6 = inttoptr i64 %5 to i64*
  %cmp5 = icmp ult i64 %data, %2
  br i1 %cmp5, label %if.then6, label %if.end8

if.then6:                                         ; preds = %if.end3
  %add.ptr7 = getelementptr inbounds i64, i64* %curr.0, i64 1
  %7 = load i64, i64* %add.ptr7, align 8
  %8 = inttoptr i64 %7 to i64*
  br label %cleanup, !llvm.loop !5

if.end8:                                          ; preds = %if.end3
  %cmp9 = icmp ugt i64 %data, %2
  br i1 %cmp9, label %if.then10, label %if.end12

if.then10:                                        ; preds = %if.end8
  %add.ptr11 = getelementptr inbounds i64, i64* %curr.0, i64 2
  %9 = load i64, i64* %add.ptr11, align 8
  %10 = inttoptr i64 %9 to i64*
  br label %cleanup, !llvm.loop !5

if.end12:                                         ; preds = %if.end8
  %cmp13 = icmp eq i64* %4, null
  br i1 %cmp13, label %if.then14, label %if.end25

if.then14:                                        ; preds = %if.end12
  %cmp15 = icmp eq i64* %parent.0, null
  br i1 %cmp15, label %if.then16, label %if.else

if.then16:                                        ; preds = %if.then14
  store i64* %6, i64** @root, align 8
  br label %if.end24

if.else:                                          ; preds = %if.then14
  %add.ptr17 = getelementptr inbounds i64, i64* %parent.0, i64 1
  %11 = load i64, i64* %add.ptr17, align 8
  %12 = inttoptr i64 %11 to i64*
  %cmp18 = icmp eq i64* %curr.0, %12
  br i1 %cmp18, label %if.then19, label %if.else21

if.then19:                                        ; preds = %if.else
  %13 = ptrtoint i64* %6 to i64
  %add.ptr20 = getelementptr inbounds i64, i64* %parent.0, i64 1
  store i64 %13, i64* %add.ptr20, align 8
  br label %if.end23

if.else21:                                        ; preds = %if.else
  %14 = ptrtoint i64* %6 to i64
  %add.ptr22 = getelementptr inbounds i64, i64* %parent.0, i64 2
  store i64 %14, i64* %add.ptr22, align 8
  br label %if.end23

if.end23:                                         ; preds = %if.else21, %if.then19
  br label %if.end24

if.end24:                                         ; preds = %if.end23, %if.then16
  %15 = bitcast i64* %curr.0 to i8*
  call void @free(i8* %15) #4
  br label %cleanup

if.end25:                                         ; preds = %if.end12
  %cmp26 = icmp eq i64* %6, null
  br i1 %cmp26, label %if.then27, label %if.end39

if.then27:                                        ; preds = %if.end25
  %cmp28 = icmp eq i64* %parent.0, null
  br i1 %cmp28, label %if.then29, label %if.else30

if.then29:                                        ; preds = %if.then27
  store i64* %4, i64** @root, align 8
  br label %if.end38

if.else30:                                        ; preds = %if.then27
  %add.ptr31 = getelementptr inbounds i64, i64* %parent.0, i64 1
  %16 = load i64, i64* %add.ptr31, align 8
  %17 = inttoptr i64 %16 to i64*
  %cmp32 = icmp eq i64* %curr.0, %17
  br i1 %cmp32, label %if.then33, label %if.else35

if.then33:                                        ; preds = %if.else30
  %18 = ptrtoint i64* %4 to i64
  %add.ptr34 = getelementptr inbounds i64, i64* %parent.0, i64 1
  store i64 %18, i64* %add.ptr34, align 8
  br label %if.end37

if.else35:                                        ; preds = %if.else30
  %19 = ptrtoint i64* %4 to i64
  %add.ptr36 = getelementptr inbounds i64, i64* %parent.0, i64 2
  store i64 %19, i64* %add.ptr36, align 8
  br label %if.end37

if.end37:                                         ; preds = %if.else35, %if.then33
  br label %if.end38

if.end38:                                         ; preds = %if.end37, %if.then29
  %20 = bitcast i64* %curr.0 to i8*
  call void @free(i8* %20) #4
  br label %cleanup

if.end39:                                         ; preds = %if.end25
  %call = call i64 @adjust(i64* %curr.0)
  store i64 %call, i64* %curr.0, align 8
  br label %cleanup

cleanup:                                          ; preds = %if.end39, %if.end38, %if.end24, %if.then10, %if.then6
  %cleanup.dest.slot.0 = phi i32 [ 2, %if.then6 ], [ 2, %if.then10 ], [ 1, %if.end24 ], [ 1, %if.end38 ], [ 1, %if.end39 ]
  %curr.1 = phi i64* [ %8, %if.then6 ], [ %10, %if.then10 ], [ %curr.0, %if.end24 ], [ %curr.0, %if.end38 ], [ %curr.0, %if.end39 ]
  %parent.1 = phi i64* [ %curr.0, %if.then6 ], [ %curr.0, %if.then10 ], [ %parent.0, %if.end24 ], [ %parent.0, %if.end38 ], [ %parent.0, %if.end39 ]
  %retval.1 = phi i64 [ %retval.0, %if.then6 ], [ %retval.0, %if.then10 ], [ 1, %if.end24 ], [ 1, %if.end38 ], [ 1, %if.end39 ]
  switch i32 %cleanup.dest.slot.0, label %cleanup42.loopexit [
    i32 2, label %while.cond
  ]

cleanup42.loopexit:                               ; preds = %cleanup
  %retval.1.lcssa = phi i64 [ %retval.1, %cleanup ]
  br label %cleanup42

cleanup42:                                        ; preds = %cleanup42.loopexit, %if.then2
  %retval.2 = phi i64 [ 0, %if.then2 ], [ %retval.1.lcssa, %cleanup42.loopexit ]
  br label %return

return:                                           ; preds = %cleanup42, %if.then
  %retval.3 = phi i64 [ 0, %if.then ], [ %retval.2, %cleanup42 ]
  ret i64 %retval.3
}
; CHECK: end remove

; Function Attrs: nounwind uwtable
define dso_local void @traverse(i64* %node) #0 {
; CHECK: start traverse 1:
entry:
  %cmp = icmp eq i64* %node, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %0 = load i64, i64* %node, align 8
  %add.ptr = getelementptr inbounds i64, i64* %node, i64 1
  %1 = load i64, i64* %add.ptr, align 8
  %2 = inttoptr i64 %1 to i64*
  %add.ptr1 = getelementptr inbounds i64, i64* %node, i64 2
  %3 = load i64, i64* %add.ptr1, align 8
  %4 = inttoptr i64 %3 to i64*
  call void @traverse(i64* %2)
  call void @write(i64 %0)
  call void @traverse(i64* %4)
  br label %return

return:                                           ; preds = %if.end, %if.then
  ret void
}
; CHECK: end traverse

declare dso_local void @write(i64) #3

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; CHECK: start main 0:
entry:
  store i64* null, i64** @root, align 8
  %call = call i64 (...) @read()
  br label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %cmp1 = icmp ult i64 0, %call
  br i1 %cmp1, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %0 = add i64 %call, -1
  %xtraiter = and i64 %call, 7
  %1 = icmp ult i64 %0, 7
  br i1 %1, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body.lr.ph.new

for.body.lr.ph.new:                               ; preds = %for.body.lr.ph
  %unroll_iter = sub i64 %call, %xtraiter
  br label %for.body

for.cond.for.cond.cleanup_crit_edge.unr-lcssa.loopexit: ; preds = %if.end.7
  %i.02.unr.ph = phi i64 [ %inc.7, %if.end.7 ]
  br label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa

for.cond.for.cond.cleanup_crit_edge.unr-lcssa:    ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa.loopexit, %for.body.lr.ph
  %i.02.unr = phi i64 [ 0, %for.body.lr.ph ], [ %i.02.unr.ph, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa.loopexit ]
  %lcmp.mod = icmp ne i64 %xtraiter, 0
  br i1 %lcmp.mod, label %for.body.epil.preheader, label %for.cond.for.cond.cleanup_crit_edge

for.body.epil.preheader:                          ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa
  br label %for.body.epil

for.body.epil:                                    ; preds = %for.inc.epil, %for.body.epil.preheader
  %i.02.epil = phi i64 [ %i.02.unr, %for.body.epil.preheader ], [ %inc.epil, %for.inc.epil ]
  %epil.iter = phi i64 [ %xtraiter, %for.body.epil.preheader ], [ %epil.iter.sub, %for.inc.epil ]
  %call1.epil = call i64 (...) @read()
  %call2.epil = call i64 (...) @read()
  %cmp3.epil = icmp eq i64 %call1.epil, 0
  br i1 %cmp3.epil, label %if.then.epil, label %if.else.epil

if.else.epil:                                     ; preds = %for.body.epil
  %call5.epil = call i64 @remove(i64 %call2.epil)
  br label %if.end.epil

if.then.epil:                                     ; preds = %for.body.epil
  %call4.epil = call i64 @insert(i64 %call2.epil)
  br label %if.end.epil

if.end.epil:                                      ; preds = %if.then.epil, %if.else.epil
  br label %for.inc.epil

for.inc.epil:                                     ; preds = %if.end.epil
  %inc.epil = add i64 %i.02.epil, 1
  %cmp.epil = icmp ult i64 %inc.epil, %call
  %epil.iter.sub = sub i64 %epil.iter, 1
  %epil.iter.cmp = icmp ne i64 %epil.iter.sub, 0
  br i1 %epil.iter.cmp, label %for.body.epil, label %for.cond.for.cond.cleanup_crit_edge.epilog-lcssa, !llvm.loop !6

for.cond.for.cond.cleanup_crit_edge.epilog-lcssa: ; preds = %for.inc.epil
  br label %for.cond.for.cond.cleanup_crit_edge

for.cond.for.cond.cleanup_crit_edge:              ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, %for.cond.for.cond.cleanup_crit_edge.epilog-lcssa
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.for.cond.cleanup_crit_edge, %for.cond.preheader
  br label %for.end

for.body:                                         ; preds = %if.end.7, %for.body.lr.ph.new
  %i.02 = phi i64 [ 0, %for.body.lr.ph.new ], [ %inc.7, %if.end.7 ]
  %niter = phi i64 [ %unroll_iter, %for.body.lr.ph.new ], [ %niter.nsub.7, %if.end.7 ]
  %call1 = call i64 (...) @read()
  %call2 = call i64 (...) @read()
  %cmp3 = icmp eq i64 %call1, 0
  br i1 %cmp3, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %call4 = call i64 @insert(i64 %call2)
  br label %if.end

if.else:                                          ; preds = %for.body
  %call5 = call i64 @remove(i64 %call2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %inc = add nuw nsw i64 %i.02, 1
  %niter.nsub = sub i64 %niter, 1
  %call1.1 = call i64 (...) @read()
  %call2.1 = call i64 (...) @read()
  %cmp3.1 = icmp eq i64 %call1.1, 0
  br i1 %cmp3.1, label %if.then.1, label %if.else.1

for.end:                                          ; preds = %for.cond.cleanup
  %2 = load i64*, i64** @root, align 8
  call void @traverse(i64* %2)
  ret i32 0

if.else.1:                                        ; preds = %if.end
  %call5.1 = call i64 @remove(i64 %call2.1)
  br label %if.end.1

if.then.1:                                        ; preds = %if.end
  %call4.1 = call i64 @insert(i64 %call2.1)
  br label %if.end.1

if.end.1:                                         ; preds = %if.then.1, %if.else.1
  %inc.1 = add nuw nsw i64 %inc, 1
  %niter.nsub.1 = sub i64 %niter.nsub, 1
  %call1.2 = call i64 (...) @read()
  %call2.2 = call i64 (...) @read()
  %cmp3.2 = icmp eq i64 %call1.2, 0
  br i1 %cmp3.2, label %if.then.2, label %if.else.2

if.else.2:                                        ; preds = %if.end.1
  %call5.2 = call i64 @remove(i64 %call2.2)
  br label %if.end.2

if.then.2:                                        ; preds = %if.end.1
  %call4.2 = call i64 @insert(i64 %call2.2)
  br label %if.end.2

if.end.2:                                         ; preds = %if.then.2, %if.else.2
  %inc.2 = add nuw nsw i64 %inc.1, 1
  %niter.nsub.2 = sub i64 %niter.nsub.1, 1
  %call1.3 = call i64 (...) @read()
  %call2.3 = call i64 (...) @read()
  %cmp3.3 = icmp eq i64 %call1.3, 0
  br i1 %cmp3.3, label %if.then.3, label %if.else.3

if.else.3:                                        ; preds = %if.end.2
  %call5.3 = call i64 @remove(i64 %call2.3)
  br label %if.end.3

if.then.3:                                        ; preds = %if.end.2
  %call4.3 = call i64 @insert(i64 %call2.3)
  br label %if.end.3

if.end.3:                                         ; preds = %if.then.3, %if.else.3
  %inc.3 = add nuw nsw i64 %inc.2, 1
  %niter.nsub.3 = sub i64 %niter.nsub.2, 1
  %call1.4 = call i64 (...) @read()
  %call2.4 = call i64 (...) @read()
  %cmp3.4 = icmp eq i64 %call1.4, 0
  br i1 %cmp3.4, label %if.then.4, label %if.else.4

if.else.4:                                        ; preds = %if.end.3
  %call5.4 = call i64 @remove(i64 %call2.4)
  br label %if.end.4

if.then.4:                                        ; preds = %if.end.3
  %call4.4 = call i64 @insert(i64 %call2.4)
  br label %if.end.4

if.end.4:                                         ; preds = %if.then.4, %if.else.4
  %inc.4 = add nuw nsw i64 %inc.3, 1
  %niter.nsub.4 = sub i64 %niter.nsub.3, 1
  %call1.5 = call i64 (...) @read()
  %call2.5 = call i64 (...) @read()
  %cmp3.5 = icmp eq i64 %call1.5, 0
  br i1 %cmp3.5, label %if.then.5, label %if.else.5

if.else.5:                                        ; preds = %if.end.4
  %call5.5 = call i64 @remove(i64 %call2.5)
  br label %if.end.5

if.then.5:                                        ; preds = %if.end.4
  %call4.5 = call i64 @insert(i64 %call2.5)
  br label %if.end.5

if.end.5:                                         ; preds = %if.then.5, %if.else.5
  %inc.5 = add nuw nsw i64 %inc.4, 1
  %niter.nsub.5 = sub i64 %niter.nsub.4, 1
  %call1.6 = call i64 (...) @read()
  %call2.6 = call i64 (...) @read()
  %cmp3.6 = icmp eq i64 %call1.6, 0
  br i1 %cmp3.6, label %if.then.6, label %if.else.6

if.else.6:                                        ; preds = %if.end.5
  %call5.6 = call i64 @remove(i64 %call2.6)
  br label %if.end.6

if.then.6:                                        ; preds = %if.end.5
  %call4.6 = call i64 @insert(i64 %call2.6)
  br label %if.end.6

if.end.6:                                         ; preds = %if.then.6, %if.else.6
  %inc.6 = add nuw nsw i64 %inc.5, 1
  %niter.nsub.6 = sub i64 %niter.nsub.5, 1
  %call1.7 = call i64 (...) @read()
  %call2.7 = call i64 (...) @read()
  %cmp3.7 = icmp eq i64 %call1.7, 0
  br i1 %cmp3.7, label %if.then.7, label %if.else.7

if.else.7:                                        ; preds = %if.end.6
  %call5.7 = call i64 @remove(i64 %call2.7)
  br label %if.end.7

if.then.7:                                        ; preds = %if.end.6
  %call4.7 = call i64 @insert(i64 %call2.7)
  br label %if.end.7

if.end.7:                                         ; preds = %if.then.7, %if.else.7
  %inc.7 = add i64 %inc.6, 1
  %niter.nsub.7 = sub i64 %niter.nsub.6, 1
  %niter.ncmp.7 = icmp ne i64 %niter.nsub.7, 0
  br i1 %niter.ncmp.7, label %for.body, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa.loopexit, !llvm.loop !7
}
; CHECK: end main

declare dso_local i64 @read(...) #3

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.1 (https://github.com/llvm/llvm-project.git 072c90a863aac1334a4950b3da262a025516dea0)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.unroll.disable"}
!4 = distinct !{!4, !3}
!5 = distinct !{!5, !3}
!6 = distinct !{!6, !3}
!7 = distinct !{!7, !8, !3}
!8 = !{!"llvm.loop.mustprogress"}
