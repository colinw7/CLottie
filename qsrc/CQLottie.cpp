#include <CQLottie.h>
#include <CQLottieCanvas.h>
#include <CQLottieToolBar.h>
#include <CQLottieStatusBar.h>
#include <CQLottieTimeLine.h>
#include <CQLottiePath.h>
#include <CQLottieTree.h>
#include <CQLottieSettings.h>
#include <CLottie.h>

#include <QPainterPathUtil.h>
#include <CEncode64.h>
#include <CBezierPath.h>
#include <CArcToBezier.h>
#include <CHRTimer.h>

#include <QTabWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QFileDialog>

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsDropShadowEffect>

#include <iostream>
#include <set>
#include <cassert>
#include <cmath>

#include <svg/play_svg.h>
#include <svg/pause_svg.h>
#include <svg/play_one_svg.h>
#include <svg/clock_svg.h>

//---

namespace {

#if 0
CRGBA toRGBA(const QColor &color) {
  return CRGBA(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}
#endif

std::optional<double> combineOpacities(const std::optional<double> &o1,
                                       const std::optional<double> &o2)
{
  if (! o1) return o2;
  if (! o2) return o1;
  return 100.0*((o1.value()/100.0)*(o2.value()/100.0));
}

void errOnce(uint id, const std::string &msg) {
  static std::set<uint> ids;
  if (ids.find(id) == ids.end()) {
    std::cerr << "Error: " << msg << "\n";
    ids.insert(id);
  }
}

void warnOnce(uint id, const std::string &msg) {
  static std::set<uint> ids;
  if (ids.find(id) == ids.end()) {
    std::cerr << "Warning: " << msg << "\n";
    ids.insert(id);
  }
}

}

//---

class CQLottieFactory : public CLottieFactory {
 public:
  CQLottieFactory(CQLottie *lottie) :
   lottie_(lottie) {
  }

  CLottieAsset *makeAsset(CLottie *) override {
    return new CQLottieAsset(lottie_);
  }

  CLottieLayer *makeLayer(CLottie *) override {
    return new CQLottieLayer(lottie_);
  }

  CLottieShape *makeShape(CLottie *) override {
    return new CQLottieShape(lottie_);
  }

 private:
  CQLottie *lottie_ { nullptr };
};

//---

CQLottie::
CQLottie()
{
  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  //---

  toolbar_ = new CQLottieToolBar(this);

  layout->addWidget(toolbar_);

  //---

  auto *cframe = new QFrame;
  cframe->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  layout->addWidget(cframe);

  //---

  auto *clayout = new QHBoxLayout(cframe);
  clayout->setMargin(0); clayout->setSpacing(0);

  canvas_ = new CQLottieCanvas(this);
  canvas_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  clayout->addWidget(canvas_);

  //---

  auto *controlFrame = new QFrame;
  controlFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  auto *controlLayout = new QVBoxLayout(controlFrame);
  controlLayout->setMargin(0); controlLayout->setSpacing(0);

  clayout->addWidget(controlFrame);

  auto *tab = new QTabWidget;

  controlLayout->addWidget(tab);

  //---

  auto *objectFrame = new QFrame;
  objectFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  auto *objectLayout = new QVBoxLayout(objectFrame);
  objectLayout->setMargin(0); objectLayout->setSpacing(0);

  tree_ = new CQLottieTree(this);

  objectLayout->addWidget(tree_);

  objectTree_ = new CQLottieObjectTree(this);

  objectLayout->addWidget(objectTree_);

  tab->addTab(objectFrame, "Objects");

  //---

  settings_ = new CQLottieSettings(this);

  tab->addTab(settings_, "Settings");

  //---

  timeLine_ = new CQLottieTimeLine(this);

  layout->addWidget(timeLine_);

  //---

  path_ = new CQLottiePath(this);

  layout->addWidget(path_);

  //---

  status_ = new CQLottieStatusBar(this);

  layout->addWidget(status_);

  //---

  lottie_ = new CLottie;

  lottie_->setFactory(new CQLottieFactory(this));

  //---

  timer_ = new QTimer;

  connect(timer_, SIGNAL(timeout()), this, SLOT(tickSlot()));

  //---

  setShowTimeLine(false);
  setShowPath(false);
}

void
CQLottie::
setDebug(bool b)
{
  lottie_->setDebug(b);
}

void
CQLottie::
setPrint(bool b)
{
  lottie_->setPrint(b);
}

bool
CQLottie::
load(const std::string &filename)
{
  if (! lottie_->load(filename))
    return false;

  const auto *root = lottie_->root();

  w_ = root->width ().value_or(100);
  h_ = root->height().value_or(100);

  displayRange_.setEqualScale(equalScale_);
  displayRange_.setWindowRange(0, 0, w_, h_);

  fps_ = std::max(root->frameRate().value_or(1.0), 1.0);

  dt_ = 1000.0/fps_;

  timer_->start(int(dt_));

  //---

  toolbar_->updateWidgets();

  tree_->load();

  return true;
}

void
CQLottie::
setEqualScale(bool b)
{
  equalScale_ = b;

  displayRange_.setEqualScale(b);
}

void
CQLottie::
setPixelSize(int w, int h)
{
  displayRange_.setEqualScale(equalScale_);
  displayRange_.setPixelRange(0, h - 1, w - 1, 0);

}

void
CQLottie::
loadSlot()
{
  auto dir = ".";

  auto fileName = QFileDialog::getOpenFileName(this, "Open Model File", dir, "Files (*.json)");
  if (! fileName.length()) return;

  if (! load(fileName.toStdString()))
    std::cerr << "Failed to load file\n";

  updateAll();
}

void
CQLottie::
updateAll()
{
  update();

  canvas_->invalidate();
}

void
CQLottie::
playSlot()
{
  running_ = true;
}

void
CQLottie::
pauseSlot()
{
  running_ = false;
}

void
CQLottie::
stepSlot()
{
  running_ = true;

  tickSlot();

  running_ = false;
}

void
CQLottie::
tickSlot()
{
  if (! running_) {
    status_->setTicksLabel(QString("Frame: %1 (%2 secs)").arg(ticks_).arg(secs_));
    return;
  }

  ++ticks_;

  updateAnim();
}

void
CQLottie::
setTicks(int t)
{
  ticks_ = t;

  updateAnim();
}

void
CQLottie::
updateAnim()
{
  const auto *root = lottie_->root();

  secs_  = dt_*ticks_/1000.0;
  isecs_ = uint(secs_);

  if (ticks_ < root->frameStart())
    return;

  if (ticks_ > root->frameStop()) {
    ticks_ = 0;

    secs_  = 0.0;
    isecs_ = 0;
  }

  status_->setTicksLabel(QString("Frame: %1 (%2 secs)").arg(ticks_).arg(secs_));

  update();

  canvas_->invalidate();

  timeLine_->update();

  toolbar_->updateWidgets();
}

void
CQLottie::
draw(QPainter *painter, bool update)
{
  CElapsedTimer etime("CQLottie::draw");

  const auto *root = lottie_->root();
  if (! root) return;

  selectedPaths_.clear();
  bboxRects_    .clear();

  assetLayers_.clear();

  //---

  DrawState drawState;

  drawState.painter = painter;

  drawState.displayRange = displayRange_;

  getTimeFrame(drawState.timeFrame);

  drawRoot(drawState, root, update);

  //---

  for (const auto &selectedPath : selectedPaths_) {
    painter->save();

    painter->setTransform(selectedPath.transform);

    painter->setPen  (selectedPath.pen);
    painter->setBrush(selectedPath.brush);

    drawPainterPath(painter, selectedPath.path);

    setSelectedPenBrush(painter);

    drawPainterPath(painter, selectedPath.path);

    painter->restore();
  }

  for (const auto &selectedRect : bboxRects_) {
    painter->save();

    drawState.painter->setTransform(toQTransform(displayRange_.getMatrix()));

    setBBoxPenBrush(painter);

    painter->drawRect(selectedRect.rect);

    painter->restore();
  }

  //---

  for (auto *stroker : strokers_)
    delete stroker;

  strokers_.clear();

  //---

  //printAssetLayers();
}

void
CQLottie::
printAssetLayers() const
{
  for (const auto &pa : assetLayers_) {
    std::cerr << "Asset: " << pa.first->id().value_or("") << ":\n";

    for (auto *layer : pa.second) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);
      assert(qlayer);

      std::cerr << "  Layer: " << qlayer->hierName() << "\n";
    }
  }
}

void
CQLottie::
drawRoot(const DrawState &drawState, const CLottieRoot *root, bool update)
{
  CElapsedTimer etime("CQLottie::drawRoot");

  if (root->isHidden().value_or(false))
    return;

  //---

  if (! root->childLayers().empty())
    drawChildLayers(drawState, root->childLayers(), update);

  //---

  auto bbox = root->bbox();

  for (auto *layer : root->childLayers())
    bbox += layer->bbox();

  const_cast<CLottieRoot *>(root)->setBBox(bbox);
}

void
CQLottie::
drawChildLayers(const DrawState &drawState, const Layers &childLayers, bool update)
{
  if (isDoubleBuffer()) {
    if (update) {
      for (auto *layer : childLayers) {
        auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);
        if (qlayer->isHidden().value_or(false)) continue;

        qlayer->setDoubleBuffer(true);

        drawLayer(drawState, layer, update);
      }
    }

    for (auto it = childLayers.rbegin(); it != childLayers.rend(); ++it) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(*it);
      if (qlayer->isHidden().value_or(false)) continue;

      if (qlayer->isEnabled() && qlayer->isChanged()) {
        drawState.painter->drawImage(0, 0, qlayer->image());

        drawState.layer->setChanged(true);
      }
    }
  }
  else {
    // assign matte layers
    CQLottieLayer *matteTarget = nullptr;
    bool           hasMatteTarget = false;

    for (auto *layer : childLayers) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);
      if (qlayer->isHidden().value_or(false)) continue;

      qlayer->setMatteTargetLayer(nullptr);
      qlayer->setMatteModeLayer  (nullptr);

      if (qlayer->matteTarget()) {
        if (matteTarget) {
#if 0
          std::cerr << "Duplicate matte target for layer '" <<
            hierName(qlayer, drawState) << "'\n";
#endif

          qlayer     ->setMatteTargetLayer(matteTarget);
          matteTarget->setMatteModeLayer  (qlayer);

          if (! qlayer->matteMode()) {
#if 0
            std::cerr << "Non-Matte mode layer '" <<
              hierName(qlayer, drawState) << "' with target\n";
#endif
            qlayer->setMatteMode(0);
          }

          hasMatteTarget = true;

          matteTarget = nullptr;
        }

        matteTarget = qlayer;
        continue;
      }

      if (matteTarget) {
        if (! qlayer->matteMode()) {
#if 0
          std::cerr << "Non-Matte mode layer '" <<
            hierName(qlayer, drawState) << "' with target\n";
#endif

          qlayer->setMatteMode(0);
        }

        qlayer     ->setMatteTargetLayer(matteTarget);
        matteTarget->setMatteModeLayer  (qlayer);

        hasMatteTarget = true;

        matteTarget = nullptr;
      }
      else {
        if (qlayer->matteMode()) {
#if 0
          std::cerr << "Matte mode layer '" <<
            hierName(qlayer, drawState) << "' with no target\n";
#endif
        }
      }

      if (qlayer->matteParent()) {
        std::cerr << "Unhandled Matte parent layer '" << hierName(qlayer, drawState) << "'\n";

        auto *matteLayer = lottie_->getLayerById(*qlayer->matteParent());

        if (! matteLayer)
          std::cerr << "Matte parent layer not found '" << *qlayer->matteParent() << "'\n";
      }
    }

    // draw matte layers
    if (hasMatteTarget) {
      for (auto *layer : childLayers) {
        auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);
        if (qlayer->isHidden().value_or(false)) continue;

        if ((qlayer->matteMode  () && qlayer->matteTargetLayer()) ||
            (qlayer->matteTarget() && qlayer->matteModeLayer  ())) {
          qlayer->setDoubleBuffer(true);

          drawLayer(drawState, qlayer, update);
        }
      }

      // compose matte layers and composed matte mode layer
      for (auto *layer : childLayers) {
        auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);
        if (qlayer->isHidden().value_or(false)) continue;

        if (qlayer->matteMode() && qlayer->matteTargetLayer()) {
          auto matteImage =
            matteLayerImage(qlayer, qlayer->matteTargetLayer(), *qlayer->matteMode());

          qlayer->setMatteImage(matteImage);
        }
      }
    }

    // draw layers
    for (auto it = childLayers.rbegin(); it != childLayers.rend(); ++it) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(*it);
      if (qlayer->isHidden().value_or(false)) continue;

      if (qlayer->matteTarget() && qlayer->matteModeLayer())
        continue;

      if (qlayer->effect()) {
        qlayer->setDoubleBuffer(true);

        drawLayer(drawState, qlayer, update);
      }

      if (qlayer->matteMode() && qlayer->matteTargetLayer()) {
        assert(qlayer->isDoubleBuffer());

        drawState.painter->drawImage(0, 0, qlayer->matteImage());
      }
      else if (qlayer->effect()) {
        assert(qlayer->isDoubleBuffer());

        drawState.painter->drawImage(0, 0, qlayer->effectImage());
      }
      else {
        if (qlayer->isDoubleBuffer())
          drawState.painter->drawImage(0, 0, qlayer->image());
        else
          drawLayer(drawState, qlayer, update);
      }
    }
  }
}

