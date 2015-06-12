#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "map.h"

/*
 * basic map functions
 */

map_e *mapAllocElem(void);
void mapFreeElem(map_e *elem);

map_e *mapAllocElem(void) {
    map_e *elem = malloc(sizeof(map_e));
    if (elem == NULL) {
        perror("mapAllocElem");
        exit(EXIT_FAILURE);
    }
    return elem;
}

void mapFreeElem(map_e *elem) {
    free(elem->string);
    free(elem);
}

void mapInit(map_t *m) {
    m->head = NULL;
}

void mapDestroy(map_t *m) {
    map_e *elem = m->head;
    while (elem != NULL) {
        map_e *next = elem->next;
        mapFreeElem(elem);
        elem = next;
    }
}

/*
 * Function creates new element with given int value and string representation
 * and insert new element to the head of the map structure
 */
void mapPut(map_t *m, char *string, uint32_t integer) {
    map_e *elem = mapAllocElem();
    elem->string = malloc((strlen(string) + 1) * sizeof(char));
    if (elem->string == NULL) {
        perror("mapPut");
        exit(EXIT_FAILURE);
    }
    strcpy(elem->string, string);
    elem->integer = integer;
    elem->next = m->head;
    m->head = elem;
}

/*
 * Function returns the value in the map structure related with input string
 */
uint32_t mapGet(map_t *m, char *string) {
    map_e *elem = m->head;
    while (strcmp(elem->string, string)) {
        if (elem->next == NULL) {
            printf("%s not found in map.\n", string);
            exit(EXIT_FAILURE);
        }
        elem = elem->next;
    }
    return elem->integer;
}
