#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv-writer.h"
#include "symbol_table.h"

// Helper function to find unique keys across all rows in a table
char** getUniqueKeys(Table* table, int* uniqueKeyCount) {
    int capacity = 10;
    char** keys = malloc(capacity * sizeof(char*));
    int count = 0;

    for (int i = 0; i < table->rowCount; i++) {
        Row* row = table->rows[i];
        for (int j = 0; j < row->keyCount; j++) {
            char* key = row->keys[j];
            int found = 0;
            for (int k = 0; k < count; k++) {
                if (strcmp(keys[k], key) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                if (count >= capacity) {
                    capacity *= 2;
                    keys = realloc(keys, capacity * sizeof(char*));
                }
                keys[count++] = key;
            }
        }
    }

    *uniqueKeyCount = count;
    return keys;
}

void saveSymbolTableToMultipleCSVs(const char* outputDir) {
    for (int i = 0; i < tableCount; i++) {
        Table* table = tables[i];

        // Build filename like: outputDir/user.csv
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s.csv", outputDir, table->name);

        FILE* fp = fopen(filepath, "w");
        if (fp == NULL) {
            fprintf(stderr, "Error: Could not open file %s for writing.\n", filepath);
            continue;
        }

        int keyCount = 0;
        char** uniqueKeys = getUniqueKeys(table, &keyCount);

        // Write header
        fprintf(fp, "id,parent_id");
        for (int k = 0; k < keyCount; k++) {
            fprintf(fp, ",%s", uniqueKeys[k]);
        }
        fprintf(fp, "\n");

        // Write each row
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];

            // Write id and parent_id
            fprintf(fp, "%d,%d", row->id, row->parentId);

            for (int k = 0; k < keyCount; k++) {
                const char* key = uniqueKeys[k];
                int found = 0;

                for (int m = 0; m < row->keyCount; m++) {
                    if (strcmp(row->keys[m], key) == 0) {
                        const char* value = row->values[m] ? row->values[m] : "";
                        fprintf(fp, ",\"%s\"", value);
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

        free(uniqueKeys);
        fclose(fp);
        printf("Saved: %s\n", filepath);
    }
}
