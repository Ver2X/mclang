// after mem2reg std/arith_1.c become:

// define dso_local i32 @main(){
// entry:
//   %add = add nsw i32 2, 100
//   ret i32 %add
// }