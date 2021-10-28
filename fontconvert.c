/*
TrueType to Adafruit_GFX font converter.  Derived from Peter Jakobs'
Adafruit_ftGFX fork & makefont tool, and Paul Kourany's Adafruit_mfGFX.

NOT AN ARDUINO SKETCH.  This is a command-line tool for preprocessing
fonts to be used with the Adafruit_GFX Arduino library.

For UNIX-like systems.  Outputs to stdout; redirect to header file, e.g.:
  ./fontconvert ~/Library/Fonts/FreeSans.ttf --size=18 --dpi=141 --progmem > FreeSans18pt7b.h

REQUIRES FREETYPE LIBRARY.  www.freetype.org

See notes at end for glyph nomenclature & other tidbits.

Modified by Chernov A.A. <valexlin@gmail.com>
 * Added field bitmapSize to struct GFXfont.
 * Added command line parsing via gnugetopt_long().
 * Added command line argument to specify DPI.
*/
#ifndef ARDUINO

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <ft2build.h>
#include FT_GLYPH_H
#include FT_MODULE_H
#include FT_TRUETYPE_DRIVER_H

#include "gfxfont.h" // patched Adafruit_GFX font structures

#define MAX_S_LEN		512
#define MAX_GLYPH_NAME_LEN	128
#define MAX_NUMBER_STR_SZ	9
#define MAX_RANGE_SZ	16

// Accumulate bits for output, with periodic hexadecimal byte write
void enbit(uint8_t value) {
	static uint8_t row = 0, sum = 0, bit = 0x80, firstCall = 1;
	if (value)
		sum |= bit;          // Set bit if needed
	if (!(bit >>= 1)) {    // Advance to next bit, end of byte reached?
		if (!firstCall) {    // Format output table nicely
			if (++row >= 12) { // Last entry on line?
				printf(",\n  "); //   Newline format output
				row = 0;         //   Reset row counter
			} else {           // Not end of line
				printf(", ");    //   Simple comma delim
			}
		}
		printf("0x%02X", sum); // Write byte value
		sum = 0;               // Clear for next byte
		bit = 0x80;            // Reset bit counter
		firstCall = 0;         // Formatting flag
	}
}

static void print_help() {
	printf("Usage: fontconvert <options> [font_file]\n");
	printf("font_file - path to the font file.\n");
	printf("options:\n");
	printf("--size=<font_size>           |-s        specify font size\n");
	printf("--chars=<f1-l1,f2-l2,cc,...> |-r        specify characters set as range list:\n");
	printf("                                        where f1 - first char codepoint in range 1;\n");
	printf("                                        where l1 - last char codepoint in range 1;\n");
	printf("                                        where f2 - first char codepoint in range 2;\n");
	printf("                                        where l2 - last char codepoint in range 2;\n");
	printf("                                        where cc - one char codepoint to add to range;\n");
	printf("                                        etc...\n");
	printf("--onechar=<code>             |-c        specify only one character\n");
	printf("--ascii                      |-a        specify ACSII mode: one charset range: first=0x20, last=0x7E\n");
	printf("--dpi=<dpi_value>            |-d        specify DPI\n");
	printf("--hinting=[no|bytecode|auto] |-t        specify hinting mode\n");
	printf("--progmem[=1|0|yes|no]       |-d        use 'PROGMEM' specification for font data declarations\n");
	printf("--help                       |-h        show this page and exit.\n");
}

static int my_atoi(const char* str) {
	if (!str || !str[0])
		return -1;
	size_t len = strlen(str);
	char* strcp = 0;
	int res = 0;
	char* endptr = 0;
	if (len > 1 && str[len - 1] == 'h') {
		strcp = strdup(str);
		strcp[len - 1] = 0;
		res = strtol(strcp, &endptr, 16);
	} else
		res = strtol(str, &endptr, 0);
	if (*endptr != 0)
		res = -1;
	if (strcp)
		free(strcp);
	return res;
}

/**
 * @brief Parse string as ranges list into GFXglyphRange array
 * @param ranges destination array of ranges
 * @param str insput string
 * @return count of records if parsed successfully, -1 otherwise.
 *
 * Example of strings that can be used:
 *   0x20-0x7E,0xA9,0xAE
 *   20h-7Eh,A9h,AEh
 *   0x20-0x7E,0x401,0x410-0x44F,0x451,0xA9,0xAE
 */
