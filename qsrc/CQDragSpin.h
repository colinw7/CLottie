#ifndef CQDragSpin_H
#define CQDragSpin_H

#include <QFrame>

class CQDragRealArea;
class CQDragIntegerArea;

class CQRealSpin;
class CQIntegerSpin;

class CQDragRealSpin : public QFrame {
  Q_OBJECT

  Q_PROPERTY(double value READ value WRITE setValue)

 public:
  CQDragRealSpin(QWidget *parent=nullptr);

  double value() const;
  void setValue(double r);

  void setRange(double min, double max);

 private:
  void connectSlots(bool);

 private Q_SLOTS:
  void spinValueChanged(double);
  void dragValueChanged(double);

 Q_SIGNALS:
  void valueChanged(double);

 private:
  CQRealSpin*     spin_ { nullptr };
  CQDragRealArea* area_ { nullptr };
};

//---

class CQDragIntegerSpin : public QFrame {
  Q_OBJECT

  Q_PROPERTY(double value READ value WRITE setValue)

 public:
  CQDragIntegerSpin(QWidget *parent=nullptr);

  int value() const;
  void setValue(int r);

  void setRange(int min, int max);

 private:
  void connectSlots(bool);

 private Q_SLOTS:
  void spinValueChanged(int);
  void dragValueChanged(int);

 Q_SIGNALS:
  void valueChanged(int);

 private:
  CQIntegerSpin*     spin_ { nullptr };
  CQDragIntegerArea* area_ { nullptr };
};

#endif
