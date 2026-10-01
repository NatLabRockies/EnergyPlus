/* -------------------------------------------------------------------------
 * stack.c
 * A stack of pointers.
 * Copyright 2010 QTronic GmbH. All rights reserved.
 * -------------------------------------------------------------------------*/

#include "stack.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

Stack *stackNew(int initialSize, int inc)
{
    Stack *s = (Stack *)malloc(sizeof(Stack));
    s->stack = NULL;
    s->stackSize = 0;
    s->stackPos = -1;
    s->initialSize = initialSize;
    s->inc = inc;
    return s;
}

int stackIsEmpty(Stack *s)
{
    return s->stackPos == -1;
}

// add an element to stack and grow stack if required
// returns 1 to indicate success and 0 for error
int stackPushFMI(Stack *s, void *e)
{
    s->stackPos++;
    if (s->stackPos == s->stackSize) {
        s->stackSize += (s->stack ? s->inc : s->initialSize);
        s->stack = (void **)realloc(s->stack, s->stackSize * sizeof(void *));
        if (!s->stack) return 0; // error;
    }
    s->stack[s->stackPos] = e;
    return 1; // success
}

// return top element (possibly NULL), if stack not empty
// runtime error if stack is empty
void *stackPeek(Stack *s)
{
    assert(!stackIsEmpty(s));
    return s->stack[s->stackPos];
}

// remove top element (possibly NULL) from stack and return it
// runtime error if stack is empty
void *stackPopFMI(Stack *s)
{
    assert(!stackIsEmpty(s));
    return s->stack[s->stackPos--];
}

// return the last n elements as null terminated array,
// or NULL if memory allocation fails
void **stackLastPopedAsArray0(Stack *s, int n)
{
    int i;
    void **array = (void **)malloc((n + 1) * sizeof(void *));
    if (!array) return NULL; // failure
    for (i = 0; i < n; i++) {
        array[i] = s->stack[i + s->stackPos + 1];
    }
    array[n] = NULL; // terminating NULL
    return array;
}

// return stack as possibly empty array, or NULL if memory allocation fails
// On sucessful return, the stack is empty.
void **stackPopAllAsArray(Stack *s, int *size)
{
    int i;
    void **array = (void **)malloc((s->stackPos + 1) * sizeof(void *));
    if (!array) return NULL; // failure
    *size = s->stackPos + 1;
    for (i = 0; i < *size; i++)
        array[i] = s->stack[i];
    s->stackPos = -1;
    return array;
}

// release the given stack
void stackFree(Stack *s)
{
    if (s->stack) free(s->stack);
    free(s);
}
