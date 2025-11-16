#include <CQLottiePath.h>
#include <CQLottie.h>
#include <CQLottieStatusBar.h>

#include <CQColorEdit.h>
#include <CQRealSpin.h>
#include <CQUtil.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

#if 0
namespace {

QTransform toQTransform(const CMatrix2D &m) {
  double a, b, c, d, tx, ty;

  m.getValues(&a, &b, &c, &d, &tx, &ty);

  return QTransform(a, c, b, d, tx, ty);
}

}
#endif

//---

CQLottiePath::
CQLottiePath(CQLottie *lottie) :
 lottie_(lottie)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);

  canvas_ = new CQLottiePathCanvas(this);
  canvas_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  layout->addWidget(canvas_);

  control_ = new CQLottiePathControl(this);
  control_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  layout->addWidget(control_);

  //---

  control_->updateWidgets();
}

void
CQLottiePath::
setProperty(CLottieProperty *prop)
{
  prop_ = prop;

  canvas_->update();
}

void
CQLottiePath::
setShape(CLottieShape *shape)
{
  shape_ = shape;

  canvas_->update();
}

//---

CQLottiePathCanvas::
CQLottiePathCanvas(CQLottiePath *path) :
 path_(path)
{
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);

  auto font = this->font();
  font.setPointSizeF(font.pointSizeF()*0.8);
  setFont(font);
}

void
CQLottiePathCanvas::
setBgColor(const QColor &c)
{
  bgColor_ = c;

  update();
}

void
CQLottiePathCanvas::
setTrimRange(double s, double e)
{
  trimStart_ = s;
  trimEnd_   = e;

  update();
}

