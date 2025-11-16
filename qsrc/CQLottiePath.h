#ifndef CQLottiePath_H
#define CQLottiePath_H

#include <CDisplayRange2D.h>

#include <QFrame>

class CQLottie;
class CQLottiePathCanvas;
class CQLottiePathControl;
class CLottieProperty;
class CLottieShape;

class CQColorEdit;
class CQRealSpin;

class CQLottiePath : public QFrame {
  Q_OBJECT

 public:
  CQLottiePath(CQLottie *lottie);

  CQLottie *lottie() const { return lottie_; }

  CQLottiePathCanvas *canvas() const { return canvas_; }

  CLottieProperty *property() const { return prop_; }
  void setProperty(CLottieProperty *prop);

  CLottieShape *shape() const { return shape_; }
  void setShape(CLottieShape *shape);

 private:
  CQLottie*            lottie_  { nullptr };
  CQLottiePathCanvas*  canvas_  { nullptr };
  CQLottiePathControl* control_ { nullptr };
  CLottieProperty*     prop_    { nullptr };
  CLottieShape*        shape_   { nullptr };
};

//---

class CQLottiePathCanvas : public QFrame {
  Q_OBJECT

 public:
  CQLottiePathCanvas(CQLottiePath *path);

  const QColor &bgColor() const { return bgColor_; }
  void setBgColor(const QColor &c);

  double trimStart() const { return trimStart_; }
  double trimEnd() const { return trimEnd_; }

  void setTrimRange(double s, double e);

  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;

  void mouseMoveEvent(QMouseEvent *) override;

  QSize sizeHint() const override;

 private:
  CQLottiePath*   path_ { nullptr };
  CDisplayRange2D displayRange_;

  QColor bgColor_ { 255, 255, 255 };

  double trimStart_ { 0.0 };
  double trimEnd_   { 1.0 };
};

//---

class CQLottiePathControl : public QFrame {
  Q_OBJECT

 public:
  CQLottiePathControl(CQLottiePath *path);

  void updateWidgets();

 private:
  void connectSlots(bool b);

 private Q_SLOTS:
  void bgFillSlot(const QColor &);
  void rangeChanged();

 private:
  CQLottiePath* path_ { nullptr };

  CQColorEdit* bgFillEdit_    { nullptr };
  CQRealSpin*  trimStartEdit_ { nullptr };
  CQRealSpin*  trimEndEdit_   { nullptr };
};

#endif
