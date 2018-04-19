

#include <string.h>
#include <assert.h>
#include <util/input.h>
#include <debug.h>
#include <fs/file.h>
#include "../include/parser.h"

#define BUFFER_SIZE (80-1)

struct ReaderState {
    FD file;
    int index;
    int inPreviousBuffer;
    char *frontBuffer;
    char *backBuffer;
    char bufferA[BUFFER_SIZE];
    char bufferB[BUFFER_SIZE];
} reader;

// Constructor
void rd_init() {
    reader.file = FD_NULL;
    reader.index = 0;
    reader.inPreviousBuffer = 0;

    reader.frontBuffer = reader.bufferA;
    reader.backBuffer = reader.bufferB;

    memset(reader.bufferA, 0, BUFFER_SIZE);
    memset(reader.bufferB, 0, BUFFER_SIZE);
}

struct ReaderState *rd_setInput(FD file) {
    struct ReaderState *state = malloc(sizeof(struct ReaderState));
    memcpy(state, &reader, sizeof(struct ReaderState));

    rd_init();
    reader.file = file;

    return state;
}

void rd_recover(struct ReaderState *oldState) {
    memcpy(&reader, oldState, sizeof(struct ReaderState));
    free(oldState);
}

int rd_readChar() {

    int read = 0;
    int index = reader.index++;
    int character = reader.frontBuffer[index];

    if (character == 0) {

        char *aux = reader.frontBuffer;
        reader.frontBuffer = reader.backBuffer;
        reader.backBuffer = aux;

        if (!reader.inPreviousBuffer) {
            if (reader.file != FD_NULL) {

                if (file_eof(reader.file))
                    return EOF;

                read = file_read(reader.file, reader.frontBuffer, BUFFER_SIZE - 1);

            } else if (reader.file == FD_NULL) {
                if (indentation == 0) {
                    kdebug("> ");
                } else {
                    kdebug("%d > ", indentation);
                }
                read = readString(reader.frontBuffer, BUFFER_SIZE - 1);
            }
            reader.frontBuffer[read] = '\0';
        } else {
            reader.inPreviousBuffer = 0;
        }

        character = reader.frontBuffer[0];
        reader.index = 1;
    }

    return character;
}

void rd_unreadChar() {

    reader.index--;

    if (reader.index < 0) {

        char *aux = reader.frontBuffer;
        reader.frontBuffer = reader.backBuffer;
        reader.backBuffer = aux;

        reader.index = BUFFER_SIZE - 1;
        reader.inPreviousBuffer = 1;
    }
}
