#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv-writer.h"
#include "symbol_table.h"

// Helper function to write a CSV for a single table
void writeCSVForTable(FILE *fp, Table *table) {
    // Write header based on the first row
    fprintf(fp, "id");
    for (int i = 0; i < table->rowCount; i++) {
        Row* row = table->rows[i];
        for (int j = 0; j < row->keyCount; j++) {
            if (j > 0) {
                fprintf(fp, ",");
            }
            fprintf(fp, "%s", row->keys[j]);
        }
    }
    fprintf(fp, "\n");

    // Write rows with data
    for (int i = 0; i < table->rowCount; i++) {
        Row* row = table->rows[i];
        fprintf(fp, "%d", row->id);
        for (int j = 0; j < row->keyCount; j++) {
            fprintf(fp, ",\"%s\"", row->values[j] ? row->values[j] : "");
        }
        fprintf(fp, "\n");
    }
}

// Function to handle nested objects and arrays and generate multiple CSVs
void saveSymbolTableToMultipleCSVs(const char *outputDir) {
    // Iterate over all tables and create a CSV for each one
    for (int i = 0; i < tableCount; i++) {
        Table* table = tables[i];

        // Generate file name based on the table name
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/%s.csv", outputDir, table->name);

        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
            fprintf(stderr, "Error: Could not open file %s for writing.\n", filename);
            continue;
        }

        // Write CSV for this table
        writeCSVForTable(fp, table);
        fclose(fp);
        printf("CSV saved to %s\n", filename);
    }
}

// Main driver function to save symbol table and create CSVs
void processSymbolTableAndSaveCSV() {
    // Define output directory (ensure it's created ahead of time)
    const char* outputDir = "./output"; // Or any directory you prefer

    // Save symbol table as CSV
    saveSymbolTableToMultipleCSVs(outputDir);
}
