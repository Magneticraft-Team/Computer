//
// Created by cout970 on 2017-09-06.
//

#ifndef MAGNETICRAFTCOMPUTER_BOOT_H
#define MAGNETICRAFTCOMPUTER_BOOT_H

// this is needed to mark the main function as the first function in the output file, so the bios can call it
void main (void) __attribute__ ((section (".text.main")));

#endif //MAGNETICRAFTCOMPUTER_BOOT_H