QImage
CQLottie::
matteLayerImage(CQLottieLayer *layer, CQLottieLayer *clipLayer, int matteMode) const
{
  // Matte Mode
  //  0 : Normal
  //  1 : Alpha
  //  2 : Inverted Alpha
  //  3 : Luminance
  //  4 : Inverted Luminance

  if (matteMode == 0)
    return layer->image();

  if (matteMode < 0 || matteMode > 4)
    std::cerr << "Unhandled matte mode: " << matteMode << "\n";

  int iw = layer->image().width ();
  int ih = layer->image().height();

  auto matteImage = QImage(iw, ih, QImage::Format_ARGB32);

  const auto &layerImage = layer->image();
  const auto &clipImage  = clipLayer->image();

  matteImage.fill(0);

#if 0
  assert(clipLayer->image().width () == iw);
  assert(clipLayer->image().height() == ih);

  QPainter mattePainter(&matteImage);

  //mattePainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);

//mattePainter.drawImage(0, 0, clipImage);
  mattePainter.drawImage(0, 0, layerImage);
#else
  for (int y = 0; y < ih; ++y) {
    for (int x = 0; x < iw; ++x) {
      auto c = clipImage.pixelColor(x, y);

      double a = 0.0;

      if      (matteMode == 1) {
        a = c.alpha()/255.0;
      }
      else if (matteMode == 2) {
        a = 1.0 - c.alpha()/255.0;
      }
      else if (matteMode == 3) {
        a = c.lightness()/255.0;
      }
      else if (matteMode == 4) {
        a = 1.0 - c.lightness()/255.0;
      }

      auto c1 = layerImage.pixelColor(x, y);

      auto a1 = c1.alpha()/255.0;

      c1.setAlpha(255*a*a1);

      matteImage.setPixelColor(x, y, c1);
    }
  }
#endif

  return matteImage;
}

void
CQLottie::
getTimeFrame(CLottieUtil::TimeFrame &timeFrame) const
{
  const auto *root = (lottie_ ? lottie_->root() : nullptr);
  if (! root) return;

  timeFrame.frameStart = root->frameStart();
  timeFrame.frameStop  = root->frameStop ();

  timeFrame.secs  = secs_;
  timeFrame.frame = ticks_;
}

void
CQLottie::
drawLayer(const DrawState &drawState, CLottieLayer *layer, bool update)
{
  CElapsedTimer etime("CQLottie::drawLayer '" + layer->name().value_or("") + "'");

  if (layer->isHidden().value_or(false))
    return;

  auto frame = double(drawState.timeFrame.frame) + drawState.timeFrame.delta;

  if (layer->frameIn()) {
    if (frame < layer->frameIn().value())
      return;
  }

  if (layer->frameOut()) {
    if (frame > layer->frameOut().value())
      return;
  }

  //---

  auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);

  auto drawState1 = drawState;

//drawState1.matrix = getLayerMatrix(drawState, layer);

  //---

  bool hasMask = false;

  auto *mask = layer->mask();

  if (mask) {
    auto maskMode = mask->mode.value_or("");

    if (maskMode == "a" || maskMode == "s" || maskMode == "i" ||
        maskMode == "l" || maskMode == "d" || maskMode == "f")
      hasMask = true;

    if (hasMask) {
      if (qlayer->isDoubleBuffer()) {
        std::cerr << "Masked Double Buffer Layer\n";
        hasMask = false;
      }
    }
  }

  if (hasMask) {
    drawState1.maskData = new MaskData;

    drawState1.maskData->mask = layer->mask();
  }

  //---

  qlayer->resize(canvas_->width(), canvas_->height());

  if (qlayer->isDoubleBuffer()) {
    drawState1.painter = qlayer->painter();
    drawState1.layer   = qlayer;

    qlayer->clear();

    qlayer->setChanged(false);
  }

  qlayer->setHierName(hierName(qlayer, drawState));

  //---

  auto typeId = layer->typeId().value_or(-1);

  if      (typeId == 0) { // Precomposition Layer
    drawPrecompLayer(drawState1, layer);
  }
  else if (typeId == 1) { // Solid Layer
    drawSolidLayer(drawState1, layer);
  }
  else if (typeId == 2) { // Image Layer
    drawImageLayer(drawState1, layer);
  }
  else if (typeId == 3) { // Null Layer
  }
  else if (typeId == 4) { // Shape Layer
  }
  else {
    warnOnce(__LINE__, "Invalid layer type id: " + std::to_string(typeId));
  }

  //---

  drawState1.objects.push_front(layer);

  drawLayerShapes(drawState1, layer);

#if 0
  drawLayerAssets(drawState1.painter, drawState1, layer);
#endif

  if (! layer->childLayers().empty())
    drawChildLayers(drawState1, layer->childLayers(), update);

  //---

  if (hasMask) {
    maskLayer(drawState, drawState1.maskData, layer);

    delete drawState1.maskData;
  }
  else {
    auto bbox = layer->bbox();

    for (auto *layer : layer->childLayers())
      bbox += layer->bbox();

    for (auto *shape : layer->shapes())
      bbox += shape->bbox();

    layer->setBBox(bbox);
  }

  //---

  if (isShowBBox() && layer->isHierSelected() && layer->bbox().isSet()) {
    auto rect = CQLottieUtil::toQRect(layer->bbox());

    addBBoxRect(drawState.painter, rect);
  }

  //---

  if (layer->autoOrient().value_or(false) && layer->transform()->position.isAnimated()) {
    drawState.painter->save();

    auto displayMatrix = drawState.getDisplayMatrix();

    drawState.painter->setTransform(toQTransform(displayMatrix));

    auto bezierPath = lottie_->getPositionPath(layer->transform());

    QPainterPath path;
    toQPath(bezierPath, path);

    drawState.painter->setPen(Qt::red);

    drawPainterPath(drawState.painter, path);

    drawState.painter->restore();
  }

  //---

  auto *effect = layer->effect();

  if (effect) {
    auto type = effect->type().value_or(0);

    auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);

    QImage effectImage;

    if      (type == 5) { // custom
    }
    else if (type == 21) { // fill
      OptColor c;
      OptReal  o;

      for (auto *v : effect->values()) {
        auto vtype = v->type.value_or(-1);
        auto vname = v->name.value_or("");

        if      (vtype == 0) {
          // Horizontal Feather, Vertical Feather, Opacity
          if (vname == "Opacity" && v->scalar)
            o = v->scalar->tvalue(drawState.timeFrame);
        }
        else if (vtype == 2) {
          if (v->color)
            c = v->color->value();
        }
        else if (vtype == 7) {
          // All Masks, Invert
        }
        else if (vtype == 10) {
          // Fill Mask
        }
        else
          std::cerr << "Invalid fill effect value type: " << vtype << "\n";
      }

      if (c) {
        if (o)
          c->setAlpha(o.value());

        effectImage = CQLottieUtil::fillImage(qlayer->image(), *c);
      }
      else
        std::cerr << "No color for fill effect\n";
    }
    else if (type == 25) { // drop shadow
      int     blurRadius  = 5;
      auto    shadowColor = QColor(Qt::black);
      QPointF offset      = QPointF(3, 3);

      effectImage =
        CQLottieUtil::applyDropShadow(qlayer->image(), blurRadius, shadowColor, offset);
    }
    else {
      std::cerr << "Unhandled effect: " << type << "\n";
    }

    if (! effectImage.isNull())
      qlayer->setEffectImage(effectImage);
    else
      qlayer->setEffectImage(qlayer->image());
  }
}

void
CQLottie::
maskLayer(const DrawState &drawState, MaskData *maskData, CLottieLayer *layer)
{
  auto maskMode = maskData->mask->mode.value_or("");

  //---

  auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);

  CBezierPath maskBezierPath;
  pathToBezier(maskData->mask->path, drawState, maskBezierPath);

  QPainterPath maskPath;
  toQPath(maskBezierPath, maskPath);

  //---

  QPainterPath epath;

  auto expand = maskData->mask->expand.tvalue(drawState.timeFrame).value_or(0);

  if (expand != 0) {
    QPainterPathStroker stroker;

    stroker.setCapStyle(Qt::FlatCap);
    stroker.setJoinStyle(Qt::MiterJoin);
    stroker.setMiterLimit(0);

    stroker.setDashOffset(0.0);
    stroker.setDashPattern(Qt::SolidLine);

    stroker.setWidth(std::abs(expand));

    epath = stroker.createStroke(maskPath);
  }

  //---

  auto opacity = maskData->mask->opacity.tvalue(drawState.timeFrame).value_or(100);

  //---

  bool invert = false;

  if (maskMode == "s")
    invert = true;

  if (maskData->mask->inverted.value_or(false))
    invert = ! invert;

  //---

  if (invert) {
    QPainterPath path1;
    path1.addRect(QRectF(0, 0, w_, h_));

    maskPath = path1.subtracted(maskPath);
  }

  //---

  auto pmatrix = drawState.getDisplayMatrix();
  auto smatrix = getLayerMatrix(drawState, layer);

  auto dmatrix = pmatrix*smatrix.getMatrix();

  //---

  CQLottieLayer::ImagePainter imagePainter;
  qlayer->createImagePainter(imagePainter);

  auto drawState1 = drawState;

  drawState1.painter = imagePainter.painter;

  drawState1.painter->setTransform(toQTransform(dmatrix));

  // draw mask paths
  QPainterPath clipPath;

  if (expand < 0) {
    clipPath = maskPath.subtracted(epath);

    drawState1.painter->setClipPath(clipPath);
  }
  else
    drawState1.painter->setClipPath(maskPath);

  for (const auto &pathData : maskData->paths)
    drawPathData(drawState1.painter, pathData);

  if (isShowSelect() && layer->isHierSelected()) {
    drawState1.painter->setTransform(toQTransform(dmatrix));

    if (expand < 0)
      addSelectedPath(drawState1.painter, clipPath);
    else
      addSelectedPath(drawState1.painter, maskPath);
  }

  delete imagePainter.painter;

  auto image = imagePainter.image;

  if (opacity < 100)
    image = CQLottieUtil::alphaImage(image, opacity/100.0);

  //---

  QImage eimage;

  if (expand > 0) {
    CQLottieLayer::ImagePainter eimagePainter;
    qlayer->createImagePainter(eimagePainter);

    auto drawState2 = drawState;

    drawState2.painter = eimagePainter.painter;

    drawState2.painter->setTransform(toQTransform(dmatrix));

    // draw mask paths
    drawState2.painter->setClipPath(epath);

    for (const auto &pathData : maskData->paths)
      drawPathData(drawState2.painter, pathData);

    if (isShowSelect() && layer->isHierSelected()) {
      drawState2.painter->setTransform(toQTransform(dmatrix));

      addSelectedPath(drawState2.painter, epath);
    }

    delete eimagePainter.painter;

    eimage = eimagePainter.image;
  }

  //---

  drawState.painter->drawImage(0, 0, image);

  if (expand > 0)
    drawState.painter->drawImage(0, 0, eimage);

  qlayer->setImage(image); // TODO: eimage
}

