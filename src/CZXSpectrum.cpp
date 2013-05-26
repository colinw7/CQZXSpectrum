#include "CZXSpectrum.h"
#include "CZXSpectrumKeyMap.h"
#include "CZXSpectrumTZX.h"
#include "CZXSpectrumZ80.h"
#include "CStrUtil.h"

#include <cassert>
#include "roms/spectrum48.h"

using std::string;
using std::cerr;
using std::endl;

struct CZXSpectrumPortData : public CZ80PortData {
 private:
  CZXSpectrum &spectrum_;
  uchar        key_state_[8];

 public:
  CZXSpectrumPortData(CZXSpectrum &spectrum);

  void out(uchar port, uchar value);

  uchar in(uchar port, uchar qual);

  void keyPress  (CKeyType key_type);
  void keyRelease(CKeyType key_type);

  bool processKey(CKeyType type, bool press, uint *row, uint *col);
};

CZXSpectrum::
CZXSpectrum() :
 port_data_(NULL), scale_(1), border_(15)
{
  z80_.setBytes(spectrum48_data, 0, SPECTRUM48_DATA_LEN);

  //z80_.setMemFlags(0, SPECTRUM48_DATA_LEN, CZ80_MEM_READ_ONLY);

  z80_.setMemFlags(DISPLAY_START, ATTR_END - DISPLAY_START + 1,
                   CZ80_MEM_SCREEN);

  port_data_ = new CZXSpectrumPortData(*this);

  z80_.setPortData(port_data_);
}

CZXSpectrum::
~CZXSpectrum()
{
  delete port_data_;
}

void
CZXSpectrum::
reset()
{
  z80_.reset();

  z80_.setBytes(spectrum48_data, 0, SPECTRUM48_DATA_LEN);
}

uint
CZXSpectrum::
getScreenPixelWidth()
{
  return getCharWidth ()*getScreenCharWidth ();
}

uint
CZXSpectrum::
getScreenPixelHeight()
{
  return getCharHeight()*getScreenCharHeight();
}

bool
CZXSpectrum::
onScreen(ushort pos, ushort len)
{
  return ! (pos + len < DISPLAY_START || pos >= DISPLAY_END);
}

bool
CZXSpectrum::
getScreenPos(ushort pos, int *x, int *y)
{
  if (pos < DISPLAY_START || pos >= DISPLAY_END)
    return false;

  ushort pos1 = pos & (DISPLAY_START - 1);

  *y = ((pos1 & 0x00e0) >> 2) +
       ((pos1 & 0x0700) >> 8) +
       ((pos1 & 0x1800) >> 5);

  *x = (pos1 & 0x1F) << 3;

  return true;
}

const CRGBA &
CZXSpectrum::
getColor(uint i)
{
  static CRGBA colors[16];
  static bool  colors_set;

  if (! colors_set) {
    double d = 238.0/255.0;
    double f = 1.0;

    colors[0] = CRGBA(0,0,0); colors[ 8] = CRGBA(0,0,0);
    colors[1] = CRGBA(0,0,d); colors[ 9] = CRGBA(0,0,f);
    colors[2] = CRGBA(d,0,0); colors[10] = CRGBA(f,0,0);
    colors[3] = CRGBA(d,0,d); colors[11] = CRGBA(f,0,f);
    colors[4] = CRGBA(0,d,0); colors[12] = CRGBA(0,f,0);
    colors[5] = CRGBA(0,d,d); colors[13] = CRGBA(0,f,f);
    colors[6] = CRGBA(d,d,0); colors[14] = CRGBA(f,f,0);
    colors[7] = CRGBA(d,d,d); colors[15] = CRGBA(f,f,f);

    colors_set = true;
  }

  assert(i < 16);

  return colors[i];
}

bool
CZXSpectrum::
loadTZX(const string &filename)
{
  CZXSpectrumTZX tzx(*this, filename);

  return tzx.load();
}

bool
CZXSpectrum::
loadZ80(const string &filename)
{
  CZXSpectrumZ80 z80(*this, filename);

  return z80.load();
}

void
CZXSpectrum::
drawBorder(CZXSpectrumRenderer *renderer)
{
  ushort w = getScreenCharWidth();
  ushort h = getScreenCharHeight();

  ushort w8 = w << 3;
  ushort h8 = h << 3;

  renderer->setForeground(getBorderColor());

//renderer->fill();

  renderer->fillRectangle(0, 0, w8*scale_ + 2*border_, h8*scale_ + 2*border_);
}

void
CZXSpectrum::
draw(CZXSpectrumRenderer *renderer, int footer)
{
  ushort w = getScreenCharWidth();
  ushort h = getScreenCharHeight();

  ushort w8 = w << 3;
  ushort h8 = h << 3;

  renderer->setForeground(CRGBA(0,0,1));

//renderer->fill();

  renderer->fillRectangle(0, 0, w8*scale_ + 2*border_, h8*scale_ + 2*border_ + footer);

  renderer->setForeground(CRGBA(1,1,1));

  renderer->fillRectangle(border_, border_, w8*scale_, h8*scale_);

  renderer->setForeground(CRGBA(0,0,0));

  ushort pos = getScreenMemStart();

#if 0
  for (int o = 0; o < 8; ++o) {
    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x, ++pos) {
        renderer->setForeground(CRGBA(o/8.,o/8.,o/8.));

        ushort px =  x << 3;
        ushort py = (y << 3) + o;

        for (ushort k = 0; k < 8; ++k, ++px)
          drawPoint(px, py);
      }
    }
  }

  pos = getScreenMemStart();
