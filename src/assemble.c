#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct linked_map {
    char *string;
    uint32_t integer;
    struct linked_map *next;
} map_t;

void put(map_t *root, char *string, uint32_t integer);
uint32_t get(map_t *root, char *string);

int main(int argc, char **argv) {
  return EXIT_SUCCESS;
}

void put(map_t *root, char *string, uint32_t integer) {
    if (root == NULL) {
        root = malloc(sizeof(map_t));
        root->string = string;
        root->integer = integer;
        root->next = NULL;
    }
    while (root->next != NULL) {
        root = root->next;
    }
    map_t *newMap = malloc(sizeof(map_t));
    newMap->string = string;
    newMap->integer = integer;
    newMap->next = NULL;
    root->next = newMap;
}

uint32_t get(map_t *root, char *string) {
    while (strcmp(root->string, string)) {
        root = root->next;
    }
    return root->integer;
}
