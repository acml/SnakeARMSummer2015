#ifndef MAP_STRUCTS
#define MAP_STRUCTS
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
 * Holds all maps necessary for assemble
 */
typedef struct assemble_maps {
    map_t typeMap;
    map_t condMap;
    map_t opcodeMap;
    map_t shiftMap;
    map_t labelMap;
} maps_t;
#endif
