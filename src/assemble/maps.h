#ifndef MAPS_H
#define MAPS_H

#include "map.h"

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

maps_t initMaps(void);
void destroyMaps(maps_t maps);

#endif
