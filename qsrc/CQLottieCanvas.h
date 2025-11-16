#ifndef CQLottieCanvas_H
#define CQLottieCanvas_H

#include <QWidget>

class CQLottie;

class CQLottieCanvas : public QWidget {
  Q_OBJECT

 public:
  CQLottieCanvas(CQLottie *lottie);

  void invalidate();

  void resizeEvent(QResizeEvent *) override;

  void paintEvent(QPaintEvent *) override;

  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent (QMouseEvent *) override;

  void keyPressEvent(QKeyEvent *) override;

  QSize sizeHint() const override { return QSize(1600, 1600); }

 private Q_SLOTS:
  void customContextMenuSlot(const QPoint &);

 private:
  CQLottie* lottie_ { nullptr };

  bool needsUpdate_ { true };
};

#endif
