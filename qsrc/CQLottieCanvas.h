#ifndef CQLottieCanvas_H
#define CQLottieCanvas_H

#include <QWidget>

class CQLottie;

class CQLottieCanvas : public QWidget {
  Q_OBJECT

 public:
  CQLottieCanvas(CQLottie *lottie);

  bool isCheckerBoard() const { return checkerBoard_; }
  void setCheckerBoard(bool b) { checkerBoard_ = b; }

  double checkerBoardSize() const { return checkerBoardSize_; }
  void setCheckerBoardSize(double r) { checkerBoardSize_ = r; }

  //---

  void invalidate();

  //---

  void resizeEvent(QResizeEvent *) override;

  void paintEvent(QPaintEvent *) override;

  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent (QMouseEvent *) override;

  void keyPressEvent(QKeyEvent *) override;

  QSize sizeHint() const override { return QSize(1600, 1600); }

 private:
  void drawCheckerboard(QPainter *painter, int cs) const;

 private Q_SLOTS:
  void customContextMenuSlot(const QPoint &);

 private:
  CQLottie* lottie_ { nullptr };

  bool   checkerBoard_     { false };
  double checkerBoardSize_ { 48 };
  bool   needsUpdate_      { true };
};

#endif
