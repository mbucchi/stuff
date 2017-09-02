#ifndef IIC2133_STACK_H
#define IIC2133_STACK_H

#include <stdlib.h>

struct stack_node{
    void* content;
    struct stack_node* next;
};

typedef struct stack{
    size_t count;    /* number of elements in the stack */
    struct stack_node* top;
} Stack;





/** Inicializa un stack vacío */
Stack* stack_init();

/** Destruye el stack, liberando la memoria */
void stack_destroy(Stack* stack);

/** Pushea el puntero al top del stack */
void stack_push(Stack* stack, void* value);

/** Poppea el top del stack. NO CHECKEA SI ESTA VACIO */
void* stack_pop(Stack* stack);


#endif //IIC2133_STACK_H