void
CQLottie::
drawLayerShapes(DrawState &drawState, const CLottieLayer *layer)
{
  bool isMerge = bool(drawState.merge.shape);
  bool isTrim  = bool(drawState.trim.shape);

  auto drawState1 = drawState;

  drawState1.merge.paths.clear();
  drawState1.trim .paths.clear();

//auto *repeater = layer->calcRepeater();
  auto *repeaterShape = layer->getRepeaterShape();

  auto *repeater = (repeaterShape ? repeaterShape->repeater() : nullptr);

  for (auto it = layer->shapes().rbegin(); it != layer->shapes().rend(); ++it) {
    auto *qshape = dynamic_cast<CQLottieShape *>(*it);

    if (repeater) {
      auto drawState2 = drawState1;

      drawState2.objects.push_front(repeater);

      // model::Repeater::Transform::matrix ?
      auto repeatCopies = int(repeater->copies.tvalue(drawState2.timeFrame, 1.0).value());

      for (int i = 0; i < repeatCopies; ++i) {
        repeater->ind = i;
//      drawState2.repeatInd = i;

        drawShape(drawState2, qshape);
      }
    }
    else
      drawShape(drawState1, qshape);

    drawState1.objects.front().siblings.push_front(qshape);
  }

  //---

  if (drawState1.merge.shape) {
    if (! isMerge) {
      //auto *mergeShape = layer->getMergeShape();
      auto *mergeShape = getDrawMergeShape(drawState1);
      assert(mergeShape == drawState1.merge.shape);

      drawMergeShapes(drawState1, mergeShape);
    }
    else {
      auto *mergeShape = getDrawMergeShape(drawState);
      assert(mergeShape == drawState1.merge.shape);

      for (const auto &bezierPath : drawState1.merge.paths)
        drawState.merge.paths.push_back(bezierPath);
    }
  }

  //---

  if (drawState1.trim.shape) {
    if (! isTrim) {
      //auto *trimShape = layer->getTrimShape();
      auto *trimShape = getDrawTrimShape(drawState1);
      assert(trimShape == drawState1.trim.shape);

      drawTrimShapes(drawState1, trimShape);
    }
    else {
      auto *trimShape = getDrawTrimShape(drawState);
      assert(trimShape == drawState1.trim.shape);

      for (const auto &bezierPath : drawState1.trim.paths)
        drawState.trim.paths.push_back(bezierPath);
    }
  }
}

void
CQLottie::
drawMergeShapes(DrawState &drawState, const CLottieShape *mergeShape)
{
  int np = drawState.merge.paths.size();
  if (np <= 0) return;

  //---

  // calc merged path
  assert(mergeShape == drawState.merge.shape);
  int mergeMode = mergeShape->merge()->mode.value_or(0);

  QPainterPath path;

  if      (mergeMode == 1) {
    auto bezierPath = drawState.merge.paths[0];

    for (int i = 1; i < np; ++i) {
      const auto &bezierPath1 = drawState.merge.paths[i];

      bezierPath.combine(bezierPath1);
    }

    toQPath(bezierPath, path);
  }
  else if (mergeMode == 2 || mergeMode == 3 || mergeMode == 4) {
    const auto &bezierPath = drawState.merge.paths[0];

    toQPath(bezierPath, path);

    for (int i = 1; i < np; ++i) {
      const auto &bezierPath1 = drawState.merge.paths[i];

      QPainterPath path1;
      toQPath(bezierPath1, path1);

      if      (mergeMode == 2) {
        path = path.united(path1);
      }
      else if (mergeMode == 3) {
        path = path.subtracted(path1);
      }
      else if (mergeMode == 4) {
        path = path.intersected(path1);
      }
    }
  }
  else {
    std::cerr << "invalid merge mode " << mergeMode << "\n";
  }

  //---

  auto *fill = mergeShape->calcFill();

  if (fill) {
    if (fill->color.isSet())
      drawState.fill.color = fill->color.tvalue(drawState.timeFrame);

    if (fill->opacity.isSet())
      drawState.fill.opacity = fill->opacity.tvalue(drawState.timeFrame);

    if (fill->fillRule)
      drawState.fill.rule = fill->fillRule.value();
  }

  //---

  if (drawState.fill.rule == 2)
    path.setFillRule(Qt::OddEvenFill);
  else
    path.setFillRule(Qt::WindingFill);

  //---

  drawState.painter->save();

  //---

  // draw path
  auto pmatrix = drawState.getDisplayMatrix();
  auto smatrix = getShapeMatrix(drawState, mergeShape);

  auto dmatrix = pmatrix*smatrix.getMatrix();

  drawState.painter->setTransform(toQTransform(dmatrix));

  setPenBrush(drawState, mergeShape);

  //---

  if (drawState.trim.shape) {
    auto *trimShape = getDrawTrimShape(drawState);
    assert(trimShape == drawState.trim.shape);

    PathData pathData;
    setPathData(drawState, path, smatrix, pathData);

    drawState.trim.paths.push_back(pathData);
  }
  else {
    if (drawState.stroker) {
      auto lpath = drawState.stroker->createStroke(path);

      lpath.setFillRule(Qt::WindingFill);

      drawPainterPath(drawState.painter, lpath);
    }
    else
      drawPainterPath(drawState.painter, path);

    //---

    auto bbox = CQLottieUtil::toBBox(CQLottieUtil::transformRect(smatrix, path.boundingRect()));

    const_cast<CLottieShape *>(mergeShape)->setBBox(bbox);

    //---

    if (isShowSelect() && mergeShape->isHierSelected())
      addSelectedPath(drawState.painter, path);
  }

  //---

  drawState.painter->restore();

  if (drawState.layer)
    drawState.layer->setChanged(true);
}

void
CQLottie::
drawTrimShapes(DrawState &drawState, const CLottieShape *trimShape)
{
  drawState.painter->save();

  CBBox2D bbox;

  // draw paths
  for (const auto &pathData : drawState.trim.paths) {
    CBezierPath bezierPath;
    fromQPath(pathData.path, bezierPath);

    auto bezierPath1 = trimPath(drawState, trimShape, bezierPath);
    if (bezierPath1.isEmpty()) continue;

    QPainterPath ppath;
    toQPath(bezierPath1, ppath);

    if (drawState.maskData) {
      PathData pathData1 = pathData;
      pathData1.path = ppath;

      drawState.maskData->paths.push_back(pathData1);
    }
    else {
      auto ppath1 = drawPathDataPath(drawState.painter, pathData, ppath);

      if (isShowSelect() && trimShape->isHierSelected())
       addSelectedPath(drawState.painter, ppath1);

      auto pbbox1 = bezierPath1.bbox();
      auto bbox1  = CQLottieUtil::transformBBox(pathData.smatrix, pbbox1);

      bbox += bbox1;
    }
  }

  if (bbox.isSet())
    const_cast<CLottieShape *>(trimShape)->setBBox(bbox);

  //---

  drawState.painter->restore();

  if (drawState.layer)
    drawState.layer->setChanged(true);
}

#if 0
void
CQLottie::
drawLayerAssets(DrawState &drawState, const CLottieLayer *layer)
{
  auto *asset = getLayerAsset(layer);
  if (! asset) return;

  drawAsset(drawState, asset);
}
#endif

void
CQLottie::
drawAsset(const DrawState &drawState, CLottieAsset *asset)
{
  CElapsedTimer etime("CQLottie::drawAsset '" + asset->id().value_or("") + "'");

  if (asset->childLayers().empty())
    return;

  //---

  auto drawState1 = drawState;

  drawState1.objects.push_front(asset);

  //---

  bool update = false;

#if 1
  if (! asset->childLayers().empty())
    drawChildLayers(drawState1, asset->childLayers(), update);
#else
  if (isDoubleBuffer()) {
    for (auto *layer : asset->childLayers()) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);

      qlayer->setDoubleBuffer(true);

      drawLayer(drawState1, layer, update);
    }

    for (auto it = asset->childLayers().rbegin(); it != asset->childLayers().rend(); ++it) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(*it);

      if (qlayer->isEnabled() && qlayer->isChanged()) {
        drawState1.painter->drawImage(0, 0, qlayer->image());

        drawState1.layer->setChanged(true);
      }
    }
  }
  else {
    for (auto it = asset->childLayers().rbegin(); it != asset->childLayers().rend(); ++it) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(*it);

      drawLayer(drawState1, qlayer, update);
    }
  }
#endif

  //---

  CBBox2D bbox;

  for (auto *layer : asset->childLayers())
    bbox += layer->bbox();

  const_cast<CLottieAsset *>(asset)->setBBox(bbox);

  //---

  if (isShowBBox() && asset->isHierSelected() && asset->bbox().isSet()) {
    auto rect = CQLottieUtil::toQRect(asset->bbox());

    addBBoxRect(drawState1.painter, rect);
  }
}

void
CQLottie::
drawPrecompLayer(const DrawState &drawState, CLottieLayer *layer)
{
  //warnOnce(__LINE__, "Unhandled precomposition layer type");

  auto *asset = getPrecompLayerAsset(layer);
  if (! asset) return;

  auto frame = double(drawState.timeFrame.frame) + drawState.timeFrame.delta;

  DrawState drawState1 = drawState;

  if (layer->precomp() && layer->precomp()->startTime) {
    auto startTime = layer->precomp()->startTime.value();

    if (frame < startTime)
      return;

     drawState1.timeFrame.delta -= startTime;
  }

  if (layer->frameIn())
    drawState1.timeFrame.delta -= layer->frameIn().value();

  if (layer->precomp() && layer->precomp()->timeRemap.isSet()) {
    auto t = layer->precomp()->timeRemap.tvalue(drawState.timeFrame).value();

    drawState1.timeFrame.secs  = t;
    drawState1.timeFrame.frame = t*fps_;
  }

  drawState1.objects.push_front(layer);

  drawAsset(drawState1, asset);

  auto bbox = asset->bbox();

  const_cast<CLottieLayer *>(layer)->setBBox(bbox);
}

void
CQLottie::
drawSolidLayer(const DrawState &drawState, const CLottieLayer *layer)
{
  // get solid rect size and color
  auto *solid = layer->solid();
  if (! solid) return;

  auto w = solid->width .value_or(layer->width ().value_or(0));
  auto h = solid->height.value_or(layer->height().value_or(0));

  if (w <= 0 || h <= 0)
    return;

  auto color = solid->color.value_or(CRGBA(0, 0, 0));

  //---

  auto pmatrix = drawState.getDisplayMatrix();
  auto smatrix = getLayerMatrix(drawState, layer);

  auto dmatrix = pmatrix*smatrix.getMatrix();

  //---

  auto p1 = CPoint2D(    0,     0);
  auto p2 = CPoint2D(w - 1, h - 1);

  auto bbox  = CBBox2D(p1, p2);
  auto bbox1 = CQLottieUtil::transformBBox(smatrix, bbox);

  //---

  // draw solid rect (with optional mask)
  drawState.painter->save();

  drawState.painter->setTransform(toQTransform(dmatrix));

  drawState.painter->setPen  (CQLottieUtil::toQColor(color)); // NoPen ?
  drawState.painter->setBrush(CQLottieUtil::toQColor(color));

  // add mask
  auto *mask = layer->mask();

  if (mask) {
    CBezierPath bezierPath;
    pathToBezier(mask->path, drawState, bezierPath);

    QPainterPath ppath;
    toQPath(bezierPath, ppath);

    drawState.painter->setClipPath(ppath);
  }

  drawState.painter->drawRect(CQLottieUtil::toQRect(bbox));

  drawState.painter->restore();

  if (drawState.layer)
    drawState.layer->setChanged(true);

  //---

  const_cast<CLottieLayer *>(layer)->setBBox(bbox1);
}

