//
//
// Created by cout970 on 15/01/18.
//


#ifndef DRIVER_BASE_H
#define DRIVER_BASE_H

// DEBUG_ENV: This is defined if the program will run in a debug environment, this is opposed to COMPUTER_ENV,
// this uses a implementation of the api that runs on normal PCs and can be debugged.
#ifndef DEBUG_ENV
// This is defined if the code will be compiled to mips and run into the computer or the emulator
#define COMPUTER_ENV
#endif

#ifdef COMPUTER_ENV
// this is needed to mark the main function as the first function in the output file, so the bios can call it
void main (void) __attribute__ ((section (".text.main")));
#endif


#endif //DRIVER_BASE_H
