#include <CQZXSpectrum.h>
#include <CZXSpectrum.h>
#include <CQZ80Dbg.h>

#include <CArgs.h>
#include <CQApp.h>
#include <CQUtil.h>
#include <CQUtilEvent.h>
#include <CQUtilGeom.h>
#include <CQUtilRGBA.h>

#include <QWidget>
#include <QPainter>
#include <QTimer>

int
main(int argc, char **argv)
{
  CQApp app(argc, argv);

  CArgs cargs("-v:f        (verbose) "
              "-dump:f     (enable dump) "
              "-bin:f      (input file is binary) "
              "-snapshot:f (input file is snapshot) "
              "-tzx:f      (input file is tzx) "
              "-z80:f      (input file is z80) "
              "-icount:f   (output instruction counts on exit) "
              "-scale:i=1  (scale factor) "
              "-debug:f    (debug) "
              );

  cargs.parse(&argc, argv);

  bool verbose  = cargs.getBooleanArg("-v");
  bool dump     = cargs.getBooleanArg("-dump");
  bool bin      = cargs.getBooleanArg("-bin");
  bool snapshot = cargs.getBooleanArg("-snapshot");
  bool isTZX    = cargs.getBooleanArg("-tzx");
  bool isZ80    = cargs.getBooleanArg("-z80");
//bool icount   = cargs.getBooleanArg("-icount");
  int  scale    = cargs.getIntegerArg("-scale");
  bool debug    = cargs.getBooleanArg("-debug");

  CZXSpectrum *spectrum = new CZXSpectrum;

  CZ80 *z80 = spectrum->getZ80();

  z80->setVerbose(verbose);
  z80->setDump(dump);

  spectrum->setScale(scale);

  //------

  int border = 16*scale;

  int w = scale*spectrum->getScreenPixelWidth () + 2*border;
  int h = scale*spectrum->getScreenPixelHeight() + 2*border;

  CQZXSpectrum *qspectrum = new CQZXSpectrum(spectrum, w, h);

  //qspectrum->setWindowTitle("ZXSpectrum II Emulator");

  z80->setScreen(qspectrum);

  //z80->setSpeedData(new CZXSpectrumSpeedData(*qspectrum));

  //------

  for (int i = 1; i < argc; ++i) {
    if      (bin)
      z80->loadBin(argv[i]);
    else if (snapshot)
      z80->loadSnapshot(argv[i]);
    else if (isTZX)
      spectrum->loadTZX(argv[i]);
    else if (isZ80)
      spectrum->loadZ80(argv[i]);
    else
      z80->load(argv[i]);
  }

  if (! snapshot && ! isZ80)
    z80->setPC(0);

  qspectrum->show();

  //------

  if (debug)
    qspectrum->addDebug();

  //------

  if (! debug)
    qspectrum->exec();

  return app.exec();
}

//------

CQZXSpectrum::
CQZXSpectrum(CZXSpectrum *spectrum, int w, int h) :
 CZ80Screen(*spectrum->getZ80()), spectrum_(spectrum), border_(0), memChanged_(false)
{
  setFocusPolicy(Qt::StrongFocus);

  border_ = 4*spectrum->getScale();

  resize(w, h);
}

CQZXSpectrum::
~CQZXSpectrum()
{
}

void
CQZXSpectrum::
exec()
{
  itimer_ = new QTimer;
  stimer_ = new QTimer;

  connect(itimer_, SIGNAL(timeout()), this, SLOT(itimeOut()));
  connect(stimer_, SIGNAL(timeout()), this, SLOT(stimeOut()));

  itimer_->start(1);
  stimer_->start(400);
}

CQZ80Dbg *
CQZXSpectrum::
addDebug()
{
  if (! dbg_) {
    dbg_ = new CQZ80Dbg(spectrum_->getZ80());

    dbg_->init();

    QFont fixedFont("Courier New", 16);

    dbg_->setFixedFont(fixedFont);
  }

  dbg_->show();
  dbg_->raise();

  return dbg_;
}

void
CQZXSpectrum::
screenMemChanged(ushort start, ushort len)
{
  if (spectrum_->onScreen(start, len))
    memChanged_ = true;
}

void
CQZXSpectrum::
redraw()
{
  update();
}

void
CQZXSpectrum::
paintEvent(QPaintEvent *)
{
  //renderer->startDoubleBuffer();

  QPainter painter(this);

  CQZXSpectrumRenderer nrenderer(this, &painter);

//int footer = 16;

//spectrum_->draw(&nrenderer, footer);

  spectrum_->drawMem(&nrenderer, 16384, 6144);

  //renderer->endDoubleBuffer();

  memChanged_ = false;
}

void
CQZXSpectrum::
keyPressEvent(QKeyEvent *e)
{
  CKeyEvent *kevent = CQUtil::convertEvent(e);

  CKeyType type = kevent->getType();

  CZ80 *z80 = spectrum_->getZ80();

  if      (type == CKEY_TYPE_F1)
    z80->saveSnapshot();
  else if (type == CKEY_TYPE_F2)
    z80->resetOpCounts();
  else if (type == CKEY_TYPE_F4)
    spectrum_->dumpBasic();
  else if (type == CKEY_TYPE_F8)
    spectrum_->reset();
  else if (type == CKEY_TYPE_F12)
    exit(0);
  else
    z80->keyPress(*kevent);

  doSteps();
}

void
CQZXSpectrum::
keyReleaseEvent(QKeyEvent *e)
{
  CKeyEvent *kevent = CQUtil::convertEvent(e);

  CKeyType type = kevent->getType();

  if (type >= CKEY_TYPE_F1 && type <= CKEY_TYPE_F12)
    return;

  CZ80 *z80 = spectrum_->getZ80();

  z80->keyRelease(*kevent);

  doSteps();
}

void
CQZXSpectrum::
itimeOut()
{
  doSteps();
}

void
CQZXSpectrum::
stimeOut()
{
  if (memChanged_)
    update();
}

void
CQZXSpectrum::
doSteps()
{
  CZ80 *z80 = spectrum_->getZ80();

  for (uint i = 0; i < 1000; ++i)
    z80->step();
}

//------------

void
CQZXSpectrumRenderer::
setForeground(const CRGBA &fg)
{
  fg_ = fg;
}

void
CQZXSpectrumRenderer::
fillRectangle(const CIBBox2D &bbox)
{
  painter_->fillRect(CQUtil::toQRect(bbox), QBrush(CQUtil::rgbaToColor(fg_)));
}

void
CQZXSpectrumRenderer::
drawPoint(const CIPoint2D &p)
{
  painter_->setPen(CQUtil::rgbaToColor(fg_));

  painter_->drawPoint(p.getX(), p.getY());
}
