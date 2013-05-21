/**
 * This module contains functions to create "maps" based on image files. The 
 * information about the map should be stored in a targa file (*.tga).
 */

#define _MAP_C_

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <Glut/glut.h>
#endif
#include "map.h"



/*
 * Function definitions
 */

/*
 * Reads raw data into 1d array file. 
 * @param filename TARGA file name.
 * @param size of the entire data set
 * @return 1d array.
 */
int * mapCreate(char *filename, int nSize) {

    FILE * rawFile;
    int * tempData;
    rawFile = fopen(filename, "r");
    tempData = (int*) malloc(sizeof (int) * nSize);

    if (!rawFile) {
        printf("File could not be opened.\n");
        free(tempData);
        exit(1);
    }
    fread(tempData, 4, nSize, rawFile);
    fclose (rawFile);
    return tempData;

}

/*
 * Function to convert data to little endian
 * @param array of data
 * @param size of the array
 */
void convertLittleEndian(int * data, int size) {

    int i;
    for (i = 0; i < (size); i++) {
        int value = data[i];
        int b0 = (value >> 24) & 0xFF;
        int b1 = (value >> 16) & 0xFF;
        int b2 = (value >> 8) & 0xFF;
        int b3 = value & 0xFF;
        int actualValue = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
        data[i] = actualValue;
    }
}



/*
 * Destroys the map
 */
void mapDestroy(int *map, int height) {

    /* Deallocate the memory */
    if (map == NULL)
        return;

    free(map);
}

/* End of file -------------------------------------------------------------- */




