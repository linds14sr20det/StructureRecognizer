/**
 * This module is part of the "Terrains" series.
 * Module No.: 056
 *
 * Wireframe/solid model & effect of the scaling parameters.
 * Lights.
 */
#include <stdlib.h>
#include <stdio.h>
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
#include "camera.h"
#include "tgaMagic.h"
#include "map.h"
#include "alg.h"

/*
 * Definitions
 */
#ifndef max
#endif
#define MAP_SIZE 512 // Size Of Our .RAW Height Map ( NEW )

/* 
 * Menu options
 */
enum {
    MENU_AMBIENT_LIGHT = 1,
    MENU_EDGE_DETECT,
    MENU_EXIT
};

/*
 * "Constants"
 */
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const GLfloat BASIC_ANGLE = 5.0f;

/*
 * Global variables
 */

/* Scaling factors */
int DISTANCE_FACTOR = 10;
int HEIGHT_FACTOR = 5;

/* Aspect ratio of the window */
GLdouble aspectRatio;


/* Basic dimension */
int BASIC_DIMENSION;

/* The world size */
int WORLD_SIZE;

/* The map */
int* mapData;
int ** mapArray;
int mapWidth = MAP_SIZE;
int mapHeight = MAP_SIZE;

/* The actual model */
GLfloat **X, **Y, **Z; /* Vertices */
GLfloat **Nx, **Ny, **Nz; /* Normals */
GLfloat **Tx, **Ty, **Tz; /* Normals */

/* The camera */
Camera theCamera;

/* Texture values */
GLuint textureID;
/* texture id for example */
GLuint texId = 0;
GLubyte *image;

/*
 * Protoypes
 */
void RenderScene();
void SetupRC();
void ChangeSize(GLsizei w, GLsizei h);
void KeyPressedStd(unsigned char key, int x, int y);
int BuildPopupMenu();
void SelectFromMenu(int id);
void paintModel();
void initialize();
void finalize();
void setPerspectiveProjection(GLdouble aspectRatio);
void resetCamera(Camera *camera);
void calcModelCoordinates();
void addNormal(M3DVector3f n, int r, int c);
GLfloat** createMatrix(int m, int n);
void destroyMatrix(GLfloat **A, int m);

/*
 * Function definitions
 */

/**
 * Creates a m by n matrix.
 * @pre Number of rows is positive (m>0).
 * @pre Number of columns is positive (n>0).
 * @param m Number of rows in the matrix.
 * @param n Number of columns in the matrix.
 * @return Reference to the matrix.
 */
GLfloat** createMatrix(int m, int n) {
    GLfloat **A;
    int i;

    /* Allocate the memory for the matrix */
    A = (GLfloat**) calloc(m, sizeof (GLfloat*));
    for (i = 0; i < m; i++) {
        A[i] = (GLfloat*) calloc(n, sizeof (GLfloat));
    }

    return A;
}

/**
 * Destroys a matrix. Note: It's not necessary to reset the original
 * pointer to NULL.
 * @pre Valid matrix (non-null).
 * @param m Number of rows in the matrix.
 * @param A Reference to the matrix.
 */
void destroyMatrix(GLfloat **A, int m) {
    int i;

    /* Deallocate the memory of the matrix */
    for (i = 0; i < m; i++) {
        free(A[i]);
    }
    free(A);
}

/* 
 * This function runs the texture loading portion of the program
 * @param the filename of the texture to be applied to the map
 */
void init(const char *filename) {
    /* init OpenGL */
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* load tga texture */
    if (!(texId = loadTGATexture(filename))) {
        exit(-1);
    }
}

/*
 * Initialize the app.
 */
