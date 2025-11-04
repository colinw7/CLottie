#include <CQLottieToolBar.h>
#include <CQLottie.h>

#include <CQIconButton.h>

#include <QPushButton>
#include <QHBoxLayout>

CQLottieToolBar::
CQLottieToolBar(CQLottie *lottie) :
 lottie_(lottie)
{
  auto addToolButton = [&](const QString &name, const QString &iconName,
                           const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  auto spacerBox = [&]() {
    auto *frame = new QFrame;
    frame->setFixedSize(QSize(8, 8));
    return frame;
  };

  //---

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *tlayout = new QHBoxLayout(this);

  auto *playButton  = addToolButton("play" , "PLAY"    , "Play" , SLOT(playSlot()));
  auto *pauseButton = addToolButton("pause", "PAUSE"   , "Pause", SLOT(pauseSlot()));
  auto *stepButton  = addToolButton("step" , "PLAY_ONE", "Step" , SLOT(stepSlot()));

  auto *timelineButton = addToolButton("timeLine" , "CLOCK", "Time Line" , SLOT(timeLineSlot()));

  tlayout->addWidget(playButton);
  tlayout->addWidget(pauseButton);
  tlayout->addWidget(stepButton);

  tlayout->addWidget(spacerBox());

  tlayout->addWidget(timelineButton);

  auto *loadButton = new QPushButton("Load");

  connect(loadButton, SIGNAL(clicked()), this, SLOT(loadSlot()));

  tlayout->addStretch(1);
  tlayout->addWidget(loadButton);
}

void
CQLottieToolBar::
playSlot()
{
  lottie_->playSlot();
}

void
CQLottieToolBar::
pauseSlot()
{
  lottie_->pauseSlot();
}

void
CQLottieToolBar::
stepSlot()
{
  lottie_->stepSlot();
}

void
CQLottieToolBar::
timeLineSlot()
{
  lottie_->setShowTimeLine(! lottie_->isShowTimeLine());
}

void
CQLottieToolBar::
loadSlot()
{
  lottie_->loadSlot();
}
