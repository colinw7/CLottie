#include <CQLottieImage.h>
#include <CQImageDisplay.h>

#include <CQColorChooser.h>

#include <QLabel>
#include <QVBoxLayout>

CQLottieImage::
CQLottieImage()
{
  auto *layout = new QVBoxLayout(this);

  //---

  auto *controlFrame  = new QFrame;
  auto *controlLayout = new QHBoxLayout(controlFrame);

  layout->addWidget(controlFrame);

  infoLabel_ = new QLabel;
  bgColor_   = new CQColorChooser;

  bgColor_->setAutoFillBackground(true);

  bgColor_->setStyles(CQColorChooser::Text | CQColorChooser::ColorButton);

  connect(bgColor_, SIGNAL(colorChanged(const QColor &)), this, SLOT(bgColorChanged()));

  controlLayout->addWidget(infoLabel_);
  controlLayout->addStretch(1);
  controlLayout->addWidget(bgColor_);

  //---

  imageDisplay_ = new CQImageDisplay;

  layout->addWidget(imageDisplay_);

  //---

  bgColor_->setColor(imageDisplay_->background());
}

QImage
CQLottieImage::
image() const
{
  return imageDisplay_->getImage();
}

void
CQLottieImage::
setImage(const QImage &i)
{
  imageDisplay_->setImage(i);

  imageDisplay_->update();

  QString str;

  if (! i.isNull())
    str = QString("%1 x %2").arg(i.width()).arg(i.height());
  else
    str = "<empty>";

  infoLabel_->setText(str);
}

void
CQLottieImage::
bgColorChanged()
{
  auto *chooser = qobject_cast<CQColorChooser *>(sender());

  imageDisplay_->setBackground(chooser->color());

  imageDisplay_->update();
}
