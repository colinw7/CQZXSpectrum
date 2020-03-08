#include <CZXSpectrum.h>
#include <CZ80.h>
#include <CZ80Screen.h>
#include <QWidget>

class CQZ80Dbg;
class QTimer;

class CQZXSpectrum : public QWidget, public CZ80Screen {
  Q_OBJECT

 public:
  CQZXSpectrum(CZXSpectrum *spectrum, int w, int h);

 ~CQZXSpectrum();

  void exec();

  CQZ80Dbg *addDebug();

  void screenMemChanged(ushort pos, ushort len) override;

  void redraw();

  void paintEvent(QPaintEvent *) override;

  void keyPressEvent  (QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;

  void doSteps();

 public slots:
  void itimeOut();
  void stimeOut();

 private:
  CZXSpectrum* spectrum_   { nullptr };
  int          border_     { 2 };
  bool         memChanged_ { false };
  CQZ80Dbg*    dbg_        { nullptr };
  QTimer*      itimer_     { nullptr };
  QTimer*      stimer_     { nullptr };
};

//------

class CQZXSpectrumRenderer : public CZXSpectrumRenderer {
 public:
  CQZXSpectrumRenderer(CQZXSpectrum *qspectrum, QPainter *painter) :
   qspectrum_(qspectrum), painter_(painter) {
  }

  CQZXSpectrum *qspectrum() const { return qspectrum_; }

  void setForeground(const CRGBA &fg) override;

  void fillRectangle(const CIBBox2D &bbox) override;

  void drawPoint(const CIPoint2D &p) override;

 private:
  CQZXSpectrum* qspectrum_ { nullptr };
  QPainter*     painter_   { nullptr };
  CRGBA         fg_        { 0, 0, 0 };
};
