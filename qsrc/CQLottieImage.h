#ifndef CQLottieImage_H
#define CQLottieImage_H

#include <QFrame>

class CQImageDisplay;

class CQColorChooser;

class QLabel;

class CQLottieImage : public QFrame {
  Q_OBJECT

 public:
  CQLottieImage();

  QImage image() const;
  void setImage(const QImage &i);

 private Q_SLOTS:
  void bgColorChanged();

 private:
  CQImageDisplay* imageDisplay_ { nullptr };
  QLabel*         infoLabel_    { nullptr };
  CQColorChooser* bgColor_      { nullptr };
};

#endif
