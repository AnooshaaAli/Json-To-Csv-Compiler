#ifndef CSV_WRITER_H
#define CSV_WRITER_H

char** getUniqueKeys(Table* table, int* uniqueKeyCount);
void saveSymbolTableToMultipleCSVs(const char* outputDir);

#endif