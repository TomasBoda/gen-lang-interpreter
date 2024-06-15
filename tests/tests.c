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

static value_t create_number(double number_value) {
    value_t value;
    value.type = TYPE_NUMBER;
    value.as.number = number_value;
    return value;
}

static value_t create_boolean(bool boolean_value) {
    value_t value;
    value.type = TYPE_BOOLEAN;
    value.as.boolean = boolean_value;
    return value;
}

static value_t create_string(char* string_value) {
    value_t value;
    value.type = TYPE_STRING;
    value.as.string = string_value;
    return value;
}

static value_t create_array(int size) {
    value_t value;
    value.type = TYPE_ARRAY;
    array_t* array = array_init(size);
    value.as.array = *array;
    return value;
}

static bool compare_number(number_t expected, number_t actual, char* test_name, int index) {
    if (expected != actual) {
        printf("\033[31mFAILED:\033[0m (%s) expected number value to be %.2f, got %.2f (%d. value)\n", test_name, expected, actual, index);
        return false;
    }

    return true;
}

static bool compare_boolean(boolean_t expected, boolean_t actual, char* test_name, int index) {
    if (expected != actual) {
        printf("\033[31mFAILED:\033[0m (%s) expected boolean value to be %d, got %d (%d. value)\n", test_name, expected, actual, index);
        return false;
    }

    return true;
}

static bool compare_string(string_t expected, string_t actual, char* test_name, int index) {
    if (strcmp(expected, actual) != 0) {
        printf("\033[31mFAILED:\033[0m (%s) expected string value to be \"%s\", got \"%s\" (%d. value)\n", test_name, expected, actual, index);
        return false;
    }

    return true;
}

static bool compare_array(array_t expected, array_t actual, char* test_name, int index) {
    if (expected.size != actual.size) {
        printf("\033[31mFAILED:\033[0m (%s) expected array size to be \"%d\", got \"%d\" (%d. value)\n", test_name, expected.size, actual.size, index);
        return false;
    }

    for (int i = 0; i < expected.size; i++) {
        value_t expected_element = expected.elements[i];
        value_t actual_element = actual.elements[i];

        if (expected_element.type != actual_element.type) {
            printf("\033[31mFAILED:\033[0m (%s) expected value type to be %d, got %d (%d. value)\n", test_name, expected_element.type, actual_element.type, index);
            return false;
        }

        switch (expected_element.type) {
            case TYPE_NUMBER: {
                bool result = compare_number(expected_element.as.number, actual_element.as.number, test_name, index);
                if (!result) return false;
                break;
            }
            case TYPE_BOOLEAN: {
                bool result = compare_boolean(expected_element.as.boolean, actual_element.as.boolean, test_name, index);
                if (!result) return false;
                break;
            }
            case TYPE_STRING: {
                bool result = compare_string(expected_element.as.string, actual_element.as.string, test_name, index);
                if (!result) return false;
                break;
            }
            case TYPE_ARRAY: {
                bool result = compare_array(expected_element.as.array, actual_element.as.array, test_name, index);
                if (!result) return false;
                break;
            }
            default: {
                printf("\033[31mERROR:\033[0m Unknown datatype in tests, cannot compare (%d. value)\n", index);
                exit(1);
                break;
            }
        }
    }

    return true;
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
            printf("\033[31mFAILED:\033[0m (%s) expected value type to be %d, got %d (%d. value)\n", test_name, expected.type, actual.type, i + 1);
            return;
        }

        switch (expected.type) {
            case TYPE_NUMBER: {
                bool result = compare_number(expected.as.number, actual.as.number, test_name, i + 1);
                if (!result) return;
                break;
            }
            case TYPE_BOOLEAN: {
                bool result = compare_boolean(expected.as.boolean, actual.as.boolean, test_name, i + 1);
                if (!result) return;
                break;
            }
            case TYPE_STRING: {
                bool result = compare_string(expected.as.string, actual.as.string, test_name, i + 1);
                if (!result) return;
                break;
            }
            case TYPE_ARRAY: {
                bool result = compare_array(expected.as.array, actual.as.array, test_name, i + 1);
                if (!result) return;
                break;
            }
            default: {
                printf("\033[31mERROR:\033[0m Unknown datatype in tests, cannot compare (%d. value)\n", i + 1);
                exit(1);
                break;
            }
        }
    }

    tests_passed++;
    printf("\033[32mPASSED:\033[0m (%s), %d assertions\n", test_name, expected_output->count);
    return;
}

