#ifndef CZXSPECTRUM_H
#define CZXSPECTRUM_H

#include <CZ80.h>
#include <CImagePtr.h>
#include <CIBBox2D.h>
#include <CRGBA.h>

struct CZXSpectrumPortData;

class CZXSpectrumRenderer {
 public:
  CZXSpectrumRenderer() { }

  virtual ~CZXSpectrumRenderer() { }

  virtual void setForeground(const CRGBA &fg) = 0;

  virtual void fillRectangle(const CIBBox2D &bbox) = 0;

  virtual void drawPoint(const CIPoint2D &p) = 0;
};

class CZXSpectrum {
 public:
  static const uint DISPLAY_START = 0x4000; // 16384
  static const uint DISPLAY_END   = 0x5800; // 22528

  static const uint ATTR_START    = 0x5800; // 22528
  static const uint ATTR_END      = 0x5b00; // 23296

  static const uint PRINTER_START = 0x5b00; // 23296
  static const uint PRINTER_END   = 0x5c00; // 23552

  static const uint SYSVAR_START  = 0x5c00; // 23552
  static const uint SYSVAR_END    = 0x5cb6; // 23734

  static const uint MDRIVE_START  = 0x5cb6; // 23734
  static const uint MDRIVE_END    = 0x5cb6; // ???

 private:
  CZ80                 z80_;
  CZXSpectrumPortData *port_data_;
  int                  scale_;
  int                  border_;
  int                  borderColor_;

 public:
  CZXSpectrum();
 ~CZXSpectrum();

  void reset();

  void setScale(int scale) { scale_ = scale; }
  int  getScale() const { return scale_; }

  void setBorder(int border) { border_ = border; }
  int  getBorder() const { return border_; }

  void setBorderColor(int c) { borderColor_ = c; }
  int  getBorderColor() const { return borderColor_; }

  CZ80 *getZ80() { return &z80_; }

  static uint getScreenPixelWidth ();
  static uint getScreenPixelHeight();

  static uint getScreenCharWidth () { return 32; }
  static uint getScreenCharHeight() { return 24; }

  static uint getCharWidth () { return 8; }
  static uint getCharHeight() { return 8; }

  static uint getScreenMemStart() { return 0x4000; }
  static uint getScreenMemEnd  () { return 0x5800; }

  bool onScreen(ushort pos, ushort len);

  bool getScreenPos(ushort pos, int *x, int *y);

  const CRGBA &getColor(uint i);

  const CRGBA &getBorderColor() {
    return getColor(border_);
  }

  bool loadTZX(const std::string &filename);
  bool loadZ80(const std::string &filename);

  void dumpBasic();
  bool decodeBasic(const uchar *data, uint len);

  void drawBorder(CZXSpectrumRenderer *renderer);

  void draw(CZXSpectrumRenderer *renderer, int footer);

  void drawMem(CZXSpectrumRenderer *renderer, ushort pos, ushort len);

  void setForeground(CZXSpectrumRenderer *renderer, uint x, uint y);

  void setBackground(CZXSpectrumRenderer *renderer, uint x, uint y);

  void getPixelColor(uint x, uint y, uint *fg, uint *bg, bool *flash);

  void drawPoint(CZXSpectrumRenderer *renderer, int x, int y);
};

#endif
