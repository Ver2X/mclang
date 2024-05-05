#include "mclang.h"

static void gen_expr(Node *node);
static void gen_stmt(Node *node);

std::fstream FileOut;
std::fstream FileOptOut;

static FILE *output_file;
int Depth;
static const char *argreg8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static const char *argreg16[] = {"%di", "%si", "%dx", "%cx", "%r8w", "%r9w"};
static const char *argreg32[] = {"%edi", "%esi", "%edx",
                                 "%ecx", "%r8d", "%r9d"};
static const char *argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

static Obj *current_fn;

static void println(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(output_file, fmt, ap);
  va_end(ap);
  fprintf(output_file, "\n");
}

int count_diff_if = 1;
// use to identify diff "if" statements
static int count() { return count_diff_if++; }

static void push(void) {
  println("  push %%rax");
  Depth++;
}

static void pop(const char *arg) {
  println("  pop %s", arg);
  Depth--;
}

// round up
int alignTo(int n, int Align) { return (n + Align - 1) / Align * Align; }

// stack machine

// assign offsets to local variables.
static void assign_lvar_offsets(Obj *prog) {

  for (Obj *fn = prog; fn; fn = fn->Next) {
    if (!fn->IsFunction)
      continue;

    int Offset = 0;
    for (Obj *Var = fn->Locals; Var; Var = Var->Next) {
      Offset += Var->Ty->Size;
      Offset = alignTo(Offset, Var->Ty->Align);
      Var->Offset = -Offset;
    }

    fn->StackSize = alignTo(Offset, 16);
  }
}

// emit global data
static void emit_data(Obj *prog) {
  for (Obj *Var = prog; Var; Var = Var->Next) {
    if (Var->IsFunction)
      continue;
    println("  .data");
    println("  .globl %s", Var->Name);
    println("%s:", Var->Name);
    if (Var->InitData) {
      for (int i = 0; i < Var->Ty->Size; i++)
        println("  .byte %d", Var->InitData[i]);
    } else {
      println("  .zero %d", Var->Ty->Size);
    }
  }
}

static void store_gp(int r, int Offset, int sz) {
  switch (sz) {
  case 1:
    println("  mov %s, %d(%%rbp)", argreg8[r], Offset);
    return;
  case 2:
    println("  mov %s, %d(%%rbp)", argreg16[r], Offset);
    return;
  case 4:
    println("  mov %s, %d(%%rbp)", argreg32[r], Offset);
    return;
  case 8:
    println("  mov %s, %d(%%rbp)", argreg64[r], Offset);
    return;
  }

  unreachable();
}

void codegen(Obj *prog, FILE *out, std::string file, std::string file_opt) {
  FileOut.open(file, std::ios_base::out);
  output_file = out;
  FileOptOut.open(file_opt, std::ios_base::out);
  // FileOut.open(output_file, out);
  // first setup Offset
  assign_lvar_offsets(prog);
  // emit_data(prog);
  // emit_text(prog);
  // printf("before emitGlobalDataIR:\n");

  emitIR(prog, file);

  FileOut.flush();
  FileOut.close();
}
