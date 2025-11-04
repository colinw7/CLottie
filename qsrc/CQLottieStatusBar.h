#ifndef CQLottieStatusBar_H
#define CQLottieStatusBar_H

#include <QFrame>

class CQLottie;

class QLabel;

class CQLottieStatusBar : public QFrame {
  Q_OBJECT

 public:
  CQLottieStatusBar(CQLottie *lottie);

  void setTicksLabel(const QString &text);
  void setStatusLabel(const QString &text);

 private:
  CQLottie *lottie_ { nullptr };

  QLabel* statusLabel_ { nullptr };
  QLabel* ticksLabel_  { nullptr };
};

#endif
