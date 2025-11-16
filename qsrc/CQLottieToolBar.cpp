#include <CQLottieToolBar.h>
#include <CQLottie.h>

#include <CQIconButton.h>
#include <CQSlider.h>
#include <CQUtil.h>

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

  QFontMetrics fm(font());

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  //---

  auto *tlayout = new QHBoxLayout(this);

  auto *playButton  = addToolButton("play" , "PLAY"    , "Play" , SLOT(playSlot()));
  auto *pauseButton = addToolButton("pause", "PAUSE"   , "Pause", SLOT(pauseSlot()));
  auto *stepButton  = addToolButton("step" , "PLAY_ONE", "Step" , SLOT(stepSlot()));

  tlayout->addWidget(playButton);
  tlayout->addWidget(pauseButton);
  tlayout->addWidget(stepButton);

  frameSlider_ = new CQSlider;

  auto tw = fm.horizontalAdvance("XXXXXXXXXXXXXXXX");
  frameSlider_->setFixedWidth(tw);

  tlayout->addWidget(frameSlider_);

  tlayout->addWidget(spacerBox());

  auto *timelineButton = addToolButton("timeLine" , "CLOCK", "Time Line" , SLOT(timeLineSlot()));

  tlayout->addWidget(timelineButton);

  tlayout->addStretch(1);

  auto *loadButton = new QPushButton("Load");

  connect(loadButton, SIGNAL(clicked()), this, SLOT(loadSlot()));

  tlayout->addWidget(loadButton);

  //---

  connectSlots(true);

  updateWidgets();
}

void
CQLottieToolBar::
updateWidgets()
{
  connectSlots(false);

  CLottieUtil::TimeFrame timeFrame;
  lottie_->getTimeFrame(timeFrame);

  frameSlider_->setRange(timeFrame.frameStart.value_or(0), timeFrame.frameStop.value_or(100));

  frameSlider_->setValue(timeFrame.frame);

  connectSlots(true);
}

void
CQLottieToolBar::
connectSlots(bool b)
{
  CQUtil::connectDisconnect(b, frameSlider_, SIGNAL(valueChanged(int)),
                            this, SLOT(frameSlot(int)));
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
frameSlot(int t)
{
  connectSlots(false);

  lottie_->setTicks(t);

  connectSlots(true);
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
