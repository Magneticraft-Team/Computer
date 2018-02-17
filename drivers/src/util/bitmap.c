//
// Created by cout970 on 11/02/18.
//

#include "util/bitmap.h"

void bitmap_set(Byte *bitmap, Int index, Boolean value) {
    int x = index / 8;
    int y = index % 8;
    if (value) {
        bitmap[x] |= 1 << y;
    } else {
        bitmap[x] &= ~(1 << y);
    }
}

Boolean bitmap_get(Byte *bitmap, Int index) {
    int x = index / 8;
    int y = index % 8;
    return (bitmap[x] & (1 << y)) ? TRUE : FALSE;
}