#include "stack.h"

/** Inicializa un stack vacío */
Stack* stack_init(){
    Stack* stack = malloc(sizeof(Stack));
    stack->count = 0;
    return stack;
}

/** Destruye el stack, liberando la memoria */
void stack_destroy(Stack* stack){
    struct stack_node *current, *next;
    current = stack->top;

    for (int i = 0; i < stack->count; ++i) {
        next = current->next;
        free(current);
        current = next;
    }

    free(stack);
}

/** Pushea el puntero al top del stack */
void stack_push(Stack* stack, void* value){
    struct stack_node *node = malloc(sizeof(struct stack_node));
    node->next = stack->top;
    node->content = value;
    stack->top = node;
    stack->count++;
}

/** Poppea el top del stack. NO CHECKEA SI ESTA VACIO */
void* stack_pop(Stack* stack){
    struct stack_node *node = stack->top;
    void* value = node->content;

    stack->top = node->next;
    free(node);
    stack->count--;
    return value;
}