static int parse_ranges(GFXglyphRange* ranges, const char* str, int max_sz) {
	int res = -1;
	const char* ptr = str;
	int i = 0;
	uint16_t first = 0;
	uint16_t last = 0;
	char number_str[MAX_NUMBER_STR_SZ];
	char* number_str_ins_ptr = number_str;
	int have_errors = 0;
	while (1) {
		if (*ptr == '-') {
			first = (uint16_t)my_atoi(number_str);
			if (first == (uint16_t)-1) {
				have_errors = 1;
				break;
			}
			// prepare for next number
			number_str_ins_ptr = number_str;
		} else if (*ptr == ',' || *ptr == ';' || *ptr == 0) {
			last = my_atoi(number_str);
			if (last == (uint16_t)-1) {
				have_errors = 1;
				break;
			}
			if (last < first) {
				have_errors = 1;
				break;
			}
			if (0 == first)
				first = last;
			ranges[i].first = first;
			ranges[i].last = last;
			// prepare for next number pair
			i++;
			first = 0;
			last = 0;
			number_str_ins_ptr = number_str;
		} else {
			if (number_str_ins_ptr - number_str < MAX_NUMBER_STR_SZ - 1) {
				*number_str_ins_ptr = *ptr;
				number_str_ins_ptr++;
				*number_str_ins_ptr = 0;
			} else {
				have_errors = 1;
				break;
			}
		}
		if (i >= max_sz)
			break;
		if (*ptr == 0)
			break;
		ptr++;
	}
	if (!have_errors)
		res = i;
	return res;
}

static int range_comparator(const void * n1, const void * n2) {
	GFXglyphRange* r1 = (GFXglyphRange*)n1;
	GFXglyphRange* r2 = (GFXglyphRange*)n2;
	if (r1->first > r2->first)
		return 1;
	else if (r1->first < r2->first)
		return -1;
	int r1_sz = r1->last - r1->first + 1;
	int r2_sz = r2->last - r2->first + 1;
	return r1_sz == r2_sz ? 0 : (r1_sz > r2_sz ? 1 : -1);
}

