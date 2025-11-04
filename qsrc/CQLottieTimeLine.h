#ifndef CQLottieTimeLine_H
#define CQLottieTimeLine_H

#include <CDisplayRange2D.h>

#include <QFrame>

class CQLottie;
class CQLottieTimeLineCanvas;
class CLottieProperty;

class CQLottieTimeLine : public QFrame {
  Q_OBJECT

 public:
  CQLottieTimeLine(CQLottie *lottie);

  CQLottie *lottie() const { return lottie_; }

  CLottieProperty *property() const { return prop_; }
  void setProperty(CLottieProperty *prop);

 private:
  CQLottie*               lottie_ { nullptr };
  CQLottieTimeLineCanvas* canvas_ { nullptr };
  CLottieProperty*        prop_   { nullptr };
};

//---

class CQLottieTimeLineCanvas : public QFrame {
  Q_OBJECT

 public:
  CQLottieTimeLineCanvas(CQLottieTimeLine *timeLine);

  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;

  void mouseMoveEvent(QMouseEvent *) override;

  QSize sizeHint() const override;

 private:
  CQLottieTimeLine *timeLine_ { nullptr };
  CDisplayRange2D   displayRange_;
};

#endif
