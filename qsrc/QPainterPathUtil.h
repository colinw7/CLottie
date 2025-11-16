#ifndef QPainterPathUtil_H
#define QPainterPathUtil_H

#include <QPainterPath>

class QPainterPathVisitor {
 public:
  QPainterPathVisitor() = default;

  virtual ~QPainterPathVisitor() = default;

  virtual void init() { }
  virtual void term() { }

  virtual void moveTo (const QPointF &p) = 0;
  virtual void lineTo (const QPointF &p) = 0;
  virtual void quadTo (const QPointF &p1, const QPointF &p2) = 0;
  virtual void curveTo(const QPointF &p1, const QPointF &p2, const QPointF &p3) = 0;

 public:
  const QPainterPath *path { nullptr };

  int     i { -1 };
  int     n { 0 };
  QPointF lastP;
  QPointF nextP;
};

//---

namespace QPainterPathUtil {

void visitPath(const QPainterPath &path, QPainterPathVisitor &visitor);

}

#endif
