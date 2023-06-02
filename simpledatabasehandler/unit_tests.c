#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simpledatabasehandler.h"

void test_initDatabase()
{
    // Initialize a new database
    Database db;
    int numColumns = 2;
    char *columns[] = {"First Name", "Last Name"};
    int result = initDatabase(&db, "test_db", "./", numColumns, columns);

    // Verify that the database was initialized correctly
    if (result != 0) {
        printf("test_initDatabase failed: initDatabase returned %d\n", result);
        exit(1);
    }
    if (strcmp(db.name, "test_db") != 0) {
        printf("test_initDatabase failed: db.name is %s instead of test_db\n", db.name);
        exit(1);
    }
    if (strcmp(db.directory, "/tmp") != 0) {
        printf("test_initDatabase failed: db.directory is %s instead of /tmp\n", db.directory);
        exit(1);
    }
    if (db.numColumns != numColumns) {
        printf("test_initDatabase failed: db.numColumns is %d instead of %d\n", db.numColumns, numColumns);
        exit(1);
    }
    if (strcmp(db.columns[0], "First Name") != 0) {
        printf("test_initDatabase failed: db.columns[0] is %s instead of First Name\n", db.columns[0]);
        exit(1);
    }
    if (strcmp(db.columns[1], "Last Name") != 0) {
        printf("test_initDatabase failed: db.columns[1] is %s instead of Last Name\n", db.columns[1]);
        exit(1);
    }
    if (db.lockFile != -1) {
        printf("test_initDatabase failed: db.lockFile is %d instead of -1\n", db.lockFile);
        exit(1);
    }
    if (db.rows != 0) {
        printf("test_initDatabase failed: db.rows is %d instead of 0\n", db.rows);
        exit(1);
    }
    if (db.data != NULL) {
        printf("test_initDatabase failed: db.data is not NULL\n");
        exit(1);
    }

    // Free the memory used by the database
    freeDatabase(&db);
}

void test_DB_PUSH()
{
    // Initialize a new database
    Database db;
    int numColumns = 2;
    char *columns[] = {"First Name", "Last Name"};
    initDatabase(&db, "test_db", "./", numColumns, columns);

    // Push a new row to the database
    char *values[] = {"John", "Doe"};
    DB_PUSH(&db, values);

    // Verify that the row was added to the database
    if (db.rows != 1) {
        printf("test_DB_PUSH failed: db.rows is %d instead of 1\n", db.rows);
        exit(1);
    }
    if (strcmp(db.data[0][0], "John") != 0) {
        printf("test_DB_PUSH failed: db.data[0][0] is %s instead of John\n", db.data[0][0]);
        exit(1);
    }
    if (strcmp(db.data[0][1], "Doe") != 0) {
        printf("test_DB_PUSH failed: db.data[0][1] is %s instead of Doe\n", db.data[0][1]);
        exit(1);
    }

    // Free the memory used by the database
    freeDatabase(&db);
}

int main()
{
    // Run the unit tests
    test_initDatabase();
    test_DB_PUSH();

    printf("All tests passed!\n");

    return 0;
}