void
CQLottie::
drawImageLayer(const DrawState &drawState, const CLottieLayer *layer)
{
  // get layer asset
  auto *asset = getLayerAsset(layer);
  if (! asset) return;

  //---

  // get image from asset
  auto image = getAssetImage(asset, /*create*/true);
  if (image.isNull()) return;

  int w = asset->width ().value_or(100);
  int h = asset->height().value_or(100);

  //---

  auto pmatrix = drawState.getDisplayMatrix();
  auto smatrix = getLayerMatrix(drawState, layer);

  auto dmatrix = pmatrix*smatrix.getMatrix();

  //---

  auto p1 = CPoint2D(    0,     0);
  auto p2 = CPoint2D(w - 1, h - 1);

  auto bbox  = CBBox2D(p1, p2);
  auto bbox1 = CQLottieUtil::transformBBox(smatrix, bbox);

  //---

  drawState.painter->save();

  drawState.painter->setTransform(toQTransform(dmatrix));

  auto qrect = CQLottieUtil::toQRect(bbox);

  drawState.painter->drawImage(qrect, image);

  if (isShowSelect() && layer->isHierSelected())
    addSelectedRect(drawState.painter, qrect);

  drawState.painter->restore();

  if (drawState.layer)
    drawState.layer->setChanged(true);

  //---

  const_cast<CLottieLayer *>(layer)->setBBox(bbox1);
  const_cast<CLottieAsset *>(asset)->setBBox(bbox1);
}

CLottieAsset *
CQLottie::
getPrecompLayerAsset(const CLottieLayer *layer) const
{
  auto *precomp = layer->precomp();
  if (! precomp) return nullptr;

  if (! precomp->refId)
    return nullptr;

  // get precomp asset
  auto refId = precomp->refId.value();

  auto *asset = lottie_->getAssetById(refId);

  if (! asset) {
    warnOnce(__LINE__, "Asset not found " + refId);
    return nullptr;
  }

  const_cast<CQLottie *>(this)->assetLayers_[asset].
    push_back(const_cast<CLottieLayer *>(layer));

  return asset;
}

CLottieAsset *
CQLottie::
getLayerAsset(const CLottieLayer *layer) const
{
  if (! layer->refId())
    return nullptr;

  // get layer asset
  auto refId = layer->refId().value();

  auto *asset = lottie_->getAssetById(refId);

  if (! asset) {
    warnOnce(__LINE__, "Asset not found " + *layer->refId());
    return nullptr;
  }

  const_cast<CQLottie *>(this)->assetLayers_[asset].
    push_back(const_cast<CLottieLayer *>(layer));

  return asset;
}

QImage
CQLottie::
getAssetImage(CLottieAsset *asset, bool create) const
{
  // get image from asset
  assert(asset->id());

  auto assetId = asset->id().value();

  auto pi = assetImage_.find(assetId);

  if (pi == assetImage_.end()) {
    if (! create)
      return QImage();

    //---

    auto embedded = asset->embedded().value_or(false);
    auto dir      = asset->dir().value_or(".");
    auto path     = asset->path().value_or("");

    QImage image;

    if (! embedded) {
      auto filename = dir + "/" + path;

      if (! image.load(QString::fromStdString(filename)))
        warnOnce(__LINE__, "Invalid image " + filename);
    }
    else {
      static std::string uri_base64 = "data:image/png;base64,";
      static size_t      uri_base64_len = uri_base64.size();

      if (path.size() > uri_base64_len && path.substr(0, uri_base64_len) == uri_base64) {
        auto str1 = CEncode64Inst->decode(path.substr(uri_base64_len));

        auto len1 = str1.size();

        uchar *data1 = new uchar [len1 + 1];

        memcpy(data1, str1.c_str(), len1);
        data1[len1] = '\0';

        if (! image.loadFromData(data1, len1))
          warnOnce(__LINE__, "Invalid image " + dir);

        delete [] data1;
      }
      else {
        std::cerr << "Unhandled embedded image '" << path << "'\n";
      }
    }

    //---

    ImageData imageData;

    imageData.image  = image;
    imageData.width  = image.width();
    imageData.height = image.height();

    auto *th = const_cast<CQLottie *>(this);

    pi = th->assetImage_.insert(pi, AssetImage::value_type(assetId, imageData));
  }

  const auto &imageData = (*pi).second;

  return imageData.image;
}

void
CQLottie::
drawShape(DrawState &drawState, CLottieShape *shape)
{
  CElapsedTimer etime("CQLottie::drawShape '" + shape->name().value_or("") + "'");

  if (shape->isHidden().value_or(false))
    return;

  //---

  auto type = shape->type().value_or("");

  auto unhandledShape = [&](const std::string &msg) {
    warnOnce(__LINE__, "Unhandled shape: " + msg + "(" + type + ")");
  };

  auto shapeType = shape->shapeType();

  if      (shapeType == CLottieShape::ShapeType::ELLIPSE) { // ellipse
    drawEllipse(drawState, shape);
  }
  else if (shapeType == CLottieShape::ShapeType::FILL) { // fill
    drawState.fill.shape = shape;
  }
  else if (shapeType == CLottieShape::ShapeType::GRADIENT_FILL) { // gradient fill
    drawState.fillGradient.shape = shape;

//  gradientFillShape(drawState, shape);
  }
  else if (shapeType == CLottieShape::ShapeType::GRADIENT_STROKE) { // gradient stroke
    drawState.strokeGradient.shape = shape;
  }
  else if (shapeType == CLottieShape::ShapeType::GROUP) { // group
  }
  else if (shapeType == CLottieShape::ShapeType::PATH) { // path
    drawPath(drawState, shape);
  }
  else if (shapeType == CLottieShape::ShapeType::POLYSTAR) { // polystar
    drawPolystar(drawState, shape);
  }
  else if (shapeType == CLottieShape::ShapeType::RECTANGLE) { // rectangle
    drawRectangle(drawState, shape);
  }
  else if (shapeType == CLottieShape::ShapeType::STROKE) { // stroke
    drawState.stroke.shape = shape;
  }
  else if (shapeType == CLottieShape::ShapeType::TRANSFORM) { // transform shape
  }
  else if (shapeType == CLottieShape::ShapeType::TRIM) { // trim path
    assert(shape->trim());

    drawState.trim.shape = shape;
  }
  else if (shapeType == CLottieShape::ShapeType::MERGE) { // merge path
    assert(shape->merge());

    drawState.merge.shape = shape;

    // 1 : Normal
    // 2 : Add
    // 3 : Subtract
    // 4 : Intersect
    // 5 : Exclude Intersections
    //merge.mode  = shape->merge()->mode.value_or(0);
  }
  else if (shapeType == CLottieShape::ShapeType::REPEATER) { // repeater
  }
  else if (shapeType == CLottieShape::ShapeType::ROUNDED) { // rounded
    drawState.rounded.shape = shape;
  }
  else {
    unhandledShape("???");
  }

  //---

  if (! shape->shapes().empty()) {
    bool isMerge = bool(drawState.merge.shape);
    bool isTrim  = bool(drawState.trim.shape);

    auto drawState1 = drawState;

    drawState1.merge.paths.clear();
    drawState1.trim .paths.clear();

    drawState1.objects.push_front(shape);

    auto *repeaterShape = shape->getRepeaterShape();

    auto *repeater = (repeaterShape ? repeaterShape->repeater() : nullptr);

    //---

    for (auto it = shape->shapes().rbegin(); it != shape->shapes().rend(); ++it) {
      auto *qshape = dynamic_cast<CQLottieShape *>(*it);

      if (repeater) {
        auto drawState2 = drawState1;

        drawState2.objects.push_front(repeater);

        // model::Repeater::Transform::matrix ?
        auto repeatCopies = int(repeater->copies.tvalue(drawState2.timeFrame, 1.0).value());

        for (int i = 0; i < repeatCopies; ++i) {
          repeater->ind = i;
//        drawState2.repeatInd = i;

          drawShape(drawState2, qshape);
        }
      }
      else
        drawShape(drawState1, qshape);

      drawState1.objects.front().siblings.push_front(qshape);
    }

    //---

    if (drawState1.merge.shape) {
      if (! isMerge) {
        //auto *mergeShape = shape->getMergeShape();
        auto *mergeShape = getDrawMergeShape(drawState1);
        assert(mergeShape == drawState1.merge.shape);

        drawMergeShapes(drawState1, mergeShape);
      }
      else {
        auto *mergeShape = getDrawMergeShape(drawState);
        assert(mergeShape == drawState1.merge.shape);

        for (const auto &bezierPath : drawState1.merge.paths)
          drawState.merge.paths.push_back(bezierPath);
      }
    }

    if (drawState1.trim.shape) {
      if (! isTrim) {
        //auto *trimShape = shape->getTrimShape();
        auto *trimShape = getDrawTrimShape(drawState1);
        assert(trimShape == drawState1.trim.shape);

        drawTrimShapes(drawState1, trimShape);
      }
      else {
        auto *trimShape = getDrawTrimShape(drawState1);
        assert(trimShape == drawState1.trim.shape);

        for (const auto &bezierPath : drawState1.trim.paths)
          drawState.trim.paths.push_back(bezierPath);
      }
    }

    //---

    auto bbox = shape->bbox();

    for (auto *shape1 : shape->shapes())
      bbox += shape1->bbox();

    const_cast<CLottieShape *>(shape)->setBBox(bbox);
  }
}

QGradient
CQLottie::
calcGradientFill(const TimeFrame &timeFrame, CLottieShape::GradientFill *gradientFill) const
{
  auto startPoint = gradientFill->startPoint.tvalue(timeFrame, CPoint2D(0, 0)).value();
  auto endPoint   = gradientFill->endPoint  .tvalue(timeFrame, CPoint2D(0, 0)).value();

  //---

  auto colors = gradientFill->colors.tvalue(timeFrame);

  auto nc = int(colors->vals.size());
  auto ns = gradientFill->stopCount.value_or(-1);

  if (ns > 0)
   ns = std::min(ns, nc/4);

  struct Stop {
    double pos { 0.0 };
    CRGBA  color;
  };

  std::vector<Stop> stops;

  for (int i = 0; i < ns; ++i) {
    Stop stop;

    stop.pos   = colors->vals[4*i];
    stop.color = CRGBA(colors->vals[4*i + 1], colors->vals[4*i + 2], colors->vals[4*i + 3]);

    stops.push_back(stop);
  }

  //---

  auto type = gradientFill->type.value_or(1);

  QGradient gradient;

  if      (type == 1) {
    QLinearGradient lgradient;

    lgradient.setStart(startPoint.x, startPoint.y);
    lgradient.setFinalStop(endPoint.x, endPoint.y);

    for (const auto &stop : stops)
      lgradient.setColorAt(stop.pos, CQLottieUtil::toQColor(stop.color));

    gradient = lgradient;
  }
  else if (type == 2) {
    QRadialGradient rgradient;

    rgradient.setCenter(startPoint.x, startPoint.y);

    auto v = CVector2D(startPoint, endPoint);
    auto r = v.length();

    rgradient.setCenterRadius(r);

    auto highlightLength = gradientFill->highlightLength.tvalue(timeFrame, 0).value()/100.0;
    auto highlightAngle  = gradientFill->highlightAngle .tvalue(timeFrame, 0).value();

    auto angle = CMathGen::DegToRad(v.angle() + highlightAngle);

    rgradient.setFocalPoint(startPoint.x + std::cos(angle)*highlightLength*r,
                            startPoint.y + std::sin(angle)*highlightLength*r);

    rgradient.setFocalRadius(0.0);

    for (const auto &stop : stops)
      rgradient.setColorAt(stop.pos, CQLottieUtil::toQColor(stop.color));

    gradient = rgradient;
  }
  else
    std::cerr << "Unknown gradient type : " << type << "\n";

  return gradient;
}

