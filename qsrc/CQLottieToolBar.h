#ifndef CQLottieToolBar_H
#define CQLottieToolBar_H

#include <QFrame>

class CQLottie;

class CQSlider;

class CQLottieToolBar : public QFrame {
  Q_OBJECT

 public:
  CQLottieToolBar(CQLottie *lottie);

  void updateWidgets();

 private:
  void connectSlots(bool);

 private Q_SLOTS:
  void loadSlot();

  void playSlot();
  void pauseSlot();
  void stepSlot();

  void frameSlot(int);

  void timeLineSlot();

 private:
  CQLottie* lottie_      { nullptr };
  CQSlider* frameSlider_ { nullptr };
};

#endif
