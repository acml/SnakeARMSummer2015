#include <stdint.h>

#include "map.h"
#include "maps.h"

/*
 * Initialisation of necessary maps
 */
map_t initTypeMap(void);
map_t initCondMap(void);
map_t initOpcodeMap(void);
map_t initShiftMap(void);

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
 * Maps instructions to different types
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

    //##########~Extension~##########
    mapPut(&typeMap, "bl", 3);
    mapPut(&typeMap, "bx", 4);
    mapPut(&typeMap, "push", 5);
    mapPut(&typeMap, "pop", 5);

    return typeMap;
}

/*
 * Maps condition mnemonics to their codes
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
 * Maps opcode mnemonics to their codes
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
 * Maps shift type mnemonics to their codes
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