#endif

  for (int o = 0; o < 8; ++o) {
    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x, ++pos) {
        uchar c = z80_.getByte(pos);

        if (c != 0) {
          renderer->setForeground(CRGBA(0,0,0));

          ushort px =  x << 3;
          ushort py = (y << 3) + o;

          uchar mask = 0x80;

          for (ushort k = 0; k < 8; ++k, ++px) {
            if (c & mask)
              drawPoint(renderer, px, py);

            mask >>= 1;
          }
        }
      }
    }
  }
}

void
CZXSpectrum::
drawMem(CZXSpectrumRenderer *renderer, ushort pos, ushort len)
{
  int x, y;

  for (ushort i = 0; i < len; ++i, ++pos) {
    uchar c = z80_.getByte(pos);

    if (! getScreenPos(pos, &x, &y))
      continue;

    uchar mask = 0x80;

    for (ushort k = 0; k < 8; ++k) {
      if (c & mask)
        setForeground(renderer, x, y);
      else
        setBackground(renderer, x, y);

      mask >>= 1;

      drawPoint(renderer, x, y);

      ++x;
    }
  }
}

void
CZXSpectrum::
setForeground(CZXSpectrumRenderer *renderer, uint x, uint y)
{
  bool flash;
  uint bg, fg;

  getPixelColor(x, y, &fg, &bg, &flash);

  renderer->setForeground(getColor(fg));
}

void
CZXSpectrum::
setBackground(CZXSpectrumRenderer *renderer, uint x, uint y)
{
  bool flash;
  uint bg, fg;

  getPixelColor(x, y, &fg, &bg, &flash);

  renderer->setForeground(getColor(bg));
}

void
CZXSpectrum::
getPixelColor(uint x, uint y, uint *fg, uint *bg, bool *flash)
{
  uint bright;

  uint pos = ATTR_START + (x >> 3) + ((y >> 3) << 5);

  uchar c = z80_.getByte(pos);

  bright = (c & 0x40) >> 3;

  *fg = ( c       & 0x07) | bright;
  *bg = ((c >> 3) & 0x07) | bright;

  *flash = ((c & 0x80) != 0);
}

void
CZXSpectrum::
drawPoint(CZXSpectrumRenderer *renderer, int x, int y)
{
  assert(x >= 0 && x < 256);
  assert(y >= 0 && y < 192);

  int px = x*scale_ + border_;
  int py = y*scale_ + border_;

  if (scale_ == 1)
    renderer->drawPoint(px, py);
  else
    renderer->fillRectangle(px, py, scale_, scale_);
}

//-----------------

CZXSpectrumPortData::
CZXSpectrumPortData(CZXSpectrum &spectrum) :
 CZ80PortData(*spectrum.getZ80()), spectrum_(spectrum)
{
  for (ushort i = 0; i < 8; ++i)
    key_state_[i] = 0xff;
}

void
CZXSpectrumPortData::
out(uchar port, uchar value)
{
  if ((port & 0x0001) == 0) {
    spectrum_.setBorderColor(int(value & 0x07));

    spectrum_.getZ80()->getScreen()->memChanged(16384, 1);
  }
  else {
    static bool warn;

    if (! warn) {
      cerr << "Out: Port " << (int) port << " Value " << (int) value << endl;

      warn = true;
    }
  }
}

uchar
CZXSpectrumPortData::
in(uchar port, uchar qual)
{
  uchar res = 0xff;

  if      ((port & 0x01) == 0) {
    if ((qual & 0x01) == 0) { res &= key_state_[0] /*SHIFT,Z,X,C,V   */; }
    if ((qual & 0x02) == 0) { res &= key_state_[1] /*A,S,D,F,G       */; }
    if ((qual & 0x04) == 0) { res &= key_state_[2] /*Q,W,E,R,T       */; }
    if ((qual & 0x08) == 0) { res &= key_state_[3] /*1,2,3,4,5       */; }
    if ((qual & 0x10) == 0) { res &= key_state_[4] /*0,9,8,7,6       */; }
    if ((qual & 0x20) == 0) { res &= key_state_[5] /*P,O,I,U,Y       */; }
    if ((qual & 0x40) == 0) { res &= key_state_[6] /*ENTER,L,K,J,H   */; }
    if ((qual & 0x80) == 0) { res &= key_state_[7] /*SPACE,SYM.M,N,B */; }
  }
  else if (port == 0x1f) // Kempston Joystick
    return 0;
  else
    cerr << "In: Port " << (int) port << " Value " << (int) qual << endl;

  return res;
}

void
CZXSpectrumPortData::
keyPress(CKeyType type)
{
  uint row, col;

  if (! processKey(type, true, &row, &col))
    return;
}

void
CZXSpectrumPortData::
keyRelease(CKeyType type)
{
  uint row, col;

  if (! processKey(type, false, &row, &col))
    return;
}

bool
CZXSpectrumPortData::
processKey(CKeyType type, bool press, uint *row, uint *col)
{
  *row = 0;
  *col = 0;

  for (uint i = 0; i < key_map_size; ++i) {
    if (key_map[i].keycode != type)
      continue;

    for (uint j = 0; key_map[i].pos[j].row > 0; ++j) {
      if (press)
        RESET_BIT(key_state_[key_map[i].pos[j].row - 1],
                             key_map[i].pos[j].col - 1);
      else
        SET_BIT  (key_state_[key_map[i].pos[j].row - 1],
                             key_map[i].pos[j].col - 1);
    }

    return true;
  }

#if 0
  for (*row = 0; *row < 8; ++(*row))
    for (*col = 0; *col < 5; ++(*col))
      if (key_map_lower[*row][*col] == type)
        return true;

  for (*row = 0; *row < 8; ++(*row))
    for (*col = 0; *col < 5; ++(*col))
      if (key_map_upper[*row][*col] == type)
        return true;
#endif

  cerr << "Unhandled Key: " << type << endl;

  return false;
}
