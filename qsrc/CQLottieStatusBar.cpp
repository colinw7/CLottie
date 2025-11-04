#include <CQLottieStatusBar.h>
#include <CQLottie.h>

#include <QHBoxLayout>
#include <QLabel>

CQLottieStatusBar::
CQLottieStatusBar(CQLottie *lottie) :
 lottie_(lottie)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);

  ticksLabel_  = new QLabel(" ");
  statusLabel_ = new QLabel(" ");

  layout->addWidget(statusLabel_);
  layout->addStretch(1);
  layout->addWidget(ticksLabel_);
}

void
CQLottieStatusBar::
setTicksLabel(const QString &text)
{
  ticksLabel_->setText(text);
}

void
CQLottieStatusBar::
setStatusLabel(const QString &text)
{
  statusLabel_->setText(text);
}
