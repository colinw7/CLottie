#include <QPainterPathUtil.h>

namespace QPainterPathUtil {

void visitPath(const QPainterPath &path, QPainterPathVisitor &visitor) {
  visitor.path = &path;

  visitor.n = path.elementCount();

  visitor.init();

  for (visitor.i = 0; visitor.i < visitor.n; ++visitor.i) {
    const auto &e = path.elementAt(visitor.i);

    if      (e.isMoveTo()) {
      QPointF p(e.x, e.y);

      if (visitor.i < visitor.n - 1) {
        auto e1 = path.elementAt(visitor.i + 1);

        visitor.nextP = QPointF(e1.x, e1.y);
      }
      else
        visitor.nextP = p;

      visitor.moveTo(p);

      visitor.lastP = p;
    }
    else if (e.isLineTo()) {
      QPointF p(e.x, e.y);

      if (visitor.i < visitor.n - 1) {
        auto e1 = path.elementAt(visitor.i + 1);

        visitor.nextP = QPointF(e1.x, e1.y);
      }
      else
        visitor.nextP = p;

      visitor.lineTo(p);

      visitor.lastP = p;
    }
    else if (e.isCurveTo()) {
      QPointF p(e.x, e.y);

      QPointF p1, p2;

      QPainterPath::ElementType e1t { QPainterPath::MoveToElement };
      QPainterPath::ElementType e2t { QPainterPath::MoveToElement };

      if (visitor.i < visitor.n - 1) {
        auto e1 = path.elementAt(visitor.i + 1);

        e1t = e1.type;

        p1 = QPointF(e1.x, e1.y);
      }

      if (visitor.i < visitor.n - 2) {
        auto e2 = path.elementAt(visitor.i + 2);

        e2t = e2.type;

        p2 = QPointF(e2.x, e2.y);
      }

      if (e1t == QPainterPath::CurveToDataElement) {
        ++visitor.i;

        if (e2t == QPainterPath::CurveToDataElement) {
          ++visitor.i;

          if (visitor.i < visitor.n - 1) {
            auto e3 = path.elementAt(visitor.i + 1);

            visitor.nextP = QPointF(e3.x, e3.y);
          }
          else
            visitor.nextP = p;

          visitor.curveTo(p, p1, p2);

          visitor.lastP = p;
        }
        else {
          if (visitor.i < visitor.n - 1) {
            auto e3 = path.elementAt(visitor.i + 1);

            visitor.nextP = QPointF(e3.x, e3.y);
          }
          else
            visitor.nextP = p;

          visitor.quadTo(p, p1);

          visitor.lastP = p;
        }
      }
    }
    else
      assert(false);
  }

  visitor.term();
}

}
