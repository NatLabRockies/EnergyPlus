/* -------------------------------------------------------------------------
 * stack.c
 * A stack of pointers.
 * Copyright 2010 QTronic GmbH. All rights reserved.
 * -------------------------------------------------------------------------*/

#ifndef STACK_H
#define STACK_H

typedef struct
{
    void **stack;
    int stackSize;   // allocated size of stack
    int stackPos;    // array index of top element, -1 if stack is empty.
    int initialSize; // how many element to allocate initially
    int inc;         // how many elements to allocate when stack gets full
} Stack;

Stack *stackNew(int initialSize, int inc);
int stackIsEmpty(Stack *s);
int stackPushFMI(Stack *s, void *e);
void *stackPeek(Stack *s);
void *stackPopFMI(Stack *s);
void **stackPopAllAsArray(Stack *s, int *size);
void **stackLastPopedAsArray0(Stack *s, int n);
void stackFree(Stack *s);

#endif // STACK_H
