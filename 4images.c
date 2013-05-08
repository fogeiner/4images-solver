#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <assert.h>
#include <ctype.h>


#define LETTERS_COUNT 26
#define DEBUG_FLAG 0
#define DEBUG(X) if(DEBUG_FLAG) {fprintf(stderr, "%s\n", X);}
#define DEBUG_FUNCTION_ENTRY if(DEBUG_FLAG){fprintf(stderr, "%s:%d (entry)\n", __FUNCTION__, __LINE__);}
#define DEBUG_FUNCTION_EXIT if(DEBUG_FLAG){fprintf(stderr, "%s:%d (exit)\n", __FUNCTION__, __LINE__);}

static sqlite3 *create_db(char *db_path) {
    DEBUG_FUNCTION_ENTRY;
    int rc;
    sqlite3 *db;

    rc = sqlite3_open(db_path, &db);
    assert(rc == SQLITE_OK);

    DEBUG_FUNCTION_EXIT;
    return db;
}

static void create_table(sqlite3 *db) {
    DEBUG_FUNCTION_ENTRY;

    int rc;
    char *error;

    const char sql[] = {
        "DROP TABLE IF EXISTS words;"
        "CREATE TABLE words (\
        word VARCHAR(30) NOT NULL UNIQUE,\
        length INTEGER NOT NULL,\
        a INTEGER, b INTEGER, c INTEGER, d INTEGER,\
        e INTEGER, f INTEGER, g INTEGER, h INTEGER,\
        i INTEGER, j INTEGER,\
        k INTEGER, l INTEGER, m INTEGER, n INTEGER,\
        o INTEGER, p INTEGER, q INTEGER, r INTEGER,\
        s INTEGER, t INTEGER, u INTEGER, v INTEGER,\
        w INTEGER, x INTEGER, y INTEGER, z INTEGER);"};

    rc = sqlite3_exec(db, sql, NULL, NULL, &error);
    assert(rc == SQLITE_OK);

    DEBUG_FUNCTION_EXIT;
}

static void get_stats(const char *word, int *length, int s[]) {
    DEBUG_FUNCTION_ENTRY;
    int i;

    if (length != NULL) {
        *length = 0;
    }
    for (i = 0; i < LETTERS_COUNT; ++i) {
        s[i] = 0;
    }

    for (i = 0; i < strlen(word); ++i) {
        char symbol = word[i];
        if (isalpha(symbol)) {
            s[tolower(symbol) - 'a']++;
            if (length != NULL) {
                (*length)++;
            }
        }
    }

    DEBUG_FUNCTION_EXIT;
}

static void populate_table(sqlite3 *db, const char *filename) {
    DEBUG_FUNCTION_ENTRY;
    
    int words_read = 0;

    int rc;
    int i;
    ssize_t read;

    sqlite3_stmt *stmt;
    char sql[] = "INSERT INTO words VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    assert(rc == SQLITE_OK);
    
    FILE *file;
    file = fopen(filename, "r");
    assert(file != NULL);

    size_t length_line = 0;
    int length = 0;
    int s[LETTERS_COUNT];
    char *line = NULL;

    while ((read = getline(&line, &length_line, file)) != -1) {
        line[read - 1] = '\0';

        words_read++;
        if (words_read % 1000 == 0) {
            printf("Read %d words; current: %s\n", words_read, line);
        }


        get_stats(line, &length, s);

        for(i = 1; i <= 28; ++i) {
            if (i == 1) {
                rc = sqlite3_bind_text(stmt, i, line, -1, SQLITE_TRANSIENT);
            } else if (i == 2) {
                rc = sqlite3_bind_int(stmt, i, length);
            } else {
                rc = sqlite3_bind_int(stmt, i, s[i - 3]);
            }
            assert(rc == SQLITE_OK);
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "Error adding word %s (%d): %s\n", line, sqlite3_errcode(db), sqlite3_errmsg(db));
        }
        sqlite3_reset(stmt);
    }

    free(line);
    fclose(file);
    sqlite3_finalize(stmt);
    DEBUG_FUNCTION_EXIT;
}

static void init(char *db_path, char *dict_path) {
    DEBUG_FUNCTION_ENTRY;
    sqlite3 *db = create_db(db_path);
    create_table(db);
    populate_table(db, dict_path);
    sqlite3_close(db);
    DEBUG_FUNCTION_EXIT;
}

static void find_words(char *db_path, int length,  char letters[]) {
    DEBUG_FUNCTION_ENTRY;
    int rc;
    int i;
    sqlite3 *db = create_db(db_path);
    sqlite3_stmt *stmt;
    char sql[] = "SELECT word from words WHERE length = ? AND "
        "a <= ? AND b <= ? AND c <= ? AND "
        "d <= ? AND e <= ? AND f <= ? AND "
        "g <= ? AND h <= ? AND i <= ? AND "
        "j <= ? AND k <= ? AND l <= ? AND "
        "m <= ? AND n <= ? AND o <= ? AND "
        "p <= ? AND q <= ? AND r <= ? AND "
        "s <= ? AND t <= ? AND u <= ? AND "
        "v <= ? AND w <= ? AND x <= ? AND "
        "y <= ? AND z <= ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    int s[LETTERS_COUNT];
    get_stats(letters, NULL, s);
    for (i = 1 ; i <= 27; ++i) {
        if (i == 1) {
            rc = sqlite3_bind_int(stmt, i, length);
        } else {
            rc = sqlite3_bind_int(stmt, i, s[i - 2]);
        }
        assert(rc == SQLITE_OK);
    }

    do {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            const char *word = (const char*)sqlite3_column_text(stmt, 0);
            printf("%s\n", word);
        } else if (rc != SQLITE_DONE) {
            fprintf(stderr, "Error searching db: %s\n", sqlite3_errmsg(db));
            break;
        }
    } while (rc != SQLITE_DONE);
    

    sqlite3_finalize(stmt);

    DEBUG_FUNCTION_EXIT;
}

int main(int argc, char *argv[]) {
    DEBUG_FUNCTION_ENTRY;
    if (argc == 4 && !strcmp(argv[1], "init")) { 
        DEBUG("main init")
        char *db_path = argv[2];
        char *dict_path = argv[3];
        init(db_path, dict_path);
    } else if (argc == 4) {
        DEBUG("main word_guess")
        char *db_path = argv[1];
        int length = atoi(argv[2]);
        char *letters = argv[3];
        find_words(db_path, length, letters);
    } else {
        DEBUG("main error_input");
        fprintf(stderr, "Usage:\t%s init <path_to_db> <path_to_dict>\n\t%s <path_to_db> <length> <letters>\n", argv[0], argv[0]);
        return -1;
    }

    DEBUG_FUNCTION_EXIT;
    return 0;
}

