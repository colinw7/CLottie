#include <CQLottie.h>
#include <CQLottieCanvas.h>
#include <CQLottieToolBar.h>
#include <CQLottieStatusBar.h>
#include <CQLottieTimeLine.h>
#include <CQLottieTree.h>
#include <CQLottieSettings.h>
#include <CLottie.h>

#include <CEncode64.h>
#include <CBezierPath.h>
#include <CArcToBezier.h>

#include <QTabWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QFileDialog>

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

QPointF toQPoint(const CPoint2D &point) {
  return QPointF(point.x, point.y);
}

CPoint2D toPoint(const QPointF &point) {
  return CPoint2D(point.x(), point.y());
}

QRectF toQRect(const CBBox2D &rect) {
  return QRectF(toQPoint(rect.getLL()), toQPoint(rect.getUR())).normalized();
}

CBBox2D toBBox(const QRectF &rect) {
  return CBBox2D(toPoint(rect.topLeft()), toPoint(rect.bottomRight()));
}

QColor toQColor(const CRGBA &color) {
  return QColor(color.getRedI(), color.getGreenI(), color.getBlueI(), color.getAlphaI());
}

#if 0
CRGBA toRGBA(const QColor &color) {
  return CRGBA(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}
#endif

QTransform toQTransform(const CMatrix2D &m) {
  double a, b, c, d, tx, ty;

  m.getValues(&a, &b, &c, &d, &tx, &ty);

  return QTransform(a, c, b, d, tx, ty);
}

QPainterPath toQPath(const CBezierPath &bezierPath) {
  QPainterPath path;

  bool first = true;

  for (const auto &b : bezierPath.beziers()) {
    auto p1 = b.getFirstPoint();
    auto p2 = b.getControlPoint1();
    auto p3 = b.getControlPoint2();
    auto p4 = b.getLastPoint();

    if (first) {
      path.moveTo(toQPoint(p1));

      first = false;
    }

    path.cubicTo(toQPoint(p2), toQPoint(p3), toQPoint(p4));

    if (b.isBreak())
      first = true;
  }

  if (bezierPath.isClosed())
    path.closeSubpath();

  return path;
}

CBBox2D transformBBox(const CMatrix2D &m, const CBBox2D &bbox) {
  if (bbox.isSet()) {
    CPoint2D p1, p2, p3, p4;

    m.multiplyPoint(bbox.getLL(), p1);
    m.multiplyPoint(bbox.getLR(), p2);
    m.multiplyPoint(bbox.getUL(), p3);
    m.multiplyPoint(bbox.getUR(), p4);

    CBBox2D bbox1(p1, p2);

    bbox1 += p3;
    bbox1 += p4;

    return bbox1;
  }
  else
    return bbox;
}

Qt::PenCapStyle toLineCap(int lineCap) {
  switch (lineCap) {
    default:
    case 1: return Qt::FlatCap;
    case 2: return Qt::RoundCap;
    case 3: return Qt::SquareCap;
  }
}

Qt::PenJoinStyle toLineJoin(int lineJoin) {
  switch (lineJoin) {
    default:
    case 1: return Qt::MiterJoin;
    case 2: return Qt::RoundJoin;
    case 3: return Qt::BevelJoin;
  }
}

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

  int w = root->width ().value_or(100);
  int h = root->height().value_or(100);

  displayRange_.setEqualScale(equalScale_);
  displayRange_.setWindowRange(0, 0, w, h);

  fps_ = std::max(root->frameRate().value_or(1.0), 1.0);

  dt_ = 1000.0/fps_;

  timer_->start(int(dt_));

  //---

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
}

void
CQLottie::
draw(QPainter *painter, bool update)
{
  const auto *root = lottie_->root();
  if (! root) return;

  DrawState drawState;

  drawState.painter = painter;

#if 0
  drawState.displayRanges.push_back(displayRange_);
#else
  drawState.displayRange = displayRange_;
#endif

  getTimeFrame(drawState.timeFrame);

  drawRoot(drawState, root, update);
}

void
CQLottie::
drawRoot(const DrawState &drawState, const CLottieRoot *root, bool update)
{
  if (root->hidden().value_or(false))
    return;

  auto bbox = root->bbox();

  drawChildLayers(drawState, root->childLayers(), update);

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

        qlayer->setDoubleBuffer(true);

        drawLayer(drawState, layer, update);
      }
    }

    for (auto it = childLayers.rbegin(); it != childLayers.rend(); ++it) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(*it);

      if (qlayer->isEnabled() && qlayer->isChanged()) {
        drawState.painter->drawImage(0, 0, qlayer->image());

        drawState.layer->setChanged(true);
      }
    }
  }
  else {
    CQLottieLayer *matteLayer = nullptr;

    for (auto *layer : childLayers) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);

      if (qlayer->matteTarget()) {
        qlayer->setDoubleBuffer(true);

        drawLayer(drawState, qlayer, update);

        matteLayer = qlayer;

        continue;
      }

      if (qlayer->matteMode()) {
        qlayer->setDoubleBuffer(true);

        drawLayer(drawState, qlayer, update);

        qlayer->setMatteLayer(matteLayer);
      }

      matteLayer = nullptr;
    }

    for (auto it = childLayers.rbegin(); it != childLayers.rend(); ++it) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(*it);

      if (qlayer->matteTarget())
        continue;

      if (qlayer->matteMode()) {
        if (qlayer->matteLayer()) {
          auto matteImage = matteLayerImage(qlayer, qlayer->matteLayer());

          drawState.painter->drawImage(0, 0, matteImage);
        }
      }
      else {
        drawLayer(drawState, qlayer, update);
      }
    }
  }
}

