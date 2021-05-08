define void @main() {
; CHECK: start main 0:
  ret void
ToRemove1:
  ret void
ToRemoveChain0:
  br label %ToRemoveChain1
ToRemoveChain1:
  ret void
}
; CHECK: end main