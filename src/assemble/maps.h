#ifndef MAPS_H
#define MAPS_H

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

/*
 * Initializition of necessary maps
 */
maps_t initMaps(void);
void destroyMaps(maps_t maps);

#endif