int main(int argc, char *argv[]) {
	int i, j;
	int err;
	int size = 0;
	int one_char = 0;
	int ascii_mode = 0;
	int use_progmem = 0;
	unsigned int bitmapOffset = 0;
	int x, y, byte;
	int dpi = 96;
	int hinting = 0;	// 0 - no, 1 - bytecode, 2 - auto
	char c, *ptr;
	int help_only = 0;
	char filePath[MAX_S_LEN] = { 0 };
	char* fontName;
	FT_Library library;
	FT_Face face;
	FT_Glyph glyph;
	FT_Int32 load_flags;
	FT_Bitmap *bitmap;
	FT_BitmapGlyphRec *g;
	GFXglyph *table;
	GFXglyphRange ranges[MAX_RANGE_SZ];
	int ranges_count = 0;
	int range_specified = 0;
	int chars_count = 0;
	FT_UInt* table_glyphs;
	FT_ULong char_;
	uint8_t bit;
	char glyphName[MAX_GLYPH_NAME_LEN] = { 0 };

	// Unless overridden, default first and last chars are
	// ' ' (space) and '~', respectively

	// parse command line
	while (1) {
		static struct option long_options[] = {
			{"size",    required_argument, 0, 's'},
			{"chars",   required_argument, 0, 'r'},
			{"onechar", required_argument, 0, 'c'},
			{"ascii",   no_argument,       0, 'a'},
			{"dpi",     required_argument, 0, 'd'},
			{"hinting", required_argument, 0, 't'},
			{"progmem", optional_argument, 0, 'p'},
			{"help",    no_argument,       0, 'h'},
			{0, 0, 0, 0}
		};
		int ret;
		/* getopt_long stores the option index here. */
		int option_index = 0;

		ret = getopt_long(argc, argv, "s:f:l:c:ad:t:ph?",
						  long_options, &option_index);

		/* Detect the end of the options. */
		if (ret == -1)
			break;

		switch (ret) {
			case 0:
				break;
			case 's':
				size = atoi(optarg);
				break;
			case 'r':
				ranges_count = parse_ranges(ranges, optarg, MAX_RANGE_SZ);
				range_specified = 1;
				break;
			case 'c':
				one_char = my_atoi(optarg);
				break;
			case 'a':
				ascii_mode = 1;
				break;
			case 'd':
				dpi = atoi(optarg);
				break;
			case 't':
				if (strcasecmp(optarg, "no") == 0)
					hinting = 0;
				else if (strcasecmp(optarg, "bytecode") == 0)
					hinting = 1;
				else if (strcasecmp(optarg, "auto") == 0)
					hinting = 2;
			case 'p':
				if (optarg) {
					if (strcasecmp(optarg, "yes") == 0 || strcmp(optarg, "1") == 0)
						use_progmem = 1;
					else
						use_progmem = 0;
				}
				else
					use_progmem = 1;
				break;
			case 'h':
			case '?':
				help_only = 1;
				break;
			default:
				return -1;
		}
	}
	if (optind < argc) {
		strncpy(filePath, argv[optind++], MAX_S_LEN);
		filePath[MAX_S_LEN - 1] = 0;
	}
	if (help_only) {
		print_help();
		return 0;
	}
	if (filePath[0] == 0) {
		fprintf(stderr, "You must specify path to the font file!\n");
		print_help();
		return 1;
	}
	if (size == 0) {
		fprintf(stderr, "You must specify valid font size!\n");
		print_help();
		return 1;
	}

	if (range_specified) {
		if (ranges_count < 0) {
			fprintf(stderr, "Failed to parse characters set ranges!\n");
			print_help();
			return 1;
		}
		// Sort character set ranges
		qsort(ranges, (size_t)ranges_count, sizeof(GFXglyphRange), range_comparator);
		// validate ranges: check duplicates and/or interceptions
		int have_errors = 0;
		for (i = 1; i < ranges_count; i++) {
			if (ranges[i].first >= ranges[i - 1].first && ranges[i].first <= ranges[i - 1].last) {
				have_errors = 1;
				break;
			} else if (ranges[i].last < ranges[i - 1].last) {
				have_errors = 1;
				break;
			}
		}
		if (have_errors) {
			fprintf(stderr, "In characters set ranges found duplicates or interceptions!\n");
			print_help();
			return 1;
		}
		// Combine consecutive ranges
		for (i = ranges_count - 1; i > 0; i--) {
			if (ranges[i].first == ranges[i - 1].last + 1) {
				ranges[i - 1].last = ranges[i].last;
				for (j = i; j < ranges_count - 1; j++)
					memcpy(&ranges[j], &ranges[j + 1], sizeof(GFXglyphRange));
				ranges_count--;
			}
		}
	}
	if (ascii_mode) {
		if (ranges_count > 0) {
			fprintf(stderr, "In ASCII mode, the character set ranges can't specified!\n");
			print_help();
			return 1;
		}
		if (one_char != 0) {
			fprintf(stderr, "You cannot specify both ASCII mode and single character mode!\n");
			print_help();
			return 1;
		}
		ranges[0].first = 0x20;		// ' ' SPACE
		ranges[0].last = 0x7E;		// '~' TILDE
		ranges_count = 1;
	}
	if (one_char != 0) {
		if (one_char < 0) {
			fprintf(stderr, "Invalid character code!\n");
			print_help();
			return 1;
		}
		if (ranges_count > 0) {
			fprintf(stderr, "In one char mode, the character set ranges can't specified!\n");
			print_help();
			return 1;
		} else {
			ranges[0].first = one_char;
			ranges[0].last = one_char;
			ranges_count = 1;
		}
	}
	if (ranges_count == 0) {
		ranges[0].first = 0x20;		// ' ' SPACE
		ranges[0].last = 0x7E;		// '~' TILDE
		ranges_count = 1;
	}
	if (dpi == 0) {
		fprintf(stderr, "Invalid value of DPI!\n");
		return 1;
	}

	// MONO renderer provides clean image with perfect crop
	// (no wasted pixels) via bitmap struct.
	switch (hinting) {
		case 0:		// no
			load_flags = FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
			break;
		case 1:		// bytecode
			load_flags = FT_LOAD_DEFAULT | FT_LOAD_TARGET_MONO;
			break;
		case 2:		// auto
			load_flags = FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_MONO;
			break;
		default:	// no
			load_flags = FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
			break;
	}

	ptr = strrchr(filePath, '/'); // Find last slash in filename
	if (ptr)
		ptr++; // First character of filename (path stripped)
	else
		ptr = filePath; // No path; font in local dir.

	// Calc table size (characters count)
	chars_count = 0;
	for (i = 0; i < ranges_count; i++)
		chars_count += ranges[i].last - ranges[i].first + 1;

	// Allocate space for font name and glyph table
	if ((!(fontName = malloc(strlen(ptr) + 28))) ||
		(!(table = (GFXglyph *)malloc(chars_count * sizeof(GFXglyph)))) ||
		(!(table_glyphs = (FT_UInt *)malloc(chars_count * sizeof(FT_UInt))))) {
		fprintf(stderr, "malloc error\n");
		return 1;
	}

	// Derive font table names from filename.  Period (filename
	// extension) is truncated and replaced with the font size & bits.
	strcpy(fontName, ptr);
	ptr = strrchr(fontName, '.'); // Find last period (file ext)
	if (!ptr)
		ptr = &fontName[strlen(fontName)]; // If none, append
	if (1 == ranges_count) {
		// Insert font size.  fontName was alloc'd w/extra
		// space to allow this, we're not sprintfing into Forbidden Zone.
		if (ranges[0].first == ranges[0].last)
			sprintf(ptr, "%dpt_char%02X", size, ranges[0].first);
		else if (ranges[0].first == 0x20 && ranges[0].last == 0x7E)
			sprintf(ptr, "%dpt_ascii", size);
		else
			sprintf(ptr, "%dpt_%02X_%02X", size, ranges[0].first, ranges[0].last);
	}
	else
	{
		sprintf(ptr, "%dpt_mixed", size);
	}
	// Space and punctuation chars in name replaced w/ underscores.
	for (i = 0; (c = fontName[i]); i++) {
		if (isspace(c) || ispunct(c))
			fontName[i] = '_';
	}

	// Init FreeType lib, load font
	if ((err = FT_Init_FreeType(&library))) {
		fprintf(stderr, "FreeType init error: %d\n", err);
		return err;
	}

	// Use TrueType engine version 35, without subpixel rendering.
	// This improves clarity of fonts since this library does not
	// support rendering multiple levels of gray in a glyph.
	// See https://github.com/adafruit/Adafruit-GFX-Library/issues/103
	FT_UInt interpreter_version = TT_INTERPRETER_VERSION_35;
	FT_Property_Set(library, "truetype", "interpreter-version",
					&interpreter_version);

	if ((err = FT_New_Face(library, filePath, 0, &face))) {
		fprintf(stderr, "Font load error: %d\n", err);
		FT_Done_FreeType(library);
		return err;
	}

	// << 6 because '26dot6' fixed-point format
	if ((err = FT_Set_Char_Size(face, size << 6, 0, dpi, 0))) {
		fprintf(stderr, "Set font char size error: %d\n", err);
		FT_Done_FreeType(library);
		return err;
	}

	// Always use unicode charmap
	if ((err = FT_Select_Charmap(face, FT_ENCODING_UNICODE))) {
		fprintf(stderr, "Select unicode charmap error: %d\n", err);
		FT_Done_FreeType(library);
		return err;
	}

	// Print header
	printf("/*******************************************************************\n");
	printf(" *  Generated by fontconvert utility:\n");
	printf(" * Font Name: '%s', filepath: '%s'\n", face->family_name, filePath);
	printf(" * Size: %dpt\n", size);
	printf(" * DPI: %d\n", dpi);
	printf(" * Hinting: ");
	switch (hinting) {
		case 0:
			printf("no");
			break;
		case 1:
			printf("bytecode");
			break;
		case 2:
			printf("auto");
			break;
		default:
			printf("no");
			break;
	}
	printf("\n");
	printf("Characters set ranges:\n");
	for (i = 0; i < ranges_count; i++) {
		printf("  %d: 0x%04X - 0x%04X (", i, ranges[i].first, ranges[i].last);
		table_glyphs[0] = FT_Get_Char_Index(face, ranges[i].first);
		if ((err = FT_Get_Glyph_Name(face, table_glyphs[0], glyphName, MAX_GLYPH_NAME_LEN)) == 0)
			glyphName[MAX_GLYPH_NAME_LEN - 1] = 0;
		else
			strcpy(glyphName, "unknown");
		printf("'%s' - ", glyphName);
		table_glyphs[0] = FT_Get_Char_Index(face, ranges[i].last);
		if ((err = FT_Get_Glyph_Name(face, table_glyphs[0], glyphName, MAX_GLYPH_NAME_LEN)) == 0)
			glyphName[MAX_GLYPH_NAME_LEN - 1] = 0;
		else
			strcpy(glyphName, "unknown");
		printf("'%s')\n", glyphName);
	}
	printf(" *******************************************************************/\n");
	printf("\n");

	// Currently all symbols from 'first' to 'last' in all character set ranges are processed.
	// Fonts may contain WAY more glyphs than that, but this code
	// will need to handle encoding stuff to deal with extracting
	// the right symbols, and that's not done yet.
	// fprintf(stderr, "%ld glyphs\n", face->num_glyphs);

	if (use_progmem)
		printf("const uint8_t %s_Bitmaps[] PROGMEM = {\n  ", fontName);
	else
		printf("const uint8_t %s_Bitmaps[] = {\n  ", fontName);

	// Process glyphs and output huge bitmap data array
	j = 0;
	for (i = 0; i < ranges_count; i++) {
		for (char_ = ranges[i].first; char_ <= ranges[i].last; char_++, j++) {
			if ((table_glyphs[j] = FT_Get_Char_Index(face, char_)) == 0) {
				fprintf(stderr, "undefined character code 0x%04X\n", (unsigned int)char_);
				continue;
			}

			if ((err = FT_Load_Glyph(face, table_glyphs[j], load_flags))) {
				fprintf(stderr, "Error %d loading char '0x%04X'\n", err, (unsigned int)char_);
				continue;
			}

			if ((err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO))) {
				fprintf(stderr, "Error %d rendering char '0x%04X'\n", err, (unsigned int)char_);
				continue;
			}

			if ((err = FT_Get_Glyph(face->glyph, &glyph))) {
				fprintf(stderr, "Error %d getting glyph '0x%04X'\n", err, (unsigned int)char_);
				continue;
			}

			bitmap = &face->glyph->bitmap;
			g = (FT_BitmapGlyphRec *)glyph;

			// Minimal font and per-glyph information is stored to
			// reduce flash space requirements.  Glyph bitmaps are
			// fully bit-packed; no per-scanline pad, though end of
			// each character may be padded to next byte boundary
			// when needed.  16-bit offset means 64K max for bitmaps,
			// code currently doesn't check for overflow.  (Doesn't
			// check that size & offsets are within bounds either for
			// that matter...please convert fonts responsibly.)
			table[j].bitmapOffset = bitmapOffset;
			table[j].width = bitmap->width;
			table[j].height = bitmap->rows;
			table[j].xAdvance = face->glyph->advance.x >> 6;
			table[j].xOffset = g->left;
			table[j].yOffset = 1 - g->top;

			for (y = 0; y < bitmap->rows; y++) {
				for (x = 0; x < bitmap->width; x++) {
					byte = x / 8;
					bit = 0x80 >> (x & 7);
					enbit(bitmap->buffer[y * bitmap->pitch + byte] & bit);
				}
			}

			// Pad end of char bitmap to next byte boundary if needed
			int n = (bitmap->width * bitmap->rows) & 7;
			if (n) {     // Pixel count not an even multiple of 8?
				n = 8 - n; // # bits to next multiple
				while (n--)
					enbit(0);
			}
			bitmapOffset += (bitmap->width * bitmap->rows + 7) / 8;

			FT_Done_Glyph(glyph);
		}
	}

	printf(" };\n\n"); // End bitmap array

	// Output glyph attributes table (one per character)
	if (use_progmem)
		printf("const GFXglyph %s_Glyphs[] PROGMEM = {\n", fontName);
	else
		printf("const GFXglyph %s_Glyphs[] = {\n", fontName);
	j = 0;
	for (i = 0; i < ranges_count; i++) {
		for (char_ = ranges[i].first; char_ <= ranges[i].last; char_++, j++) {
			printf("  { %5d, %3d, %3d, %3d, %4d, %4d }", table[j].bitmapOffset,
				   table[j].width, table[j].height, table[j].xAdvance, table[j].xOffset,
				   table[j].yOffset);
			if ((err = FT_Get_Glyph_Name(face, table_glyphs[j], glyphName, MAX_GLYPH_NAME_LEN)) == 0)
				glyphName[MAX_GLYPH_NAME_LEN - 1] = 0;
			else
				glyphName[0] = 0;
			if (i == ranges_count - 1 && char_ == ranges[i].last) {
				printf(" }; // 0x%02X", (unsigned int)char_);
				if (glyphName[0])
					printf(" '%s'", glyphName);
			} else {
				printf(",   // 0x%02X", (unsigned int)char_);
				if (glyphName[0])
					printf(" '%s'", glyphName);
			}
			printf("\n");
		}
	}
	printf("\n");

	// Output characters set range list
	if (use_progmem)
		printf("const GFXglyphRange %s_Ranges[] PROGMEM = {\n", fontName);
	else
		printf("const GFXglyphRange %s_Ranges[] = {\n", fontName);
	for (i = 0; i < ranges_count - 1; i++) {
		printf("  { 0x%04X, 0x%04X },\n", ranges[i].first, ranges[i].last);
	}
	printf("  { 0x%04X, 0x%04X } };\n", ranges[ranges_count - 1].first, ranges[ranges_count - 1].last);
	printf("\n");

	// Output font structure
	if (use_progmem)
		printf("const GFXfont %s PROGMEM = {\n", fontName);
	else
		printf("const GFXfont %s = {\n", fontName);
	printf("  %s_Bitmaps,\n", fontName);
	printf("  %s_Glyphs,\n", fontName);
	printf("  %s_Ranges, %d,\n", fontName, ranges_count);
	printf("  %d,		// characters count\n", chars_count);
	if (face->size->metrics.height == 0) {
		// No face height info, assume fixed width and get from a glyph.
		printf("  %d,		// newline distance in pixels\n", table[0].height);
	} else {
		printf("  %d,		// newline distance in pixels\n", (int)(face->size->metrics.height >> 6));
	}
	printf("  %u };	// bitmap size\n\n", bitmapOffset);
	printf("// Approx. %u bytes\n", (unsigned int)(bitmapOffset + chars_count*sizeof(GFXglyph) + ranges_count*sizeof(GFXglyphRange) + sizeof(GFXfont)));

	FT_Done_FreeType(library);

	return 0;
}

