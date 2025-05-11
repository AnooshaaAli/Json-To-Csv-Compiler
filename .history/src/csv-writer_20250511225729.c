#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "csv-writer.h"
#include "symbol_table.h"

void saveSymbolTableToMultipleCSVs(const char* outputDir) {
    // Create output directory if it doesn't exist
    struct stat st = {0};
    if (stat(outputDir, &st) == -1) {
        mkdir(outputDir, 0700);
    }

    for (int i = 0; i < tableCount; i++) {
        Table* table = tables[i];

        // Construct filename
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s.csv", outputDir, table->name);

        FILE* fp = fopen(filepath, "w");
        if (!fp) {
            fprintf(stderr, "Error: Could not open file %s for writing.\n", filepath);
            continue;
        }

        // Get unique keys to form the header
        int keyCount = 0;
        char** keys = getUniqueKeys(table, &keyCount);

        // Write header
        fprintf(fp, "id,parent_id");
        for (int k = 0; k < keyCount; k++) {
            fprintf(fp, ",%s", keys[k]);
        }
        fprintf(fp, "\n");

        // Write row data
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];

            // Skip rows with only meta types like "array" or "object"
            int isMetaRow = 0;
            for (int k = 0; k < row->keyCount; k++) {
                if (row->values[k] && 
                    (strcmp(row->values[k], "array") == 0 || strcmp(row->values[k], "object") == 0)) {
                    isMetaRow = 1;
                } else {
                    isMetaRow = 0;
                    break; // at least one real value found
                }
            }
            if (isMetaRow) continue;

            fprintf(fp, "%d,%d", row->id, row->parentId);
            for (int k = 0; k < keyCount; k++) {
                int found = 0;
                for (int m = 0; m < row->keyCount; m++) {
                    if (strcmp(keys[k], row->keys[m]) == 0) {
                        char* value = row->values[m];
                        if (value == NULL || strcmp(value, "null") == 0) {
                            fprintf(fp, ",");
                        } else {
                            fprintf(fp, ",\"%s\"", value);
                        }
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    fprintf(fp, ",");
                }
            }
            fprintf(fp, "\n");
        }

        fclose(fp);
        printf("Saved %s\n", filepath);
    }
}
