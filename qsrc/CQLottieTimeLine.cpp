#include <CQLottieTimeLine.h>
#include <CQLottie.h>
#include <CQLottieStatusBar.h>

#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

namespace {

QTransform toQTransform(const CMatrix2D &m) {
  double a, b, c, d, tx, ty;

  m.getValues(&a, &b, &c, &d, &tx, &ty);

  return QTransform(a, c, b, d, tx, ty);
}

}

//---

CQLottieTimeLine::
CQLottieTimeLine(CQLottie *lottie) :
 lottie_(lottie)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);

  canvas_ = new CQLottieTimeLineCanvas(this);

  layout->addWidget(canvas_);
}

void
CQLottieTimeLine::
setProperty(CLottieProperty *prop)
{
  prop_ = prop;

  canvas_->update();
}

//---

CQLottieTimeLineCanvas::
CQLottieTimeLineCanvas(CQLottieTimeLine *timeLine) :
 timeLine_(timeLine)
{
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);

  auto font = this->font();
  font.setPointSizeF(font.pointSizeF()*0.8);
  setFont(font);
}

void
CQLottieTimeLineCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), QBrush(Qt::white));

  auto *lottie = timeLine_->lottie();

  CLottieUtil::TimeFrame timeFrame;
  lottie->getTimeFrame(timeFrame);

  auto frameStop = int(timeFrame.frameStop.value_or(0.0));

  displayRange_.setWindowRange(-1.0, -0.1, frameStop + 1.0, 1.2);

  painter.setTransform(toQTransform(displayRange_.getMatrix()));

  //---

  QPen pen(QColor(100, 100, 100));

  pen.setWidth(0);
  pen.setCosmetic(true);

  painter.setPen(pen);

  for (int i = 0; i < frameStop; ++i) {
    painter.drawLine(QPointF(i, 0.0), QPointF(i, 1.0));
  }

  //---

  pen.setColor(Qt::red);
  pen.setWidth(3);

  painter.setPen(pen);

  painter.drawLine(QPointF(timeFrame.frame, 0.0), QPointF(timeFrame.frame, 1.0));

  //---

  auto *prop = timeLine_->property();

  //---

  auto pointSize = 3.0;

  double px, py;
  displayRange_.pixelWidthToWindowWidth  (1, &px);
  displayRange_.pixelHeightToWindowHeight(1, &py);

  auto drawPoint = [&](const QPointF &p) {
    auto p1 = QPointF(p.x() - pointSize*px, p.y() - pointSize*py);
    auto p2 = QPointF(p.x() + pointSize*px, p.y() + pointSize*py);
    auto p3 = QPointF(p.x() + pointSize*px, p.y() - pointSize*py);
    auto p4 = QPointF(p.x() - pointSize*px, p.y() + pointSize*py);

    painter.drawLine(p1, p2);
    painter.drawLine(p3, p4);
  };

  //---

  QFontMetrics fm(font());

  auto n = (prop ? prop->numKeyFrames() : 0);

  for (size_t i = 0; i < n; ++i) {
//  auto *keyFrame0 = (i > 0     ? prop->keyFrame(i - 1) : nullptr);
    auto *keyFrame1 =              prop->keyFrame(i    );
    auto *keyFrame2 = (i < n - 1 ? prop->keyFrame(i + 1) : nullptr);

//  auto t0 = (keyFrame0 ? keyFrame0->timeFrame().value_or(0) : 0.0);
    auto t1 =              keyFrame1->timeFrame().value_or(0);
    auto t2 = (keyFrame2 ? keyFrame2->timeFrame().value_or(0) : frameStop);

    pen.setColor(Qt::blue);
    pen.setWidth(2);

    painter.setPen(pen);

    painter.drawLine(QPointF(t1, 0), QPointF(t1, 1));

    pen.setColor(Qt::green);
    pen.setWidth(2);

    painter.setPen(pen);

    const auto &ovalues = keyFrame1->ovalues(); // out of previous key frame
    const auto &ivalues = keyFrame1->ivalues(); // into next key frame

    auto nio = std::min(ivalues.size(), ovalues.size());

    for (size_t io = 0; io < nio; ++io) {
      const auto &ovalue = ovalues[io];
      const auto &ivalue = ivalues[io];

      auto nv = std::min(ivalue.size(), ovalue.size());

      for (size_t iv = 0; iv < nv; ++iv) {
        auto p0 = QPointF(t1, 0.0);
        auto p1 = QPointF(t1 + (t2 - t1)*ovalue.xvals[iv], ovalue.yvals[iv]);
        auto p2 = QPointF(t1 + (t2 - t1)*ivalue.xvals[iv], ivalue.yvals[iv]);
        auto p3 = QPointF(t2, 1.0);

#if 1
        QPainterPath path;

        path.moveTo(p0);
        path.cubicTo(p1, p2, p3);

        painter.drawPath(path);
#else
        painter.drawLine(p0, p1);
        painter.drawLine(p1, p2);
        painter.drawLine(p2, p3);
#endif

        drawPoint(p1);
        drawPoint(p2);
      }
    }
  }

  //---

  painter.setTransform(QTransform());

  painter.setPen(Qt::black);

  auto drawText = [&](double x, double y, const QString &str, bool above=false) {
    double tpx, tpy;
    displayRange_.windowToPixel(x, y, &tpx, &tpy);

    auto tw = fm.horizontalAdvance(str);
    if (above)
      painter.drawText(tpx - tw/2.0, tpy - fm.descent(), str);
    else
      painter.drawText(tpx - tw/2.0, tpy + fm.ascent(), str);
  };

  drawText(      0.0, 1.0, QString::number(0)        , /*above*/true);
  drawText(frameStop, 1.0, QString::number(frameStop), /*above*/true);

  for (size_t i = 0; i < n; ++i) {
    auto *keyFrame1 = prop->keyFrame(i);

    auto t1 = keyFrame1->timeFrame().value_or(0);

    double tpx, tpy;
    displayRange_.windowToPixel(t1, 0.0, &tpx, &tpy);

    timeFrame.frame = t1;
    auto str = QString::fromStdString(prop->tvalueStr(timeFrame));

    drawText(t1, 0.0, str);

    str = QString::number(t1);

    drawText(t1, 1.1, str, /*above*/true);
  }
}

void
CQLottieTimeLineCanvas::
resizeEvent(QResizeEvent *)
{
  displayRange_.setPixelRange(0, 0, width() - 1, height() - 1);
}

QSize
CQLottieTimeLineCanvas::
sizeHint() const
{
  return QSize(1000, 300);
}

void
CQLottieTimeLineCanvas::
mouseMoveEvent(QMouseEvent *me)
{
  auto pos = me->pos();

  CPoint2D p;
  displayRange_.pixelToWindow(CPoint2D(pos.x(), pos.y()), p);

  auto *lottie = timeLine_->lottie();

  lottie->status()->setStatusLabel(QString("%1 %2").arg(p.x).arg(p.y));
}
