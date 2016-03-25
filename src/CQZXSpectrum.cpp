#include <CQZXSpectrum.h>
#include <CZXSpectrum.h>

#include <CArgs.h>
#include <CQUtil.h>

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QKeyEvent>

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CZXSpectrum *spectrum = new CZXSpectrum;

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
//bool icount        = cargs.getBooleanArg("-icount");
  int  scale         = cargs.getIntegerArg("-scale");

  CZ80 *z80 = spectrum->getZ80();

  z80->setVerbose(v_flag);
  z80->setDump(dump_flag);

  spectrum->setScale(scale);

  //------

  int border = 16*scale;

  int w = scale*spectrum->getScreenPixelWidth () + 2*border;
  int h = scale*spectrum->getScreenPixelHeight() + 2*border;

  CQZXSpectrum *qspectrum = new CQZXSpectrum(spectrum, w, h);

  //qspectrum->setWindowTitle("ZXSpectrum II Emulator");

  //------

  z80->setScreen(qspectrum);

  //z80->setSpeedData(new CZXSpectrumSpeedData(*qspectrum));

  //------

  for (int i = 1; i < argc; ++i) {
    if      (bin_flag)
      z80->loadBin(argv[i]);
    else if (snapshot_flag)
      z80->loadSnapshot(argv[i]);
    else if (tzx_flag)
      spectrum->loadTZX(argv[i]);
    else if (z80_flag)
      spectrum->loadZ80(argv[i]);
    else
      z80->load(argv[i]);
  }

  if (! snapshot_flag && ! z80_flag)
    z80->setPC(0);

  qspectrum->show();

  return app.exec();
}

CQZXSpectrum::
CQZXSpectrum(CZXSpectrum *spectrum, int w, int h) :
 CZ80Screen(*spectrum->getZ80()), spectrum_(spectrum), border_(0), memChanged_(false)
{
  QTimer *itimer = new QTimer;
  QTimer *stimer = new QTimer;

  connect(itimer, SIGNAL(timeout()), this, SLOT(itimeOut()));
  connect(stimer, SIGNAL(timeout()), this, SLOT(stimeOut()));

  setFocusPolicy(Qt::StrongFocus);

  border_ = 4*spectrum->getScale();

  resize(w, h);

  itimer->start(1);
  stimer->start(400);
}

CQZXSpectrum::
~CQZXSpectrum()
{
}

void
CQZXSpectrum::
memChanged(ushort start, ushort len)
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
  CKeyType type = CQUtil::convertKey(e->key(), e->modifiers());

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
    z80->keyPress(type);

  for (uint i = 0; i < 1000; ++i)
    z80->step();
}

void
CQZXSpectrum::
keyReleaseEvent(QKeyEvent *e)
{
  CKeyType type = CQUtil::convertKey(e->key(), e->modifiers());

  if (type >= CKEY_TYPE_F1 && type <= CKEY_TYPE_F12)
    return;

  CZ80 *z80 = spectrum_->getZ80();

  z80->keyRelease(type);

  for (uint i = 0; i < 1000; ++i)
    z80->step();
}

void
CQZXSpectrum::
itimeOut()
{
  CZ80 *z80 = spectrum_->getZ80();

  for (uint i = 0; i < 1000; ++i)
    z80->step();
}

void
CQZXSpectrum::
stimeOut()
{
  if (memChanged_)
    update();
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
fillRectangle(int x, int y, int w, int h)
{
  painter_->fillRect(QRect(x, y, w, h), QBrush(CQUtil::rgbaToColor(fg_)));
}

void
CQZXSpectrumRenderer::
drawPoint(int x, int y)
{
  painter_->setPen(CQUtil::rgbaToColor(fg_));

  painter_->drawPoint(x, y);
}
