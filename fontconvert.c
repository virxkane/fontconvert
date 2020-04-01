/*
TrueType to Adafruit_GFX font converter.  Derived from Peter Jakobs'
Adafruit_ftGFX fork & makefont tool, and Paul Kourany's Adafruit_mfGFX.

NOT AN ARDUINO SKETCH.  This is a command-line tool for preprocessing
fonts to be used with the Adafruit_GFX Arduino library.

For UNIX-like systems.  Outputs to stdout; redirect to header file, e.g.:
  ./fontconvert ~/Library/Fonts/FreeSans.ttf --size=18 --dpi=141 --progmem > FreeSans18pt7b.h

REQUIRES FREETYPE LIBRARY.  www.freetype.org

Currently this only extracts the printable 7-bit ASCII chars of a font.
Will eventually extend with some int'l chars a la ftGFX, not there yet.
Keep 7-bit fonts around as an option in that case, more compact.

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
#include <ft2build.h>
#include FT_GLYPH_H
#include FT_MODULE_H
#include FT_TRUETYPE_DRIVER_H
#include <getopt.h>

#include "gfxfont.h" // patched Adafruit_GFX font structures

#define MAX_S_LEN		512

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

static void print_help()
{
  printf("Usage: fontconvert <options> [font_file]\n");
  printf("font_file - path to the font file.\n");
  printf("options:\n");
  printf("--size=<font_size>           |-s        specify font size\n");
  printf("--first=<code>               |-f        specify first character\n");
  printf("--last=<code>                |-l        specify last character\n");
  printf("--onechar=<code>             |-c        specify only one character\n");
  printf("--ascii                      |-a        specify ACSII mode: first=0x20, last=0x7E\n");
  printf("--dpi=<dpi_value>            |-d        specify DPI\n");
  printf("--hinting=[no|bytecode|auto] |-t        specify hinting mode\n");
  printf("--progmem[=1|0|yes|no]       |-d        use 'PROGMEM' specification for font data declarations\n");
  printf("--help                       |-h        show this page and exit.\n");
}

static int my_atoi(const char* str)
{
  if (!str || !str[0])
    return -1;
  size_t len = strlen(str);
  char* strcp = 0;
  int res = 0;
  char* endptr = 0;
  if (len > 1 && str[len - 1] == 'h')
  {
    strcp = strdup(str);
    strcp[len - 1] = 0;
    res = strtol(strcp, &endptr, 16);
  }
  else
    res = strtol(str, &endptr, 0);
  if (*endptr != 0)
    res = -1;
  if (strcp)
    free(strcp);
  return res;
}

int main(int argc, char *argv[]) {
  int i, j;
  int err;
  int size = 0;
  int first = 0;
  int last = 0;
  int one_char = 0;
  int ascii_mode = 0;
  int use_progmem = 0;
  int bitmapOffset = 0;
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
  uint8_t bit;

  // Unless overridden, default first and last chars are
  // ' ' (space) and '~', respectively

  // parse command line
  while (1)
  {
    static struct option long_options[] =
    {
      {"size",    required_argument, 0, 's'},
      {"first",   required_argument, 0, 'f'},
      {"last",    required_argument, 0, 'l'},
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

    switch (ret)
    {
      case 0:
        break;
      case 's':
        size = atoi(optarg);
        break;
      case 'f':
        first = my_atoi(optarg);
        break;
      case 'l':
        last = my_atoi(optarg);
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
        if (optarg)
        {
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
  if (optind < argc)
  {
    strncpy(filePath, argv[optind++], MAX_S_LEN);
    filePath[MAX_S_LEN - 1] = 0;
  }
  if (help_only)
  {
    print_help();
    return 0;
  }
  if (filePath[0] == 0)
  {
    fprintf(stderr, "You must specify path to the font file!\n");
    print_help();
    return 1;
  }
  if (size == 0)
  {
    fprintf(stderr, "You must specify valid font size!\n");
    print_help();
    return 1;
  }
  if (ascii_mode && one_char != 0)
  {
    fprintf(stderr, "You cannot specify both ASCII mode and single character mode!\n");
    return 1;
  }
  else if (ascii_mode)
  {
    if (first == 0 && last == 0)
    {
      first = 0x20;     // ' ' SPACE
      last = 0x7E;      // '~' TILDE
    }
    else
    {
      fprintf(stderr, "In ASCII mode, the first and last characters must be not specified!\n");
      print_help();
      return 1;
    }
  }
  else if (one_char != 0)
  {
    if (one_char < 0)
    {
      fprintf(stderr, "Invalid character code!\n");
      print_help();
      return 1;
    }
    if (first == 0 && last == 0)
    {
      first = one_char;
      last = one_char;
    }
    else
    {
      fprintf(stderr, "In one char mode, the first and last characters must be not specified!\n");
      print_help();
      return 1;
    }
  }
  if (first == 0 && last == 0)
  {
    first = 0x20;     // ' ' SPACE
    last = 0x7E;      // '~' TILDE
  }
  if (first < 0)
  {
    fprintf(stderr, "Invalid first character code value!\n");
    print_help();
    return 1;
  }
  if (last < 0)
  {
    fprintf(stderr, "Invalid last character code value!\n");
    print_help();
    return 1;
  }
  if (last < first) {
    i = first;
    first = last;
    last = i;
  }
  if (dpi == 0)
  {
    fprintf(stderr, "Invalid value of DPI!\n");
    return 1;
  }

  switch (hinting)
  {
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

  // Allocate space for font name and glyph table
  if ((!(fontName = malloc(strlen(ptr) + 28))) ||
      (!(table = (GFXglyph *)malloc((last - first + 1) * sizeof(GFXglyph))))) {
    fprintf(stderr, "Malloc error\n");
    return 1;
  }

  // Derive font table names from filename.  Period (filename
  // extension) is truncated and replaced with the font size & bits.
  strcpy(fontName, ptr);
  ptr = strrchr(fontName, '.'); // Find last period (file ext)
  if (!ptr)
    ptr = &fontName[strlen(fontName)]; // If none, append
  // Insert font size and 7/8 bit.  fontName was alloc'd w/extra
  // space to allow this, we're not sprintfing into Forbidden Zone.
  if (first == last)
    sprintf(ptr, "%dpt%db_char%02X", size, (last > 127) ? 8 : 7, first);
  else if (first == 0x20 && last == 0x7E)
    sprintf(ptr, "%dpt_ascii", size);
  else
    sprintf(ptr, "%dpt%db_%02X_%02X", size, (last > 127) ? 8 : 7, first, last);
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
  FT_Set_Char_Size(face, size << 6, 0, dpi, 0);

  // Print header
  printf("/*******************************************************************\n");
  printf(" *  Generated by fontconvert utility:\n");
  printf(" * Font Name: '%s', filepath: '%s'\n", face->family_name, filePath);
  printf(" * Size: %d\n", size);
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
  if (isprint(first))
  	printf(" * First character: '%c' (0x%02X)\n", (char)first, (unsigned int)first);
  else
  	printf(" * First character: (0x%02X)\n", (unsigned int)first);
  if (isprint(last))
  	printf(" * Last character: '%c' (0x%02X)\n", (char)last, (unsigned int)last);
  else
    printf(" * Last character: (0x%02X)\n", (unsigned int)last);
  printf(" *******************************************************************/\n");
  printf("\n");

  // Currently all symbols from 'first' to 'last' are processed.
  // Fonts may contain WAY more glyphs than that, but this code
  // will need to handle encoding stuff to deal with extracting
  // the right symbols, and that's not done yet.
  // fprintf(stderr, "%ld glyphs\n", face->num_glyphs);

  if (use_progmem)
    printf("const uint8_t %s_Bitmaps[] PROGMEM = {\n  ", fontName);
  else
    printf("const uint8_t %s_Bitmaps[] = {\n  ", fontName);

  // Process glyphs and output huge bitmap data array
  for (i = first, j = 0; i <= last; i++, j++) {
    // MONO renderer provides clean image with perfect crop
    // (no wasted pixels) via bitmap struct.
    if ((err = FT_Load_Char(face, i, load_flags))) {
      fprintf(stderr, "Error %d loading char '%c'\n", err, i);
      continue;
    }

    if ((err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO))) {
      fprintf(stderr, "Error %d rendering char '%c'\n", err, i);
      continue;
    }

    if ((err = FT_Get_Glyph(face->glyph, &glyph))) {
      fprintf(stderr, "Error %d getting glyph '%c'\n", err, i);
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

  printf(" };\n\n"); // End bitmap array

  // Output glyph attributes table (one per character)
  if (use_progmem)
    printf("const GFXglyph %s_Glyphs[] PROGMEM = {\n", fontName);
  else
    printf("const GFXglyph %s_Glyphs[] = {\n", fontName);
  for (i = first, j = 0; i <= last; i++, j++) {
    printf("  { %5d, %3d, %3d, %3d, %4d, %4d }", table[j].bitmapOffset,
           table[j].width, table[j].height, table[j].xAdvance, table[j].xOffset,
           table[j].yOffset);
    if (i < last) {
      printf(",   // 0x%02X", i);
      if (isprint(i)) {
        printf(" '%c'", i);
      }
      putchar('\n');
    }
  }
  printf(" }; // 0x%02X", last);
  if (isprint(last))
    printf(" '%c'", last);
  printf("\n\n");

  // Output font structure
  if (use_progmem)
    printf("const GFXfont %s PROGMEM = {\n", fontName);
  else
    printf("const GFXfont %s = {\n", fontName);
  printf("  (uint8_t  *)%s_Bitmaps,\n", fontName);
  printf("  (GFXglyph *)%s_Glyphs,\n", fontName);
  if (face->size->metrics.height == 0) {
    // No face height info, assume fixed width and get from a glyph.
    printf("  0x%02X, 0x%02X, %d, %u };\n\n", first, last, table[0].height, bitmapOffset);
  } else {
    printf("  0x%02X, 0x%02X, %d, %u };\n\n", first, last,
           (int)(face->size->metrics.height >> 6), bitmapOffset);
  }
  printf("// Approx. %d bytes\n", bitmapOffset + (last - first + 1) * 7 + 7);
  // Size estimate is based on AVR struct and pointer sizes;
  // actual size may vary.

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
