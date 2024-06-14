#ifndef gen_lang_call_stack_h
#define gen_lang_call_stack_h

#include "utils/common.h"

/**
 * @brief Object representing a call frame
 * 
 */
typedef struct {
    long ra;
    table_t* table;
} call_frame_t;

/**
 * @brief Object representing a call stack (array of call frames)
 * 
 */
typedef struct {
    call_frame_t call_frames[256];
    call_frame_t* call_frame_top;
} call_stack_t;

/**
 * @brief Initializez a new call stack object
 * 
 * @return call_stack_t* pointer to the initialized call stack object
 */
call_stack_t* call_stack_init();

/**
 * @brief Pushes new call frame to the call stack
 * 
 * @param call_stack call stack object to push the new call frame to
 * @param call_frame call frame to push to the call stack
 */
void call_stack_push(call_stack_t* call_stack, call_frame_t call_frame);

/**
 * @brief Pops a call frame from the call stack
 * 
 * @param call_stack call stack object to pop the call frame from
 * @return call_frame_t popped call frame
 */
call_frame_t* call_stack_pop(call_stack_t* call_stack);

/**
 * @brief Retrieves the current (most recent) call frame from the call stack
 * 
 * @param call_stack call stack to retrieve the call frame from
 * @return call_frame_t* retrieved call frame
 */
call_frame_t* call_stack_current(call_stack_t* call_stack);

/**
 * @brief Frees a call frame from the memory
 * 
 * @param call_frame call frame to free
 */
void call_frame_free(call_frame_t* call_frame);

/**
 * @brief Frees a call stack from the memory
 * 
 * @param call_stack call stack to free
 */
void call_stack_free(call_stack_t* call_stack);

#endif