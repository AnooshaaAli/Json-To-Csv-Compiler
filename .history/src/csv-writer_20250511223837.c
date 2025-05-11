#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv-writer.h"
#include "symbol_table.h"

void saveSymbolTableToCSV(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file %s for writing.\n", filename);
        return;
    }

    // Write CSV header
    fprintf(fp, "TableName,RowID,ParentID,Key,Value\n");

    // Iterate through all tables
    for (int i = 0; i < tableCount; i++) {
        Table* table = tables[i];
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];
            for (int k = 0; k < row->keyCount; k++) {
                // Escape commas and quotes in keys and values
                char* key = row->keys[k] ? row->keys[k] : "";
                char* value = row->values[k] ? row->values[k] : "";
                
                // Write row data
                fprintf(fp, "\"%s\",%d,%d,\"%s\",\"%s\"\n",
                        table->name,
                        row->id,
                        row->parentId,
                        key,
                        value);
            }
        }
    }

    fclose(fp);
    printf("Symbol table saved to %s\n", filename);
}