
#include <stdio.h>
#include <string.h>

#include "objects.h"
#include "new_objects.h"

#include "log.h"

void object_free(object_t *obj) {
  if (obj == NULL) return;
  if (obj->kind == STRING){
      free(obj->data.v_string);
  }
  else if (obj->kind == ARRAY) {
      array_t *array = &obj->data.v_array;
      free(array->elements);
  }
  
  int len = strlen(obj->name);
  char* temp = malloc(len+1);
  if (temp == NULL) {
    free(obj->name);
    free(obj);
    return;
  }
  strcpy(temp, obj->name);
  free(obj->name);
  free(obj);
  #ifdef LOG
      printf("Освобождена память из-под объекта %s\n", temp);
  #endif
  free(temp);
}

bool array_set(object_t *array, size_t index, object_t *value) {
  if (array == NULL || value == NULL) {
    return false;
  }

  if (array->kind != ARRAY) {
    return false;
  }

  if (index >= array->data.v_array.size) {
    return false;
  }
  array->data.v_array.elements[index] = value;
  #ifdef LOG
    printf("В объект-массив %s помещен новый объект %s по индексу %lu\n", array->name, value->name, index);
  #endif
  return true;
}

object_t *array_get(object_t *array, size_t index) {
  if (array == NULL) {
    return NULL;
  }
  if (array->kind != ARRAY) {
    return NULL;
  }
  if (index >= array->data.v_array.size) {
    return NULL;
  }
  return array->data.v_array.elements[index];
}

object_t *add(vm_t *vm, object_t *a, object_t *b) {
  if (a == NULL || b == NULL) {
    return NULL;
  }
  switch (a->kind) {
  case INTEGER:
    switch (b->kind) {
    case INTEGER:
      return new_integer(vm, a->data.v_int + b->data.v_int, strcat(a->name, b->name));
    case FLOAT:
      return new_float(vm, (float)a->data.v_int + b->data.v_float, strcat(a->name, b->name));
    default:
      return NULL;
    }
  case FLOAT:
    switch (b->kind) {
    case FLOAT:
      return new_float(vm, a->data.v_float + b->data.v_float, strcat(a->name, b->name));
    default:
      return add(vm, b, a);
    }
  case STRING:
    switch (b->kind) {
    case STRING: {
      int a_len = strlen(a->data.v_string);
      int b_len = strlen(b->data.v_string);
      int len = a_len + b_len + 1;
      char *dst = malloc(len * sizeof(char));
      dst[0] = '\0';

      strcat(dst, a->data.v_string);
      strcat(dst, b->data.v_string);

      object_t *obj = new_string(vm, dst, strcat(a->name, b->name));
      free(dst);

      return obj;
    }
    default:
      return NULL;
    }
  case VECTOR3:
    switch (b->kind) {
    case VECTOR3:
      return new_vector3(
          vm,
          add(vm, a->data.v_vector3.x, b->data.v_vector3.x),
          add(vm, a->data.v_vector3.y, b->data.v_vector3.y),
          add(vm, a->data.v_vector3.z, b->data.v_vector3.z), strcat(a->name, b->name)
      );
    default:
      return NULL;
    }
  case ARRAY:
    switch (b->kind) {
    case ARRAY: {
      size_t a_len = a->data.v_array.size;
      size_t b_len = b->data.v_array.size;
      size_t length = a_len + b_len;

      object_t *array = new_array(vm, length, strcat(a->name, b->name));

      for (int i = 0; i <= a_len; i++) {
        array_set(array, i, array_get(a, i));
      }

      for (int i = 0; i <= b_len; i++) {
        array_set(array, i + a_len, array_get(b, i));
      }

      return array;
    }
    default:
      return NULL;
    }
  default:
    return NULL;
  }
}
