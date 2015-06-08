#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "mapStructs.h"
#include "map.h"

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
            printf("%s not found", string);
            exit(EXIT_FAILURE);
        }
        elem = elem->next;
    }
    return elem->integer;
}



maps_t initMaps(void) {
    maps_t maps;
    maps.typeMap = initTypeMap();
    maps.condMap = initCondMap();
    maps.opcodeMap = initOpcodeMap();
    maps.shiftMap = initShiftMap();
    mapInit(&maps.labelMap);
    return maps;
}

void destroyMaps(maps_t maps) {
    mapDestroy(&maps.typeMap);
    mapDestroy(&maps.condMap);
    mapDestroy(&maps.opcodeMap);
    mapDestroy(&maps.shiftMap);
    mapDestroy(&maps.labelMap);
}

/*
 * TODO:
 */
map_t initTypeMap(void) {
    map_t typeMap;
    mapInit(&typeMap);
    mapPut(&typeMap, "and", 0);
    mapPut(&typeMap, "eor", 0);
    mapPut(&typeMap, "sub", 0);
    mapPut(&typeMap, "rsb", 0);
    mapPut(&typeMap, "add", 0);
    mapPut(&typeMap, "orr", 0);
    mapPut(&typeMap, "mov", 0);
    mapPut(&typeMap, "tst", 0);
    mapPut(&typeMap, "teq", 0);
    mapPut(&typeMap, "cmp", 0);
    mapPut(&typeMap, "lsl", 0);

    mapPut(&typeMap, "mul", 1);
    mapPut(&typeMap, "mla", 1);

    mapPut(&typeMap, "ldr", 2);
    mapPut(&typeMap, "str", 2);

    mapPut(&typeMap, "b", 3);
    mapPut(&typeMap, "bl", 3);

//############~Exetension~############
    mapPut(&typeMap, "bx", 4);
    mapPut(&typeMap, "push", 5);
    mapPut(&typeMap, "pop", 5);

    return typeMap;

}

/*
 * TODO:
 */
map_t initCondMap(void) {
    map_t condMap;
    mapInit(&condMap);
    mapPut(&condMap, "eq", 0x0);
    mapPut(&condMap, "ne", 0x1);
    mapPut(&condMap, "cs", 0x2);
    mapPut(&condMap, "cc", 0x3);
    mapPut(&condMap, "mi", 0x4);
    mapPut(&condMap, "pl", 0x5);
    mapPut(&condMap, "vs", 0x6);
    mapPut(&condMap, "vc", 0x7);
    mapPut(&condMap, "hi", 0x8);
    mapPut(&condMap, "ls", 0x9);
    mapPut(&condMap, "ge", 0xa);
    mapPut(&condMap, "lt", 0xb);
    mapPut(&condMap, "gt", 0xc);
    mapPut(&condMap, "le", 0xd);
    mapPut(&condMap, "al", 0xe);
    mapPut(&condMap, "", 0xe);
    return condMap;
}

/*
 * TODO:
 */
map_t initOpcodeMap(void) {
    map_t opcodeMap;
    mapInit(&opcodeMap);
    mapPut(&opcodeMap, "and", 0x0);
    mapPut(&opcodeMap, "eor", 0x1);
    mapPut(&opcodeMap, "sub", 0x2);
    mapPut(&opcodeMap, "rsb", 0x3);
    mapPut(&opcodeMap, "add", 0x4);
    mapPut(&opcodeMap, "orr", 0xc);
    mapPut(&opcodeMap, "mov", 0xd);
    mapPut(&opcodeMap, "tst", 0x8);
    mapPut(&opcodeMap, "teq", 0x9);
    mapPut(&opcodeMap, "cmp", 0xa);
    return opcodeMap;
}

/*
 * TODO:
 */
map_t initShiftMap(void) {
    map_t shiftMap;
    mapInit(&shiftMap);
    mapPut(&shiftMap, "lsl", 0x0);
    mapPut(&shiftMap, "lsr", 0x1);
    mapPut(&shiftMap, "asr", 0x2);
    mapPut(&shiftMap, "ror", 0x3);
    return shiftMap;
}
