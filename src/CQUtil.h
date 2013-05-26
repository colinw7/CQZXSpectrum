#ifndef CQUtil_H
#define CQUtil_H

#include <Qt>
#include <QColor>
#include <CKeyType.h>
#include <CRGBA.h>

namespace CQUtil {
  CKeyType convertKey(int key, Qt::KeyboardModifiers modifiers);

  QColor rgbaToColor(const CRGBA &rgba);
}

#endif