int main() {
    printf("\033[32mINFO:\033[0m Starting tests\n");
    printf("--------------------------\n");

    // TEST 01
    {
        output_t* output = output_init();

        double n1 = 2;
        double n2 = 3;
        double n3 = 4;

        output_add(output, create_number(n1 + n2 * n3));
        output_add(output, create_number(n1 * n2 + n3));
        output_add(output, create_number((n1 + n2) * n3));
        output_add(output, create_number(n1 * (n2 + n3)));

        output_add(output, create_number(n1 - n2 / n3));
        output_add(output, create_number(n1 / n2 - n3));
        output_add(output, create_number((n1 - n2) / n3));
        output_add(output, create_number(n1 - (n2 / n3)));

        double a = 12.5;
        double b = 6.2;

        output_add(output, create_number(a + b));
        output_add(output, create_number(a - b));
        output_add(output, create_number(a * b));
        output_add(output, create_number(a / b));

        test("Arithmetic operations", "./tests/cases/case-01-arithmetic-operations.gen", output);
    }

    // TEST 02
    {
        output_t* output = output_init();

        output_add(output, create_boolean(true && true));
        output_add(output, create_boolean(true && false));
        output_add(output, create_boolean(false && true));
        output_add(output, create_boolean(false && false));

        output_add(output, create_boolean(true || true));
        output_add(output, create_boolean(true || false));
        output_add(output, create_boolean(false || true));
        output_add(output, create_boolean(false || false));

        output_add(output, create_boolean(true && (true && false)));
        output_add(output, create_boolean(true && (true || false)));
        output_add(output, create_boolean(true || (true && false)));
        output_add(output, create_boolean(true || (true || false)));
        output_add(output, create_boolean(false || (true && false)));
        output_add(output, create_boolean(false || (true || false)));

        bool a = true;
        bool b = false;

        output_add(output, create_boolean(a && b));
        output_add(output, create_boolean(a || b));

        test("Boolean operations", "./tests/cases/case-02-boolean-operations.gen", output);
    }

    // TEST 03
    {
        output_t* output = output_init();

        output_add(output, create_string("John Doe"));
        output_add(output, create_string("hello"));
        output_add(output, create_string("h"));
        output_add(output, create_string("e"));
        output_add(output, create_string("l"));
        output_add(output, create_string("l"));
        output_add(output, create_string("o"));

        test("String operations", "./tests/cases/case-03-string-operations.gen", output);
    }

    // TEST 04
    {
        output_t* output = output_init();

        value_t array1 = create_array(3);
        array_add_element(&array1.as.array, 0, create_number(1));
        array_add_element(&array1.as.array, 1, create_number(2));
        array_add_element(&array1.as.array, 2, create_number(3));
        output_add(output, array1);

        value_t array2 = create_array(4);
        array_add_element(&array2.as.array, 0, create_number(1));
        array_add_element(&array2.as.array, 1, create_number(2));
        array_add_element(&array2.as.array, 2, create_number(3));
        array_add_element(&array2.as.array, 3, create_number(4));
        output_add(output, array2);

        value_t array3 = create_array(6);
        array_add_element(&array3.as.array, 0, create_number(1));
        array_add_element(&array3.as.array, 1, create_number(2));
        array_add_element(&array3.as.array, 2, create_number(3));
        array_add_element(&array3.as.array, 3, create_number(4));
        array_add_element(&array3.as.array, 4, create_number(5));
        array_add_element(&array3.as.array, 5, create_number(6));
        output_add(output, array3);

        value_t array4 = create_array(2);
        array_add_element(&array4.as.array, 0, create_number(1));
        array_add_element(&array4.as.array, 1, create_number(2));
        output_add(output, array4);

        output_add(output, create_number(1));
        output_add(output, create_number(2));

        output_add(output, create_number(12));
        output_add(output, create_number(13));

        value_t array5 = create_array(3);
        array_add_element(&array5.as.array, 0, create_number(5));
        array_add_element(&array5.as.array, 1, create_number(5));
        array_add_element(&array5.as.array, 2, create_number(5));
        output_add(output, array5);

        value_t array6 = create_array(3);
        array_add_element(&array6.as.array, 0, create_number(6));
        array_add_element(&array6.as.array, 1, create_number(6));
        array_add_element(&array6.as.array, 2, create_number(6));
        output_add(output, array6);

        output_add(output, create_number(1));
        output_add(output, create_number(2));
        output_add(output, create_number(3));
        output_add(output, create_number(4));
        output_add(output, create_number(5));
        output_add(output, create_number(6));

        test("Array operations", "./tests/cases/case-04-array-operations.gen", output);
    }

    // TEST 05
    {
        output_t* output = output_init();

        output_add(output, create_string("John Doe"));
        output_add(output, create_number(25));
        output_add(output, create_string("Downing Street"));
        output_add(output, create_string("London"));

        output_add(output, create_string("Marie Vogelhorn"));
        output_add(output, create_string("London Street"));

        output_add(output, create_string("Michalska"));
        output_add(output, create_string("Bratislava"));

        test("Object operations", "./tests/cases/case-05-object-operations.gen", output);
    }

    printf("--------------------------\n");

    if (tests_passed == tests_total) {
        printf("\033[32mINFO:\033[0m All tests \033[32mPASSED\033[0m\n");
    } else {
        printf("\033[31mINFO:\033[0m %d tests \033[31mFAILED\033[0m\n", tests_total - tests_passed);
    }

    return 0;
}
