/*
 * 晨雾的Brainfuck解释器 (CWBF)
 * Robust Brainfuck Interpreter by 晨雾
 * 特性：
 * - 内存边界检查
 * - 栈溢出保护
 * - 全面错误处理
 * - 调试支持
 * - 输入验证
 * - 性能优化
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

#define MEMORY_SIZE 30000
#define STACK_SIZE 1000
#define MAX_NESTING 1000
#define MAX_PROGRAM_SIZE 1000000

typedef enum {
    BF_OK = 0,
    BF_ERROR_MEMORY_OVERFLOW,
    BF_ERROR_MEMORY_UNDERFLOW,
    BF_ERROR_UNMATCHED_BRACKET,
    BF_ERROR_NESTING_TOO_DEEP,
    BF_ERROR_INVALID_INPUT,
    BF_ERROR_FILE_NOT_FOUND,
    BF_ERROR_PROGRAM_TOO_LARGE,
    BF_ERROR_STACK_OVERFLOW,
    BF_ERROR_INVALID_OPERATION
} bf_error_t;

typedef struct {
    uint8_t *memory;
    size_t memory_size;
    size_t data_ptr;
    size_t program_ptr;
    char *program;
    size_t program_size;
    size_t *bracket_stack;
    size_t stack_top;
    size_t stack_size;
    bool debug_mode;
    FILE *input;
    FILE *output;
    bf_error_t last_error;
    char error_message[256];
} bf_state_t;

static const char *bf_error_string(bf_error_t error) {
    switch (error) {
        case BF_OK: return "成功";
        case BF_ERROR_MEMORY_OVERFLOW: return "内存溢出";
        case BF_ERROR_MEMORY_UNDERFLOW: return "内存下溢";
        case BF_ERROR_UNMATCHED_BRACKET: return "括号不匹配";
        case BF_ERROR_NESTING_TOO_DEEP: return "嵌套过深";
        case BF_ERROR_INVALID_INPUT: return "无效输入";
        case BF_ERROR_FILE_NOT_FOUND: return "文件未找到";
        case BF_ERROR_PROGRAM_TOO_LARGE: return "程序过大";
        case BF_ERROR_STACK_OVERFLOW: return "栈溢出";
        case BF_ERROR_INVALID_OPERATION: return "无效操作";
        default: return "未知错误";
    }
}

static void bf_set_error(bf_state_t *state, bf_error_t error, const char *msg) {
    state->last_error = error;
    if (msg) {
        strncpy(state->error_message, msg, sizeof(state->error_message) - 1);
        state->error_message[sizeof(state->error_message) - 1] = '\0';
    } else {
        strncpy(state->error_message, bf_error_string(error), sizeof(state->error_message) - 1);
    }
}

static bf_state_t *bf_create_state(bool debug_mode) {
    bf_state_t *state = calloc(1, sizeof(bf_state_t));
    if (!state) {
        return NULL;
    }

    state->memory = calloc(MEMORY_SIZE, sizeof(uint8_t));
    if (!state->memory) {
        free(state);
        return NULL;
    }

    state->bracket_stack = calloc(STACK_SIZE, sizeof(size_t));
    if (!state->bracket_stack) {
        free(state->memory);
        free(state);
        return NULL;
    }

    state->memory_size = MEMORY_SIZE;
    state->stack_size = STACK_SIZE;
    state->debug_mode = debug_mode;
    state->input = stdin;
    state->output = stdout;
    state->data_ptr = 0;
    state->program_ptr = 0;
    state->stack_top = 0;
    state->last_error = BF_OK;

    return state;
}

static void bf_destroy_state(bf_state_t *state) {
    if (state) {
        free(state->memory);
        free(state->bracket_stack);
        free(state->program);
        free(state);
    }
}

static bf_error_t bf_validate_program(const char *program, size_t size) {
    int bracket_count = 0;
    int max_nesting = 0;
    int current_nesting = 0;

    for (size_t i = 0; i < size; i++) {
        char c = program[i];
        if (c == '[') {
            current_nesting++;
            bracket_count++;
            if (current_nesting > MAX_NESTING) {
                return BF_ERROR_NESTING_TOO_DEEP;
            }
            if (current_nesting > max_nesting) {
                max_nesting = current_nesting;
            }
        } else if (c == ']') {
            bracket_count--;
            current_nesting--;
            if (bracket_count < 0) {
                return BF_ERROR_UNMATCHED_BRACKET;
            }
        }
    }

    if (bracket_count != 0) {
        return BF_ERROR_UNMATCHED_BRACKET;
    }

    return BF_OK;
}

static bf_error_t bf_load_program(bf_state_t *state, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        bf_set_error(state, BF_ERROR_FILE_NOT_FOUND, strerror(errno));
        return BF_ERROR_FILE_NOT_FOUND;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size < 0 || file_size > MAX_PROGRAM_SIZE) {
        fclose(file);
        bf_set_error(state, BF_ERROR_PROGRAM_TOO_LARGE, "程序文件过大");
        return BF_ERROR_PROGRAM_TOO_LARGE;
    }

    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        bf_set_error(state, BF_ERROR_INVALID_OPERATION, "内存分配失败");
        return BF_ERROR_INVALID_OPERATION;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size) {
        free(buffer);
        bf_set_error(state, BF_ERROR_INVALID_OPERATION, "读取程序文件失败");
        return BF_ERROR_INVALID_OPERATION;
    }

    buffer[file_size] = '\0';

    // 过滤有效的Brainfuck命令
    size_t filtered_size = 0;
    for (size_t i = 0; i < bytes_read; i++) {
        char c = buffer[i];
        if (c == '>' || c == '<' || c == '+' || c == '-' || 
            c == '.' || c == ',' || c == '[' || c == ']') {
            buffer[filtered_size++] = c;
        }
    }

    bf_error_t validation_result = bf_validate_program(buffer, filtered_size);
    if (validation_result != BF_OK) {
        free(buffer);
        bf_set_error(state, validation_result, "程序验证失败");
        return validation_result;
    }

    state->program = realloc(buffer, filtered_size + 1);
    if (!state->program) {
        free(buffer);
        bf_set_error(state, BF_ERROR_INVALID_OPERATION, "内存重新分配失败");
        return BF_ERROR_INVALID_OPERATION;
    }

    state->program[filtered_size] = '\0';
    state->program_size = filtered_size;

    return BF_OK;
}

static void bf_debug_state(const bf_state_t *state) {
    if (!state->debug_mode) return;

    fprintf(stderr, "\n[调试] 程序计数器=%zu 数据指针=%zu 当前值=%u 栈顶=%zu\n",
            state->program_ptr, state->data_ptr, 
            state->memory[state->data_ptr], state->stack_top);
    
    // 显示数据指针周围的内存
    fprintf(stderr, "内存: ");
    size_t start = (state->data_ptr > 5) ? state->data_ptr - 5 : 0;
    size_t end = (state->data_ptr + 5 < state->memory_size) ? state->data_ptr + 5 : state->memory_size - 1;
    
    for (size_t i = start; i <= end; i++) {
        if (i == state->data_ptr) {
            fprintf(stderr, "[%u]", state->memory[i]);
        } else {
            fprintf(stderr, " %u ", state->memory[i]);
        }
    }
    fprintf(stderr, "\n");
}

static bf_error_t bf_execute_instruction(bf_state_t *state) {
    if (state->program_ptr >= state->program_size) {
        return BF_OK;
    }

    char instruction = state->program[state->program_ptr];
    bf_debug_state(state);

    switch (instruction) {
        case '>':
            if (state->data_ptr >= state->memory_size - 1) {
                bf_set_error(state, BF_ERROR_MEMORY_OVERFLOW, "数据指针溢出");
                return BF_ERROR_MEMORY_OVERFLOW;
            }
            state->data_ptr++;
            break;

        case '<':
            if (state->data_ptr == 0) {
                bf_set_error(state, BF_ERROR_MEMORY_UNDERFLOW, "数据指针下溢");
                return BF_ERROR_MEMORY_UNDERFLOW;
            }
            state->data_ptr--;
            break;

        case '+':
            state->memory[state->data_ptr]++;
            break;

        case '-':
            state->memory[state->data_ptr]--;
            break;

        case '.':
            fputc(state->memory[state->data_ptr], state->output);
            fflush(state->output);
            break;

        case ',': {
            int input_char = fgetc(state->input);
            if (input_char != EOF) {
                state->memory[state->data_ptr] = (uint8_t)input_char;
            } else {
                state->memory[state->data_ptr] = 0;
            }
            break;
        }

        case '[':
            if (state->memory[state->data_ptr] == 0) {
                // 跳转到匹配的 ']'
                int bracket_count = 1;
                while (bracket_count > 0 && ++state->program_ptr < state->program_size) {
                    if (state->program[state->program_ptr] == '[') {
                        bracket_count++;
                    } else if (state->program[state->program_ptr] == ']') {
                        bracket_count--;
                    }
                }
                if (state->program_ptr >= state->program_size) {
                    bf_set_error(state, BF_ERROR_UNMATCHED_BRACKET, "未匹配的 '['");
                    return BF_ERROR_UNMATCHED_BRACKET;
                }
            } else {
                // 将当前位置压入栈
                if (state->stack_top >= state->stack_size) {
                    bf_set_error(state, BF_ERROR_STACK_OVERFLOW, "括号栈溢出");
                    return BF_ERROR_STACK_OVERFLOW;
                }
                state->bracket_stack[state->stack_top++] = state->program_ptr;
            }
            break;

        case ']':
            if (state->stack_top == 0) {
                bf_set_error(state, BF_ERROR_UNMATCHED_BRACKET, "未匹配的 ']'");
                return BF_ERROR_UNMATCHED_BRACKET;
            }
            
            if (state->memory[state->data_ptr] != 0) {
                // 跳转回匹配的 '['
                state->program_ptr = state->bracket_stack[state->stack_top - 1];
            } else {
                // 从栈中弹出
                state->stack_top--;
            }
            break;

        default:
            // 跳过无效字符（过滤后不应出现）
            break;
    }

    return BF_OK;
}

static bf_error_t bf_execute_program(bf_state_t *state) {
    bf_error_t result;
    
    while (state->program_ptr < state->program_size) {
        result = bf_execute_instruction(state);
        if (result != BF_OK) {
            return result;
        }
        state->program_ptr++;
    }

    return BF_OK;
}

static void bf_print_usage(const char *program_name) {
    fprintf(stderr, "用法: %s [选项] <brainfuck文件>\n", program_name);
    fprintf(stderr, "选项:\n");
    fprintf(stderr, "  -d, --debug     启用调试模式\n");
    fprintf(stderr, "  -h, --help      显示此帮助信息\n");
    fprintf(stderr, "  -v, --version   显示版本信息\n");
}

static void bf_print_version(void) {
    fprintf(stderr, "晨雾的Brainfuck解释器 (CWBF) v1.0\n");
    fprintf(stderr, "由晨雾开发的具有健壮错误处理和调试支持的解释器\n");
}

int main(int argc, char *argv[]) {
    bool debug_mode = false;
    const char *filename = NULL;

    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug_mode = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            bf_print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            bf_print_version();
            return EXIT_SUCCESS;
        } else if (argv[i][0] != '-') {
            if (filename == NULL) {
                filename = argv[i];
            } else {
                fprintf(stderr, "错误: 指定了多个输入文件\n");
                bf_print_usage(argv[0]);
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "错误: 未知选项 '%s'\n", argv[i]);
            bf_print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "错误: 未指定输入文件\n");
        bf_print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    bf_state_t *state = bf_create_state(debug_mode);
    if (!state) {
        fprintf(stderr, "错误: 初始化解释器状态失败\n");
        return EXIT_FAILURE;
    }

    bf_error_t load_result = bf_load_program(state, filename);
    if (load_result != BF_OK) {
        fprintf(stderr, "加载程序错误: %s\n", state->error_message);
        bf_destroy_state(state);
        return EXIT_FAILURE;
    }

    if (debug_mode) {
        fprintf(stderr, "[调试] 程序加载成功\n");
        fprintf(stderr, "[调试] 程序大小: %zu 字节\n", state->program_size);
        fprintf(stderr, "[调试] 内存大小: %zu 字节\n", state->memory_size);
    }

    bf_error_t exec_result = bf_execute_program(state);
    
    if (exec_result != BF_OK) {
        fprintf(stderr, "运行时错误，位置 %zu: %s\n", 
                state->program_ptr, state->error_message);
        bf_destroy_state(state);
        return EXIT_FAILURE;
    }

    if (debug_mode) {
        fprintf(stderr, "\n[调试] 程序执行成功完成\n");
    }

    bf_destroy_state(state);
    return EXIT_SUCCESS;
}