#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include "symbol_table.h"  // <-- Add this to make Table known

void writeCSVForTable(FILE *fp, Table *table);
void saveSymbolTableToMultipleCSVs(const char *outputDir);

#endif
