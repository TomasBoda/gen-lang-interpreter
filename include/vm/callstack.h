#ifndef gen_lang_call_stack_h
#define gen_lang_call_stack_h

#include "utils/common.h"
#include "utils/table.h"

typedef struct {
    long ra;
    table_t* table;
} call_frame_t;

typedef struct {
    call_frame_t call_frames[256];
    call_frame_t* call_frame_top;
} call_stack_t;

call_stack_t* call_stack_init();
void call_stack_push(call_stack_t* call_stack, call_frame_t call_frame);
call_frame_t call_stack_pop(call_stack_t* call_stack);
call_frame_t* call_stack_current(call_stack_t* call_stack);

void call_frame_free(call_frame_t* call_frame);
void call_stack_free(call_stack_t* call_stack);

#endif