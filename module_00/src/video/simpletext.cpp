#include <cstdio>
#include <cstdlib>
#include <GL/gl.h>
#include <IL/il.h>
#include <IL/ilut.h>

#include "tgaread.h"
#include "simpletext.h"

// YLO is always 3
// YHI is always 11
struct {
	int xlo,xhi;
} glyphs[] = {
	{   0,	 2 },	// space
	{   8,   9 },	// !
	{  10,  13 },	// "
	{  14,  19 },	// #
	{  20,  25 },	// $
	{  26,  30 },	// %
	{  31,  36 },	// &
	{  42,  44 },	// '
	{  45,  47 },	// (
	{  48,  50 },	// )
	{  51,  55 },	// *
	{  55,  60 },	// +
	{  61,  63 },	// ,
	{  64,  68 },	// -
	{  69,  70 },	// .
	{  71,  75 },	// /
	{  76,  80 },	// 0
	{  81,  83 },	// 1
	{  84,  88 },	// 2
	{  89,  93 },	// 3
	{  94,  98 },	// 4
	{  99, 103 },	// 5
	{ 104, 108 },	// 6
	{ 109, 113 },	// 7
	{ 114, 118 },	// 8
	{ 119, 123 },	// 9
	{ 124, 125 },	// :
	{ 126, 128 },	// ;
	{ 129, 133 },	// <
	{ 134, 138 },	// =
	{ 139, 143 },	// >
	{ 144, 148 },	// ?
	{ 149, 153 },	// @
	{ 154, 158 },	// A
	{ 159, 163 },	// B
	{ 164, 168 },	// C
	{ 169, 173 },	// D
	{ 174, 178 },	// E
	{ 179, 183 },	// F
	{ 184, 188 },	// G
	{ 189, 193 },	// H
	{ 194, 197 },	// I
	{ 198, 202 },	// J
	{ 203, 207 },	// K
	{ 208, 212 },	// L
	{ 213, 218 },	// M
	{ 219, 223 },	// N
	{ 224, 228 },	// O
	{ 229, 233 },	// P
	{ 234, 238 },	// Q
	{ 239, 243 },	// R
	{ 244, 248 },	// S
	{ 249, 254 },	// T
	{ 255, 259 },	// U
	{ 260, 264 },	// V
	{ 265, 270 },	// W
	{ 271, 275 },	// X
	{ 276, 281 },	// Y
	{ 281, 285 },	// Z
	{ 286, 289 },	// [
	{ 290, 294 },	// "\"
	{ 295, 298 },	// ]
	{ 299, 304 },	// ^
	{ 304, 308 },	// _
	{ 309, 311 },	// `
	{ 312, 316 },	// a
	{ 317, 321 },	// b
	{ 322, 326 },	// c
	{ 327, 331 },	// d
	{ 332, 336 },	// e
	{ 337, 340 },	// f
	{ 341, 345 },	// g
	{ 346, 350 },	// h
	{ 351, 352 },	// i
	{ 353, 356 },	// j
	{ 357, 361 },	// k
	{ 362, 363 },	// l
	{ 364, 369 },	// m
	{ 370, 374 },	// n
	{ 375, 379 },	// o
	{ 380, 384 },	// p
	{ 385, 389 },	// q
	{ 390, 394 },	// r
	{ 395, 399 },	// s
	{ 400, 403 },	// t
	{ 404, 408 },	// u
	{ 409, 413 },	// v
	{ 414, 419 },	// w
	{ 420, 424 },	// x
	{ 425, 429 },	// y
	{ 430, 434 },	// z
	{ 435, 438 },	// {
	{ 440, 441 },	// |
	{ 444, 447 },	// }
	{ 448, 452 }	// ~
};

#define GlyphLo	' '
#define GlyphHi	'~'

static GLuint tex_id = 0;
static double tex_sdiv = 0;
static double tex_tdiv = 0;

static void
simpletext_init()
{
	if (tex_id != 0)
		return;

	FILE *fp = fopen("img/cure.tga", "rb");
	if (fp == 0) {
		fprintf(stderr, "Unable to load font image.\n");
		exit(1);
	}
	TGAFile *tga = tgaread(fp);
	fclose(fp);
	if (tga->depth != 8 ||
		(tga->width & (tga->width - 1)) != 0 ||
		(tga->height & (tga->height - 1)) != 0) {
		delete tga;
		fprintf(stderr, "Incorrect image type\n");
		exit(1);
	}

	glPushAttrib(GL_TEXTURE_BIT);
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	glTexImage2D(GL_TEXTURE_2D,
				 0, GL_ALPHA, tga->width, tga->height, 0, GL_ALPHA,
				 GL_UNSIGNED_BYTE, tga->pixels);
	glPopAttrib();

	tex_sdiv = 1. / tga->width;
	tex_tdiv = 1. / tga->height;
	delete tga;
}

int
simpletext(const char *string, int x, int y)
{
	int j;
	char c;
	int glyph, width;
	int pwidth;

	if (tex_id==0)
		simpletext_init();

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);

	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushAttrib(GL_TEXTURE_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_id);

	glPushMatrix();
	glTranslatef(x, y, 0);
	glBegin(GL_QUADS);
	pwidth=0;
	for (j=0; (c=string[j])!=0; j++) {
		if (c < GlyphLo || c > GlyphHi)
			c = '?';
		glyph = c-GlyphLo;

		width = glyphs[glyph].xhi - glyphs[glyph].xlo + 1;

		glTexCoord2d(glyphs[glyph].xlo * tex_sdiv, 15 * tex_tdiv);
		glVertex2i(pwidth, 0);

		glTexCoord2d(glyphs[glyph].xlo * tex_sdiv, 4 * tex_tdiv);
		glVertex2i(pwidth, 11);

		glTexCoord2d(glyphs[glyph].xhi * tex_sdiv, 4 * tex_tdiv);
		glVertex2i(pwidth+width-1, 11);

		glTexCoord2d(glyphs[glyph].xhi * tex_sdiv, 15 * tex_tdiv);
		glVertex2i(pwidth+width-1, 0);

		pwidth += width;
	}
	glEnd();
	glPopMatrix();

	glPopAttrib();	// texture bit
	glPopAttrib();	// blending bit
	glPopAttrib();	// lighting bit

	return pwidth;
}

int
simpletext_size(const char *string)
{
	int size=0;
	char c;

	while ((c = *string++) != 0) {
		if (c<GlyphLo || c>GlyphHi)
			c = '?';
		size += glyphs[c-GlyphLo].xhi - glyphs[c-GlyphLo].xlo + 1;
	}
	return size;
}