QGradient
CQLottie::
calcGradientStroke(const TimeFrame &timeFrame, CLottieShape::GradientStroke *gradientStroke) const
{
  auto startPoint = gradientStroke->startPoint.tvalue(timeFrame, CPoint2D(0, 0)).value();
  auto endPoint   = gradientStroke->endPoint  .tvalue(timeFrame, CPoint2D(0, 0)).value();

  //---

  auto colors = gradientStroke->colors.tvalue(timeFrame);

  auto nc = int(colors->vals.size());
  auto ns = gradientStroke->stopCount.value_or(-1);

  if (ns > 0)
   ns = std::min(ns, nc/4);

  struct Stop {
    double pos { 0.0 };
    CRGBA  color;
  };

  std::vector<Stop> stops;

  for (int i = 0; i < ns; ++i) {
    Stop stop;

    stop.pos   = colors->vals[4*i];
    stop.color = CRGBA(colors->vals[4*i + 1], colors->vals[4*i + 2], colors->vals[4*i + 3]);

    stops.push_back(stop);
  }

  //---

  auto type = gradientStroke->type.value_or(1);

  QGradient gradient;

  if      (type == 1) {
    QLinearGradient lgradient;

    lgradient.setStart(startPoint.x, startPoint.y);
    lgradient.setFinalStop(endPoint.x, endPoint.y);

    for (const auto &stop : stops)
      lgradient.setColorAt(stop.pos, CQLottieUtil::toQColor(stop.color));

    gradient = lgradient;
  }
  else if (type == 2) {
    QRadialGradient rgradient;

    rgradient.setCenter(startPoint.x, startPoint.y);

    auto v = CVector2D(startPoint, endPoint);
    auto r = v.length();

    rgradient.setCenterRadius(r);

#if 0
    auto highlightLength = gradientStroke->highlightLength.tvalue(timeFrame, 0).value()/100.0;
    auto highlightAngle  = gradientStroke->highlightAngle .tvalue(timeFrame, 0).value();
#else
    double highlightLength = 0.0;
    double highlightAngle  = 0.0;
#endif

    auto angle = CMathGen::DegToRad(v.angle() + highlightAngle);

    rgradient.setFocalPoint(startPoint.x + std::cos(angle)*highlightLength*r,
                            startPoint.y + std::sin(angle)*highlightLength*r);

    rgradient.setFocalRadius(0.0);

    for (const auto &stop : stops)
      rgradient.setColorAt(stop.pos, CQLottieUtil::toQColor(stop.color));

    gradient = rgradient;
  }
  else
    std::cerr << "Unknown gradient type : " << type << "\n";

  return gradient;
}

void
CQLottie::
drawEllipse(DrawState &drawState, const CLottieShape *shape)
{
#if 0
  auto positionxy = shape->pos().tvalue(drawState.timeFrame, CLottie::XYVals()).value();
  auto position   = positionxy.toPoint(CPoint2D(0, 0));

  auto sizexy = shape->size().tvalue(drawState.timeFrame, CLottie::XYVals()).value();
  auto size   = sizexy.toPoint(CPoint2D(0, 0));

  //---

  auto a1 = -M_PI/2.0;
  auto a2 = a1 + 2.0*M_PI;

  CArcToBezier::BezierList beziers;
  CArcToBezier::ArcToBeziers(position.x, position.y, size.x/2.0, size.y/2.0, a1, a2, beziers);

  CBezierPath bezierPath(beziers);
#else
  auto bezierPath = getEllipsePath(drawState.timeFrame, shape);
#endif

  drawBezierPath(drawState, shape, bezierPath);
}

CBezierPath
CQLottie::
getEllipsePath(const TimeFrame &timeFrame, const CLottieShape *shape) const
{
  auto positionxy = shape->pos().tvalue(timeFrame, CLottie::XYVals()).value();
  auto position   = positionxy.toPoint(CPoint2D(0, 0));

  auto sizexy = shape->size().tvalue(timeFrame, CLottie::XYVals()).value();
  auto size   = sizexy.toPoint(CPoint2D(0, 0));

  //---

  auto a1 = -M_PI/2.0;
  auto a2 = a1 + 2.0*M_PI;

  CArcToBezier::BezierList beziers;
  CArcToBezier::ArcToBeziers(position.x, position.y, size.x/2.0, size.y/2.0, a1, a2, beziers);

  return CBezierPath(beziers);
}

void
CQLottie::
drawPath(DrawState &drawState, const CLottieShape *shape)
{
  CBezierPath bezierPath;
  pathToBezier(shape->path_, drawState, bezierPath);

  drawBezierPath(drawState, shape, bezierPath);
}

void
CQLottie::
drawBezierPath(DrawState &drawState, const CLottieShape *shape, CBezierPath &bezierPath)
{
#if 0
  if (drawState.trim.shape) {
    auto *trimShape = getDrawTrimShape(drawState);
    assert(trimShape == drawState.trim.shape);

    bezierPath = trimPath(drawState, trimShape, bezierPath);
  }
#endif

  //---

  if (drawState.merge.shape) {
    auto *mergeShape = getDrawMergeShape(drawState);
    assert(mergeShape == drawState.merge.shape);

    drawState.merge.paths.push_back(bezierPath);

    return;
  }

  //---

  if (drawState.rounded.shape) {
    auto r = drawState.rounded.shape->rounded()->roundness.tvalue(drawState.timeFrame, 0.0).value();

    if (r > 0.0)
      bezierPath = bezierPath.rounded(r/100.0);
  }

  //---

  QPainterPath path;
  toQPath(bezierPath, path);

  //---

  auto *fillShape = getDrawFillShape(drawState);

  if (fillShape) {
    auto *fill = fillShape->fill();

    if (fill->fillRule)
      drawState.fill.rule = fill->fillRule.value();
  }

  if (drawState.fill.rule == 2)
    path.setFillRule(Qt::OddEvenFill);
  else
    path.setFillRule(Qt::WindingFill);

  //---

  drawState.painter->save();

  //---

  // draw path
  auto pmatrix = drawState.getDisplayMatrix();
  auto smatrix = getShapeMatrix(drawState, shape);

  auto dmatrix = pmatrix*smatrix.getMatrix();

  drawState.painter->setTransform(toQTransform(dmatrix));

  setPenBrush(drawState, shape);

  //---

  if      (drawState.trim.shape) {
    auto *trimShape = getDrawTrimShape(drawState);
    assert(trimShape == drawState.trim.shape);

    PathData pathData;
    setPathData(drawState, path, smatrix, pathData);

    drawState.trim.paths.push_back(pathData);
  }
  else if (drawState.maskData) {
    PathData pathData;
    setPathData(drawState, path, smatrix, pathData);

    drawState.maskData->paths.push_back(pathData);
  }
  else {
    if (drawState.stroker) {
      auto lpath = drawState.stroker->createStroke(path);

      lpath.setFillRule(Qt::WindingFill);

      drawPainterPath(drawState.painter, lpath);
    }
    else
      drawPainterPath(drawState.painter, path);

    //---

    auto bbox = CQLottieUtil::toBBox(CQLottieUtil::transformRect(smatrix, path.boundingRect()));

    const_cast<CLottieShape *>(shape)->setBBox(bbox);

    //---

    if (isShowSelect() && shape->isHierSelected())
      addSelectedPath(drawState.painter, path);
  }

  //---

  drawState.painter->restore();

  //---

  if (isShowBBox() && shape->isHierSelected() && shape->bbox().isSet()) {
    auto rect = CQLottieUtil::toQRect(shape->bbox());

    addBBoxRect(drawState.painter, rect);
  }

  //---

  if (drawState.layer)
    drawState.layer->setChanged(true);
}

CBezierPath
CQLottie::
trimPath(DrawState &drawState, const CLottieShape *trimShape, const CBezierPath &bezierPath) const
{
  auto *trim = trimShape->trim();

  auto start  = trim->start .tvalue(drawState.timeFrame,   0.0).value()/100.0;
  auto end    = trim->end   .tvalue(drawState.timeFrame, 100.0).value()/100.0;
  auto offset = trim->offset.tvalue(drawState.timeFrame,   0.0).value()/360.0;
//auto mult   = trim->multiple.value_or(1);

  auto start1 = start + offset;
  auto end1   = end   + offset;

  if (start1 < 0) start1 += 1.0;
  if (start1 > 1) start1 -= 1.0;
  if (end1   < 0) end1   += 1.0;
  if (end1   > 1) end1   -= 1.0;

  return bezierPath.split(start1, end1);
}

CLottieShape *
CQLottie::
getDrawMergeShape(const DrawState &drawState) const
{
  for (const auto &objData : drawState.objects) {
    for (auto *obj : objData.siblings) {
      if (obj->isHidden().value_or(false))
        continue;

      auto objectType = obj->objectType();

      if (objectType == CLottieObject::Type::SHAPE) {
        auto *shape = dynamic_cast<CLottieShape *>(obj);

        if (shape->shapeType() == CLottieObject::ShapeType::MERGE)
          return shape;
      }
    }
  }

  return nullptr;
}

CLottieShape *
CQLottie::
getDrawTrimShape(const DrawState &drawState) const
{
  for (const auto &objData : drawState.objects) {
    for (auto *obj : objData.siblings) {
      if (obj->isHidden().value_or(false))
        continue;

      auto objectType = obj->objectType();

      if (objectType == CLottieObject::Type::SHAPE) {
        auto *shape = dynamic_cast<CLottieShape *>(obj);

        if (shape->shapeType() == CLottieObject::ShapeType::TRIM)
          return shape;
      }
    }
  }

  return nullptr;
}

//---

QPainterPath
CQLottie::
drawPathData(QPainter *painter, const PathData &pathData) const
{
  CBezierPath bezierPath;
  fromQPath(pathData.path, bezierPath);

  QPainterPath ppath;
  toQPath(bezierPath, ppath);

  return drawPathDataPath(painter, pathData, ppath);
}

QPainterPath
CQLottie::
drawPathDataPath(QPainter *painter, const PathData &pathData, const QPainterPath &ppath) const
{
  painter->setPen  (pathData.pen);
  painter->setBrush(pathData.brush);

  painter->setTransform(pathData.transform);

  if (pathData.stroker) {
    auto lpath = pathData.stroker->createStroke(ppath);

    lpath.setFillRule(Qt::WindingFill);

    drawPainterPath(painter, lpath);

    return lpath;
  }
  else {
    drawPainterPath(painter, ppath);

    return ppath;
  }
}

void
CQLottie::
drawPainterPath(QPainter *painter, const QPainterPath &path) const
{
#if 0
  if (painter->pen().style() == Qt::CustomDashLine) {
    auto pattern = pen.dashPattern();
    auto offset  = pen.dashOffset();
  }
#endif

  painter->drawPath(path);
}

//---

void
CQLottie::
addSelectedRect(QPainter *painter, const QRectF &rect)
{
  QPainterPath path;
  path.addRect(rect);

  addSelectedPath(painter, path);
}

void
CQLottie::
addSelectedPath(QPainter *painter, const QPainterPath &path)
{
  PathData pathData;

  pathData.transform = painter->transform();
  pathData.path      = path;
  pathData.pen       = painter->pen();
  pathData.brush     = painter->brush();

  selectedPaths_.push_back(pathData);
}

void
CQLottie::
addBBoxRect(QPainter *, const QRectF &rect)
{
  RectData rectData;

  rectData.rect = rect;

  bboxRects_.push_back(rectData);
}

void
CQLottie::
setPathData(const DrawState &drawState, const QPainterPath &path,
            const CMatrixStack2D &smatrix, PathData &pathData) const
{
  pathData.transform = drawState.painter->transform();
  pathData.path      = path;
  pathData.pen       = drawState.painter->pen();
  pathData.brush     = drawState.painter->brush();
  pathData.stroker   = drawState.stroker;
  pathData.smatrix   = smatrix;
}

void
CQLottie::
pathToBezier(const CLottie::BezierProperty &path, const DrawState &drawState,
             CBezierPath &bezierPath) const
{
  if (! lottie_->pathToBezier(path, drawState.timeFrame, bezierPath)) {
    errOnce(__LINE__, "pathToBezier failed\n");
    return;
  }
}

void
CQLottie::
drawPolystar(DrawState &drawState, const CLottieShape *shape)
{
  auto bezierPath = getPolyStarPath(drawState.timeFrame, shape);

  drawBezierPath(drawState, shape, bezierPath);
}

