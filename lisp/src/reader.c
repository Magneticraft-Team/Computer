

#include <string.h>
#include <assert.h>
#include <util/input.h>
#include <debug.h>

#define BUFFER_SIZE (80-1)

struct {
    int index;
    int inPreviousBuffer;
    char *frontBuffer;
    char *backBuffer;
    char bufferA[BUFFER_SIZE];
    char bufferB[BUFFER_SIZE];
} reader;

// Constructor
void rd_init() {
    reader.index = 0;
    reader.inPreviousBuffer = 0;

    reader.frontBuffer = reader.bufferA;
    reader.backBuffer = reader.bufferB;

    memset(reader.bufferA, 0, BUFFER_SIZE);
    memset(reader.bufferB, 0, BUFFER_SIZE);
}

int rd_readChar() {

    int index = reader.index++;
    int character = reader.frontBuffer[index];

    if (character == 0) {

        char *aux = reader.frontBuffer;
        reader.frontBuffer = reader.backBuffer;
        reader.backBuffer = aux;

        if (!reader.inPreviousBuffer) {
            kdebug("> ");
            int read = readString(reader.frontBuffer, BUFFER_SIZE - 1);
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