QImage
CQLottie::
matteLayerImage(CQLottieLayer *layer, CQLottieLayer *clipLayer) const
{
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

      if (c.alpha() > 0) {
        matteImage.setPixelColor(x, y, layerImage.pixelColor(x, y));
      }
    }
  }
#endif

  return matteImage;
}

void
CQLottie::
getTimeFrame(CLottieUtil::TimeFrame &timeFrame) const
{
  const auto *root = lottie_->root();

  timeFrame.frameStart = root->frameStart();
  timeFrame.frameStop  = root->frameStop ();

  timeFrame.secs  = secs_;
  timeFrame.frame = ticks_;
}

void
CQLottie::
drawLayer(const DrawState &drawState, CLottieLayer *layer, bool update)
{
  if (layer->hidden().value_or(false))
    return;

  int frame = int(drawState.timeFrame.frame) + drawState.frameDelta;

  if (layer->frameIn()) {
    if (frame < layer->frameIn().value())
      return;
  }

  if (layer->frameOut()) {
    if (frame > layer->frameOut().value())
      return;
  }

  //---

  auto drawState1 = drawState;

  drawState1.objects.push_front(layer);

  drawState1.matrix = getLayerMatrix(drawState, layer);

  //---

  auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);

  qlayer->resize(canvas_->width(), canvas_->height());

  if (qlayer->isDoubleBuffer()) {
    drawState1.painter = qlayer->painter();
    drawState1.layer   = qlayer;

    qlayer->setChanged(false);
  }

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

  drawLayerShapes(drawState1, layer);

#if 0
  drawLayerAssets(drawState.painter, drawState1, layer);
#endif

  drawChildLayers(drawState1, layer->childLayers(), update);

  //---

  auto bbox = layer->bbox();

  for (auto *layer : layer->childLayers())
    bbox += layer->bbox();

  for (auto *shape : layer->shapes())
    bbox += shape->bbox();

  layer->setBBox(bbox);

  //---

  if (isShowBBox() && layer->isHierSelected() && layer->bbox().isSet()) {
    auto displayMatrix = drawState.getDisplayMatrix();

    drawState.painter->setTransform(toQTransform(displayMatrix));

    setBBoxPenBrush(drawState.painter);

    drawState.painter->drawRect(toQRect(layer->bbox()));
  }
}

void
CQLottie::
drawLayerShapes(DrawState &drawState, const CLottieLayer *layer)
{
  bool isMerge = bool(drawState.merge);

  auto drawState1 = drawState;

//drawState1.merge.reset();
//drawState1.trim .reset();

#if 1
  auto *repeater = layer->calcRepeater();

  for (auto it = layer->shapes().rbegin(); it != layer->shapes().rend(); ++it) {
    auto *qshape = dynamic_cast<CQLottieShape *>(*it);

    if (repeater) {
      auto drawState2 = drawState1;

      // model::Repeater::Transform::matrix ?
#if 0
      auto matrix = getTransformMatrix(drawState2, repeater->transform);
#endif
      auto repeatCopies = int(repeater->copies.tvalue(drawState2.timeFrame, 1.0).value());
#if 0
      auto repeatOffset = repeater->offset.tvalue(drawState.timeFrame, 0.0).value();

      auto repeatStartOpacity = repeater->startOpacity.tvalue(drawState.timeFrame, 100.0).value();
      auto repeatEndOpacity   = repeater->endOpacity  .tvalue(drawState.timeFrame, 100.0).value();
#endif

      for (int i = 0; i < repeatCopies; ++i) {
#if 1
        drawState2.repeatInd = i;

#if 0
        drawState2.repeatOpacity =
          CMathUtil::map(i, 0, repeatCopies - 1, repeatStartOpacity, repeatEndOpacity);

        double mult = i + repeatOffset;

        drawState2.repeatMatrix =
          lottie_->getRepeaterMatrix(drawState1.timeFrame, repeater->transform, mult);
#endif
#endif

        drawShape(drawState2, qshape);
      }
    }
    else
      drawShape(drawState1, qshape);
  }
#else
  for (auto *shape : layer->shapes) {
    drawShape(drawState.painter, drawState1, shape);
  }
#endif

  //---

  if (drawState1.merge) {
    if (! isMerge)
      drawMergeShapes(drawState1);
    else {
      for (const auto &bezierPath : drawState1.merge->paths)
        drawState.merge->paths.push_back(bezierPath);
    }
  }
}

