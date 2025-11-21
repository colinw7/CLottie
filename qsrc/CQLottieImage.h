#ifndef CQLottieImage_H
#define CQLottieImage_H

#include <QFrame>

class CQImageDisplay;

class CQColorChooser;

class QLabel;
class QCheckBox;

class CQLottieImage : public QFrame {
  Q_OBJECT

 public:
  CQLottieImage();

  QImage image() const;
  void setImage(const QImage &i);

 private Q_SLOTS:
  void bgColorChanged();
  void checkerBoardSlot(int);

 private:
  void connectSlots();

 private:
  CQImageDisplay* imageDisplay_      { nullptr };
  QLabel*         infoLabel_         { nullptr };
  CQColorChooser* bgColor_           { nullptr };
  QCheckBox*      checkerBoardCheck_ { nullptr };
};

#endif
