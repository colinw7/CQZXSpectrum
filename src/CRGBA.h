#ifndef CRGBA_H
#define CRGBA_H

struct CRGBA {
  CRGBA() { }

  CRGBA(double r1, double g1, double b1, double a1=1.0) :
   r(r1), g(g1), b(b1), a(a1) {
  }

  double r, g, b, a;
};

#endif