CBezierPath
CQLottie::
getPolyStarPath(const TimeFrame &timeFrame, const CLottieShape *shape) const
{
  CBezierPath bezierPath;

  auto *polyStar = shape->polyStar();
  if (! polyStar) return bezierPath;

  auto positionxy = polyStar->position.tvalue(timeFrame, CLottie::XYVals()).value();
  auto position   = positionxy.toPoint(CPoint2D(0, 0));

  auto type = polyStar->type.value_or(1);

  if (type != 1 && type != 2)
    warnOnce(__LINE__, "unhandled polystar type: " + std::to_string(type));

  auto numPoints = int(polyStar->points.tvalue(timeFrame, 0).value_or(0));

  auto innerRadius    = polyStar->innerRadius   .tvalue(timeFrame, 0).value_or(0);
  auto innerRoundness = polyStar->outerRoundness.tvalue(timeFrame, 0).value_or(0);
  auto outerRadius    = polyStar->outerRadius   .tvalue(timeFrame, 0).value_or(0);
  auto outerRoundness = polyStar->outerRoundness.tvalue(timeFrame, 0).value_or(0);

  auto rotation = CMathGen::DegToRad(polyStar->rotation.tvalue(timeFrame, 0).value_or(0));

  if (type == 1)
    bezierPath.addPolyStar(position, numPoints, innerRadius, outerRadius,
                           innerRoundness/100.0, outerRoundness/100.0, rotation);
  else
    bezierPath.addPolygon(position, numPoints, outerRadius,
                          outerRoundness/100.0, rotation);

  //---

  return bezierPath;
}

void
CQLottie::
drawRectangle(DrawState &drawState, const CLottieShape *shape)
{
  auto bezierPath = getRectanglePath(drawState.timeFrame, shape);

  drawBezierPath(drawState, shape, bezierPath);
}

CBezierPath
CQLottie::
getRectanglePath(const TimeFrame &timeFrame, const CLottieShape *shape) const
{
  auto positionxy = shape->pos().tvalue(timeFrame, CLottie::XYVals()).value();
  auto position   = positionxy.toPoint(CPoint2D(0, 0));

  auto sizexy = shape->size().tvalue(timeFrame, CLottie::XYVals()).value();
  auto size   = sizexy.toPoint(CPoint2D(0, 0));

  auto p1 = CPoint2D(position.x - size.x/2.0, position.y - size.y/2.0);
  auto p2 = CPoint2D(position.x + size.x/2.0, position.y + size.y/2.0);

  auto bbox = CBBox2D(p1, p2);

  //---

  CBezierPath bezierPath;

  if (shape->rectangle()) {
    auto r = shape->rectangle()->roundness.tvalue(timeFrame, 0.0).value();

    bezierPath.addRoundedRect(bbox, r, r);
  }
  else
    bezierPath.addRect(bbox);

  return bezierPath;
}

void
CQLottie::
setPenBrush(DrawState &drawState, const CLottieShape *shape)
{
  PenBrush penBrush;
  calcPenBrush(drawState, shape, penBrush);

  drawState.painter->setBrush(penBrush.brush);
  drawState.painter->setPen  (penBrush.pen);

  drawState.stroker = penBrush.stroker;
}

void
CQLottie::
calcPenBrush(const DrawState &drawState, const CLottieShape *shape, PenBrush &penBrush) const
{
  if (drawState.fillGradient.shape) {
    auto *fillShape = getDrawGradientFillShape(drawState);
    assert(fillShape == drawState.fillGradient.shape);

    auto *gradientFill = fillShape->gradientFill();
    assert( gradientFill);

#if 0
    penBrush.brush = QBrush(drawState.fillGradient.gradient);
#else
    auto gradient = calcGradientFill(drawState.timeFrame, gradientFill);
    penBrush.brush = QBrush(gradient);
#endif
  }
  else {
    auto fillColor   = getHierFillColor  (drawState, shape, drawState.fill.color);
    auto fillOpacity = getHierFillOpacity(drawState, shape, drawState.fill.opacity);

    if (fillColor) {
      auto c = CQLottieUtil::toQColor(fillColor.value());

      if (fillOpacity)
        c.setAlpha(int(255*(fillOpacity.value()/100.0)));

      penBrush.brush = QBrush(c);
    }
    else
      penBrush.brush = QBrush(Qt::NoBrush);
  }

  //---

  if (drawState.strokeGradient.shape) {
    auto *strokeShape = getDrawGradientStrokeShape(drawState);
    assert(strokeShape == drawState.strokeGradient.shape);

    auto *gradientStroke = strokeShape->gradientStroke();
    assert(gradientStroke);

    auto gradient = calcGradientStroke(drawState.timeFrame, gradientStroke);
//  auto opacity  = gradientStroke->opacity.tvalue(drawState.timeFrame, 100.0);

    auto width      = gradientStroke->width.tvalue(drawState.timeFrame);
    auto lineCap    = gradientStroke->lineCap;
    auto lineJoin   = gradientStroke->lineJoin;
    auto miterLimit = gradientStroke->miterLimit;

    penBrush.stroker = makeStroker();

    if (lineCap)
      penBrush.stroker->setCapStyle(CQLottieUtil::toLineCap(lineCap.value()));
    if (lineJoin)
      penBrush.stroker->setJoinStyle(CQLottieUtil::toLineJoin(lineJoin.value()));

    if (miterLimit)
      penBrush.stroker->setMiterLimit(miterLimit.value());

    penBrush.stroker->setDashOffset (0.0);
    penBrush.stroker->setDashPattern(Qt::SolidLine);

    if (width)
      penBrush.stroker->setWidth(width.value());

    penBrush.brush = QBrush(gradient);

    penBrush.pen.setStyle(Qt::NoPen);
  }
  else {
    auto strokeColor   = getHierStrokeColor  (drawState, shape);
    auto strokeOpacity = getHierStrokeOpacity(drawState, shape);

    if (strokeColor) {
      auto c = CQLottieUtil::toQColor(*strokeColor);

      if (strokeOpacity)
        c.setAlpha(int(255*(strokeOpacity.value()/100.0)));

      penBrush.pen.setColor(c);
    }
    else {
      penBrush.pen.setStyle(Qt::NoPen);
      //penBrush.pen.setColor(Qt::black);
    }

    auto *strokeShape = getDrawStrokeShape(drawState);

    if (strokeShape) {
      auto *stroke = strokeShape->stroke();

      penBrush.strokeWidth      = stroke->width.tvalue(drawState.timeFrame);
      penBrush.strokeLineCap    = stroke->lineCap;
      penBrush.strokeLineJoin   = stroke->lineJoin;
      penBrush.strokeMiterLimit = stroke->miterLimit;

      if (! stroke->dash.dash.empty()) {
        std::vector<double> lengths;
        double              offset { 0.0 };

        if (! stroke->dash.offset.empty())
          offset = stroke->dash.offset[0].tvalue(drawState.timeFrame).value_or(0.0);

        if (! stroke->dash.dash.empty())
          lengths.push_back(stroke->dash.dash[0].tvalue(drawState.timeFrame).value_or(0.0));

        if (! stroke->dash.gap.empty())
          lengths.push_back(stroke->dash.gap[0].tvalue(drawState.timeFrame).value_or(0.0));

        penBrush.strokeLineDash = CLineDash(lengths, offset);
      }
    }

    if (penBrush.strokeWidth)
      penBrush.pen.setWidthF(penBrush.strokeWidth.value());

    if (penBrush.strokeLineCap)
      penBrush.pen.setCapStyle(CQLottieUtil::toLineCap(penBrush.strokeLineCap.value()));

    if (penBrush.strokeLineJoin)
      penBrush.pen.setJoinStyle(CQLottieUtil::toLineJoin(penBrush.strokeLineJoin.value()));

    if (strokeShape && ! strokeShape->stroke()->dash.dash.empty())
      CQLottieUtil::penSetLineDash(penBrush.pen, penBrush.strokeLineDash);

    if (penBrush.strokeMiterLimit)
      penBrush.pen.setMiterLimit(penBrush.strokeMiterLimit.value());
  }
}

QPainterPathStroker *
CQLottie::
makeStroker() const
{
  auto *stroker = new QPainterPathStroker();

  strokers_.push_back(stroker);

  return stroker;
}

void
CQLottie::
setSelectedPenBrush(QPainter *painter)
{
  auto brush = QBrush(selectedBrushColor(), Qt::Dense6Pattern);
  brush.setTransform(painter->transform().inverted());

  painter->setBrush(brush);
  painter->setPen  (selectedPenColor());
}

void
CQLottie::
setBBoxPenBrush(QPainter *painter)
{
  QPen pen;

  pen.setColor(bboxPenColor());
  pen.setWidthF(3.0);
  pen.setCosmetic(true);

  auto brush = QBrush(selectedBrushColor(), Qt::Dense5Pattern);
  brush.setTransform(painter->transform().inverted());

  painter->setBrush(brush);
//painter->setBrush(Qt::NoBrush);
  painter->setPen  (pen);
}

//---

void
CQLottie::
mousePress(const QPoint &pos)
{
  CPoint2D p;
  displayRange_.pixelToWindow(CPoint2D(pos.x(), pos.y()), p);

  insideObject_ = nullptr;

  for (auto *shape : lottie_->shapes()) {
    const auto &bbox = shape->bbox();

    if (! bbox.inside(p))
      continue;

    if (insideObject_) {
      if (shape->bbox().area() < insideObject_->bbox().area())
        insideObject_ = shape;
    }
    else
      insideObject_ = shape;
  }

  if (! insideObject_) {
    for (auto *layer : lottie_->layers()) {
      const auto &bbox = layer->bbox();

      if (! bbox.inside(p))
        continue;

      if (insideObject_) {
        if (layer->bbox().area() < insideObject_->bbox().area())
          insideObject_ = layer;
      }
      else
        insideObject_ = layer;
    }
  }

  selectInsideObject();
}

void
CQLottie::
mouseMove(const QPoint &pos)
{
  CPoint2D p;
  displayRange_.pixelToWindow(CPoint2D(pos.x(), pos.y()), p);

  status_->setStatusLabel(QString("%1 %2").arg(p.x).arg(p.y));
}

//---

void
CQLottie::
nextGeomShape()
{
  auto *insideShape = dynamic_cast<CLottieShape *>(insideObject_);
  if (! insideShape) return;

  auto *pshape = insideShape->getParentShape();
  auto *player = insideShape->getParentLayer();

  if      (pshape) {
    bool found = false;

    for (auto *shape : pshape->shapes()) {
      if      (shape == insideShape)
        found = true;
      else if (found) {
        insideObject_ = shape;
        break;
      }
    }
  }
  else if (player) {
    bool found = false;

    for (auto *shape : player->shapes()) {
      if      (shape == insideShape)
        found = true;
      else if (found) {
        insideObject_ = shape;
        break;
      }
    }
  }

  selectInsideObject();
}

void
CQLottie::
selectInsideObject()
{
  lottie_->deselectAll();

  if (insideObject_) {
    insideObject_->setSelected(true);

    tree_->selectObject(insideObject_);
  }

  canvas()->invalidate();
}

//---

CMatrixStack2D
CQLottie::
getLayerMatrix(const DrawState &drawState, const CLottieLayer *layer) const
{
  auto m = getLayerTransformMatrix(drawState, layer);

  for (const auto &objData : drawState.objects) {
    auto *obj = objData.object;

    auto objectType = obj->objectType();

    if (objectType == CLottieObject::Type::LAYER) {
      auto *layer1 = dynamic_cast<CQLottieLayer *>(obj);

      m = getLayerTransformMatrix(drawState, layer1).append(m);
    }
  }

  return m;
}