void
CQLottiePathCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), QBrush(bgColor()));

  auto *prop  = path_->property();
  auto *shape = path_->shape();

  auto *qlottie = path_->lottie();
  auto *lottie  = qlottie->lottie();

  CLottieUtil::TimeFrame timeFrame;
  qlottie->getTimeFrame(timeFrame);

  //---

  // get selected object/item path
  CBezierPath bezierPath;

  CLottieShape::Fill           *fill           = nullptr;
  CLottieShape::Stroke         *stroke         = nullptr;
  CLottieShape::GradientFill   *gradientFill   = nullptr;
  CLottieShape::GradientStroke *gradientStroke = nullptr;
  CLottieShape::Transform      *transform      = nullptr;

  if      (prop && prop->type() == CLottieProperty::Type::BEZIER) {
    const auto *bezierProp = dynamic_cast<CLottie::BezierProperty *>(prop);

    if (bezierProp)
      lottie->pathToBezier(*bezierProp, timeFrame, bezierPath);
  }
  else if (shape) {
    CLottieShape *geomShape = nullptr;

    if (shape->isGeomShape())
      geomShape = shape;

    if (! geomShape)
      geomShape = shape->getGeomShape();

    auto *fillShape           = shape->getFillShape          ();
    auto *strokeShape         = shape->getStrokeShape        ();
    auto *gradientFillShape   = shape->getGradientFillShape  ();
    auto *gradientStrokeShape = shape->getGradientStrokeShape();

    auto *transformShape = shape->getTransformShape();

    auto *shape1 = shape;
    auto *layer1 = static_cast<CLottieLayer *>(nullptr);

    while (! fillShape && ! strokeShape && ! gradientFillShape && ! gradientStrokeShape) {
      CLottieShape *parentShape;
      CLottieLayer *parentLayer;

      if (shape1) {
        parentShape = shape1->getParentShape();
        parentLayer = shape1->getParentLayer();
      }
      else {
        parentShape = nullptr;
        parentLayer = layer1->getParentLayer();
      }

      if      (parentShape) {
        if (! geomShape)
          geomShape = parentShape->getGeomShape();

        fillShape           = parentShape->getFillShape          ();
        strokeShape         = parentShape->getStrokeShape        ();
        gradientFillShape   = parentShape->getGradientFillShape  ();
        gradientStrokeShape = parentShape->getGradientStrokeShape();

        if (! transformShape)
          transformShape = parentShape->getTransformShape();

        shape1 = parentShape;
        layer1 = nullptr;
      }
      else if (parentLayer) {
        if (! geomShape)
          geomShape = parentLayer->getGeomShape();

        fillShape           = parentLayer->getFillShape          ();
        strokeShape         = parentLayer->getStrokeShape        ();
        gradientFillShape   = parentLayer->getGradientFillShape  ();
        gradientStrokeShape = parentLayer->getGradientStrokeShape();

        if (! transformShape)
          transformShape = parentLayer->getTransformShape();

        shape1 = nullptr;
        layer1 = parentLayer;
      }
      else
        break;
    }

    fill           = (fillShape           ? fillShape          ->fill          () : nullptr);
    stroke         = (strokeShape         ? strokeShape        ->stroke        () : nullptr);
    gradientFill   = (gradientFillShape   ? gradientFillShape  ->gradientFill  () : nullptr);
    gradientStroke = (gradientStrokeShape ? gradientStrokeShape->gradientStroke() : nullptr);

    transform = (transformShape ? transformShape->transform() : nullptr);

    if (geomShape) {
      auto shapeType = geomShape->shapeType();

      if      (shapeType == CLottieShape::ShapeType::PATH) {
        const auto &path = geomShape->path();

        lottie->pathToBezier(path, timeFrame, bezierPath);
      }
      else if (shapeType == CLottieShape::ShapeType::ELLIPSE) {
        bezierPath = qlottie->getEllipsePath(timeFrame, geomShape);
      }
      else if (shapeType == CLottieShape::ShapeType::RECTANGLE) {
        bezierPath = qlottie->getRectanglePath(timeFrame, geomShape);
      }
      else if (shapeType == CLottieShape::ShapeType::POLYSTAR) {
        bezierPath = qlottie->getPolyStarPath(timeFrame, geomShape);
      }
    }
  }
  else
    return;

  auto splitBezierPath = bezierPath.split(trimStart_, trimEnd_);

  //---

  QPainterPath path;
  qlottie->toQPath(bezierPath, path);

  QPainterPath splitPath;
  qlottie->toQPath(splitBezierPath, splitPath);

  //---

  CMatrixStack2D transformMatrix;

  if (transform)
    transformMatrix = lottie->getTransformMatrix(timeFrame, transform);

  //---

  auto rect = CQLottieUtil::transformRect(transformMatrix, path.boundingRect());

  displayRange_.setEqualScale(true);

  displayRange_.setWindowRange(rect.left(), rect.bottom(), rect.right(), rect.top());

  auto matrix = displayRange_.getMatrix()*transformMatrix.getMatrix();

  painter.setTransform(qlottie->toQTransform(matrix));

  QBrush brush;

  if      (gradientFill) {
    auto gradient = qlottie->calcGradientFill(timeFrame, gradientFill);

    brush = QBrush(gradient);
  }
  else if (fill) {
    auto c = fill->color  .tvalue(timeFrame, CRGBA(255, 255, 255)).value();
    auto o = fill->opacity.tvalue(timeFrame, 100.0).value();

    auto qcolor = CQLottieUtil::toQColor(c);
    qcolor.setAlpha(int(255*(o/100.0)));

    brush.setStyle(Qt::SolidPattern);
    brush.setColor(qcolor);
  }
  else
    brush = QBrush(Qt::NoBrush);

  QPainterPathStroker *stroker { nullptr };

  QPen pen;

  if      (gradientStroke) {
    auto gradient = qlottie->calcGradientStroke(timeFrame, gradientStroke);

    brush = QBrush(gradient);

  //auto o = gradientStroke->opacity.tvalue(timeFrame, 100.0).value();

    auto width      = gradientStroke->width.tvalue(timeFrame, 0).value();
    auto lineCap    = gradientStroke->lineCap.value_or(0);
    auto lineJoin   = gradientStroke->lineJoin.value_or(0);
    auto miterLimit = gradientStroke->miterLimit.value_or(0);

    stroker = new QPainterPathStroker();

    stroker->setCapStyle(CQLottieUtil::toLineCap(lineCap));
    stroker->setJoinStyle(CQLottieUtil::toLineJoin(lineJoin));
    stroker->setMiterLimit(miterLimit);

    stroker->setDashOffset (0.0);
    stroker->setDashPattern(Qt::SolidLine);

    stroker->setWidth(width);

    pen = QPen(Qt::NoPen);
  }
  else if (stroke) {
    auto c = stroke->color  .tvalue(timeFrame, CRGBA(0, 0, 0)).value();
    auto o = stroke->opacity.tvalue(timeFrame, 100.0).value();

    auto qcolor = CQLottieUtil::toQColor(c);
    qcolor.setAlpha(int(255*(o/100.0)));

    auto width      = stroke->width.tvalue(timeFrame, 0).value();
    auto lineCap    = stroke->lineCap.value_or(0);
    auto lineJoin   = stroke->lineJoin.value_or(0);
    auto miterLimit = stroke->miterLimit.value_or(0);

    pen.setColor(qcolor);
    pen.setWidthF(width);
    pen.setCapStyle(CQLottieUtil::toLineCap(lineCap));
    pen.setJoinStyle(CQLottieUtil::toLineJoin(lineJoin));
    pen.setMiterLimit(miterLimit);
  }
  else
    pen = QPen(Qt::NoPen);

  if (! fill && ! stroke && ! gradientFill && ! gradientStroke)
    pen = QPen(Qt::black);

  //---

  painter.setPen(QColor(100, 100, 100));
  painter.setBrush(Qt::NoBrush);

  if (stroker) {
    auto lpath = stroker->createStroke(path);

    lpath.setFillRule(Qt::WindingFill);

    painter.drawPath(lpath);

    delete stroker;
  }
  else
    painter.drawPath(path);

  //---

  painter.setPen(pen);
  painter.setBrush(brush);

  if (stroker) {
    auto lpath = stroker->createStroke(splitPath);

    lpath.setFillRule(Qt::WindingFill);

    painter.drawPath(lpath);

    delete stroker;
  }
  else
    painter.drawPath(splitPath);
}

