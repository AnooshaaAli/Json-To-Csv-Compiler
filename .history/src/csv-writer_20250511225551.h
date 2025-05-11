#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include "symbol_table.h"  // <-- Add this to make Table known

void saveSymbolTableToCSV(const char* filename);
void saveSymbolTableToMultipleCSVs(const char* outputDir);
char** getUniqueKeys(Table* table, int* uniqueKeyCount);

#endif
