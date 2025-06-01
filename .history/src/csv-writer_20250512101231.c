#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h> // For basename
#include "csv-writer.h"
#include "symbol_table.h"

// Helper function to escape CSV strings (handles quotes and commas)
static char* escape_csv_string(const char* input) {
    if (!input) return strdup("");
    size_t len = strlen(input);
    size_t new_len = len + 1; // Null terminator
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '"' || input[i] == ',') new_len++;
    }
    char* result = malloc(new_len);
    if (!result) return strdup("");
    size_t j = 0;
    result[j++] = '"';
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '"') result[j++] = '"';
        result[j++] = input[i];
    }
    result[j++] = '"';
    result[j] = '\0';
    return result;
}

// Helper function to construct file path
static char* construct_file_path(const char* out_dir, const char* table_name) {
    char* filename = malloc(strlen(table_name) + 6); // .csv + null
    if (!filename) return NULL;
    sprintf(filename, "%s.csv", table_name);

    if (out_dir && strcmp(out_dir, ".") != 0) {
        char* filepath = malloc(strlen(out_dir) + strlen(filename) + 2); // / + null
        if (!filepath) {
            free(filename);
            return NULL;
        }
        sprintf(filepath, "%s/%s", out_dir, filename);
        free(filename);
        return filepath;
    }
    return filename;
}

void saveSymbolTableToCSV(const char* out_dir) {
    if (!out_dir) out_dir = "."; // Default to current directory

    // Iterate through all tables
    for (int i = 0; i < tableCount; i++) {
        Table* table = tables[i];
        if (!table->rowCount || !table->rows) continue; // Skip empty tables

        // Construct file path
        char* filepath = construct_file_path(out_dir, table->name);
        if (!filepath) {
            fprintf(stderr, "Error: Memory allocation failed for file path.\n");
            continue;
        }

        // Open file
        FILE* fp = fopen(filepath, "w");
        if (!fp) {
            fprintf(stderr, "Error: Could not open file %s for writing.\n", filepath);
            free(filepath);
            continue;
        }

        // Collect unique keys and determine if parent_id is needed
        int has_parent = 0;
        for (int j = 0; j < table->rowCount; j++) {
            if (table->rows[j]->parentId != 0) {
                has_parent = 1;
                break;
            }
        }

        // Write header
        fprintf(fp, "id");
        if (has_parent) fprintf(fp, ",%s_id", table->parentName ? table->parentName : "parent");
        for (int k = 0; k < table->rows[0]->keyCount; k++) {
            char* escaped_key = escape_csv_string(table->rows[0]->keys[k]);
            fprintf(fp, ",%s", escaped_key);
            free(escaped_key);
        }
        fprintf(fp, "\n");

        // Write rows
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];
            fprintf(fp, "%d", row->id);
            if (has_parent) fprintf(fp, ",%d", row->parentId);
            for (int k = 0; k < row->keyCount; k++) {
                char* escaped_value = escape_csv_string(row->values[k]);
                fprintf(fp, ",%s", escaped_value);
                free(escaped_value);
            }
            fprintf(fp, "\n");
            // Flush to handle large files
            if (fflush(fp) != 0) {
                fprintf(stderr, "Error: Failed to write to %s.\n", filepath);
                break;
            }
        }

        fclose(fp);
        printf("Table %s saved to %s\n", table->name, filepath);
        free(filepath);
    }
}