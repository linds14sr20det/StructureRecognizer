/**
 * This module contains functions to read/manipulate targa files (*.tga).
 */
#ifndef _TGA_H_
#define _TGA_H_

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <Glut/glut.h>
#endif

/*
 * Definitions
 */

/*
 * Constants
 */
#define	TGA_ERROR_FILE_OPEN			-5 
#define TGA_ERROR_READING_FILE		-4 
#define TGA_ERROR_INDEXED_COLOR		-3 
#define TGA_ERROR_MEMORY			-2 
#define TGA_ERROR_COMPRESSED_FILE	-1 
#define TGA_OK						 0 

/*
 * Targa structure
 */
/* OpenGL texture info */
typedef struct
{
  GLsizei width;
  GLsizei height;

  GLenum format;
  GLint	internalFormat;
  GLuint id;

  GLubyte *texels;

} gl_texture_t;

#pragma pack(push, 1)
/* tga header */
typedef struct
{
  GLubyte id_length;          /* size of image id */
  GLubyte colormap_type;      /* 1 is has a colormap */
  GLubyte image_type;         /* compression type */

  short	cm_first_entry;       /* colormap origin */
  short	cm_length;            /* colormap length */
  GLubyte cm_size;            /* colormap size */

  short	x_origin;             /* bottom left x coord origin */
  short	y_origin;             /* bottom left y coord origin */

  short	width;                /* picture width (in pixels) */
  short	height;               /* picture height (in pixels) */

  GLubyte pixel_depth;        /* bits per pixel: 8, 16, 24 or 32 */
  GLubyte image_descriptor;   /* 24 bits = 0x00; 32 bits = 0x80 */

} tga_header_t;
#pragma pack(pop)




/*
 * Prototypes
 */
void GetTextureInfo (tga_header_t *header, gl_texture_t *texinfo);
void ReadTGA8bits (FILE *fp, GLubyte *colormap, gl_texture_t *texinfo);
void ReadTGA16bits (FILE *fp, gl_texture_t *texinfo);
void ReadTGA24bits (FILE *fp, gl_texture_t *texinfo);
void ReadTGA32bits (FILE *fp, gl_texture_t *texinfo);
void ReadTGAgray8bits (FILE *fp, gl_texture_t *texinfo);
void ReadTGAgray16bits (FILE *fp, gl_texture_t *texinfo);
void ReadTGA8bitsRLE (FILE *fp, GLubyte *colormap, gl_texture_t *texinfo);
void ReadTGA16bitsRLE (FILE *fp, gl_texture_t *texinfo);
void ReadTGA24bitsRLE (FILE *fp, gl_texture_t *texinfo);
void ReadTGA32bitsRLE (FILE *fp, gl_texture_t *texinfo);
void ReadTGAgray8bitsRLE (FILE *fp, gl_texture_t *texinfo);
void ReadTGAgray16bitsRLE (FILE *fp, gl_texture_t *texinfo);
gl_texture_t * ReadTGAFile (const char *filename);
GLuint loadTGATexture (const char *filename);
void edgeDetect();


/* End of file -------------------------------------------------------------- */
#endif
