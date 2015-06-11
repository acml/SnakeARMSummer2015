#ifndef MAP_H
#define MAP_H

/*
 * Holds an element of string to integer map
 */
typedef struct map_elem {
    char *string;
    uint32_t integer;
    struct map_elem *next;
} map_e;

/*
 * Holds map
 */
typedef struct map {
    map_e *head;
} map_t;

/*
 * Map functions for operating with map_elem and map_t structures
 */
void mapInit(map_t *m);
void mapDestroy(map_t *m);
void mapPut(map_t *m, char *string, uint32_t integer);
uint32_t mapGet(map_t *m, char *string);

#endif