void initialize() {

    float pos;

    /* Create the map */
    mapData = mapCreate("Test.int.raw", MAP_SIZE * MAP_SIZE);

    /*Convert to little Endian*/
    convertLittleEndian(mapData, MAP_SIZE * MAP_SIZE);

    /*Take the 1d array and split it into a 2d array*/
    //Malloc for this
    mapArray = (int**) malloc(sizeof (int*) * MAP_SIZE);
    int i, j, k;
    for (i = 0; i < MAP_SIZE; i++) {
        mapArray[i] = (int*) malloc(sizeof (int) * MAP_SIZE);
    }
    //Actually put this in the array
    k = 0;
    for (i = 0; i < MAP_SIZE; i++) {
        for (j = 0; j < MAP_SIZE; j++) {
            mapArray[i][j] = (int) mapData[(i * MAP_SIZE) + j];
        }
    }

    /* Print report */
    printf("\n\nTerrain Modeling\n");
    printf("================\n");
    printf("Width: %d\n", mapWidth);
    printf("Height: %d\n", mapHeight);
    printf("No. of triangles: %d\n", ((mapWidth - 1)*(mapHeight - 1)*2));
    printf("No. of vertices: %d\n", (mapWidth * mapHeight));

    /* Calculate basic dimensions */
    WORLD_SIZE = mapWidth * DISTANCE_FACTOR;
    BASIC_DIMENSION = WORLD_SIZE / 100;
    pos = WORLD_SIZE / 2.0f;

    /* Initialize aspect ratio */
    aspectRatio = 1.0;

    /* Initialize the camera */
    resetCamera(&theCamera);

    /* Create the model */
    X = createMatrix(mapHeight, mapWidth);
    Y = createMatrix(mapHeight, mapWidth);
    Z = createMatrix(mapHeight, mapWidth);
    Tx = createMatrix(mapHeight, mapWidth);
    Ty = createMatrix(mapHeight, mapWidth);
    Tz = createMatrix(mapHeight, mapWidth);
    Nx = createMatrix(mapHeight, mapWidth);
    Ny = createMatrix(mapHeight, mapWidth);
    Nz = createMatrix(mapHeight, mapWidth);

    /* Calculate the model coordinates */
    calcModelCoordinates();
}

/*
 * "Finalize" the app.
 */
void finalize() {
    /* Destroy the map */
    mapDestroy(mapData, mapHeight);

    /* Destroy the model */
    destroyMatrix(X, mapHeight);
    destroyMatrix(Y, mapHeight);
    destroyMatrix(Z, mapHeight);
    destroyMatrix(Nx, mapHeight);
    destroyMatrix(Ny, mapHeight);
    destroyMatrix(Nz, mapHeight);
}

/*Close out the program*/
void ShutdownRC(void) {
    glDeleteTextures(1, &textureID);
}

/*
 * Resets the camera.
 * @param camera object
 */
void resetCamera(Camera *camera) {
    M3DVector4f k = {0.0f, 0.0f, -1.0f, 0.0f};

    /* Initialize the camera */
    m3dLoadVector3f(camera->position, 0.0f, 0.0f, WORLD_SIZE);
    m3dLoadVector3f(camera->target, 0.0f, 0.0f, 0.0f);

    m3dLoadIdentity44f(camera->basis);
    m3dSetMatrixColumn44f(camera->basis, k, 2);

    cmrMove(camera, Y_GLOBAL_AXIS, 6500);
    cmrMove(camera, Z_GLOBAL_AXIS, -5000);
    cmrTurn(camera, X_LOCAL_AXIS, -90.0f);
}

/*
 * Add the contribution to the normal vector.
 * @param vector normal
 * @param coord value
 * @param coord value
 */
void addNormal(M3DVector3f n, int r, int c) {
    M3DVector3f nc, nr;

    m3dLoadVector3f(nc, Nx[r][c], Ny[r][c], Nz[r][c]);
    m3dAddVectors3f(nr, nc, n);
    m3dNormalizeVector3f(nr);
    Nx[r][c] = m3dGetVectorX(nr);
    Ny[r][c] = m3dGetVectorY(nr);
    Nz[r][c] = m3dGetVectorZ(nr);
}

/*
 * Calculates the model coordinates 
 */
