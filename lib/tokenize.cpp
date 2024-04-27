#include "mclang.h"

using std::string;
// Input filename
static char *current_filename;

// Input string
static char *current_input;

// Reports an error and exit.
void error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error message in the following format and exit.
//
// foo.c:10: x = y + 1;
//               ^ <error message here>
static void verror_at(int line_no, char *loc, const char *fmt, va_list ap) {
  // find the line containing `loc`
  char *line = loc;
  while (current_input < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  // print out the file and line
  int indent = fprintf(stderr, "%s:%d", current_filename, line_no);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // show error message
  int pos = loc - line + indent;

  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  assert(false);
  exit(1);
}

void error_at(char *loc, const char *fmt, ...) {
  int line_no = 1;
  for (char *p = current_input; p < loc; p++) {
    if (*p == '\n')
      line_no++;
  }
  va_list ap;
  va_start(ap, fmt);
  // assert(false);
  verror_at(line_no, loc, fmt, ap);
}

void error_tok(TokenPtr Tok, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  // assert(false);
  verror_at(Tok->line_no, Tok->loc, fmt, ap);
}

// Consumes the current token if it matches `op`.
bool equal(TokenPtr Tok, const char *op) {
  return memcmp(Tok->loc, op, Tok->len) == 0 && op[Tok->len] == '\0';
}

// Ensure that the current token is `op`.
TokenPtr skip(TokenPtr Tok, const char *op) {
  if (!equal(Tok, op)) {
    error_tok(Tok, "expected '%s'", op);
  }
  return Tok->Next;
}

bool consume(TokenPtr *rest, TokenPtr Tok, const char *str) {
  if (equal(Tok, str)) {
    *rest = Tok->Next;
    return true;
  }
  *rest = Tok;
  return false;
}

// Create a new token.
static TokenPtr new_token(TokenKind Kind, char *start, char *end) {
  TokenPtr Tok = std::make_shared<Token>();
  Tok->Kind = Kind;
  Tok->loc = start;
  Tok->len = end - start;
  return Tok;
}

static bool startswith(char *p, const char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

// judge char c could be used as first char of idenitiy
static bool is_ident_first(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// judge char c could be used as rest char of idenitiy
static bool is_ident_rest(char c) {
  return is_ident_first(c) || ('0' <= c && c <= '9');
}

static int from_hex(char c) {
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('a' <= c && c <= 'f')
    return c - 'a' + 10;
  return c - 'A' + 10;
}

// Read a punctuator token from p and returns its length.
static int read_punct(char *p) {
  static const char *puncts[] = {"==", "!=", "<=", ">=", "->"};

  for (int i = 0; i < sizeof(puncts) / sizeof(*puncts); i++) {
    if (startswith(p, puncts[i]))
      return strlen(puncts[i]);
  }

  return ispunct(*p) ? 1 : 0;
}

static bool startswith_string(char *p, string &q) {
  for (int i = 0; i < q.length(); i++) {
    if (*(p + i) != q[i]) {
      return false;
    }
  }
  return true;
}

// Read a punctuator token from p and returns its length.
static int read_punct2(char *p) {
  static string puncts[] = {"==", "!=", "<=", ">=", "->"};

  for (int i = 0; i < 5; i++) {
    if (startswith_string(p, puncts[i]))
      return puncts[i].length();
  }

  return ispunct(*p) ? 1 : 0;
}

// aux fuction , judge keywords
static bool is_keyword(TokenPtr Tok) {
  static const char *kw[] = {"return", "if",     "else", "for",    "while",
                             "int",    "sizeof", "char", "struct", "union",
                             "short",  "long",   "void", "typedef"};

  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
    if (equal(Tok, kw[i]))
      return true;
  }
  return false;
}

static int read_escaped_char(char **new_pos, char *p) {
  if ('0' <= *p && *p <= '7') {
    // reand an octalnumber.
    int c = *p++ - '0';
    if ('0' <= *p && *p <= '7') {
      c = (c << 3) + (*p++ - '0');
      if ('0' <= *p && *p <= '7')
        c = (c << 3) + (*p++ - '0');
    }
    *new_pos = p;
    return c;
  }

  if (*p == 'x') {
    p++;
    if (!isxdigit(*p))
      error_at(p, "invalid hex escape sequence");

    int c = 0;
    for (; isxdigit(*p); p++)
      c = (c << 4) + from_hex(*p);
    *new_pos = p;
    return c;
  }

  *new_pos = p + 1;

  // Escape sequences are defined using themselves here. E.g.
  // '\n' is implemented using '\n'. This tautological definition
  // works because the compiler that compiles our compiler knows
  // what '\n' actually is. In other words, we "inherit" the ASCII
  // code of '\n' from the compiler that compiles our compiler,
  // so we don't have to teach the actual code here.
  //
  // This fact has huge implications not only for the correctness
  // of the compiler but also for the security of the generated code.
  // For more info, read "Reflections on Trusting Trust" by Ken Thompson.
  // https://github.com/rui314/chibicc/wiki/thompson1984.pdf
  switch (*p) {
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 't':
    return '\t';
  case 'n':
    return '\n';
  case 'v':
    return '\v';
  case 'f':
    return '\f';
  case 'r':
    return '\r';
  // [GNU] \e for the ASCII escape character is a GNU C extension.
  case 'e':
    return 27;
  default:
    return *p;
  }
}

// find a closing double-quote.
static char *string_literal_end(char *p) {
  char *start = p;
  for (; *p != '"'; p++) {
    if (*p == '\n' || *p == '\0')
      error_at(start, "unclosed string literal");
    if (*p == '\\')
      p++;
  }
  return p;
}

static TokenPtr read_string_literal(char *start) {
  char *end = string_literal_end(start + 1);
  char *buf = (char *)calloc(1, end - start);
  int len = 0;

  for (char *p = start + 1; p < end;) {
    if (*p == '\\') {
      buf[len++] = read_escaped_char(&p, p + 1);
    } else {
      buf[len++] = *p++;
    }
  }

  TokenPtr Tok = new_token(TK_STR, start, end + 1);
  Tok->Ty = arrayOf(TyChar, len + 1);
  Tok->str = buf;
  return Tok;
}

// travel again , convert idents to keywords
static void convert_keywords(TokenPtr Tok) {
  for (auto t = Tok; t->Kind != TK_EOF; t = t->Next) {
    if (is_keyword(t))
      t->Kind = TK_KEYWORD;
  }
}

// initialize line info for all tokens
static void add_line_numbers(TokenPtr Tok) {
  char *p = current_input;
  int n = 1;
  do {
    if (p == Tok->loc) {
      Tok->line_no = n;
      Tok = Tok->Next;
    }
    if (*p == '\n')
      n++;
  } while (*p++);
}

// Tokenize `current_input` and returns new tokens.
TokenPtr tokenize(char *filename, char *p, size_t &buflen) {
  current_filename = filename;
  current_input = p;
  TokenPtr head = std::make_shared<Token>();
  TokenPtr cur = head;
  // char * file_end = p + buflen;
  // printf("bufflen: %d\n", buflen);
  while (*p) {
    // skip line comments
    if (startswith(p, "//")) {
      p += 2;
      while (*p != '\n')
        p++;
      continue;
    }

    // skip Block comments
    if (startswith(p, "/*")) {
      char *q = strstr(p + 2, "*/");
      if (!q)
        error_at(p, "unclosed Block comment");
      p = q + 2;
    }

    // skip whitespace characters
    if (isspace(*p)) {
      p++;
      continue;
    }

    // numeric literal
    if (isdigit(*p)) {
      cur = cur->Next = new_token(TK_NUM, p, p);
      char *q = p;
      cur->Val = strtoul(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    // string literal
    if (*p == '"') {
      cur = cur->Next = read_string_literal(p);
      p += cur->len;
      continue;
    }

    // Identifier or keyword
    if (is_ident_first(*p)) {
      char *start = p;
      do {
        p++;
      } while (is_ident_rest(*p));
      cur = cur->Next = new_token(TK_IDENT, start, p);
      continue;
    }

    // Punctuators
    int punct_len = read_punct(p);
    if (punct_len) {
      cur = cur->Next = new_token(TK_PUNCT, p, p + punct_len);
      p += cur->len;
      continue;
    }

    error_at(p, "invalid token");
  }

  cur = cur->Next = new_token(TK_EOF, p, p);
  add_line_numbers(head->Next);
  // convert identify to keyword
  convert_keywords(head->Next);
  return head->Next;
}

// return the contens of a given file
static char *read_file(char *path, size_t &buflen) {
  FILE *fp;
  if (strcmp(path, "-") == 0) {
    // by convention, read from stdin if a given filename is "-".
    fp = stdin;
  } else {
    fp = fopen(path, "r");
    if (!fp) {
      error("cannot open %s: %s", path, strerror(errno));
    }
  }

  char *buf;
  // size_t buflen;
  FILE *out = open_memstream(&buf, &buflen);

  // read the entire file.
  for (;;) {
    char buf2[4096];
    int n = fread(buf2, 1, sizeof(buf2), fp);
    if (n == 0)
      break;
    fwrite(buf2, 1, n, out);
  }

  if (fp != stdin) {
    fclose(fp);
  }

  // make sure that the last line is properly terminated with '\n'.
  fflush(out);
  if (buflen == 0 || buf[buflen - 1] != '\n')
    fputc('\n', out);
  fputc('\0', out);
  fclose(out);
  return buf;
}

TokenPtr tokenize_file(char *path) {
  size_t buflen;
  return tokenize(path, read_file(path, buflen), buflen);
}