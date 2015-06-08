#ifndef MAPS
#define MAPS
/*
 * Map functions for operating with map_elem and map_t structures
 */
map_e *mapAllocElem(void);
void mapFreeElem(map_e *elem);
void mapInit(map_t *m);
void mapDestroy(map_t *m);
void mapPut(map_t *m, char *string, uint32_t integer);
uint32_t mapGet(map_t *m, char *string);


/*
 * Initializition of necessary maps
 */
maps_t initMaps(void);
void destroyMaps(maps_t maps);
map_t initCondMap(void);
map_t initOpcodeMap(void);
map_t initShiftMap(void);
map_t initTypeMap(void);
#endif
