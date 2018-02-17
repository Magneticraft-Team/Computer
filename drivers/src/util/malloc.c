//
// Created by cout970 on 11/02/18.
//

/**
 * Original implementation: http://www.flipcode.com/archives/Simple_Malloc_Free_Functions.shtml
 *
 * Simple malloc implementation
 *
 * This is based on a stack, where freeCell is start of free space
 * and when there is no enough space left the stack is iterated from
 * bottom to top searching for a empty spot and joining empty cells,
 * if no cell has enough size returns 0
 */

#include <types.h>

#define WORD_ALIGN(x) ((((x) + 3) >> 2) << 2)

typedef struct {
    union {
        struct {
            Int size:31;
            Int used:1;
        };
        Int raw;
    };
} Cell;

static struct {
    Cell *heapStart;
    Cell *freeCell;
} Heap;

// Compacts the heap util it finds a empty cell with a size greater than the first argument, returns null otherwise
Cell *compact(Int size) {
    Cell *current = Heap.heapStart;
    Cell *bestCell = current;
    Int bestSize = 0;

    while (1) {
        Int cellSize = current->size;

        // Reached end of heap
        if (cellSize == 0) break;

        if (!current->used) {
            // if the cell is not used, add its space to the previous empty cell
            bestSize += cellSize;
        } else {
            // End of consecutive empty cells
            if (bestSize != 0) {
                // Join all empty cells into one
                bestCell->size = bestSize;

                // If this cell is big enough, stop compacting the heap
                if (bestSize >= size) {
                    return bestCell;
                }
            }
            // Advance to the next cell
            bestSize = 0;
            bestCell = (Cell *) ((Int) current + cellSize);
        }
        // Continue to the next cell
        current = (Cell *) ((Int) current + cellSize);
    }

    // Last check for the case when the empty cell is the last cell
    if (bestSize != 0) {
        // Join all empty cells into one
        bestCell->size = bestSize;

        // If this cell is big enough, stop compacting the heap
        if (bestSize >= size) {
            return bestCell;
        }
    }

    // Unable to find a good spot
    return NULL;
}

void free(Ptr address) {
    if (!address) return;
    Cell *cell = (Cell *) ((UInt) address - sizeof(Cell));
    cell->used = 0;
}

Ptr malloc(UInt size) {
    if (size == 0) return NULL;
    // Real size of the memory to alloc
    Int toAlloc = WORD_ALIGN(size + sizeof(Cell));

    if (Heap.freeCell == NULL || Heap.freeCell->size < toAlloc) {
        Heap.freeCell = compact(toAlloc);

        // No free space to alloc this much memory
        if (Heap.freeCell == NULL) return NULL;
    }

    Cell *res = Heap.freeCell;
    Int cellSize = Heap.freeCell->size;

    if (cellSize >= toAlloc + sizeof(Cell)) {
        // Enough space for this alloc and for a new cell
        Heap.freeCell = (Cell *) ((Int) Heap.freeCell + toAlloc);
        Heap.freeCell->size = cellSize - toAlloc;
    } else {
        // Uses all space left

        // Set freeCell to NULL forcing the next allocation to search for another free cell
        // stating at the bottom of the heap
        Heap.freeCell = NULL;
        toAlloc = cellSize;
    }

    res->used = 1;
    res->size = toAlloc;
    return (Ptr) ((Int) res + sizeof(Cell));
}

// If size is not word aligned the size will be increased up to 3 bytes
void initHeap(Ptr start, UInt size) {
    UInt realSize = WORD_ALIGN(size);
    Heap.heapStart = Heap.freeCell = start;
    Heap.heapStart->used = Heap.freeCell->used = 0;
    Heap.heapStart->size = Heap.freeCell->size = realSize - sizeof(Cell);

    // Set last word to 0 , used to detect the end of the heap
    *(Int *) (start + realSize - sizeof(Cell)) = 0;
}
