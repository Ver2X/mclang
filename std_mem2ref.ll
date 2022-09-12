; ModuleID = 'f.ll'
source_filename = "f.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z1fiii(i32 noundef %kk, i32 noundef %h, i32 noundef %g) #0 {
entry:
  %add = add nsw i32 %h, %g
  %add1 = add nsw i32 17, 2
  %sub = sub nsw i32 17, 2
  %mul = mul nsw i32 17, 2
  %div = sdiv i32 17, 2
  %add2 = add nsw i32 2, 2
  %sub3 = sub nsw i32 2, 2
  %mul4 = mul nsw i32 2, 2
  %div5 = sdiv i32 2, 2
  %add6 = add nsw i32 17, 3
  %sub7 = sub nsw i32 17, 3
  %mul8 = mul nsw i32 17, 3
  %div9 = sdiv i32 17, 3
  %div10 = sdiv i32 17, 3
  %add11 = add nsw i32 %div10, 5
  ret i32 %add11
}

attributes #0 = { mustprogress noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 14.0.6 (https://github.com/llvm/llvm-project.git f28c006a5895fc0e329fe15fead81e37457cb1d1)"}
