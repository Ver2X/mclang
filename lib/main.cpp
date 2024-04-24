#include "mclang.h"

static char *opt_o;

static char *input_path;

static bool genAsm = false;

static bool genIR = false;

static bool genSigle = false;

static void usage(int status) {
  fprintf(stderr, "mclang [ -o <path>] [-c] [-S] [-emit-llvm] <file> \n");
  exit(status);
}

static void parse_args(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--help"))
      usage(0);

    if (!strcmp(argv[i], "-o")) {
      if (!argv[++i])
        usage(1);
      opt_o = argv[i];
      continue;
    }

    if (!strcmp(argv[i], "-S")) {
      genAsm = true;
      continue;
    }

    if (!strcmp(argv[i], "-c")) {
      genSigle = true;
      continue;
    }

    if (!strcmp(argv[i], "-emit-llvm")) {
      genIR = true;
      continue;
    }

    if (!strncmp(argv[i], "-o", 2)) {
      opt_o = argv[i] + 2;
      continue;
    }

    if (argv[i][0] == '-' && argv[i][1] != '\0')
      error("unknown argument: %s", argv[i]);

    input_path = argv[i];
  }

  if (!input_path)
    error("no input files");
}

static FILE *open_file(char *path) {
  if (!path || strcmp(path, "-") == 0)
    return stdout;

  FILE *out = fopen(path, "w");
  if (!out)
    error("cannot open output file: %s: %s", path, strerror(errno));
  return out;
}

std::string removeExtension(const std::string &filename) {
  size_t lastDotPos = filename.find_last_of(".");
  if (lastDotPos != std::string::npos) {
    return filename.substr(0, lastDotPos);
  }
  return filename;
}

int main(int argc, char **argv) {
  parse_args(argc, argv);
  std::string file_name(input_path);
  file_name = removeExtension(file_name);
  TokenPtr tok = tokenize_file(input_path);
  Obj *prog = parse(tok);
  if (opt_o) {
    file_name = opt_o;
  } else {
    file_name += ".ll";
  }
  FILE *out = open_file(opt_o);
  // fprintf(out, ".file 1 \"%s\"\n", input_path);
  codegen(prog, out, file_name);

  if (genAsm) {
    std::string Cmd = "clang -S " + file_name;
    system(Cmd.c_str());
  } else if (genSigle) {
    std::string Cmd = "clang -c " + file_name;
    system(Cmd.c_str());
  } else {
    std::string Cmd = "clang " + file_name;
    system(Cmd.c_str());
  }
  return 0;
}
