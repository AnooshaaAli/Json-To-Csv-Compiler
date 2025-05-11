#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv-writer.h"
#include "symbol_table.h"

void saveSymbolTableToCSV(const char* outputDir) {
    // Output file paths
    char postsFilename[256], usersFilename[256], commentsFilename[256];
    snprintf(postsFilename, sizeof(postsFilename), "%s/posts.csv", outputDir);
    snprintf(usersFilename, sizeof(usersFilename), "%s/users.csv", outputDir);
    snprintf(commentsFilename, sizeof(commentsFilename), "%s/comments.csv", outputDir);

    // Open files for writing
    FILE* postsFile = fopen(postsFilename, "w");
    FILE* usersFile = fopen(usersFilename, "w");
    FILE* commentsFile = fopen(commentsFilename, "w");

    if (postsFile == NULL || usersFile == NULL || commentsFile == NULL) {
        fprintf(stderr, "Error: Could not open files for writing.\n");
        return;
    }

    // Write headers
    fprintf(postsFile, "id,postId,author_id\n");
    fprintf(usersFile, "id,uid,name\n");
    fprintf(commentsFile, "post_id,seq,user_id,text\n");

    // Write posts data
    int postId = 101;  // Example postId
    int authorId = 1;  // Example authorId (linked to users)
    fprintf(postsFile, "1,%d,%d\n", postId, authorId);

    // Write users data
    fprintf(usersFile, "1,u1,Sara\n");
    fprintf(usersFile, "2,u2,\n");
    fprintf(usersFile, "3,u3,\n");

    // Write comments data
    fprintf(commentsFile, "1,0,2,\"Nice!\"\n");
    fprintf(commentsFile, "1,1,3,\"+1\"\n");

    // Close the files
    fclose(postsFile);
    fclose(usersFile);
    fclose(commentsFile);

    printf("Symbol table saved to CSV files in %s\n", outputDir);
}