CMatrixStack2D
CQLottie::
getShapeMatrix(const DrawState &drawState, const CLottieShape *shape) const
{
  auto m = getShapeTransformMatrix(drawState, shape);

  for (const auto &objData : drawState.objects) {
    auto *obj = objData.object;

    auto objectType = obj->objectType();

    if      (objectType == CLottieObject::Type::LAYER) {
      auto *layer = dynamic_cast<CQLottieLayer *>(obj);

      m = getLayerTransformMatrix(drawState, layer).append(m);
    }
    else if (objectType == CLottieObject::Type::REPEATER) {
      auto *repeater = dynamic_cast<CLottieRepeater *>(obj);

      auto repeatMatrix = calcRepeatMatrix(drawState, repeater);

      m = repeatMatrix.append(m);
    }
    else if (objectType == CLottieObject::Type::SHAPE) {
      auto *shape = dynamic_cast<CLottieShape *>(obj);

      auto *transformShape = shape->getTransformShape();

      if (transformShape)
        m = getShapeTransformMatrix(drawState, transformShape).append(m);
      else
        m = getShapeTransformMatrix(drawState, shape).append(m);
    }
  }

  return m;
}

CMatrixStack2D
CQLottie::
calcRepeatMatrix(const DrawState &drawState, CLottieRepeater *repeater) const
{
//assert(repeater->ind == drawState.repeatInd);

  auto repeatOffset = repeater->offset.tvalue(drawState.timeFrame, 0.0).value();

  double mult = repeater->ind.value_or(0) + repeatOffset;

  return lottie_->getRepeaterMatrix(drawState.timeFrame, repeater->transform, mult);
}

CMatrixStack2D
CQLottie::
getLayerTransformMatrix(const DrawState &drawState, const CLottieLayer *layer) const
{
  return lottie_->getTransformMatrix(drawState.timeFrame, layer->transform(),
                                     layer->autoOrient().value_or(false));
}

CMatrixStack2D
CQLottie::
getShapeTransformMatrix(const DrawState &drawState, const CLottieShape *shape) const
{
  return lottie_->getTransformMatrix(drawState.timeFrame, shape->transform());
}

#if 0
CRGBA
CQLottie::
getLayerColor(const CLottieLayer *layer, const CRGBA &def) const
{
#if 1
  if (layer->color.isTSet())
    return layer->color.tvalue(drawState.timeFrame, def);
#else
  if (layer->color.isSet())
    return layer->color.value(def);
#endif

  auto *player = layer->getParent();

  if (player)
    return getLayerColor(player, def);

  return def;
}
#endif

CQLottie::OptColor
CQLottie::
getHierFillColor(const DrawState &drawState, const CLottieShape *shape, const OptColor &def) const
{
  OptColor c;

  if (drawState.fill.shape && drawState.fill.shape->fill()) {
    c = drawState.fill.shape->fill()->color.tvalue(drawState.timeFrame);
    if (c) return c;
  }

  c = getFillColor(drawState.timeFrame, shape, def);
  if (c) return c;

  for (const auto &objData : drawState.objects) {
    bool strokeFill = false;

    for (auto *obj : objData.siblings) {
      if (obj->isHidden().value_or(false))
        continue;

      auto objectType = obj->objectType();

      if (objectType == CLottieObject::Type::SHAPE) {
        auto *shape1 = dynamic_cast<CLottieShape *>(obj);

        if      (shape1->shapeType() == CLottieObject::ShapeType::STROKE) {
          strokeFill = true;
        }
        else if (shape1->shapeType() == CLottieObject::ShapeType::FILL) {
          c = getFillColor(drawState.timeFrame, shape1, def);

          strokeFill = true;
        }
      }
    }

    if (strokeFill)
      break;
  }

  return (c ? c : def);
}

CLottieShape *
CQLottie::
getDrawFillShape(const DrawState &drawState) const
{
  for (const auto &objData : drawState.objects) {
    for (auto *obj : objData.siblings) {
      if (obj->isHidden().value_or(false))
        continue;

      auto objectType = obj->objectType();

      if (objectType == CLottieObject::Type::SHAPE) {
        auto *shape = dynamic_cast<CLottieShape *>(obj);

        if (shape->shapeType() == CLottieObject::ShapeType::FILL)
          return shape;
      }
    }
  }

  return nullptr;
}

CQLottie::OptColor
CQLottie::
getFillColor(const TimeFrame &timeFrame, const CLottieShape *shape, const OptColor &def) const
{
  OptColor c;

  if (shape->fill()) {
    c = shape->fill()->color.tvalue(timeFrame);
    if (c) return c;
  }

  if (shape->color().isTSet()) {
    c = shape->color().tvalue(timeFrame);
    if (c) return c;
  }

  auto *fillShape = shape->getFillShape();

  if (fillShape) {
    c = getFillColor(timeFrame, fillShape, def);
    if (c) return c;
  }

  return (c ? c : def);
}

CQLottie::OptReal
CQLottie::
getHierFillOpacity(const DrawState &drawState, const CLottieShape *shape, const OptReal &def) const
{
  OptReal o;

  if (drawState.fill.shape && drawState.fill.shape->fill()) {
    o = drawState.fill.shape->fill()->opacity.tvalue(drawState.timeFrame);
  }

  if (! o)
    o = getFillOpacity(drawState.timeFrame, shape, def);

#if 0
  auto *pshape = shape->getParentShape();

  if (! o && pshape)
    return getHierFillOpacity(drawState, pshape, def);

  auto *player = shape->getHierParentLayer();
  assert(player);

  auto *repeater = player->calcRepeater();

  if (repeater) {
    auto repeatOpacity = getRepeatOpacity(drawState, repeater);

    o = combineOpacities(o, repeatOpacity);
  }

  auto o1 = getHierLayerOpacity(drawState, player, def);

  if (o1)
    o = combineOpacities(o, o1);
#else
  for (const auto &objData : drawState.objects) {
    auto *obj = objData.object;

    auto objectType = obj->objectType();

    if      (objectType == CLottieObject::Type::LAYER) {
      auto *layer = dynamic_cast<CQLottieLayer *>(obj);

      auto o1 = getLayerOpacity(drawState, layer, def);

      if (o1)
        o = combineOpacities(o, o1);
    }
    else if (objectType == CLottieObject::Type::REPEATER) {
      auto *repeater = dynamic_cast<CLottieRepeater *>(obj);

      auto o1 = getRepeatOpacity(drawState, repeater);

      o = combineOpacities(o, o1);
    }
    else if (objectType == CLottieObject::Type::SHAPE) {
      auto *shape1 = dynamic_cast<CLottieShape *>(obj);

      auto o1 = getFillOpacity(drawState.timeFrame, shape1, def);

      if (o1)
        o = combineOpacities(o, o1);
    }
  }
#endif

  return (o ? o : def);
}

CQLottie::OptReal
CQLottie::
getFillOpacity(const TimeFrame &timeFrame, const CLottieShape *shape, const OptReal &def) const
{
  OptReal o;

  if (shape->fill()) {
    o = shape->fill()->opacity.tvalue(timeFrame);
    if (o) return o;
  }

  // TODO: transform opacity not inherited !

  if (shape->transform()) {
    o = shape->transform()->opacity.tvalue(timeFrame);
    if (o) return o;
  }

  auto *fillShape = shape->getFillShape();

  if (fillShape && fillShape != shape) {
    o = getFillOpacity(timeFrame, fillShape, def);
    if (o) return o;
  }

  return (o ? o : def);
}

CQLottie::OptReal
CQLottie::
getHierLayerOpacity(const DrawState &drawState, const CLottieLayer *layer, const OptReal &def) const
{
  auto o = getLayerOpacity(drawState, layer, def);

  auto *player = layer->getParentLayer();

  if (player) {
    auto o1 = getHierLayerOpacity(drawState, player, def);

    if (o1)
      o = combineOpacities(o, o1);
  }

  return (o ? o : def);
}

CQLottie::OptReal
CQLottie::
getLayerOpacity(const DrawState &drawState, const CLottieLayer *layer, const OptReal &def) const
{
  auto typeId = layer->typeId().value_or(-1);
  if (typeId == 3) return def;

  OptReal o;

  if (layer->transform())
    o = layer->transform()->opacity.tvalue(drawState.timeFrame);

  return (o ? o : def);
}

CQLottie::OptColor
CQLottie::
getHierStrokeColor(const DrawState &drawState, const CLottieShape *shape, const OptColor &def) const
{
  OptColor c;

  if (drawState.stroke.shape && drawState.stroke.shape->stroke()) {
    c = drawState.stroke.shape->stroke()->color.tvalue(drawState.timeFrame);
    if (c) return c;
  }

  c = getStrokeColor(drawState.timeFrame, shape, def);
  if (c) return c;

  for (const auto &objData : drawState.objects) {
    bool strokeFill = false;

    for (auto *obj : objData.siblings) {
      if (obj->isHidden().value_or(false))
        continue;

      auto objectType = obj->objectType();

      if (objectType == CLottieObject::Type::SHAPE) {
        auto *shape1 = dynamic_cast<CLottieShape *>(obj);

        if      (shape1->shapeType() == CLottieObject::ShapeType::STROKE) {
          c = getStrokeColor(drawState.timeFrame, shape1, def);

          strokeFill = true;
        }
        else if (shape1->shapeType() == CLottieObject::ShapeType::FILL) {
          strokeFill = true;
        }
      }
    }

    if (strokeFill)
      break;
  }

  return (c ? c : def);
}

CLottieShape *
CQLottie::
getDrawStrokeShape(const DrawState &drawState) const
{
  CLottieShape *strokeShape = nullptr;

  for (const auto &objData : drawState.objects) {
    auto *obj = objData.object;

    if (obj->isHidden().value_or(false))
      continue;

    auto objectType = obj->objectType();

    if      (objectType == CLottieObject::Type::LAYER) {
      auto *layer = dynamic_cast<CLottieLayer *>(obj);

      auto *strokeShape1 = layer->getStrokeShape();

      if (strokeShape1 && strokeShape1->stroke()) {
        strokeShape = strokeShape1;
        break;
      }
    }
    else if (objectType == CLottieObject::Type::SHAPE) {
      auto *shape = dynamic_cast<CLottieShape *>(obj);

      auto *strokeShape1 = shape->getStrokeShape();

      if (strokeShape1 && strokeShape1->stroke()) {
        strokeShape = strokeShape1;
        break;
      }
    }
  }

  return strokeShape;
}

CQLottie::OptColor
CQLottie::
getStrokeColor(const TimeFrame &timeFrame, const CLottieShape *shape, const OptColor &def) const
{
  OptColor c;

  if (shape->stroke()) {
    c = shape->stroke()->color.tvalue(timeFrame);
    if (c) return c;
  }

  if (shape->color().isTSet()) {
    c = shape->color().tvalue(timeFrame);
    if (c) return c;
  }

  auto *strokeShape = shape->getStrokeShape();

  if (strokeShape) {
    c = getStrokeColor(timeFrame, strokeShape, def);
    if (c) return c;
  }

  return (c ? c : def);
}

CQLottie::OptReal
CQLottie::
getHierStrokeOpacity(const DrawState &drawState, const CLottieShape *shape,
                     const OptReal &def) const
{
  auto o = getStrokeOpacity(drawState.timeFrame, shape, def);

#if 0
  auto *pshape = shape->getParentShape();

  if (! o && pshape)
    return getHierStrokeOpacity(drawState, pshape, def);

  auto *player = shape->getHierParentLayer();
  assert(player);

  auto *repeater = player->calcRepeater();

  if (repeater) {
    auto repeatOpacity = getRepeatOpacity(drawState, repeater);

    o = combineOpacities(o, repeatOpacity);
  }

  auto o1 = getHierLayerOpacity(drawState, player, def);

  if (o1)
    o = combineOpacities(o, o1);
#else
  for (const auto &objData : drawState.objects) {
    auto *obj = objData.object;

    auto objectType = obj->objectType();

    if      (objectType == CLottieObject::Type::LAYER) {
      auto *layer = dynamic_cast<CQLottieLayer *>(obj);

      auto o1 = getLayerOpacity(drawState, layer, def);

      if (o1)
        o = combineOpacities(o, o1);
    }
    else if (objectType == CLottieObject::Type::REPEATER) {
      auto *repeater = dynamic_cast<CLottieRepeater *>(obj);

      auto o1 = getRepeatOpacity(drawState, repeater);

      o = combineOpacities(o, o1);
    }
    else if (objectType == CLottieObject::Type::SHAPE) {
      auto *shape = dynamic_cast<CLottieShape *>(obj);

      auto o1 = getStrokeOpacity(drawState.timeFrame, shape, def);

      o = combineOpacities(o, o1);
    }
  }
#endif

  return (o ? o : def);
}

