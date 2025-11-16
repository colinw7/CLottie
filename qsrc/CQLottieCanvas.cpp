#include <CQLottieCanvas.h>
#include <CQLottie.h>

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>

CQLottieCanvas::
CQLottieCanvas(CQLottie *lottie) :
 lottie_(lottie)
{
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);

  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(customContextMenuSlot(const QPoint&)));
}

void
CQLottieCanvas::
customContextMenuSlot(const QPoint &pos)
{
  // Map point to global from the viewport to account for the header.
  auto gpos = mapToGlobal(pos);

  auto *menu = new QMenu;

  auto *zoomFullAction     = new QAction("Zoom Full", menu);
  auto *showTimeLineAction = new QAction("Show Time Line", menu);
  auto *showPathAction     = new QAction("Show Path", menu);

  showTimeLineAction->setCheckable(true);
  showPathAction    ->setCheckable(true);

  showTimeLineAction->setChecked(lottie_->isShowTimeLine());
  showPathAction    ->setChecked(lottie_->isShowPath());

  menu->addAction(zoomFullAction);
  menu->addAction(showTimeLineAction);
  menu->addAction(showPathAction);

  connect(zoomFullAction    , SIGNAL(triggered()), lottie_, SLOT(zoomFull()));
  connect(showTimeLineAction, SIGNAL(triggered(bool)), lottie_, SLOT(setShowTimeLine(bool)));
  connect(showPathAction    , SIGNAL(triggered(bool)), lottie_, SLOT(setShowPath(bool)));

  menu->exec(gpos);

  delete menu;
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
  else if (key == Qt::Key_F1) {
    lottie_->nextGeomShape();
  }
}