void calcModelCoordinates() {
    M3DVector3f v1, v2, v3, v4, n;
    GLfloat dx = -(mapWidth * DISTANCE_FACTOR) / 2.0f;
    GLfloat dz = -(mapHeight * DISTANCE_FACTOR) / 2.0f;
    int i, j;

    /* Vertices */
    for (i = 0; i < MAP_SIZE; i++) {
        for (j = 0; j < MAP_SIZE; j++) {
            X[i][j] = j * DISTANCE_FACTOR + dx;
            Y[i][j] = mapArray[i][j] / 300;
            Z[i][j] = i * DISTANCE_FACTOR + dz;
            Nx[i][j] = Ny[i][j] = Nz[i][j] = 0.0f;
        }
    }

    /* Texture Vertices*/
    for (i = 0; i < mapHeight; i++) {
        for (j = 0; j < mapWidth; j++) {
            Tx[i][j] = j;
            Ty[i][j] = mapArray[i][j];
            Tz[i][j] = i;
        }
    }

    /* Normals */
    for (i = 0; i < mapHeight - 1; i++) {
        for (j = 0; j < mapWidth - 1; j++) {
            m3dLoadVector3f(v1, X[i][j], Y[i][j], Z[i][j]);
            m3dLoadVector3f(v2, X[i + 1][j], Y[i + 1][j], Z[i + 1][j]);
            m3dLoadVector3f(v3, X[i][j + 1], Y[i][j + 1], Z[i][j + 1]);
            m3dLoadVector3f(v4, X[i + 1][j + 1], Y[i + 1][j + 1], Z[i + 1][j + 1]);

            /* First triangle */
            m3dFindNormal3f(n, v1, v2, v3);
            addNormal(n, i, j);
            addNormal(n, i + 1, j);
            addNormal(n, i, j + 1);

            /* Second triangle */
            m3dFindNormal3f(n, v3, v2, v4);
            addNormal(n, i, j + 1);
            addNormal(n, i + 1, j);
            addNormal(n, i + 1, j + 1);
        }
    }
}

/*
 * Paints the model.
 */
void paintModel() {
    int i, j;

    glPushMatrix();

    glPushMatrix();
    glPopMatrix();

    glColor3f(0.8f, 0.6f, 0.6f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId);

    for (i = 0; i < mapHeight - 1; i++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (j = 0; j < mapWidth; j++) {
            glNormal3f(Nx[i][j], Ny[i][j], Nz[i][j]);
            glTexCoord2f((float) j / (float) mapWidth, (float) i / (float) mapHeight);
            glVertex3f(X[i][j], Y[i][j], Z[i][j]);

            glNormal3f(Nx[i][j], Ny[i + 1][j], Nz[i + 1][j]);
            glTexCoord2f((float) j / (float) mapWidth, (float) (i + 1) / (float) mapHeight);
            glVertex3f(X[i][j], Y[i + 1][j], Z[i + 1][j]);
        }
        glEnd();
    }
    glPopMatrix();
}

/*
 * Renders the model/scene.
 */
void RenderScene() {
    /* Clear the color buffer and depth buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Draw the object(s) */
    paintModel();

    /* Update the screen */
    glutSwapBuffers();
}

/*
 * Initializes the rendering context.
 */