void
CQLottiePathCanvas::
resizeEvent(QResizeEvent *)
{
  displayRange_.setPixelRange(0, 0, width() - 1, height() - 1);
}

QSize
CQLottiePathCanvas::
sizeHint() const
{
  return QSize(1000, 300);
}

void
CQLottiePathCanvas::
mouseMoveEvent(QMouseEvent *me)
{
  auto pos = me->pos();

  CPoint2D p;
  displayRange_.pixelToWindow(CPoint2D(pos.x(), pos.y()), p);

  auto *lottie = path_->lottie();

  lottie->status()->setStatusLabel(QString("%1 %2").arg(p.x).arg(p.y));
}

//---

CQLottiePathControl::
CQLottiePathControl(CQLottiePath *path) :
 path_(path)
{
  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  auto addLabelEdit = [&](const QString &label, auto *w) {
    auto *frame   = new QFrame;
    auto *layout1 = new QHBoxLayout(frame);

    auto *labelW = new QLabel(label);

    layout1->addWidget(labelW);
    layout1->addWidget(w);

    layout->addWidget(frame);

    return w;
  };

//equalScale_= addLabelEdit("Equal Scale", new QCheckBox(this));
  bgFillEdit_ = addLabelEdit("Bg Fill"    , new CQColorEdit(this));

  trimStartEdit_ = addLabelEdit("Trim Start", new CQRealSpin(this));
  trimEndEdit_   = addLabelEdit("Trim End"  , new CQRealSpin(this));

  layout->addStretch(1);

  connectSlots(true);
}

void
CQLottiePathControl::
updateWidgets()
{
  auto *canvas = path_->canvas();

  connectSlots(false);

  trimStartEdit_->setValue(canvas->trimStart());
  trimEndEdit_  ->setValue(canvas->trimEnd());

  connectSlots(true);
}

void
CQLottiePathControl::
connectSlots(bool b)
{
  CQUtil::connectDisconnect(b, bgFillEdit_, SIGNAL(colorChanged(const QColor &)),
                            this, SLOT(bgFillSlot(const QColor &)));
  CQUtil::connectDisconnect(b, trimStartEdit_, SIGNAL(realValueChanged(double)),
                            this, SLOT(rangeChanged()));
  CQUtil::connectDisconnect(b, trimEndEdit_, SIGNAL(realValueChanged(double)),
                            this, SLOT(rangeChanged()));
}

void
CQLottiePathControl::
bgFillSlot(const QColor &c)
{
  auto *canvas = path_->canvas();

  canvas->setBgColor(c);
  canvas->update();
}

void
CQLottiePathControl::
rangeChanged()
{
  auto *canvas = path_->canvas();

  auto s = trimStartEdit_->value();
  auto e = trimEndEdit_  ->value();

  canvas->setTrimRange(s, e);
}
