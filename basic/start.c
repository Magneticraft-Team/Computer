//
// Created by cout970 on 2017-09-03.
//

#include "../driver/api/motherboard.h"

void main(){
    volatile struct motherboard_header *m = (struct motherboard_header *) 0xFFFF0000;

    m->logType = MOTHERBOARD_LOG_TYPE_CHAR;
    m->logByte = 42;
}