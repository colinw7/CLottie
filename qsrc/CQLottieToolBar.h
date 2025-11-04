#ifndef CQLottieToolBar_H
#define CQLottieToolBar_H

#include <QFrame>

class CQLottie;

class CQLottieToolBar : public QFrame {
  Q_OBJECT

 public:
  CQLottieToolBar(CQLottie *lottie);

 private Q_SLOTS:
  void loadSlot();

  void playSlot();
  void pauseSlot();
  void stepSlot();

  void timeLineSlot();

 private:
  CQLottie *lottie_ { nullptr };
};

#endif
