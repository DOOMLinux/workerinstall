#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void create_dynamic_array(char **array, int *size) {
  *array = NULL;
  *size = 0;
}

void push_to_dynamic_array(char ***array, int *size, char *element) {
  *size += 1; // Increase the size of the array
  *array = (char **)realloc(*array, *size * sizeof(char*)); // Resize the array
  (*array)[*size - 1] = element; // Add the new element to the end of the array
}

bool test_dynamic_array() {
  char **array = NULL;
  int size = 0;

  // Test create_dynamic_array()
  create_dynamic_array(&array, &size);
  if (array != NULL || size != 0) {
    return false;
  }

  // Test push_to_dynamic_array()
  char *element1 = "element1";
  char *element2 = "element2";
  push_to_dynamic_array(&array, &size, element1);
  push_to_dynamic_array(&array, &size, element2);
  if (strcmp(array[0], "element1") != 0 || strcmp(array[1], "element2") != 0 || size != 2) {
    return false;
  }

  // Free the memory allocated for the array
  free(array);

  return true;
}

int main() {
  if (test_dynamic_array()) {
    printf("All tests passed!\n");
  } else {
    printf("Tests failed.\n");
  }

  return 0;
}