void
CQLottie::
drawMergeShapes(DrawState &drawState)
{
  int np = drawState.merge->paths.size();
  if (np <= 0) return;

  //---

  QPainterPath path;

  if      (drawState.merge->mode == 1) {
    auto bezierPath = drawState.merge->paths[0];

    for (int i = 1; i < np; ++i) {
      const auto &bezierPath1 = drawState.merge->paths[i];

      bezierPath.combine(bezierPath1);
    }

    path = toQPath(bezierPath);
  }
  else if (drawState.merge->mode == 2 ||
           drawState.merge->mode == 3 ||
           drawState.merge->mode == 4) {
    const auto &bezierPath = drawState.merge->paths[0];

    path = toQPath(bezierPath);

    for (int i = 1; i < np; ++i) {
      const auto &bezierPath1 = drawState.merge->paths[i];

      auto path1 = toQPath(bezierPath1);

      if      (drawState.merge->mode == 2) {
        path = path.united(path1);
      }
      else if (drawState.merge->mode == 3) {
        path = path.subtracted(path1);
      }
      else if (drawState.merge->mode == 4) {
        path = path.intersected(path1);
      }
    }
  }
  else {
    std::cerr << "invalid merge mode " << drawState.merge->mode << "\n";
  }

  //---

  auto *fill = drawState.merge->shape->calcFill();

  if (fill) {
    if (fill->color.isSet())
      drawState.fill.color = fill->color.tvalue(drawState.timeFrame);

    if (fill->opacity.isSet())
      drawState.fill.opacity = fill->opacity.tvalue(drawState.timeFrame);

    if (fill->fillRule)
      drawState.fill.rule = fill->fillRule.value();
  }

  //---

  Qt::FillRule fillRule;

  if (drawState.fill.rule == 2)
    fillRule = Qt::OddEvenFill;
  else
    fillRule = Qt::WindingFill;

  path.setFillRule(fillRule);

  //---

  drawState.painter->save();

  //---

  auto pmatrix = drawState.getDisplayMatrix();
  auto smatrix = getShapeMatrix(drawState, drawState.merge->shape);

  auto dmatrix = pmatrix*smatrix;

  auto pbbox = toBBox(path.boundingRect());

  auto bbox = transformBBox(smatrix, pbbox);

  drawState.painter->setTransform(toQTransform(dmatrix));

  setPenBrush(drawState, drawState.merge->shape);

  drawState.painter->drawPath(path);

  const_cast<CLottieShape *>(drawState.merge->shape)->setBBox(bbox);

  //---

  if (isShowSelect() && drawState.merge->shape->isHierSelected()) {
    setSelectedPenBrush(drawState.painter);

    drawState.painter->drawPath(path);
  }

  //---

  drawState.painter->restore();

  if (drawState.layer)
    drawState.layer->setChanged(true);

  //---

}

#if 0
void
CQLottie::
drawLayerAssets(DrawState &drawState, const CLottieLayer *layer)
{
  CLottieAsset *asset = nullptr;

  if (layer->refId()) {
    asset = lottie_->getAssetById(*layer->refId());

    if (! asset)
      warnOnce(__LINE__, "Asset not found " + *layer->refId());
  }

  if (! asset)
    return;

  drawAsset(drawState, asset);
}
#endif

void
CQLottie::
drawAsset(const DrawState &drawState, CLottieAsset *asset)
{
  if (asset->layers().empty())
    return;

  //---

  auto drawState1 = drawState;

  drawState1.objects.push_front(asset);

  //---

  bool update = false;

  if (isDoubleBuffer()) {
    for (auto *layer : asset->layers()) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);

      qlayer->setDoubleBuffer(true);

      drawLayer(drawState1, layer, update);
    }

    for (auto it = asset->layers().rbegin(); it != asset->layers().rend(); ++it) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(*it);

      if (qlayer->isEnabled() && qlayer->isChanged()) {
        drawState.painter->drawImage(0, 0, qlayer->image());

        drawState.layer->setChanged(true);
      }
    }
  }
  else {
    for (auto it = asset->layers().rbegin(); it != asset->layers().rend(); ++it) {
      auto *qlayer = dynamic_cast<CQLottieLayer *>(*it);

      drawLayer(drawState1, qlayer, update);
    }
  }

  //---

  CBBox2D bbox;

  for (auto *layer : asset->layers())
    bbox += layer->bbox();

  const_cast<CLottieAsset *>(asset)->setBBox(bbox);

  //---

  if (isShowBBox() && asset->isHierSelected() && asset->bbox().isSet()) {
    auto displayMatrix = drawState.getDisplayMatrix();

    drawState.painter->setTransform(toQTransform(displayMatrix));

    setBBoxPenBrush(drawState.painter);

    drawState.painter->drawRect(toQRect(asset->bbox()));
  }
}

void
CQLottie::
drawPrecompLayer(const DrawState &drawState, const CLottieLayer *layer)
{
  //warnOnce(__LINE__, "Unhandled precomposition layer type");

  auto *precomp = layer->precomp();
  if (! precomp) return;

  CLottieAsset *asset = nullptr;

  if (precomp->refId) {
    auto refId = precomp->refId.value();

    asset = lottie_->getAssetById(refId);

    if (! asset)
      warnOnce(__LINE__, "Asset not found " + refId);
  }

  if (! asset)
    return;

  DrawState drawState1 = drawState;

  if (layer->frameIn())
    drawState1.frameDelta -= layer->frameIn().value();

#if 0
  const auto &parentDisplayRange = drawState1.displayRanges.back();

  double pxmin, pymin, pxmax, pymax;
  parentDisplayRange.getWindowRange(&pxmin, &pymin, &pxmax, &pymax);

  CDisplayRange2D displayRange;
  displayRange.setPixelRange(pxmin, pymax, pxmax, pymin);

  displayRange.setWindowRange(0, 0,
    precomp->width.value_or(100), precomp->height.value_or(100));

  drawState1.displayRanges.push_back(displayRange);
#endif

  drawAsset(drawState1, asset);

  auto bbox = asset->bbox();

#if 0
  auto m = getTransformMatrix(drawState, layer->transform());

  const_cast<CLottieLayer *>(layer)->setBBox(transformBBox(m, bbox));
#else
  const_cast<CLottieLayer *>(layer)->setBBox(bbox);
#endif
}

