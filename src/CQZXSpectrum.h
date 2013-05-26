#include <QWidget>
#include <CZ80.h>
#include <CZXSpectrum.h>

class CQZXSpectrum : public QWidget, public CZ80Screen {
  Q_OBJECT

 private:
  CZXSpectrum *spectrum_;
  int          border_;
  bool         memChanged_;

 public:
  CQZXSpectrum(CZXSpectrum *spectrum, int w, int h);

 ~CQZXSpectrum();

  void memChanged(ushort pos, ushort len);

  void redraw();

  void paintEvent(QPaintEvent *);
  void keyPressEvent(QKeyEvent *e);
  void keyReleaseEvent(QKeyEvent *e);

 public slots:
  void itimeOut();
  void stimeOut();
};

class CQZXSpectrumRenderer : public CZXSpectrumRenderer {
 public:
  CQZXSpectrumRenderer(CQZXSpectrum *qspectrum, QPainter *painter) :
   qspectrum_(qspectrum), painter_(painter) {
  }

  void setForeground(const CRGBA &fg);

  void fillRectangle(int x, int y, int w, int h);

  void drawPoint(int x, int y);

 private:
  CQZXSpectrum *qspectrum_;
  QPainter     *painter_;
  CRGBA         fg_;
};