CQLottie::OptReal
CQLottie::
getStrokeOpacity(const TimeFrame &timeFrame, const CLottieShape *shape,
                 const OptReal &def) const
{
  OptReal o;

  if (shape->stroke())
    o = shape->stroke()->opacity.tvalue(timeFrame, 100.0).value();

  return (o ? o : def);
}

double
CQLottie::
getRepeatOpacity(const DrawState &drawState, CLottieRepeater *repeater) const
{
//assert(repeater->ind == drawState.repeatInd);

  auto repeatStartOpacity = repeater->startOpacity.tvalue(drawState.timeFrame, 100.0).value();
  auto repeatEndOpacity   = repeater->endOpacity  .tvalue(drawState.timeFrame, 100.0).value();

  auto repeatCopies = int(repeater->copies.tvalue(drawState.timeFrame, 1.0).value());

  return CMathUtil::map(repeater->ind.value_or(0), 0, repeatCopies - 1,
                        repeatStartOpacity, repeatEndOpacity);
}

CLottieShape *
CQLottie::
getDrawGradientFillShape(const DrawState &drawState) const
{
  for (const auto &objData : drawState.objects) {
    auto *obj = objData.object;

    if (obj->isHidden().value_or(false))
      continue;

    auto objectType = obj->objectType();

    if      (objectType == CLottieObject::Type::LAYER) {
      auto *layer = dynamic_cast<CLottieLayer *>(obj);

      auto *fillShape1 = layer->getFillShape();

      if (fillShape1 && fillShape1->fill())
        return nullptr;

      auto *fillShape2 = layer->getGradientFillShape();

      if (fillShape2 && fillShape2->gradientFill())
        return fillShape2;
    }
    else if (objectType == CLottieObject::Type::SHAPE) {
      auto *shape = dynamic_cast<CLottieShape *>(obj);

      auto *fillShape1 = shape->getFillShape();

      if (fillShape1 && fillShape1->fill())
        return nullptr;

      auto *fillShape2 = shape->getGradientFillShape();

      if (fillShape2 && fillShape2->gradientFill())
        return fillShape2;
    }
  }

  return nullptr;
}

CLottieShape *
CQLottie::
getDrawGradientStrokeShape(const DrawState &drawState) const
{
  for (const auto &objData : drawState.objects) {
    auto *obj = objData.object;

    if (obj->isHidden().value_or(false))
      continue;

    auto objectType = obj->objectType();

    if      (objectType == CLottieObject::Type::LAYER) {
      auto *layer = dynamic_cast<CLottieLayer *>(obj);

      auto *strokeShape1 = layer->getStrokeShape();

      if (strokeShape1 && strokeShape1->stroke())
        return nullptr;

      auto *strokeShape2 = layer->getGradientStrokeShape();

      if (strokeShape2 && strokeShape2->gradientStroke())
        return strokeShape2;
    }
    else if (objectType == CLottieObject::Type::SHAPE) {
      auto *shape = dynamic_cast<CLottieShape *>(obj);

      auto *strokeShape1 = shape->getStrokeShape();

      if (strokeShape1 && strokeShape1->stroke())
        return nullptr;

      auto *strokeShape2 = shape->getGradientStrokeShape();

      if (strokeShape2 && strokeShape2->gradientStroke())
        return strokeShape2;
    }
  }

  return nullptr;
}

//---

std::string
CQLottie::
hierName(CLottieObject *object, const DrawState &drawState) const
{
  auto name = object->name().value_or("<none>");

  for (const auto &objData : drawState.objects) {
    auto name1 = objData.object->name().value_or("");

    if (name1 == "") {
      auto *asset = dynamic_cast<CLottieAsset *>(objData.object);

      if (asset && asset->id())
        name1 = asset->id().value();
    }

    if (name1 == "")
      name1 = "<none>";

    name = name1 + "/" + name;
  }

  return name;
}

//---

void
CQLottie::
zoom(bool zoomIn)
{
  if (zoomIn)
    displayRange_.zoomIn();
  else
    displayRange_.zoomOut();

  canvas_->invalidate();
}

void
CQLottie::
scroll(double dx, double dy)
{
  const auto *root = lottie_->root();

  int w = root->width ().value_or(100);
  int h = root->height().value_or(100);

  auto dw = w/20.0;
  auto dh = h/20.0;

  displayRange_.scroll(dw*dx, dh*dy);

  canvas_->invalidate();
}

void
CQLottie::
zoomFull()
{
  displayRange_.reset();

  canvas_->invalidate();
}

void
CQLottie::
zoomTo(const CBBox2D &bbox)
{
  if (! bbox.isSet())
    return;

  displayRange_.zoomTo(bbox.getXMin(), bbox.getYMin(), bbox.getXMax(), bbox.getYMax());

  canvas_->invalidate();
}

//---

bool
CQLottie::
isShowTimeLine() const
{
  return timeLine_->isVisible();
}

void
CQLottie::
setShowTimeLine(bool b)
{
  return timeLine_->setVisible(b);
}

bool
CQLottie::
isShowPath() const
{
  return path_->isVisible();
}

void
CQLottie::
setShowPath(bool b)
{
  return path_->setVisible(b);
}

//---

void
CQLottie::
toQPath(const CBezierPath &bezierPath, QPainterPath &path) const
{
  bool first = true;

  for (const auto &b : bezierPath.beziers()) {
    auto p1 = b.getFirstPoint();
    auto p2 = b.getControlPoint1();
    auto p3 = b.getControlPoint2();
    auto p4 = b.getLastPoint();

    if (first) {
      path.moveTo(CQLottieUtil::toQPoint(p1));

      first = false;
    }

    path.cubicTo(CQLottieUtil::toQPoint(p2),
                 CQLottieUtil::toQPoint(p3),
                 CQLottieUtil::toQPoint(p4));

    if (b.isBreak())
      first = true;
  }

  if (bezierPath.isClosed())
    path.closeSubpath();
}

QTransform
CQLottie::
toQTransform(const CMatrix2D &m) const
{
  double a, b, c, d, tx, ty;

  m.getValues(&a, &b, &c, &d, &tx, &ty);

  return QTransform(a, c, b, d, tx, ty);
}

class CBezierPathVisitor : public QPainterPathVisitor {
 public:
  CBezierPathVisitor() { }

  const CBezierPath &bezierPath() const { return bezierPath_; }

  void moveTo(const QPointF &p) override {
    bezierPath_.moveTo(toPoint(p));
  }

  void lineTo(const QPointF &p) override {
    bezierPath_.lineTo(toPoint(p));
  }

  void quadTo(const QPointF &p1, const QPointF &p2) override {
    bezierPath_.quadTo(toPoint(p1), toPoint(p2));
  }

  void curveTo(const QPointF &p1, const QPointF &p2, const QPointF &p3) override {
    bezierPath_.cubicTo(toPoint(p1), toPoint(p2), toPoint(p3));
  }

 private:
  CPoint2D toPoint(const QPointF &p) { return CPoint2D(p.x(), p.y()); }

 private:
  CBezierPath bezierPath_;
};

void
CQLottie::
fromQPath(const QPainterPath &path, CBezierPath &bezierPath) const
{
  CBezierPathVisitor visitor;

  QPainterPathUtil::visitPath(path, visitor);

  bezierPath = visitor.bezierPath();
}

//---

CQLottieLayer::
CQLottieLayer(CQLottie *lottie) :
 CLottieLayer(lottie->lottie()), lottie_(lottie)
{
}

CQLottieLayer::
~CQLottieLayer()
{
  delete imagePainter_.painter;
}

void
CQLottieLayer::
resize(int w, int h)
{
  if (w != w_ || h != h_) {
    w_ = w;
    h_ = h;

    delete imagePainter_.painter;

    imagePainter_.painter = nullptr;
    imagePainter_.image   = QImage();
  }
}

QPainter *
CQLottieLayer::
painter()
{
  if (! imagePainter_.painter)
    createImagePainter(imagePainter_);

  return imagePainter_.painter;
}

void
CQLottieLayer::
createImagePainter(ImagePainter &imagePainter) const
{
  imagePainter.image = QImage(w_, h_, QImage::Format_ARGB32);

  imagePainter.image.fill(Qt::transparent);

  imagePainter.painter = new QPainter(&imagePainter.image);
}

void
CQLottieLayer::
clear()
{
  (void) painter();

  imagePainter_.image.fill(Qt::transparent);
}

//---

QImage
CQLottieUtil::
fillImage(const QImage &sourceImage, const CRGBA &color)
{
  CElapsedTimer etime("CQLottieUtil::fillImage");

  int iw = sourceImage.width ();
  int ih = sourceImage.height();

#if 0
  auto resultImage = QImage(iw, ih, QImage::Format_ARGB32);

  auto qcolor = CQLottieUtil::toQColor(color);

  for (int y = 0; y < ih; ++y) {
    for (int x = 0; x < iw; ++x) {
      auto c = sourceImage.pixelColor(x, y);

      if (c.alpha() > 0) {
        resultImage.setPixelColor(x, y, qcolor);
      }
    }
  }
#else
  auto resultImage = sourceImage;

  auto qrgb = CQLottieUtil::toQColor(color).rgb();

  for (int y = 0; y < ih; ++y) {
    auto *l = reinterpret_cast<QRgb *>(resultImage.scanLine(y));

    for (int x = 0; x < iw; ++x) {
      const auto &rgb1 = l[x];

      if (qAlpha(rgb1) > 0)
        l[x] = qrgb;
    }
  }
#endif

  return resultImage;
}

QImage
CQLottieUtil::
alphaImage(const QImage &sourceImage, double a)
{
  CElapsedTimer etime("CQLottieUtil::alphaImage");

  int iw = sourceImage.width ();
  int ih = sourceImage.height();

  auto resultImage = sourceImage;

  for (int y = 0; y < ih; ++y) {
    auto *l = reinterpret_cast<QRgb *>(resultImage.scanLine(y));

    for (int x = 0; x < iw; ++x) {
      const auto &rgb = l[x];

      auto a1 = a*qAlpha(rgb)/255.0;

      auto rgb1 = qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), int(a1*255.0));

      l[x] = rgb1;
    }
  }

  return resultImage;
}

QImage
CQLottieUtil::
applyDropShadow(const QImage &sourceImage, int blurRadius, const QColor &shadowColor,
                const QPointF &offset)
{
  CElapsedTimer etime("CQLottieUtil::applyDropShadow");

  if (sourceImage.isNull())
    return QImage();

  // Create a QGraphicsScene
  QGraphicsScene scene;

  // Create a QGraphicsPixmapItem from the source QImage
  auto *pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(sourceImage));
  scene.addItem(pixmapItem);

  // Create and configure the QGraphicsDropShadowEffect
  auto *shadowEffect = new QGraphicsDropShadowEffect();
  shadowEffect->setBlurRadius(blurRadius);
  shadowEffect->setColor(shadowColor);
  shadowEffect->setOffset(offset);

  // Apply the effect to the pixmap item
  pixmapItem->setGraphicsEffect(shadowEffect);

  // Calculate the bounding rectangle of the scene to ensure the entire shadow is captured
  // The shadow might extend beyond the original image's bounds
  auto sceneRect = scene.itemsBoundingRect();

  // Create a new QImage to render the scene into
  QImage resultImage(sceneRect.size().toSize(), QImage::Format_ARGB32);
  resultImage.fill(Qt::transparent); // Fill with transparent background

  // Create a QPainter to draw onto the resultImage
  QPainter painter(&resultImage);
  painter.setRenderHint(QPainter::Antialiasing);

  // Render the scene into the resultImage
  scene.render(&painter, QRectF(), sceneRect);

  delete pixmapItem;
//delete shadowEffect;

  return resultImage;
}
