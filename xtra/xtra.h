#ifndef ROSETTA_H
#define ROSETTA_H

#include <stdio.h>

/* title: translate
 * param:
 *   fpin : pointer to input FILE
 * function: performs instruction indexed by the program counter 
 * returns: 1 if successful, 0 if not
 */
extern int xtra(char *name, FILE *fpin, long size);

#endif
