#ifndef CQLottieSettings_H
#define CQLottieSettings_H

#include <QFrame>

class CQLottie;

class CQColorEdit;
class QCheckBox;

class CQLottieSettings : public QFrame {
  Q_OBJECT

 public:
  CQLottieSettings(CQLottie *lottie);

 private:
  void connectSlots(bool b);
  void updateWidgets();

 private Q_SLOTS:
  void equalScaleSlot(int);
  void bgFillSlot(const QColor &);
  void showSelectSlot(int);
  void selectedFillSlot(const QColor &);
  void selectedStrokeSlot(const QColor &);
  void showBBoxSlot(int);
  void bboxStrokeSlot(const QColor &);

 private:
  CQLottie *lottie_ { nullptr };

  QCheckBox*   equalScale_       { nullptr };
  CQColorEdit* bgFillEdit_       { nullptr };
  QCheckBox*   showSelect_       { nullptr };
  CQColorEdit* selectFillEdit_   { nullptr };
  CQColorEdit* selectStrokeEdit_ { nullptr };
  QCheckBox*   showBBox_         { nullptr };
  CQColorEdit* bboxStrokeEdit_   { nullptr };
};

#endif
