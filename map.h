/**
 * This module contains functions to create "maps" based on image files. The 
 * information about the map should be stored in a targa file (*.tga).
 */
#ifndef _MAP_H_
#define _MAP_H_

/*
 * Protoypes
 */
int * mapCreate(char *filename, int nSize);
void mapDestroy(int *map, int height);

/* End of file -------------------------------------------------------------- */
#endif

