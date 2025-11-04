#include <CQLottieSettings.h>
#include <CQLottie.h>

#include <CQColorEdit.h>
#include <CQUtil.h>

#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>

CQLottieSettings::
CQLottieSettings(CQLottie *lottie) :
 lottie_(lottie)
{
  std::vector<QLabel *> labels;

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  auto addLabelEdit = [&](const QString &label, auto *w) {
    auto *frame   = new QFrame;
    auto *layout1 = new QHBoxLayout(frame);

    auto *labelW = new QLabel(label);

    labels.push_back(labelW);

    layout1->addWidget(labelW);
    layout1->addWidget(w);

    layout->addWidget(frame);

    return w;
  };

  equalScale_       = addLabelEdit("Equal Scale"  , new QCheckBox(this));
  bgFillEdit_       = addLabelEdit("Bg Fill"      , new CQColorEdit(this));
  showSelect_       = addLabelEdit("Show Select"  , new QCheckBox(this));
  selectFillEdit_   = addLabelEdit("Select Fill"  , new CQColorEdit(this));
  selectStrokeEdit_ = addLabelEdit("Select Stroke", new CQColorEdit(this));
  showBBox_         = addLabelEdit("Show BBox"    , new QCheckBox(this));
  bboxStrokeEdit_   = addLabelEdit("BBox Stroke"  , new CQColorEdit(this));

  auto alignLabels = [&]() {
    QFontMetrics fm(font());

    int lw = 0;

    for (auto *label : labels)
      lw = std::max(lw, label->sizeHint().width());

    for (auto *label : labels)
      label->setFixedWidth(lw);
  };

  layout->addStretch(1);

  alignLabels();

  //---

  connectSlots(true);

  updateWidgets();
}

void
CQLottieSettings::
connectSlots(bool b)
{
  CQUtil::connectDisconnect(b, equalScale_, SIGNAL(stateChanged(int)),
                            this, SLOT(equalScaleSlot(int)));
  CQUtil::connectDisconnect(b, bgFillEdit_, SIGNAL(colorChanged(const QColor &)),
                            this, SLOT(bgFillSlot(const QColor &)));
  CQUtil::connectDisconnect(b, showSelect_, SIGNAL(stateChanged(int)),
                            this, SLOT(showSelectSlot(int)));
  CQUtil::connectDisconnect(b, selectFillEdit_, SIGNAL(colorChanged(const QColor &)),
                            this, SLOT(selectedFillSlot(const QColor &)));
  CQUtil::connectDisconnect(b, selectStrokeEdit_, SIGNAL(colorChanged(const QColor &)),
                            this, SLOT(selectedStrokeSlot(const QColor &)));
  CQUtil::connectDisconnect(b, showBBox_, SIGNAL(stateChanged(int)),
                            this, SLOT(showBBoxSlot(int)));
  CQUtil::connectDisconnect(b, bboxStrokeEdit_, SIGNAL(colorChanged(const QColor &)),
                            this, SLOT(bboxStrokeSlot(const QColor &)));
}

void
CQLottieSettings::
updateWidgets()
{
  connectSlots(false);

  equalScale_      ->setChecked(lottie_->isEqualScale());
  bgFillEdit_      ->setColor(lottie_->bgColor());
  showSelect_      ->setChecked(lottie_->isShowSelect());
  selectFillEdit_  ->setColor(lottie_->selectedBrushColor());
  selectStrokeEdit_->setColor(lottie_->selectedPenColor());
  showBBox_        ->setChecked(lottie_->isShowBBox());
  bboxStrokeEdit_  ->setColor(lottie_->bboxPenColor());

  connectSlots(true);
}

void
CQLottieSettings::
equalScaleSlot(int state)
{
  lottie_->setEqualScale(state);

  lottie_->updateAll();
}

void
CQLottieSettings::
bgFillSlot(const QColor &c)
{
  lottie_->setBgColor(c);

  lottie_->updateAll();
}

void
CQLottieSettings::
showSelectSlot(int state)
{
  lottie_->setShowSelect(state);

  lottie_->updateAll();
}

void
CQLottieSettings::
selectedFillSlot(const QColor &c)
{
  lottie_->setSelectedBrushColor(c);

  lottie_->updateAll();
}

void
CQLottieSettings::
selectedStrokeSlot(const QColor &c)
{
  lottie_->setSelectedPenColor(c);

  lottie_->updateAll();
}

void
CQLottieSettings::
showBBoxSlot(int state)
{
  lottie_->setShowBBox(state);

  lottie_->updateAll();
}

void
CQLottieSettings::
bboxStrokeSlot(const QColor &c)
{
  lottie_->setBBoxPenColor(c);

  lottie_->updateAll();
}
