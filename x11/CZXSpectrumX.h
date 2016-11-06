#ifndef CZXSpectrumX_H
#define CZXSpectrumX_H

#include <CZXSpectrum.h>
#include <CZ80.h>
#include <CXLibPixelRenderer.h>
#include <CXLib.h>
#include <CAtExit.h>

class CZXSpectrumXScreen : public CZ80Screen {
 public:
  CZXSpectrumXScreen(CZXSpectrum &spectrum1, CXLibPixelRenderer *renderer1, int scale1) :
   CZ80Screen(*spectrum1.getZ80()), spectrum(spectrum1), renderer(renderer1),
   scale(scale1), border(16*scale), footer(16) {
    renderer->setBackground(CRGB(0,0,0));
  }

  virtual ~CZXSpectrumXScreen() { }

  void memChanged(ushort pos, ushort len);

  CZXSpectrum &getSpectrum() const { return spectrum; }

  void redraw();
  void redraw1();

  void drawBorder();
  void drawMhz(double mhz);

 private:
  CZXSpectrum        &spectrum;
  CXLibPixelRenderer *renderer;
  int                 scale;
  int                 border;
  int                 footer;
};

//---

struct CZXSpectrumEventAdapter : public CXEventAdapter {
  CZXSpectrumXScreen *screen;
  CZ80               *z80;

  CZXSpectrumEventAdapter(CZXSpectrumXScreen *screen1, CZ80 *z801) :
    CXEventAdapter(KeyPress | KeyRelease | Expose),
    screen(screen1), z80(z801) {
  }

  bool keyPressEvent  (const CKeyEvent &kevent);
  bool keyReleaseEvent(const CKeyEvent &kevent);

  bool exposeEvent();

  bool idleEvent();
};

struct CZXSpectrumXAtExit : public CAtExit {
  CZXSpectrum *spectrum;

  CZXSpectrumXAtExit(CZXSpectrum *spectrum1) :
   spectrum(spectrum1) {
  }

  void operator()() {
    spectrum->getZ80()->dumpOpCounts(std::cout);
  }
};

#endif