void
CQLottie::
drawSolidLayer(const DrawState &drawState, const CLottieLayer *layer)
{
  //warnOnce(__LINE__, "Unhandled shape layer type");

  auto *solid = layer->solid();
  if (! solid) return;

  // draw solid color
  auto w = solid->width .value_or(layer->width ().value_or(0));
  auto h = solid->height.value_or(layer->height().value_or(0));

  if (w <= 0 || h <= 0)
    return;

  auto p1 = CPoint2D(    0,     0);
  auto p2 = CPoint2D(w - 1, h - 1);

  auto bbox = CBBox2D(p1, p2);

  const_cast<CLottieLayer *>(layer)->setBBox(bbox);

  //---

  drawState.painter->save();

  auto pmatrix = drawState.getDisplayMatrix()*drawState.matrix;

  drawState.painter->setTransform(toQTransform(pmatrix));

  auto color = solid->color.value_or(CRGBA(0, 0, 0));

  drawState.painter->setPen  (toQColor(color));
  drawState.painter->setBrush(toQColor(color));

  if (layer->mask()) {
    auto *mask = layer->mask();

    CBezierPath bezierPath;
    pathToBezier(mask->path, drawState, bezierPath);

    auto ppath = toQPath(bezierPath);

    drawState.painter->setClipPath(ppath);
  }

  drawState.painter->drawRect(toQRect(bbox));

  drawState.painter->restore();

  if (drawState.layer)
    drawState.layer->setChanged(true);
}

void
CQLottie::
drawImageLayer(const DrawState &drawState, const CLottieLayer *layer)
{
  CLottieAsset *asset = nullptr;

  if (layer->refId()) {
    asset = lottie_->getAssetById(*layer->refId());

    if (! asset)
      warnOnce(__LINE__, "Asset not found " + *layer->refId());
  }

  if (! asset)
    return;

  auto pi = assetImage_.find(asset->id());

  if (pi == assetImage_.end()) {
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

    pi = assetImage_.insert(pi, AssetImage::value_type(asset->id(), image));
  }

  auto image = (*pi).second;
  if (image.isNull()) return;

  int w = asset->width ().value_or(100);
  int h = asset->height().value_or(100);

  auto p1 = CPoint2D(    0,     0);
  auto p2 = CPoint2D(w - 1, h - 1);

  auto bbox = CBBox2D(p1, p2);

  //---

  drawState.painter->save();

  auto pmatrix = drawState.getDisplayMatrix()*drawState.matrix;

  drawState.painter->setTransform(toQTransform(pmatrix));

  drawState.painter->drawImage(toQRect(bbox), image);

  drawState.painter->restore();

  if (drawState.layer)
    drawState.layer->setChanged(true);
}

void
CQLottie::
drawShape(DrawState &drawState, CLottieShape *shape)
{
  if (shape->hidden().value_or(false))
    return;

  //---

  auto type = shape->type().value_or("");

  auto unhandledShape = [&](const std::string &msg) {
    warnOnce(__LINE__, "Unhandled shape: " + msg + "(" + type + ")");
  };

  if      (type == "el") { // ellipse
    drawEllipse(drawState, shape);
  }
  else if (type == "fl") { // fill
    drawState.fill.shape = shape;

    drawState.fill.color   = getFillColor  (drawState, shape, drawState.fill.color);
    drawState.fill.opacity = getFillOpacity(drawState, shape, drawState.fill.opacity);
    drawState.fill.rule    = shape->fill()->fillRule.value_or(1);

    //unhandledShape("fill");
  }
  else if (type == "gf") { // gradient fill
    gradientFillShape(drawState, shape);
  }
  else if (type == "gs") { // gradient stroke
    gradientStrokeShape(drawState, shape);
  }
  else if (type == "gr") { // group
    //unhandledShape("gr : group");
  }
  else if (type == "sh") { // path
    drawPath(drawState, shape);
  }
  else if (type == "sr") { // polystar
    drawPolystar(drawState, shape);
  }
  else if (type == "rc") { // rectangle
    drawRectangle(drawState, shape);
  }
  else if (type == "st") { // stroke
    drawState.stroke.shape = shape;

    drawState.stroke.color   = getStrokeColor  (drawState, shape, drawState.stroke.color);
    drawState.stroke.opacity = getStrokeOpacity(drawState, shape, drawState.stroke.opacity);

    if (shape->stroke()) {
      drawState.stroke.width      = shape->stroke()->width.tvalue(drawState.timeFrame);
      drawState.stroke.lineCap    = shape->stroke()->lineCap;
      drawState.stroke.lineJoin   = shape->stroke()->lineJoin;
      drawState.stroke.miterLimit = shape->stroke()->miterLimit;
    }
  }
  else if (type == "tr") { // transform shape
    drawState.transform.shapes.push_back(shape);

    drawState.matrix = getShapeMatrix(drawState, shape);
  }
  else if (type == "tm") { // trim path
    //unhandledShape("trim path");

    if (shape->trim()) {
      DrawState::Trim trim;

      trim.shape  = shape;
      trim.start  = shape->trim()->start .tvalue(drawState.timeFrame,   0.0).value()/100.0;
      trim.end    = shape->trim()->end   .tvalue(drawState.timeFrame, 100.0).value()/100.0;
      trim.offset = shape->trim()->offset.tvalue(drawState.timeFrame,   0.0).value()/360.0;
      trim.mult   = shape->trim()->multiple.value_or(1);

      drawState.trim = trim;
    }
  }
  else if (type == "mm") { // merge path
    //unhandledShape("merge path");

    // 1 : Normal
    // 2 : Add
    // 3 : Subtract
    // 4 : Intersect
    // 5 : Exclude Intersections

    if (shape->merge() && shape->merge()->mode) {
      DrawState::Merge merge;

      merge.shape = shape;
      merge.mode  = shape->merge()->mode.value();

      drawState.merge = merge;
    }
  }
  else if (type == "rp") { // repeater
    //unhandledShape("repeater");

    if (shape->repeater()) {
#if 0
      auto *repeater = shape->repeater();

      DrawState::Repeat repeat;

      repeat.copies       = int(repeater->copies.tvalue(drawState.timeFrame, 1.0).value());
      repeat.offset       = repeater->offset.tvalue(drawState.timeFrame, 0.0).value();
      repeat.composite    = repeater->composite.value_or(0);
      repeat.transform    = repeater->transform;
      repeat.startOpacity = repeater->startOpacity.tvalue(drawState.timeFrame, 100.0).value();
      repeat.endOpacity   = repeater->endOpacity  .tvalue(drawState.timeFrame, 100.0).value();

#if 0
      std::cerr << "repeat.copies: " << repeat.copies << "\n";
      std::cerr << "repeat.offset: " << repeat.offset << "\n";
      std::cerr << "repeat.composite: " << repeat.composite << "\n";
      std::cerr << "repeat.matrix: " << getTransformMatrix(drawState, repeat.transform) << "\n";
      std::cerr << "repeat.startOpacity: " << repeat.startOpacity << "\n";
      std::cerr << "repeat.endOpacity: " << repeat.endOpacity << "\n";
#endif

      drawState.repeat = repeat;
#endif
    }
  }
  else {
    unhandledShape("???");
  }

  //---

  if (! shape->shapes().empty()) {
    bool isMerge = bool(drawState.merge);

    auto drawState1 = drawState;

    drawState1.objects.push_front(shape);

  //drawState1.merge.reset();
  //drawState1.trim .reset();

    //---

#if 1
    for (auto it = shape->shapes().rbegin(); it != shape->shapes().rend(); ++it)
      drawShape(drawState1, *it);
#else
    for (auto *shape : shape->shapes)
      drawShape(drawState1, shape);
#endif

    //---

    if (drawState1.merge) {
      if (! isMerge)
        drawMergeShapes(drawState1);
      else {
        for (const auto &bezierPath : drawState1.merge->paths)
          drawState.merge->paths.push_back(bezierPath);
      }
    }

    //---

    auto bbox = shape->bbox();

    for (auto *shape1 : shape->shapes())
      bbox += shape1->bbox();

    const_cast<CLottieShape *>(shape)->setBBox(bbox);
  }
}

