#ifndef _DYNAMICARRAY_H_
  #define _DYNAMICARRAY_H_

  #include <stdio.h>

  typedef struct {
    int *array;
    size_t used;
    size_t size;
  } Array;

  void initArray(Array *a, size_t initialSize);
  void insertArray(Array *a, int element);
  void freeArray(Array *a);

#endif
