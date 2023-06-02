#include <simpledatabasehandler.h>

int initDatabase(Database *db, const char *name, const char *directory, int numColumns, char **columns)
{
    db->directory = strdup(directory);
    db->name = strdup(name);
    db->numColumns = numColumns;
    db->columns = (char **)malloc(numColumns * sizeof(char *));
    db->rows = 1;
    printf("malloc columns\n");
    for (int i = 0; i < numColumns; i++)
    {
        db->columns[i] = (char *)malloc(MAX_NAME_LENGTH * sizeof(char));
    }
    printf("lock database\n");
    if (lockDatabase(db) != 0)
    {
        return -1; // If database is locked, or locking the database fails, we CANNOT continue on with initializing the database.
    }
    printf("define data\n");
    char ***data = (char ***)malloc(db->rows * sizeof(char **));
    for (int i = 0; i < db->rows; i++)
    {
        data[i] = (char **)malloc(db->numColumns * sizeof(char *));
        for (int j = 0; j < db->numColumns; j++)
        {
            data[i][j] = (char *)malloc(MAX_NAME_LENGTH * sizeof(char));
        }
    }
    db->data = data;
    // Open the database file for reading
    printf("read db file\n");
    char path[MAX_DIRECTORY_LENGTH + MAX_NAME_LENGTH];
    sprintf(path, "%s/%s", directory, name);
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return -1; // Failed to open file
    }

    printf("read database file\n");
    // Read each line of the file and parse the fields
    char line[MAX_LINE_LENGTH];
    int row = 0;
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL)
    {
        char *field = strtok(line, "|");
        int col = 0;
        while (field != NULL)
        {
            strcpy(db->data[row][col], field);
            field = strtok(NULL, "|");
            col++;
        }
        row++;
    }
    db->rows = row;
    db->data = data;
    // Close the file
    fclose(file);
    // Allocate memory for the column names in the database
    db->columns = (char **)malloc(numColumns * sizeof(char *));
for (int i = 0; i < numColumns; i++)
{
    db->columns[i] = columns[i];
}
    return 0;
}

void freeDatabase(Database *db)
{
    unlockDatabase(db);
    for (int i = 0; i < db->numColumns; i++)
    {
        free(db->columns[i]);
    }
    free(db->columns);
    // Free the memory used by the data in the database
    for (int i = 0; i < db->rows; i++)
    {
        for (int j = 0; j < db->numColumns; j++)
        {
            free(db->data[i][j]);
        }
        free(db->data[i]);
    }
    free(db->data);
    free(db);
}

int lockDatabase(Database *db)
{
    printf("%s", db->directory);
    printf("%s", db->name);
    printf("\n");
    char lockFilePath[MAX_DIRECTORY_LENGTH + MAX_NAME_LENGTH + 6];
    sprintf(lockFilePath, "%s/%s.lock", db->directory, db->name);
    db->lockFile = open(lockFilePath, O_CREAT | O_EXCL, 0644);
    if (db->lockFile == -1)
    {
        printf(lockFilePath);
        printf(" database locked\n");
        return -1;
    }
    printf("successful lock");
    return 0;
}

int unlockDatabase(Database *db)
{
    char lockFilePath[MAX_DIRECTORY_LENGTH + MAX_NAME_LENGTH + 6];
    sprintf(lockFilePath, "%s/%s.lock", db->directory, db->name);
    close(db->lockFile);
    remove(lockFilePath);
    return 0;
}

void DB_PUSH(Database *db, char **values)
{
    // Check for null pointers
    if (db == NULL || values == NULL) {
        fprintf(stderr, "Error: Null pointer passed to DB_PUSH\n");
        return;
    }

    // Check that the number of columns is greater than 0
    if (db->numColumns <= 0) {
        fprintf(stderr, "Error: Database has no columns\n");
        return;
    }

    // Check that values has the correct length
    int numValues = 0;
    while (values[numValues] != NULL) {
        numValues++;
    }
    if (numValues != db->numColumns) {
        fprintf(stderr, "Error: Incorrect number of values passed to DB_PUSH\n");
        return;
    }

    // Allocate memory for a new row in the database
    char ***newData = (char ***)malloc((db->rows + 1) * sizeof(char **));
    if (newData == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory for new row\n");
        return;
    }

    // Copy the existing rows into the new data array
    for (int i = 0; i < db->rows; i++) {
        newData[i] = db->data[i];
    }

    // Allocate memory for the new row
    newData[db->rows] = (char **)malloc(db->numColumns * sizeof(char *));
    if (newData[db->rows] == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory for new row\n");
        return;
    }

    // Allocate memory for each string in the new row
    for (int i = 0; i < db->numColumns; i++) {
        newData[db->rows][i] = (char *)malloc(MAX_VALUE_LENGTH * sizeof(char));
        if (newData[db->rows][i] == NULL) {
            fprintf(stderr, "Error: Failed to allocate memory for new row\n");
            return;
        }
    }

    // Copy the values into the new row
    for (int i = 0; i < db->numColumns; i++)
    {
        strcpy(newData[db->rows][i], values[i]);
    }

    // Update the database with the new data array
    db->data = newData;
    db->rows++;
}