void
CQLottie::
gradientFillShape(DrawState &drawState, const CLottieShape *shape)
{
  //unhandledShape("gradient fill");

  auto *gradientFill = shape->gradientFill();
  if (! gradientFill) return;

  auto startPoint = gradientFill->startPoint.tvalue(drawState.timeFrame, CPoint2D(0, 0)).value();
  auto endPoint   = gradientFill->endPoint  .tvalue(drawState.timeFrame, CPoint2D(0, 0)).value();

//std::cerr << startPoint << " " << endPoint << "\n";

  auto colors = gradientFill->colors.tvalue(drawState.timeFrame);

//for (const auto &c : colors->vals)
//  std::cerr << c << "\n";

  auto nc = colors->vals.size();

  drawState.fillGradient.enabled = true;

  auto &gradient = drawState.fillGradient.gradient;

  gradient.setStart(startPoint.x, startPoint.y);
  gradient.setFinalStop(endPoint.x, endPoint.y);

  for (size_t i = 0; i < nc/4; ++i) {
    auto c = CRGBA(colors->vals[4*i + 1], colors->vals[4*i + 2], colors->vals[4*i + 3]);

    gradient.setColorAt(colors->vals[4*i], toQColor(c));
  }
}

void
CQLottie::
gradientStrokeShape(DrawState &drawState, const CLottieShape *shape)
{
  //unhandledShape("gradient stroke");

  auto *gradientStroke = shape->gradientStroke();
  if (! gradientStroke) return;

  auto startPoint = gradientStroke->startPoint.tvalue(drawState.timeFrame, CPoint2D(0, 0)).value();
  auto endPoint   = gradientStroke->endPoint  .tvalue(drawState.timeFrame, CPoint2D(0, 0)).value();

//std::cerr << startPoint << " " << endPoint << "\n";

  auto colors = gradientStroke->colors.tvalue(drawState.timeFrame);

//for (const auto &c : colors->vals)
//  std::cerr << c << "\n";

  auto nc = colors->vals.size();

  drawState.strokeGradient.enabled = true;

  auto &gradient = drawState.strokeGradient.gradient;

  gradient.setStart(startPoint.x, startPoint.y);
  gradient.setFinalStop(endPoint.x, endPoint.y);

  for (size_t i = 0; i < nc/4; ++i) {
    auto c = CRGBA(colors->vals[4*i + 1], colors->vals[4*i + 2], colors->vals[4*i + 3]);

    gradient.setColorAt(colors->vals[4*i], toQColor(c));
  }

  drawState.strokeGradient.opacity = gradientStroke->opacity.tvalue(drawState.timeFrame, 100.0);

  drawState.strokeGradient.width      = gradientStroke->width.tvalue(drawState.timeFrame);
  drawState.strokeGradient.lineCap    = gradientStroke->lineCap;
  drawState.strokeGradient.lineJoin   = gradientStroke->lineJoin;
  drawState.strokeGradient.miterLimit = gradientStroke->miterLimit;
}

