#include <CZXSpectrumX.h>
#include <CZXSpectrum.h>
#include <CZ80SpeedData.h>
#include <CArgs.h>
#include <CXMachine.h>
#include <CXWindow.h>
#include <CXImage.h>
#include <CXLibPixelRenderer.h>
#include <CAtExit.h>

class CZXSpectrumSpeedData : public CZ80SpeedData {
 private:
  CZXSpectrumXScreen &screen_;

 public:
  CZXSpectrumSpeedData(CZXSpectrumXScreen &screen) :
   CZ80SpeedData(*screen.getSpectrum().getZ80()), screen_(screen) {
  }

  void output() override {
    screen_.drawMhz(getMhz());

    screen_.screenMemChanged(16384, 6144);
  }
};

class CZXSpectrumXRenderer : public CZXSpectrumRenderer {
 public:
  CZXSpectrumXRenderer(CXLibPixelRenderer *renderer) :
   renderer_(renderer) {
  }

  void setForeground(const CRGBA &fg) override;

  void fillRectangle(const CIBBox2D &bbox) override;

  void drawPoint(const CIPoint2D &p) override;

 private:
  CXLibPixelRenderer *renderer_ { nullptr };
};

int
main(int argc, char **argv)
{
  CZXSpectrum spectrum;

  CArgs cargs("-v:f        (verbose) "
              "-dump:f     (enable dump) "
              "-bin:f      (input file is binary) "
              "-snapshot:f (input file is snapshot) "
              "-tzx:f      (input file is tzx) "
              "-z80:f      (input file is z80) "
              "-icount:f   (output instruction counts on exit) "
              "-scale:i=1  (scale factor) "
              );

  cargs.parse(&argc, argv);

  bool v_flag        = cargs.getBooleanArg("-v");
  bool dump_flag     = cargs.getBooleanArg("-dump");
  bool bin_flag      = cargs.getBooleanArg("-bin");
  bool snapshot_flag = cargs.getBooleanArg("-snapshot");
  bool tzx_flag      = cargs.getBooleanArg("-tzx");
  bool z80_flag      = cargs.getBooleanArg("-z80");
  bool icount        = cargs.getBooleanArg("-icount");
  int  scale         = int(cargs.getIntegerArg("-scale"));

  CZ80 *z80 = spectrum.getZ80();

  z80->setVerbose(v_flag);
  z80->setDump(dump_flag);

  spectrum.setScale(scale);

  //------

  int border = 16*scale;
  int footer = 16;

  int w = scale*spectrum.getScreenPixelWidth () + 2*border;
  int h = scale*spectrum.getScreenPixelHeight() + 2*border;

  CXWindow *xwindow = new CXWindow(w, h + footer);

  xwindow->setWindowTitle("ZX Spectrum Emulator");

  xwindow->selectKeyInput ();
  xwindow->selectExposures();

  CXLibPixelRenderer *renderer = new CXLibPixelRenderer(xwindow);

  //------

  CZXSpectrumXScreen *screen = new CZXSpectrumXScreen(spectrum, renderer, scale);

  z80->setScreen(screen);

  z80->setSpeedData(new CZXSpectrumSpeedData(*screen));

  //------

  xwindow->map();

  //------

  for (int i = 1; i < argc; ++i) {
    if      (bin_flag)
      z80->loadBin(argv[i]);
    else if (snapshot_flag)
      z80->loadSnapshot(argv[i]);
    else if (tzx_flag)
      spectrum.loadTZX(argv[i]);
    else if (z80_flag)
      spectrum.loadZ80(argv[i]);
    else
      z80->load(argv[i]);
  }

  if (! snapshot_flag && ! z80_flag)
    z80->setPC(0);

  if (icount)
    new CZXSpectrumXAtExit(&spectrum);

  CZXSpectrumEventAdapter *adapter = new CZXSpectrumEventAdapter(screen, z80);

  CXMachineInst->setEventAdapter(adapter);

  screen->redraw();

  CXMachineInst->mainLoop(100);
}

void
CZXSpectrumXScreen::
screenMemChanged(ushort pos, ushort len)
{
  CZXSpectrumXRenderer xr(renderer);

  spectrum.drawMem(&xr, pos, len);
}

void
CZXSpectrumXScreen::
redraw()
{
  CZXSpectrumXRenderer xr(renderer);

  spectrum.drawBorder(&xr);

  screenMemChanged(16384, 6144);
}

void
CZXSpectrumXScreen::
redraw1()
{
  renderer->startDoubleBuffer();

  CZXSpectrumXRenderer xr(renderer);

  spectrum.draw(&xr, footer);

  renderer->endDoubleBuffer();
}

void
CZXSpectrumXScreen::
drawMhz(double mhz)
{
  int w = renderer->getWidth ();
  int h = renderer->getHeight();

  renderer->setForeground(CRGBA(1,1,1));

  renderer->fillRectangle(CIBBox2D(0, h - footer, w - 1, h - 1));

  std::string str = CStrUtil::toString(mhz);

  renderer->setForeground(CRGBA(0,0,0));

  renderer->drawString(CIPoint2D(4, h - footer + 1), str);
}

//-------------

bool
CZXSpectrumEventAdapter::
keyPressEvent(const CKeyEvent &kevent)
{
  CKeyType type = kevent.getType();

  if      (type == CKEY_TYPE_F1)
    z80->saveSnapshot();
  else if (type == CKEY_TYPE_F2)
    z80->resetOpCounts();
  else if (type == CKEY_TYPE_F4)
    screen->getSpectrum().dumpBasic();
  else if (type == CKEY_TYPE_F8)
    screen->getSpectrum().reset();
  else if (type == CKEY_TYPE_F12)
    exit(0);
  else
    z80->keyPress(kevent);

  for (uint i = 0; i < 1000; ++i)
    z80->step();

  return true;
}

bool
CZXSpectrumEventAdapter::
keyReleaseEvent(const CKeyEvent &kevent)
{
  CKeyType type = kevent.getType();

  if (type >= CKEY_TYPE_F1 && type <= CKEY_TYPE_F12)
    return false;

  z80->keyRelease(kevent);

  for (uint i = 0; i < 1000; ++i)
    z80->step();

  return true;
}

bool
CZXSpectrumEventAdapter::
exposeEvent()
{
  screen->redraw();

  return true;
}

bool
CZXSpectrumEventAdapter::
idleEvent()
{
  for (uint i = 0; i < 1000; ++i)
    z80->step();

  return true;
}

//-------------

void
CZXSpectrumXRenderer::
setForeground(const CRGBA &fg)
{
  renderer_->setForeground(fg);
}

void
CZXSpectrumXRenderer::
fillRectangle(const CIBBox2D &bbox)
{
  renderer_->fillRectangle(bbox);
}

void
CZXSpectrumXRenderer::
drawPoint(const CIPoint2D &p)
{
  renderer_->drawPoint(p);
}
