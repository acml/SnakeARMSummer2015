#ifndef BINARY_WRITER
#define BINARY_WRITER
void storeWord(uint8_t *memory, uint32_t address, uint32_t word);
void writeBinary(char **argv, uint8_t *memory, uint32_t memoryLength);
#endif