void SetupRC() {
    /* Set the background color */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /* Set color shading model to flat */
    glShadeModel(GL_SMOOTH);

    /* Counter-clockwise polygons are front facing */
    glFrontFace(GL_CCW);

    /* Enable depth buffer test */
    glEnable(GL_DEPTH_TEST);

    /* Enable culling */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    /* Enable lighting */
    glEnable(GL_LIGHTING);
    GLfloat ambient[] = {1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glEnable(GL_LIGHT0);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    /* Rescale normals to unit length */
    glEnable(GL_NORMALIZE);

}

/*
 * Resizes the window.
 * @param w Width of the window.
 * @param h Height of the window.
 */
void ChangeSize(GLsizei w, GLsizei h) {
    /* Prevent a divide by zero, when window is too short you cant make a
     * window of zero width)
     */
    if (h == 0)
        h = 1;

    /* Set the viewport to be the entire window */
    glViewport(0, 0, w, h);

    /* Calculate the aspect ratio of the window */
    aspectRatio = (GLfloat) w / (GLfloat) h;

    /* Set the projection */
    setPerspectiveProjection(aspectRatio);

    /* Look at... */
    cmrLookAt(&theCamera);
}

/*
 * Sets the perspective projection.
 * @param aspectRatio Aspect ratio of the window.
 */
void setPerspectiveProjection(GLdouble aspectRatio) {
    GLdouble zNear = 1.0;
    GLdouble zFar = 5 * WORLD_SIZE;
    GLdouble angle = 45.0;

    /* Select the projection matrix */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* Set the projection */
    gluPerspective(angle, aspectRatio, zNear, zFar);

    /* Select the modelview matrix */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/*
 * Callback used for key pressed event.
 * @param key ASCII code of the key.
 * @param x X-coordinate of the mouse in the window.
 * @param y Y-coordinate of the mouse in the window.
 */
void KeyPressedStd(unsigned char key, int x, int y) {
    /* Check the key pressed */
    if (key >= '1' && key <= '9') {
        BASIC_DIMENSION = (key - '0')*(WORLD_SIZE / 100);
    } else if (key == 'r') {
        /* Reset visualization parameters */
        resetCamera(&theCamera);
    } else if (cmrIsCtrlKey(key)) {
        /* Process camera ctrl key */
        cmrProcessCtrlKey(&theCamera, key,
                cmrIsMoveCtrlKey(key) ? BASIC_DIMENSION : BASIC_ANGLE);
    } else if (key == '+') {
        HEIGHT_FACTOR++;
        WORLD_SIZE = max(mapWidth*DISTANCE_FACTOR, 255 * HEIGHT_FACTOR);
        BASIC_DIMENSION = WORLD_SIZE / 100;
        calcModelCoordinates();
        setPerspectiveProjection(aspectRatio);
    } else if (key == '-') {
        HEIGHT_FACTOR--;
        if (HEIGHT_FACTOR <= 1)
            HEIGHT_FACTOR = 1;
        WORLD_SIZE = max(mapWidth*DISTANCE_FACTOR, 255 * HEIGHT_FACTOR);
        BASIC_DIMENSION = WORLD_SIZE / 100;
        calcModelCoordinates();
        setPerspectiveProjection(aspectRatio);
    } else if (key == '>') {
        DISTANCE_FACTOR++;
        WORLD_SIZE = max(mapWidth*DISTANCE_FACTOR, 255 * HEIGHT_FACTOR);
        BASIC_DIMENSION = WORLD_SIZE / 100;
        calcModelCoordinates();
        setPerspectiveProjection(aspectRatio);
    } else if (key == '<') {
        DISTANCE_FACTOR--;
        if (DISTANCE_FACTOR <= 10)
            DISTANCE_FACTOR = 10;
        WORLD_SIZE = max(mapWidth*DISTANCE_FACTOR, 255 * HEIGHT_FACTOR);
        BASIC_DIMENSION = WORLD_SIZE / 100;
        calcModelCoordinates();
        setPerspectiveProjection(aspectRatio);
    } else if (key == 'x') {
        exit(0);
    } else {
        return;
    }
    cmrLookAt(&theCamera);
    glutPostRedisplay();
}

/*
 * Menu callback.
 * @param id Item id.
 */
void SelectFromMenu(int id) {
    /* Check item id */
    switch (id) {
        case MENU_EDGE_DETECT:
            edgeDetect();
            break;
        case MENU_EXIT:
            exit(0);
    }

    /* Redisplay the model */
    glutPostRedisplay();
}

/*
 * Creates the popup menu and its items.
 * @return Id of the popup menu.
 */
int BuildPopupMenu() {
    int menu;

    /* Creates the menu */
    menu = glutCreateMenu(SelectFromMenu);
    glutAddMenuEntry("Edge Detect", MENU_EDGE_DETECT);
    glutAddMenuEntry("Exit", MENU_EXIT);

    return menu;
}

/*
 * Main function.
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return Status code.
 */
int main(int argc, char **argv) {
    /* Initialize the app */
    initialize();

    /* Initialization process */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Edge Detect");
    init("knee.tga");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutKeyboardFunc(KeyPressedStd);
    BuildPopupMenu();
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    SetupRC();

    /* "Main" loop */
    glutMainLoop();

    /* Finalize the app */
    finalize();

    ShutdownRC();
    return EXIT_SUCCESS;
}
/* End of file -------------------------------------------------------------- */