/* -------------------------------------------------------------------------

Character metrics are slightly different from classic GFX & ftGFX.
In classic GFX: cursor position is the upper-left pixel of each 5x7
character; lower extent of most glyphs (except those w/descenders)
is +6 pixels in Y direction.
W/new GFX fonts: cursor position is on baseline, where baseline is
'inclusive' (containing the bottom-most row of pixels in most symbols,
except those with descenders; ftGFX is one pixel lower).

Cursor Y will be moved automatically when switching between classic
and new fonts.  If you switch fonts, any print() calls will continue
along the same baseline.

                    ...........#####.. -- yOffset
                    ..........######..
                    ..........######..
                    .........#######..
                    ........#########.
   * = Cursor pos.  ........#########.
                    .......##########.
                    ......#####..####.
                    ......#####..####.
       *.#..        .....#####...####.
       .#.#.        ....##############
       #...#        ...###############
       #...#        ...###############
       #####        ..#####......#####
       #...#        .#####.......#####
====== #...# ====== #*###.........#### ======= Baseline
                    || xOffset

glyph->xOffset and yOffset are pixel offsets, in GFX coordinate space
(+Y is down), from the cursor position to the top-left pixel of the
glyph bitmap.  i.e. yOffset is typically negative, xOffset is typically
zero but a few glyphs will have other values (even negative xOffsets
sometimes, totally normal).  glyph->xAdvance is the distance to move
the cursor on the X axis after drawing the corresponding symbol.

There's also some changes with regard to 'background' color and new GFX
fonts (classic fonts unchanged).  See Adafruit_GFX.cpp for explanation.
*/

#endif /* !ARDUINO */
