#include <CQDragSpin.h>
#include <CQDragArea.h>

#include <CQRealSpin.h>
#include <CQIntegerSpin.h>

#include <QHBoxLayout>

CQDragRealSpin::
CQDragRealSpin(QWidget *parent) :
 QFrame(parent)
{
  auto *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  spin_ = new CQRealSpin;
  area_ = new CQDragRealArea;

  layout->addWidget(spin_);
  layout->addWidget(area_);

  area_->setValue(value());

  connectSlots(true);
}

void
CQDragRealSpin::
connectSlots(bool b)
{
  if (b) {
    connect(spin_, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    connect(area_, SIGNAL(dragValueChanged(double)), this, SLOT(dragValueChanged(double)));
  }
  else {
    disconnect(spin_, SIGNAL(valueChanged(double)), this, SLOT(spinValueChanged(double)));
    disconnect(area_, SIGNAL(dragValueChanged(double)), this, SLOT(dragValueChanged(double)));
  }
}

void
CQDragRealSpin::
spinValueChanged(double r)
{
  connectSlots(false);

  area_->setValue(r);

  connectSlots(true);

  Q_EMIT valueChanged(r);
}

void
CQDragRealSpin::
dragValueChanged(double r)
{
  connectSlots(false);

  spin_->setValue(r);

  connectSlots(true);

  Q_EMIT valueChanged(r);
}

double
CQDragRealSpin::
value() const
{
  return spin_->value();
}

void
CQDragRealSpin::
setValue(double r)
{
  connectSlots(false);

  spin_->setValue(r);
  area_->setValue(r);

  connectSlots(true);
}

void
CQDragRealSpin::
setRange(double min, double max)
{
  connectSlots(false);

  spin_->setRange(min, max);

  area_->setDragRange(min, max);

  connectSlots(true);
}

//---

CQDragIntegerSpin::
CQDragIntegerSpin(QWidget *parent) :
 QFrame(parent)
{
  auto *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  spin_ = new CQIntegerSpin;
  area_ = new CQDragIntegerArea;

  layout->addWidget(spin_);
  layout->addWidget(area_);

  area_->setValue(value());

  connectSlots(true);
}

void
CQDragIntegerSpin::
connectSlots(bool b)
{
  if (b) {
    connect(spin_, SIGNAL(valueChanged(int)), this, SLOT(spinValueChanged(int)));
    connect(area_, SIGNAL(dragValueChanged(int)), this, SLOT(dragValueChanged(int)));
  }
  else {
    disconnect(spin_, SIGNAL(valueChanged(int)), this, SLOT(spinValueChanged(int)));
    disconnect(area_, SIGNAL(dragValueChanged(int)), this, SLOT(dragValueChanged(int)));
  }
}

void
CQDragIntegerSpin::
spinValueChanged(int i)
{
  connectSlots(false);

  area_->setValue(i);

  connectSlots(true);

  Q_EMIT valueChanged(i);
}

void
CQDragIntegerSpin::
dragValueChanged(int i)
{
  connectSlots(false);

  spin_->setValue(i);

  connectSlots(true);

  Q_EMIT valueChanged(i);
}

int
CQDragIntegerSpin::
value() const
{
  return spin_->value();
}

void
CQDragIntegerSpin::
setValue(int i)
{
  connectSlots(false);

  spin_->setValue(i);
  area_->setValue(i);

  connectSlots(true);
}

void
CQDragIntegerSpin::
setRange(int min, int max)
{
  connectSlots(false);

  spin_->setRange(min, max);

  area_->setDragRange(min, max);

  connectSlots(true);
}
