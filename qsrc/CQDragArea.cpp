#include <CQDragArea.h>

#include <QMouseEvent>
#include <QPainter>

#include <cmath>

CQDragRealArea::
CQDragRealArea(QWidget *parent) :
 CQDragArea(parent)
{
}

void
CQDragRealArea::
setValue(double r)
{
  value_ = r;

  if (dragMax() != dragMin())
    f_ = (value_ - dragMin())/(dragMax() - dragMin());
  else
    f_ = 0.0;

  update();
}

void
CQDragRealArea::
mousePressEvent(QMouseEvent *e)
{
  pressed_ = true;
  lastDx_  = 0;
  pos_     = e->globalPos();

  setCursor(Qt::SizeHorCursor);
}

void
CQDragRealArea::
mouseMoveEvent(QMouseEvent *e)
{
  if (! pressed_)
    return;

  auto pos = e->globalPos();

  int s1 = width()/2;
  int s2 = width() - s1;

  int dx = pos.x() - pos_.x();

  dx = std::min(std::max(dx, -s1), s2);

  if (dx == lastDx_)
    return;

  f_ = 1.0*(dx + s1)/width();

  value_ = dragMin() + f_*(dragMax() - dragMin());

  Q_EMIT dragValueChanged(value_);

  lastDx_ = dx;

  update();
}

void
CQDragRealArea::
mouseReleaseEvent(QMouseEvent *)
{
  pressed_ = false;

  setCursor(Qt::ArrowCursor);
}

void
CQDragRealArea::
mouseDoubleClickEvent(QMouseEvent *e)
{
  auto pos = e->pos();

  f_ = 0.0;

  if (width() > 0)
    f_ = (1.0*pos.x())/width();

  value_ = dragMin() + f_*(dragMax() - dragMin());

  Q_EMIT dragValueChanged(value_);

  update();
}

void
CQDragRealArea::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  auto px = f_*(width() - 4);

  painter.setBrush(QColor(200, 200, 255));
  painter.drawRect(QRect(2, 2, px + 2, height() - 4));

  painter.setBrush(QColor(240, 240, 240));
  painter.drawRect(QRect(px + 2, 2, width() - 4 - px, height() - 4));

#if 0
  QFontMetrics fm(font());

  painter.setPen(Qt::black);
  painter.drawText(2, height()/2 + (fm.ascent() - fm.descent())/2, QString("%1").arg(value_));
#endif
}

QSize
CQDragRealArea::
sizeHint() const
{
  QFontMetrics fm(font());

  return QSize(fm.horizontalAdvance("XXXX") + 4, fm.height() + 4);
}

//---

CQDragIntegerArea::
CQDragIntegerArea(QWidget *parent) :
 CQDragArea(parent)
{
}

void
CQDragIntegerArea::
setValue(int i)
{
  value_  = i;
  rvalue_ = i;

  if (dragMax() != dragMin())
    f_ = (rvalue_ - dragMin())/(dragMax() - dragMin());
  else
    f_ = 0.0;

  update();
}

void
CQDragIntegerArea::
mousePressEvent(QMouseEvent *e)
{
  pressed_ = true;
  lastDx_  = 0;
  pos_     = e->globalPos();

  setCursor(Qt::SizeHorCursor);
}

void
CQDragIntegerArea::
mouseMoveEvent(QMouseEvent *e)
{
  if (! pressed_)
    return;

  auto pos = e->globalPos();

  int s1 = width()/2;
  int s2 = width() - s1;

  int dx = pos.x() - pos_.x();

  dx = std::min(std::max(dx, -s1), s2);

  if (dx == lastDx_)
    return;

  f_ = 1.0*(dx + s1)/width();

  rvalue_ = dragMin() + f_*(dragMax() - dragMin());
  value_  = std::round(rvalue_);

  Q_EMIT dragValueChanged(value_);

  lastDx_ = dx;

  update();
}

void
CQDragIntegerArea::
mouseReleaseEvent(QMouseEvent *)
{
  pressed_ = false;

  setCursor(Qt::ArrowCursor);
}

void
CQDragIntegerArea::
mouseDoubleClickEvent(QMouseEvent *e)
{
  auto pos = e->pos();

  f_ = 0.0;

  if (width() > 0)
    f_ = (1.0*pos.x())/width();

  rvalue_ = dragMin() + f_*(dragMax() - dragMin());
  value_  = std::round(rvalue_);

  Q_EMIT dragValueChanged(value_);

  update();
}

void
CQDragIntegerArea::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  auto px = f_*(width() - 4);

  painter.setBrush(QColor(200, 200, 255));
  painter.drawRect(QRect(2, 2, px + 2, height() - 4));

  painter.setBrush(QColor(240, 240, 240));
  painter.drawRect(QRect(px + 2, 2, width() - 4 - px, height() - 4));

#if 0
  QFontMetrics fm(font());

  painter.setPen(Qt::black);
  painter.drawText(2, height()/2 + (fm.ascent() - fm.descent())/2, QString("%1").arg(value_));
#endif
}

QSize
CQDragIntegerArea::
sizeHint() const
{
  QFontMetrics fm(font());

  return QSize(fm.horizontalAdvance("XXXX") + 4, fm.height() + 4);
}

//---

CQDragArea::
CQDragArea(QWidget *parent) :
 QFrame(parent)
{
}
