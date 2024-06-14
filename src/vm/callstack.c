#include "vm/callstack.h"

call_stack_t* call_stack_init() {
    call_stack_t* call_stack = (call_stack_t*)malloc(sizeof(call_stack_t));
    call_stack->call_frame_top = call_stack->call_frames;
    return call_stack;
}

void call_stack_push(call_stack_t* call_stack, call_frame_t call_frame) {
    *call_stack->call_frame_top = call_frame;
    call_stack->call_frame_top++;
}

call_frame_t* call_stack_pop(call_stack_t* call_stack) {
    call_stack->call_frame_top--;
    return call_stack->call_frame_top;
}

call_frame_t* call_stack_current(call_stack_t* call_stack) {
    if (call_stack->call_frame_top == call_stack->call_frames) {
        return NULL;
    }

    return call_stack->call_frame_top - 1;
}

void call_frame_free(call_frame_t* call_frame) {
    table_free(call_frame->table);
    free(call_frame->table);
    call_frame->table = NULL;
}

void call_stack_free(call_stack_t* call_stack) {
    while (call_stack->call_frame_top != call_stack->call_frames) {
        call_frame_t* call_frame = call_stack_pop(call_stack);
        call_frame_free(call_frame);
    }
}