#ifndef CQDragArea_H
#define CQDragArea_H

#include <QFrame>

class CQDragArea : public QFrame {
  Q_OBJECT

  Q_PROPERTY(bool dragable  READ isDragable  WRITE setDragable )
  Q_PROPERTY(bool showValue READ isShowValue WRITE setShowValue)

 public:
  CQDragArea(QWidget *parent=nullptr);

  bool isDragable() const { return dragable_; }
  void setDragable(bool b) { dragable_ = b; }

  bool isShowValue() const { return showValue_; }
  void setShowValue(bool b) { showValue_ = b; }

 protected:
  bool dragable_  { true };
  bool showValue_ { false };

  bool   pressed_ { false };
  QPoint pos_;
  double f_       { 0.0 };
  int    lastDx_  { 0 };
};

class CQDragRealArea : public CQDragArea {
  Q_OBJECT

  Q_PROPERTY(double value   READ value   WRITE setValue  )
  Q_PROPERTY(double dragMin READ dragMin WRITE setDragMin)
  Q_PROPERTY(double dragMax READ dragMax WRITE setDragMax)

 public:
  CQDragRealArea(QWidget *parent=nullptr);

  double value() const { return value_; }
  void setValue(double r);

  double dragMin() const { return dragMin_; }
  void setDragMin(double r) { dragMin_ = r; update(); }

  double dragMax() const { return dragMax_; }
  void setDragMax(double r) { dragMax_ = r; update(); }

  void setDragRange(double min, double max) { dragMin_ = min; dragMax_ = max; update(); }

 Q_SIGNALS:
  void dragValueChanged(double v);

 private:
  void mousePressEvent  (QMouseEvent *) override;
  void mouseMoveEvent   (QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

  void mouseDoubleClickEvent(QMouseEvent *) override;

  void paintEvent(QPaintEvent *) override;

  QSize sizeHint() const override;

 private:
  double dragMin_ { -1.0 };
  double dragMax_ { 1.0 };
  double value_   { 0.0 };
};

//----

class CQDragIntegerArea : public CQDragArea {
  Q_OBJECT

  Q_PROPERTY(int value   READ value   WRITE setValue  )
  Q_PROPERTY(int dragMin READ dragMin WRITE setDragMin)
  Q_PROPERTY(int dragMax READ dragMax WRITE setDragMax)

 public:
  CQDragIntegerArea(QWidget *parent=nullptr);

  int value() const { return value_; }
  void setValue(int i);

  int dragMin() const { return dragMin_; }
  void setDragMin(int i) { dragMin_ = i; update(); }

  int dragMax() const { return dragMax_; }
  void setDragMax(int i) { dragMax_ = i; update(); }

  void setDragRange(int min, int max) { dragMin_ = min; dragMax_ = max; update(); }

 Q_SIGNALS:
  void dragValueChanged(int v);

 private:
  void mousePressEvent  (QMouseEvent *) override;
  void mouseMoveEvent   (QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

  void mouseDoubleClickEvent(QMouseEvent *) override;

  void paintEvent(QPaintEvent *) override;

  QSize sizeHint() const override;

 private:
  int    dragMin_ { 0 };
  int    dragMax_ { 100 };
  double rvalue_  { 0.0 };
  int    value_   { 0 };
};

#endif
