#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lexer/lexer.h"
#include "compiler/compiler.h"
#include "compiler/bytecode.h"
#include "vm/vm.h"
#include "vm/output.h"
#include "utils/common.h"
#include "utils/io.h"

static int tests_total = 0;
static int tests_passed = 0;

static value_t create_number(double numeric_value) {
    value_t value;
    value.type = TYPE_NUMBER;
    value.as.number = numeric_value;
    return value;
}

static value_t create_boolean(bool boolean_value) {
    value_t value;
    value.type = TYPE_BOOLEAN;
    value.as.boolean = boolean_value;
    return value;
}

static void test(char* test_name, char* file_path, output_t* expected_output) {
    tests_total++;

    char* source_code = read_file(file_path);

    lexer_init(source_code);
    compiler_init();

    bytecode_t* bytecode = compile();
    vm_init(bytecode);
    vm_run(true);

    output_t* actual_output = vm_get_output();

    if (actual_output->count != expected_output->count) {
        printf("\033[31mFAILED:\033[0m (%s) expected output of size %d, got %d\n", test_name, expected_output->count, actual_output->count);
        return;
    }

    for (int i = 0; i < actual_output->count; i++) {
        value_t expected = expected_output->values[i];
        value_t actual = actual_output->values[i];

        if (expected.type != actual.type) {
            printf("\033[31mFAILED:\033[0m (%s) expected value type to be %d, got %d\n", test_name, expected.type, actual.type);
            return;
        }

        switch (expected.type) {
            case TYPE_NUMBER: {
                number_t expected_value = expected.as.number;
                number_t actual_value = actual.as.number;

                if (expected_value != actual_value) {
                    printf("\033[31mFAILED:\033[0m (%s) expected number value to be %.2f, got %.2f\n", test_name, expected_value, actual_value);
                    return;
                }

                break;
            }
            case TYPE_BOOLEAN: {
                boolean_t expected_value = expected.as.boolean;
                boolean_t actual_value = actual.as.boolean;

                if (expected_value != actual_value) {
                    printf("\033[31mFAILED:\033[0m (%s) expected boolean value to be %d, got %d\n", test_name, expected_value, actual_value);
                    return;
                }

                break;
            }
            case TYPE_STRING: {
                string_t expected_value = expected.as.string;
                string_t actual_value = actual.as.string;

                if (strcmp(expected_value, actual_value) != 0) {
                    printf("\033[31mFAILED:\033[0m (%s) expected string value to be \"%s\", got \"%s\"\n", test_name, expected_value, actual_value);
                    return;
                }

                break;
            }
            default: {
                printf("\033[31mERROR:\033[0m Unknown datatype in tests, cannot compare\n");
                exit(1);
                break;
            }
        }
    }

    tests_passed++;
    printf("\033[32mPASSED:\033[0m (%s)\n", test_name);
    return;
}

int main() {
    printf("\033[32mINFO:\033[0m Starting tests\n");
    printf("--------------------------\n");

    // TEST 01
    {
        output_t* output = output_init();
        output_add(output, create_number(2 + 3 * 4));
        output_add(output, create_number(2 * 3 + 4));
        output_add(output, create_number((2 + 3) * 3));
        output_add(output, create_number(2 * (3 + 4)));
        output_add(output, create_boolean(true && true));
        output_add(output, create_boolean(true && false));
        output_add(output, create_boolean(false && true));
        output_add(output, create_boolean(false && false));
        output_add(output, create_boolean(true || true));
        output_add(output, create_boolean(true || false));
        output_add(output, create_boolean(false || true));
        output_add(output, create_boolean(false || false));

        test("Printing", "./tests/cases/case-01.gen", output);
    }

    // TEST 02
    {
        output_t* output = output_init();
        output_add(output, create_number(1));
        output_add(output, create_number(2));
        output_add(output, create_number(3));
        output_add(output, create_number(4));
        output_add(output, create_number(5));
        output_add(output, create_number(12));
        output_add(output, create_number(3.51));

        test("Arrays", "./tests/cases/case-02.gen", output);
    }

    printf("--------------------------\n");

    if (tests_passed == tests_total) {
        printf("\033[32mINFO:\033[0m All tests \033[32mPASSED\033[0m\n");
    } else {
        printf("\033[31mINFO:\033[0m %d tests \033[31mFAILED\033[0m\n", tests_total - tests_passed);
    }

    return 0;
}
