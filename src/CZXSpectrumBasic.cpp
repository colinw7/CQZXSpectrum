#include "CZXSpectrum.h"

using std::cerr;
using std::endl;

static const char *
tokens[] = {
 // 163-171: 128-specific tokens, random, keyboard and screen manipulation functions
 "SPECTRUM", "PLAY", "RND", "INKEY$", "PI", "FN", "POINT", "SCREEN$", "ATTR",
 // 172-183: PRINT position control tokens, trig functions
 "AT",  "TAB", "VAL$", "CODE", "VAL", "LEN", "SIN", "COS", "TAN", "ASN", "ACS", "ATN",
 // 184-194: math, system and string functions
 "LN", "EXP", "INT", "SQR", "SGN", "ABS", "PEEK", "IN", "USR", "STR$", "CHR$",
 // 195-205: boolean and comparison operators, control flow tokens
 "NOT", "BIN", "OR", "AND", "<=", ">=", "<>", "LINE", "THEN", "TO", "STEP",
 // 206-214: U-D function, file and stream manipulation statements
 "DEF FN", "CAT", "FORMAT", "MOVE", "ERASE", "OPEN #",  "CLOSE #", "MERGE", "VERIFY",
 // 215-222: sound, graphics and colour manipulation statements
 "BEEP", "CIRCLE", "INK", "PAPER", "FLASH", "BRIGHT", "INVERSE", "OVER",
 // 223-229: port,printer, control flow and data statements
 "OUT", "LPRINT", "LLIST", "STOP", "READ", "DATA", "RESTORE",
 // 230-238: K-cursor statements on keys A-I
 "NEW", "BORDER", "CONTINUE", "DIM", "REM", "FOR", "GO TO", "GO SUB", "INPUT",
 // 239-247: K-cursor statements on keys J-R
 "LOAD", "LIST", "LET", "PAUSE", "NEXT", "POKE", "PRINT", "PLOT", "RUN",
 // 249-255: K-cursor statements on keys S-Z
 "SAVE", "RANDOMIZE", "IF", "CLS", "CLEAR", "RETURN", "COPY"
};

void
CZXSpectrum::
dumpBasic()
{
  uchar data[4096];

  uint len = sizeof(data);

  z80_.getBytes(data, MDRIVE_START, len);

  uint i = 0;

  while (i < len && data[i] != 0x80)
    ++i;

  if (i < len && data[i] == 0x80)
    ++i;

  int i1 = i;

  while (i < len && data[i] != 0x80)
    ++i;

  if (i < len && data[i] == 0x80)
    --i;

  int i2 = i;

#if 0
  for (i = i1; i < len; ++i)
    cerr << hex << int(data[i]) << " " << dec;

  cerr << endl;
#endif

  decodeBasic(&data[i1], i2 - i1 + 1);
}

bool
CZXSpectrum::
decodeBasic(const uchar *data, uint len)
{
  ushort pos = 0;

  bool new_line = true;

  while (pos < len) {
    if (new_line) {
      ushort num = data[pos]*256 + data[pos + 1];

      pos += 2;

      ushort len1;

      memcpy(&len1, &data[pos], 2);

      pos += 2;

      cerr << int(num) << " ";

      new_line = false;
    }
    else {
      if      (data[pos] > 0xa3)
        cerr << tokens[data[pos] - 0xa3] << " ";
      else if (data[pos] == 13) { // Newline
        cerr << endl;

        new_line = true;
      }
      else if (data[pos] == 14) // Number
        pos += 5; // Skip
      else if (data[pos] < 32)
        cerr << int(data[pos]);
      else if (data[pos] >= 127)
        cerr << int(data[pos]);
      else
        cerr << char(data[pos]);

      ++pos;
    }
  }

  return true;
}
