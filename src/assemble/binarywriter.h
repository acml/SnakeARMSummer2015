#ifndef BINARYWRITER_H
#define BINARYWRITER_H

void writeBinary(char **argv, uint8_t *memory, uint32_t memoryLength);
void storeWord(uint8_t *memory, uint32_t address, uint32_t word);

#endif