void
CQLottie::
drawEllipse(DrawState &drawState, const CLottieShape *shape)
{
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

  drawBezierPath(drawState, shape, bezierPath);
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
  if (drawState.trim) {
    auto offset = drawState.trim->offset;

    auto start = drawState.trim->start + offset;
    auto end   = drawState.trim->end   + offset;

    if (start < 0) start += 1.0;
    if (start > 1) start -= 1.0;
    if (end   < 0) end   += 1.0;
    if (end   > 1) end   -= 1.0;

    if (start > end) {
      start = 1.0 - start;
      end   = 1.0 - end;
    }

    bezierPath = bezierPath.split(start, end);
  }

  //---

#if 1
  auto *merge = shape->calcHierMerge();

  if (merge) {
    if (drawState.merge)
      drawState.merge->paths.push_back(bezierPath);

    return;
  }
#else
  if (drawState.merge) {
    //drawState.merge->shape = shape;

    drawState.merge->paths.push_back(bezierPath);

    return;
  }
#endif

  //---

  auto path = toQPath(bezierPath);

  if (drawState.fill.rule == 2)
    path.setFillRule(Qt::OddEvenFill);
  else
    path.setFillRule(Qt::WindingFill);

  //---

  drawState.painter->save();

  //---

  auto pmatrix = drawState.getDisplayMatrix();
  auto smatrix = getShapeMatrix(drawState, shape);

  auto dmatrix = pmatrix*smatrix;

  drawState.painter->setTransform(toQTransform(dmatrix));

  setPenBrush(drawState, shape);

  if (drawState.stroker) {
    auto lpath = drawState.stroker->createStroke(path);

    lpath.setFillRule(Qt::WindingFill);

    drawState.painter->drawPath(lpath);
  }
  else
    drawState.painter->drawPath(path);

  //---

  auto bbox = transformBBox(smatrix, bezierPath.bbox());

  const_cast<CLottieShape *>(shape)->setBBox(bbox);

  //---

  if (isShowSelect() && shape->isHierSelected()) {
    setSelectedPenBrush(drawState.painter);

    drawState.painter->drawPath(path);
  }

  if (isShowBBox() && shape->isHierSelected() && shape->bbox().isSet()) {
    auto displayMatrix = pmatrix;

    drawState.painter->setTransform(toQTransform(displayMatrix));

    setBBoxPenBrush(drawState.painter);

    drawState.painter->drawRect(toQRect(shape->bbox()));
  }

  //---

  drawState.painter->restore();

  if (drawState.layer)
    drawState.layer->setChanged(true);
}

void
CQLottie::
pathToBezier(const CLottie::BezierProperty &path, const DrawState &drawState,
             CBezierPath &bezierPath) const
{
  auto points  = path.tvvalue(drawState.timeFrame, CLottie::PointList())->points;
  auto ipoints = path.tivalue(drawState.timeFrame, CLottie::PointList())->points;
  auto opoints = path.tovalue(drawState.timeFrame, CLottie::PointList())->points;
  auto closed  = path.tclosed(drawState.timeFrame);

  //---

  auto n = points.size();
  if (n == 0) return;

  if (ipoints.size() != n || opoints.size() != n) {
    errOnce(__LINE__, "Path i/o points mismatch\n");
    return;
  }

//QPainterPath ppath;

  auto p1 = points[0];

//ppath.moveTo(toQPoint(p1));
  bezierPath.moveTo(p1);

  for (size_t i = 1; i < n; ++i) {
    auto p2 = points[i - 1] + opoints[i - 1];
    auto p3 = points[i    ] + ipoints[i    ];
    auto p4 = points[i    ];

//  ppath.cubicTo(toQPoint(p2), toQPoint(p3), toQPoint(p4));
    bezierPath.cubicTo(p2, p3, p4);
  }

  if (closed) {
    auto p2 = points[n - 1] + opoints[n - 1];
    auto p3 = points[0    ] + ipoints[0    ];
    auto p4 = points[0    ];

//  ppath.cubicTo(toQPoint(p2), toQPoint(p3), toQPoint(p4));
    bezierPath.cubicTo(p2, p3, p4);

//  ppath.closeSubpath();
    bezierPath.setClosed(true);
  }
}

void
CQLottie::
drawPolystar(DrawState &drawState, const CLottieShape *shape)
{
//unhandledShape("polystar");

  auto *polyStar = shape->polyStar();
  if (! polyStar) return;

  auto positionxy = polyStar->position.tvalue(drawState.timeFrame, CLottie::XYVals()).value();
  auto position   = positionxy.toPoint(CPoint2D(0, 0));

  auto type = polyStar->type.value_or(1);

  if (type != 1 && type != 2)
    warnOnce(__LINE__, "unhandled polystar type: " + std::to_string(type));

  auto numPoints = int(polyStar->points.tvalue(drawState.timeFrame, 0).value_or(0));

  auto innerRadius    = polyStar->innerRadius   .tvalue(drawState.timeFrame, 0).value_or(0);
  auto innerRoundness = polyStar->outerRoundness.tvalue(drawState.timeFrame, 0).value_or(0);
  auto outerRadius    = polyStar->outerRadius   .tvalue(drawState.timeFrame, 0).value_or(0);
  auto outerRoundness = polyStar->outerRoundness.tvalue(drawState.timeFrame, 0).value_or(0);

  auto rotation = CMathGen::DegToRad(polyStar->rotation.tvalue(drawState.timeFrame, 0).value_or(0));

  CBezierPath bezierPath;

  if (type == 1)
    bezierPath.addPolyStar(position, numPoints, innerRadius, outerRadius,
                           innerRoundness/100.0, outerRoundness/100.0, rotation);
  else
    bezierPath.addPolygon(position, numPoints, outerRadius,
                          outerRoundness/100.0, rotation);

  //---

  drawBezierPath(drawState, shape, bezierPath);
}

