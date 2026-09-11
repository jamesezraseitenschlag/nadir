// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "eval.h"

static Value run_source(const char* source, Interpreter* interp, bool is_repl) {
    Lexer lexer;
    lexer_init(&lexer, source);

    Parser parser;
    parser_init(&parser, &lexer);

    ASTNode* program = parser_parse(&parser);
    Value res = val_null();
    if (!parser.had_error && program) {
        res = interpreter_run(interp, program);
        if (is_repl && res.type != VAL_NULL) {
            val_print(res);
            printf("\n");
        }
    } else if (program) {
        ast_node_free(program);
    }
    return res;
}

static void run_file(const char* path) {
    char* source = read_file(path);
    if (!source) return;

    Interpreter* interp = interpreter_new();
    run_source(source, interp, false);
    interpreter_free(interp);
    free(source);
}

static int count_char(const char* str, char c) {
    int cnt = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == c) cnt++;
    }
    return cnt;
}

static void start_repl(void) {
    printf("nadir apex runtime\n");
    printf("type exit to quit\n\n");

    Interpreter* interp = interpreter_new();
    char buffer[8192] = {0};
    char line[1024];

    int open_braces = 0;
    int open_parens = 0;
    int open_brackets = 0;

    while (1) {
        if (open_braces > 0 || open_parens > 0 || open_brackets > 0) {
            printf("... ");
        } else {
            printf(">>> ");
        }
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        // trim whitespace
        char* trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        int len = (int)strlen(trimmed);
        while (len > 0 && (trimmed[len - 1] == '\n' || trimmed[len - 1] == '\r' || trimmed[len - 1] == ' ' || trimmed[len - 1] == ';')) {
            trimmed[--len] = '\0';
        }

        if (open_braces == 0 && open_parens == 0 && open_brackets == 0) {
            if (STRNCASECMP(trimmed, "exit", 4) == 0 || STRNCASECMP(trimmed, "quit", 4) == 0) {
                break;
            }
            if (STRNCASECMP(trimmed, "clear", 5) == 0 || STRNCASECMP(trimmed, "cls", 3) == 0) {
                #ifdef _WIN32
                system("cls");
                #else
                system("clear");
                #endif
                continue;
            }
            if (STRNCASECMP(trimmed, "help", 4) == 0) {
                printf("commands: exit, clear, help, import(<path>)\n\n");
                continue;
            }
            if (STRNCASECMP(trimmed, "import", 6) == 0) {
                char* p = trimmed + 6;
                while (*p == ' ' || *p == '\t' || *p == '(') p++;
                if (*p == '\'' || *p == '"') p++;
                char path_buf[512] = {0};
                int plen = 0;
                while (*p && *p != '\'' && *p != '"' && *p != ')' && *p != ';' && plen < 510) {
                    path_buf[plen++] = *p++;
                }
                while (plen > 0 && (path_buf[plen - 1] == ' ' || path_buf[plen - 1] == '\t')) {
                    path_buf[--plen] = '\0';
                }
                path_buf[plen] = '\0';
                if (plen > 0) {
                    char* file_src = read_file(path_buf);
                    if (file_src) {
                        run_source(file_src, interp, false);
                        free(file_src);
                        printf("[imported] %s\n", path_buf);
                    } else {
                        fprintf(stderr, "Error: Could not import file \"%s\"\n", path_buf);
                    }
                } else {
                    fprintf(stderr, "Usage: import(path/to/class.cls)\n");
                }
                continue;
            }
        }

        strcat(buffer, line);
        // multiline balance tracking
        open_braces += count_char(line, '{') - count_char(line, '}');
        open_parens += count_char(line, '(') - count_char(line, ')');
        open_brackets += count_char(line, '[') - count_char(line, ']');

        if (open_braces <= 0 && open_parens <= 0 && open_brackets <= 0) {
            open_braces = 0;
            open_parens = 0;
            open_brackets = 0;

            if (strlen(buffer) > 0) {
                run_source(buffer, interp, true);
                buffer[0] = '\0';
            }
        }
    }

    printf("Exiting Nadir REPL.\n");
    interpreter_free(interp);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        Interpreter* interp = interpreter_new();
        for (int i = 1; i < argc; i++) {
            char* source = read_file(argv[i]);
            if (source) {
                run_source(source, interp, false);
                free(source);
            }
        }
        interpreter_free(interp);
    } else {
        start_repl();
    }
    return 0;
}