#include <CQLottieCanvas.h>
#include <CQLottie.h>

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

CQLottieCanvas::
CQLottieCanvas(CQLottie *lottie) :
 lottie_(lottie)
{
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);
}

void
CQLottieCanvas::
invalidate()
{
  needsUpdate_ = true;

  update();
}

void
CQLottieCanvas::
resizeEvent(QResizeEvent *)
{
  lottie_->setPixelSize(width(), height());

  needsUpdate_ = true;
}

void
CQLottieCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing);

  painter.fillRect(rect(), lottie_->bgColor());

  lottie_->draw(&painter, needsUpdate_);

  needsUpdate_ = false;
}

void
CQLottieCanvas::
mousePressEvent(QMouseEvent *me)
{
  auto pos = me->pos();

  lottie_->mousePress(pos);
}

void
CQLottieCanvas::
mouseMoveEvent(QMouseEvent *me)
{
  auto pos = me->pos();

  lottie_->mouseMove(pos);
}

void
CQLottieCanvas::
keyPressEvent(QKeyEvent *ke)
{
  auto key = ke->key();

  if      (key == Qt::Key_Plus) {
    lottie_->zoom(true);
  }
  else if (key == Qt::Key_Minus) {
    lottie_->zoom(false);
  }
  else if (key == Qt::Key_Left) {
    lottie_->scroll(-1, 0);
  }
  else if (key == Qt::Key_Right) {
    lottie_->scroll(1, 0);
  }
  else if (key == Qt::Key_Up) {
    lottie_->scroll(0, 1);
  }
  else if (key == Qt::Key_Down) {
    lottie_->scroll(0, -1);
  }
  else if (key == Qt::Key_Home) {
    lottie_->zoomFull();
  }

  invalidate();
}