void
CQLottie::
drawRectangle(DrawState &drawState, const CLottieShape *shape)
{
  auto positionxy = shape->pos().tvalue(drawState.timeFrame, CLottie::XYVals()).value();
  auto position   = positionxy.toPoint(CPoint2D(0, 0));

  auto sizexy = shape->size().tvalue(drawState.timeFrame, CLottie::XYVals()).value();
  auto size   = sizexy.toPoint(CPoint2D(0, 0));

  auto p1 = CPoint2D(position.x - size.x/2.0, position.y - size.y/2.0);
  auto p2 = CPoint2D(position.x + size.x/2.0, position.y + size.y/2.0);

  auto bbox = CBBox2D(p1, p2);

  //---

  CBezierPath bezierPath;

  if (shape->rectangle()) {
    auto r = shape->rectangle()->roundness.tvalue(drawState.timeFrame, 0.0).value();

    bezierPath.addRoundedRect(bbox, r, r);
  }
  else
    bezierPath.addRect(bbox);

  drawBezierPath(drawState, shape, bezierPath);
}

void
CQLottie::
setPenBrush(DrawState &drawState, const CLottieShape *shape)
{
  if (drawState.fillGradient.enabled) {
    drawState.painter->setBrush(drawState.fillGradient.gradient);
  }
  else {
    auto fillColor   = getFillColor  (drawState, shape, drawState.fill.color);
    auto fillOpacity = getFillOpacity(drawState, shape, drawState.fill.opacity);

    if (fillColor) {
      auto c = toQColor(fillColor.value());

      if (fillOpacity)
        c.setAlpha(int(255*(fillOpacity.value()/100.0)));

      drawState.painter->setBrush(c);
    }
    else
      drawState.painter->setBrush(Qt::NoBrush);
  }

  //---

  QPen pen;

  if (drawState.strokeGradient.enabled) {
    drawState.stroker = new QPainterPathStroker();

    if (drawState.strokeGradient.lineCap)
      drawState.stroker->setCapStyle(toLineCap(drawState.strokeGradient.lineCap.value()));
    if (drawState.strokeGradient.lineJoin)
      drawState.stroker->setJoinStyle(toLineJoin(drawState.strokeGradient.lineJoin.value()));

    if (drawState.strokeGradient.miterLimit)
      drawState.stroker->setMiterLimit(drawState.strokeGradient.miterLimit.value());

    drawState.stroker->setDashOffset (0.0);
    drawState.stroker->setDashPattern(Qt::SolidLine);

    if (drawState.strokeGradient.width)
      drawState.stroker->setWidth(drawState.strokeGradient.width.value());

    drawState.painter->setBrush(drawState.strokeGradient.gradient);

    pen.setStyle(Qt::NoPen);
  }
  else {
    auto strokeColor   = getStrokeColor  (drawState, shape, drawState.stroke.color);
    auto strokeOpacity = getStrokeOpacity(drawState, shape, drawState.stroke.opacity);

    if (strokeColor) {
      auto c = toQColor(*strokeColor);

      if (strokeOpacity)
        c.setAlpha(int(255*(strokeOpacity.value()/100.0)));

      pen.setColor(c);
    }
    else {
      pen.setStyle(Qt::NoPen);
      //pen.setColor(Qt::black);
    }

    if (drawState.stroke.width)
      pen.setWidth(drawState.stroke.width.value());

    if (drawState.stroke.lineCap)
      pen.setCapStyle(toLineCap(drawState.stroke.lineCap.value()));

    if (drawState.stroke.lineJoin)
      pen.setJoinStyle(toLineJoin(drawState.stroke.lineJoin.value()));

    if (drawState.stroke.miterLimit)
      pen.setMiterLimit(drawState.stroke.miterLimit.value());
  }

  drawState.painter->setPen(pen);
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
  pen.setWidth(3);
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

  CLottieObject *insideObject = nullptr;

  for (auto *shape : lottie_->shapes()) {
    const auto &bbox = shape->bbox();

    if (! bbox.inside(p))
      continue;

    if (insideObject) {
      if (shape->bbox().area() < insideObject->bbox().area())
        insideObject = shape;
    }
    else
      insideObject = shape;
  }

  if (insideObject) {
//  std::cerr << insideObject->name().value_or("") << "\n";

    lottie_->deselectAll();

    insideObject->setSelected(true);

    tree_->selectObject(insideObject);

//  objectTree()->setObject(insideObject);

    canvas()->invalidate();
  }
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

CMatrix2D
CQLottie::
getLayerMatrix(const DrawState &drawState, const CLottieLayer *layer) const
{
#if 0
  auto m = lottie_->getTransformMatrix(drawState.timeFrame, layer);
#else
  auto m = getTransformMatrix(drawState, layer->transform());
#endif

  auto *player = layer->getParentLayer();

  if (player)
    m = getLayerMatrix(drawState, player)*m;

  return m;
}

CMatrix2D
CQLottie::
getShapeMatrix(const DrawState &drawState, const CLottieShape *shape) const
{
#if 0
  auto m = lottie_->getTransformMatrix(drawState.timeFrame, shape);
#else
  auto m = getTransformMatrix(drawState, shape->transform());
#endif

  auto *shape1 = shape;

  while (shape1->getParentShape()) {
    auto *pshape = shape1->getParentShape();

    auto *transformShape = pshape->getTransformShape();

    if (transformShape)
      m = getTransformMatrix(drawState, transformShape->transform())*m;
    else
      m = getTransformMatrix(drawState, pshape->transform())*m;

    shape1 = pshape;
  }

#if 0
  auto *player = shape->getParentLayer();

  if (player)
    m = getLayerMatrix(drawState, player)*m;
#endif

  for (auto *obj : drawState.objects) {
    auto *layer = dynamic_cast<CQLottieLayer *>(obj);
    if (! layer) continue;

    auto *repeater = layer->calcRepeater();

    if (repeater) {
      auto repeatMatrix = calcRepeatMatrix(drawState, repeater);
      //assert(repeatMatrix == drawState.repeatMatrix);

      //std::cerr << "Repeater for layer " << layer->name().value_or("") << "\n";
      m = repeatMatrix*m;
    }

    m = getTransformMatrix(drawState, layer->transform())*m;
  }

  return m;
}

CMatrix2D
CQLottie::
calcRepeatMatrix(const DrawState &drawState, CLottieRepeater *repeater) const
{
  auto repeatOffset = repeater->offset.tvalue(drawState.timeFrame, 0.0).value();

  double mult = drawState.repeatInd.value_or(0) + repeatOffset;

  return lottie_->getRepeaterMatrix(drawState.timeFrame, repeater->transform, mult);
}

CMatrix2D
CQLottie::
getTransformMatrix(const DrawState &drawState, CLottie::Transform *transform) const
{
  return lottie_->getTransformMatrix(drawState.timeFrame, transform);
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
getFillColor(const DrawState &drawState, const CLottieShape *shape, const OptColor &def) const
{
  OptColor c;

  if      (shape->fill())
    c = shape->fill()->color.tvalue(drawState.timeFrame);
  else if (drawState.fill.shape && drawState.fill.shape->fill())
    c = drawState.fill.shape->fill()->color.tvalue(drawState.timeFrame);

  if (! c)
    c = shape->color().tvalue(drawState.timeFrame);

  if (! c) {
    auto *pshape = shape->getParentShape();

    if (pshape)
      c = getFillColor(drawState, pshape);
  }

  return (c ? c : def);
}

CQLottie::OptReal
CQLottie::
getFillOpacity(const DrawState &drawState, const CLottieShape *shape, const OptReal &def) const
{
  OptReal o;

  if      (shape->fill())
    o = shape->fill()->opacity.tvalue(drawState.timeFrame);
  else if (drawState.fill.shape && drawState.fill.shape->fill())
    o = drawState.fill.shape->fill()->opacity.tvalue(drawState.timeFrame);
  else if (shape->transform())
    o = shape->transform()->opacity.tvalue(drawState.timeFrame);

  auto *pshape = shape->getParentShape();

  if (! o && pshape)
    return getFillOpacity(drawState, pshape, def);

  auto *player = shape->getHierParentLayer();
  assert(player);

  auto *repeater = player->calcRepeater();

  if (repeater) {
    auto repeatOpacity = getRepeatOpacity(drawState, repeater);

    o = combineOpacities(o, repeatOpacity);
  }

  auto o1 = getLayerOpacity(drawState, player, def);

  if (o1)
    o = combineOpacities(o, o1);

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

  auto *player = layer->getParentLayer();

  if (player) {
    auto o1 = getLayerOpacity(drawState, player, def);

    if (o1)
      o = combineOpacities(o, o1);
  }

  return (o ? o : def);
}

CQLottie::OptColor
CQLottie::
getStrokeColor(const DrawState &drawState, const CLottieShape *shape, const OptColor &def) const
{
  OptColor c;

  if (shape->stroke())
    c = shape->stroke()->color.tvalue(drawState.timeFrame);

  if (! c)
    c = shape->color().tvalue(drawState.timeFrame);

  if (! c) {
    auto *pshape = shape->getParentShape();

    if (pshape)
      c = getStrokeColor(drawState, pshape);
  }

  return (c ? c : def);
}

CQLottie::OptReal
CQLottie::
getStrokeOpacity(const DrawState &drawState, const CLottieShape *shape, const OptReal &def) const
{
  OptReal o;

  if (shape->stroke())
    o = shape->stroke()->opacity.tvalue(drawState.timeFrame, 100.0).value();

  auto *pshape = shape->getParentShape();

  if (! o && pshape)
    return getStrokeOpacity(drawState, pshape, def);

  auto *player = shape->getHierParentLayer();
  assert(player);

  auto *repeater = player->calcRepeater();

  if (repeater) {
    auto repeatOpacity = getRepeatOpacity(drawState, repeater);

    o = combineOpacities(o, repeatOpacity);
  }

  auto o1 = getLayerOpacity(drawState, player, def);

  if (o1)
    o = combineOpacities(o, o1);

  return (o ? o : def);
}

double
CQLottie::
getRepeatOpacity(const DrawState &drawState, CLottieRepeater *repeater) const
{
  auto repeatStartOpacity = repeater->startOpacity.tvalue(drawState.timeFrame, 100.0).value();
  auto repeatEndOpacity   = repeater->endOpacity  .tvalue(drawState.timeFrame, 100.0).value();

  auto repeatCopies = int(repeater->copies.tvalue(drawState.timeFrame, 1.0).value());

  return CMathUtil::map(drawState.repeatInd.value_or(0),
                        0, repeatCopies - 1, repeatStartOpacity, repeatEndOpacity);
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
}

void
CQLottie::
zoomFull()
{
  displayRange_.reset();
}

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

//---

CQLottieLayer::
CQLottieLayer(CQLottie *lottie) :
 CLottieLayer(lottie->lottie()), lottie_(lottie)
{
}

CQLottieLayer::
~CQLottieLayer()
{
  delete painter_;
}

void
CQLottieLayer::
resize(int w, int h)
{
  if (w != w_ || h != h_) {
    w_ = w;
    h_ = h;

    delete painter_;

    painter_ = nullptr;

    image_ = QImage(w_, h_, QImage::Format_ARGB32);
  }

  clear();
}

QPainter *
CQLottieLayer::
painter()
{
  if (! painter_)
    painter_ = new QPainter(&image_);

  return painter_;
}

void
CQLottieLayer::
clear()
{
  image_.fill(0);
}
