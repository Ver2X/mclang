#include "mclang.h"

static void gen_expr(Node *node);
static void gen_stmt(Node *node);

std::fstream file_out;

static FILE *output_file;
int depth;
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
  depth++;
}

static void pop(const char *arg) {
  println("  pop %s", arg);
  depth--;
}

// round up
int align_to(int n, int align) { return (n + align - 1) / align * align; }

// stack machine

// assign offsets to local variables.
static void assign_lvar_offsets(Obj *prog) {

  for (Obj *fn = prog; fn; fn = fn->next) {
    if (!fn->is_function)
      continue;

    int offset = 0;
    for (Obj *var = fn->locals; var; var = var->next) {
      offset += var->ty->size;
      offset = align_to(offset, var->ty->align);
      var->offset = -offset;
    }

    fn->stack_size = align_to(offset, 16);
  }
}

// emit global data
static void emit_data(Obj *prog) {
  for (Obj *var = prog; var; var = var->next) {
    if (var->is_function)
      continue;
    println("  .data");
    println("  .globl %s", var->name);
    println("%s:", var->name);
    if (var->init_data) {
      for (int i = 0; i < var->ty->size; i++)
        println("  .byte %d", var->init_data[i]);
    } else {
      println("  .zero %d", var->ty->size);
    }
  }
}

static void store_gp(int r, int offset, int sz) {
  switch (sz) {
  case 1:
    println("  mov %s, %d(%%rbp)", argreg8[r], offset);
    return;
  case 2:
    println("  mov %s, %d(%%rbp)", argreg16[r], offset);
    return;
  case 4:
    println("  mov %s, %d(%%rbp)", argreg32[r], offset);
    return;
  case 8:
    println("  mov %s, %d(%%rbp)", argreg64[r], offset);
    return;
  }

  unreachable();
}

void codegen(Obj *prog, FILE *out, std::string file) {
  file_out.open(file, std::ios_base::out);
  output_file = out;
  // file_out.open(output_file, out);
  // first setup offset
  assign_lvar_offsets(prog);
  // emit_data(prog);
  // emit_text(prog);
  // printf("before emit_global_data_ir:\n");

  emit_ir(prog, file);

  file_out.flush();
  file_out.close();
}
