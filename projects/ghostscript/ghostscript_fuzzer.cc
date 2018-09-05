#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <string.h>

#include "iapi.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    static void *minst = nullptr;
    if (minst == nullptr) {
        int code = gsapi_new_instance(&minst, NULL);
        if (code < 0) {
            abort();
        }

        code = gsapi_set_arg_encoding(minst, GS_ARG_ENCODING_UTF8);
        if (code != 0) {
            return 0;
        }


        const char *argv[] = {
            "gs",
            "-q",
            "-dSAFER",
            "-sDEVICE=ppmraw",
            "-sstdout=/dev/null",
            nullptr,
        };
        code = gsapi_init_with_args(minst, sizeof(argv)/sizeof(*argv)-1, const_cast<char**>(argv));
        if (code != 0) {
            abort();
        }
    }

    int code, exit_code;
    gsapi_run_string_begin(minst, 0, &exit_code);
    gsapi_run_string_continue(minst, "(", 1, 0, &exit_code);
    const uint8_t *p = data, *e = data+size;
    while (p != e) {
        const char *needs_escape = "()\\";
        auto *f = std::find_first_of(p, e, needs_escape, needs_escape + strlen(needs_escape));
        gsapi_run_string_continue(minst, reinterpret_cast<const char *>(p),
                                  f - p, 0, &exit_code);
        if (f != e) {
            gsapi_run_string_continue(minst, "\\", 1, 0, &exit_code);
            gsapi_run_string_continue(minst, reinterpret_cast<const char *>(f), 1, 0, &exit_code);
            ++f;
        }
        p = f;
    }
    gsapi_run_string_continue(minst, ") cvx stopped pop", strlen(") stopped pop"), 0, &exit_code);
    code = gsapi_run_string_end(minst, 0, &exit_code);

    if (code != 0) {
        fprintf(stderr, "error occurred: %d\n", code);
        abort();
        gsapi_exit(minst);
        gsapi_delete_instance(minst);
        minst = nullptr;
    }

    return 0;
}
