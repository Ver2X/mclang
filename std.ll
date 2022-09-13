; ModuleID = 'f.cpp'
source_filename = "f.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z1fiii(i32 noundef %kk, i32 noundef %h, i32 noundef %g) #0 {
entry:
  %kk.addr = alloca i32, align 4
  %h.addr = alloca i32, align 4
  %g.addr = alloca i32, align 4
  %t = alloca i32, align 4
  %a = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  store i32 %kk, i32* %kk.addr, align 4
  store i32 %h, i32* %h.addr, align 4
  store i32 %g, i32* %g.addr, align 4
  store i32 2, i32* %a, align 4
  store i32 4, i32* %c, align 4
  %0 = load i32, i32* %h.addr, align 4
  %1 = load i32, i32* %g.addr, align 4
  %add = add nsw i32 %0, %1
  store i32 %add, i32* %t, align 4
  store i32 47, i32* %c, align 4
  store i32 53, i32* %c, align 4
  store i32 300, i32* %c, align 4
  store i32 17, i32* %c, align 4
  %2 = load i32, i32* %c, align 4
  %3 = load i32, i32* %a, align 4
  %add1 = add nsw i32 %2, %3
  store i32 %add1, i32* %d, align 4
  %4 = load i32, i32* %c, align 4
  %5 = load i32, i32* %a, align 4
  %sub = sub nsw i32 %4, %5
  store i32 %sub, i32* %d, align 4
  %6 = load i32, i32* %c, align 4
  %7 = load i32, i32* %a, align 4
  %mul = mul nsw i32 %6, %7
  store i32 %mul, i32* %d, align 4
  %8 = load i32, i32* %c, align 4
  %9 = load i32, i32* %a, align 4
  %div = sdiv i32 %8, %9
  store i32 %div, i32* %d, align 4
  %10 = load i32, i32* %a, align 4
  %add2 = add nsw i32 2, %10
  store i32 %add2, i32* %d, align 4
  %11 = load i32, i32* %a, align 4
  %sub3 = sub nsw i32 2, %11
  store i32 %sub3, i32* %d, align 4
  %12 = load i32, i32* %a, align 4
  %mul4 = mul nsw i32 2, %12
  store i32 %mul4, i32* %d, align 4
  %13 = load i32, i32* %a, align 4
  %div5 = sdiv i32 2, %13
  store i32 %div5, i32* %d, align 4
  %14 = load i32, i32* %c, align 4
  %add6 = add nsw i32 %14, 3
  store i32 %add6, i32* %d, align 4
  %15 = load i32, i32* %c, align 4
  %sub7 = sub nsw i32 %15, 3
  store i32 %sub7, i32* %d, align 4
  %16 = load i32, i32* %c, align 4
  %mul8 = mul nsw i32 %16, 3
  store i32 %mul8, i32* %d, align 4
  %17 = load i32, i32* %c, align 4
  %div9 = sdiv i32 %17, %c
  store i32 %div9, i32* %d, align 4
  %18 = load i32, i32* %c, align 4
  %div10 = sdiv i32 %18, 3
  %add11 = add nsw i32 %div10, 5
  store i32 %add11, i32* %d, align 4
  %19 = load i32, i32* %d, align 4
  ret i32 %19
}

attributes #0 = { mustprogress noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }


!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 14.0.6 (https://github.com/llvm/llvm-project.git f28c006a5895fc0e329fe15fead81e37457cb1d1)"}
