#ifndef SIMPLE_DATABASE_HANDLER
#define SIMPLE_DATABASE_HANDLER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_NAME_LENGTH 50
#define MAX_DIRECTORY_LENGTH 100
#define MAX_LINE_LENGTH 1024
#define MAX_VALUE_LENGTH 256

typedef struct {
    char *name;
    char *directory;
    int numColumns;
    char **columns;
    int lockFile;
    int rows;
    char ***data;
} Database;

int lockDatabase(Database *db);
int unlockDatabase(Database *db);

int initDatabase(Database *db, const char *name, const char *directory, int numColumns, char *columns[]);
void freeDatabase(Database *db);
void DB_PUSH(Database *db, char **values);

#endif