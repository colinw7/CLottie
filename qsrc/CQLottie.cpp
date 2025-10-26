#include <CQLottie.h>
#include <CLottie.h>
#include <CEncode64.h>
#include <CBezierPath.h>
#include <CArcToBezier.h>

#include <CQIconButton.h>
#include <CQColorChooser.h>
#include <CQRealSpin.h>

#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QMenu>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QTimer>

#include <iostream>
#include <set>
#include <cassert>
#include <cmath>

#include <svg/play_svg.h>
#include <svg/pause_svg.h>
#include <svg/play_one_svg.h>

//---

namespace {

QPointF toQPoint(const CPoint2D &point) {
  return QPointF(point.x, point.y);
}

QRectF toQRect(const CBBox2D &rect) {
  return QRectF(toQPoint(rect.getLL()), toQPoint(rect.getUR())).normalized();
}

QColor toQColor(const CRGBA &color) {
  return QColor(color.getRedI(), color.getGreenI(), color.getBlueI(), color.getAlphaI());
}

CRGBA toRGBA(const QColor &color) {
  return CRGBA(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

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

    if (first)
      path.moveTo(toQPoint(p1));

    path.cubicTo(toQPoint(p2), toQPoint(p3), toQPoint(p4));

    first = false;
  }

  if (bezierPath.isClosed())
    path.closeSubpath();

  return path;
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

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  std::string filename;
  bool        debug = false;
  bool        print = false;

  for (auto i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      auto arg = std::string(&argv[i][1]);

      if      (arg == "debug")
        debug = true;
      else if (arg == "print")
        print = true;
      else
        std::cerr << "Unhandled option: " << arg << "\n";
    }
    else
      filename = argv[i];
  }

  if (filename == "")
    exit(1);

  auto *lottie = new CQLottie;

  lottie->setDebug(debug);
  lottie->setPrint(print);

  if (! lottie->load(filename)) {
    std::cerr << "Failed to load '" << filename << "'\n";
    exit(1);
  }

  lottie->show();

  return app.exec();
}

//---

CQLottie::
CQLottie()
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

  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  toolbar_ = new QFrame;
  toolbar_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  layout->addWidget(toolbar_);

  auto *tlayout = new QHBoxLayout(toolbar_);

  auto *playButton  = addToolButton("play" , "PLAY"    , "Play" , SLOT(playSlot()));
  auto *pauseButton = addToolButton("pause", "PAUSE"   , "Pause", SLOT(pauseSlot()));
  auto *stepButton  = addToolButton("step" , "PLAY_ONE", "Step" , SLOT(stepSlot()));

  tlayout->addWidget(playButton);
  tlayout->addWidget(pauseButton);
  tlayout->addWidget(stepButton);

  auto *loadButton = new QPushButton("Load");

  connect(loadButton, SIGNAL(clicked()), this, SLOT(loadSlot()));

  tlayout->addStretch(1);
  tlayout->addWidget(loadButton);

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
  controlLayout->setMargin(0); clayout->setSpacing(0);

  clayout->addWidget(controlFrame);

  tree_ = new CQLottieTree(this);

  controlLayout->addWidget(tree_);

  objectTree_ = new CQLottieObjectTree(this);

  controlLayout->addWidget(objectTree_);

  //---

  status_ = new QFrame;
  status_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  layout->addWidget(status_);

  auto *slayout = new QHBoxLayout(status_);

  ticksLabel_  = new QLabel(" ");
  statusLabel_ = new QLabel(" ");

  slayout->addWidget(statusLabel_);
  slayout->addStretch(1);
  slayout->addWidget(ticksLabel_);

  //---

  lottie_ = new CLottie;

  //---

  timer_ = new QTimer;

  connect(timer_, SIGNAL(timeout()), this, SLOT(tickSlot()));
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

//displayRange_.setEqualScale(true);
  displayRange_.setWindowRange(0, 0, w, h);

  fps_ = std::max(root->frameRate(), 1.0);

  dt_ = 1000.0/fps_;

  timer_->start(int(dt_));

  //---

  tree_->load();

  return true;
}

void
CQLottie::
setPixelSize(int w, int h)
{
  displayRange_.setPixelRange(0, h - 1, w - 1, 0);
}

void
CQLottie::
loadSlot()
{
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
    ticksLabel_->setText(QString("Frame: %1 (%2 secs)").arg(ticks_).arg(secs_));
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

  ticksLabel_->setText(QString("Frame: %1 (%2 secs)").arg(ticks_).arg(secs_));

  update();
}

void
CQLottie::
draw(QPainter *painter)
{
  const auto *root = lottie_->root();

  drawRoot(painter, root);
}

void
CQLottie::
drawRoot(QPainter *painter, const CLottieRoot *root)
{
  if (root->hidden().value_or(false))
    return;

  DrawState drawState;

  getTimeFrame(drawState.timeFrame);

#if 1
  for (auto it = root->layers().rbegin(); it != root->layers().rend(); ++it)
    drawLayer(painter, drawState, *it);
#else
  for (auto *layer : root->layers())
    drawLayer(painter, drawState, layer);
#endif
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
drawLayer(QPainter *painter, const DrawState &drawState, const CLottieLayer *layer)
{
  if (layer->hidden().value_or(false))
    return;

  if (layer->frameIn() && int(drawState.timeFrame.frame) < layer->frameIn().value())
    return;

  if (layer->frameOut() && int(drawState.timeFrame.frame) > layer->frameOut().value())
    return;

  //---

  auto drawState1 = drawState;

  drawState1.matrix = drawState.preMatrix*getLayerMatrix(drawState, layer);

  //---

  auto typeId = layer->typeId().value_or(-1);

  if      (typeId == 0) { // Precomposition Layer
    drawPrecompLayer(painter, drawState1, layer);
  }
  else if (typeId == 1) { // Solid Layer
    drawSolidLayer(painter, drawState1, layer);
  }
  else if (typeId == 2) { // Image Layer
    drawImageLayer(painter, drawState1, layer);
  }
  else if (typeId == 3) { // Null Layer
  }
  else if (typeId == 4) { // Shape Layer
  }
  else {
    warnOnce(__LINE__, "Invalid layer type id: " + std::to_string(typeId));
  }

  drawLayerShapes(painter, drawState1, layer);

#if 0
  drawLayerAssets(painter, drawState1, layer);
#endif
}

void
CQLottie::
drawLayerShapes(QPainter *painter, const DrawState &drawState, const CLottieLayer *layer)
{
  auto drawState1 = drawState;

  drawState1.merge.reset();
//drawState1.trim .reset();

#if 1
  for (auto it = layer->shapes().rbegin(); it != layer->shapes().rend(); ++it) {
    if (drawState1.repeat) {
      auto drawState2 = drawState1;

      auto matrix = getTransformMatrix(drawState, drawState1.repeat->transform);

      for (int i = 0; i < drawState1.repeat->copies; ++i) {
        drawState2.preMatrix = matrix*drawState2.preMatrix;

        drawShape(painter, drawState2, *it);
      }
    }
    else
      drawShape(painter, drawState1, *it);
  }
#else
  for (auto *shape : layer->shapes) {
    drawShape(painter, drawState1, shape);
  }
#endif

  //---

  drawMergeShapes(painter, drawState1);
}

void
CQLottie::
drawMergeShapes(QPainter *painter, const DrawState &drawState)
{
  if (! drawState.merge)
    return;

  int np = drawState.merge->paths.size();
  if (np <= 0) return;

  //---

  auto path = drawState.merge->paths[0];

  for (int i = 1; i < np; ++i) {
    if      (drawState.merge->mode == 0) {
    }
    else if (drawState.merge->mode == 1) {
      path = path.united(drawState.merge->paths[i]);
    }
    else if (drawState.merge->mode == 2) {
      path = path.subtracted(drawState.merge->paths[i]);
    }
    else if (drawState.merge->mode == 3) {
      path = path.intersected(drawState.merge->paths[i]);
    }
    else if (drawState.merge->mode == 4) {
      std::cerr << "merge mode 4 unimplemented\n";
    }
    else {
      std::cerr << "invalid merge mode " << drawState.merge->mode << "\n";
    }
  }

  //---

  if (drawState.fill.rule == 2)
    path.setFillRule(Qt::OddEvenFill);
  else
    path.setFillRule(Qt::WindingFill);

  //---

  painter->save();

  //---

  auto pmatrix = displayRange_.getMatrix()*drawState.matrix;

  painter->setTransform(toQTransform(pmatrix));

  setPenBrush(painter, drawState, drawState.merge->shape);

  painter->drawPath(path);

  //---

  if (drawState.merge->shape->selected()) {
    setSelectedPenBrush(painter);

    painter->drawPath(path);
  }

  //---

  painter->restore();
}

#if 0
void
CQLottie::
drawLayerAssets(QPainter *painter, DrawState &drawState, const CLottieLayer *layer)
{
  CLottieAsset *asset = nullptr;

  if (layer->refId()) {
    asset = lottie_->getAssetById(*layer->refId());

    if (! asset)
      warnOnce(__LINE__, "Asset not found " + *layer->refId());
  }

  if (! asset)
    return;

  drawAsset(painter, drawState, asset);
}
#endif

void
CQLottie::
drawAsset(QPainter *painter, const DrawState &drawState, const CLottieAsset *asset)
{
  if (asset->layers().empty())
    return;

  auto drawState1 = drawState;

  drawState1.preMatrix = drawState.matrix;

#if 1
  for (auto it = asset->layers().rbegin(); it != asset->layers().rend(); ++it)
    drawLayer(painter, drawState1, *it);
#else
  for (auto *layer : asset->layers())
    drawLayer(painter, drawState1, layer);
#endif
}

void
CQLottie::
drawPrecompLayer(QPainter *painter, const DrawState &drawState, const CLottieLayer *layer)
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

  drawState1.preMatrix = getLayerMatrix(drawState, layer);

  drawAsset(painter, drawState1, asset);
}

void
CQLottie::
drawSolidLayer(QPainter *painter, const DrawState &drawState, const CLottieLayer *layer)
{
  //warnOnce(__LINE__, "Unhandled shape layer type");

  auto *solid = layer->solid();
  if (! solid) return;

  // draw solid color
  auto w = solid->width .value_or(layer->width ().value_or(0));
  auto h = solid->height.value_or(layer->height().value_or(0));

  if (w > 0 && h > 0) {
    auto p1 = CPoint2D(    0,     0);
    auto p2 = CPoint2D(w - 1, h - 1);

    auto bbox = CBBox2D(p1, p2);

    //---

    painter->save();

    auto pmatrix = displayRange_.getMatrix()*drawState.matrix;

    painter->setTransform(toQTransform(pmatrix));

    auto color = solid->color.value_or(CRGBA(0, 0, 0));

    painter->setPen  (toQColor(color));
    painter->setBrush(toQColor(color));

    if (layer->mask()) {
      auto *mask = layer->mask();

      CBezierPath bezierPath;
      pathToBezier(mask->path, drawState, bezierPath);

      auto ppath = toQPath(bezierPath);

      painter->setClipPath(ppath);
    }

    painter->drawRect(toQRect(bbox));

    painter->restore();
  }
}

void
CQLottie::
drawImageLayer(QPainter *painter, const DrawState &drawState, const CLottieLayer *layer)
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

  if (! image.isNull()) {
    int w = asset->width ().value_or(100);
    int h = asset->height().value_or(100);

    auto p1 = CPoint2D(    0,     0);
    auto p2 = CPoint2D(w - 1, h - 1);

    auto bbox = CBBox2D(p1, p2);

    //---

    painter->save();

    auto pmatrix = displayRange_.getMatrix()*drawState.matrix;

    painter->setTransform(toQTransform(pmatrix));

    painter->drawImage(toQRect(bbox), image);

    painter->restore();
  }
}

void
CQLottie::
drawShape(QPainter *painter, DrawState &drawState, const CLottieShape *shape)
{
  if (shape->hidden().value_or(false))
    return;

  auto type = shape->type();

  auto unhandledShape = [&](const std::string &msg) {
    warnOnce(__LINE__, "Unhandled shape: " + msg + "(" + type + ")");
  };

  if      (type == "el") { // ellipse
    drawEllipse(painter, drawState, shape);
  }
  else if (type == "fl") { // fill
    drawState.fill.shape = shape;

    drawState.fill.color   = getFillColor  (drawState, shape, drawState.fill.color);
    drawState.fill.opacity = getFillOpacity(drawState, shape, drawState.fill.opacity);
    drawState.fill.rule    = shape->fill()->fillRule.value_or(1);

    //unhandledShape("fill");
  }
  else if (type == "gf") { // gradient fill
    gradientFill(drawState, shape);
  }
  else if (type == "gs") { // gradient stroke
    gradientStroke(drawState, shape);
  }
  else if (type == "gr") { // group
    //unhandledShape("gr : group");
  }
  else if (type == "sh") { // path
    drawPath(painter, drawState, shape);
  }
  else if (type == "sr") { // polystar
    drawPolystar(painter, drawState, shape);
  }
  else if (type == "rc") { // rectangle
    drawRectangle(painter, drawState, shape);
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

    drawState.matrix = drawState.preMatrix*getShapeMatrix(drawState, shape);
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

      merge.mode = shape->merge()->mode.value();

      drawState.merge = merge;
    }
  }
  else if (type == "rp") { // repeater
    //unhandledShape("repeater");

    if (shape->repeater()) {
      auto *repeater = shape->repeater();

      DrawState::Repeat repeat;

      repeat.copies       = int(repeater->copies.tvalue(drawState.timeFrame, 1.0).value());
      repeat.offset       = repeater->offset.tvalue(drawState.timeFrame, 0.0).value();
      repeat.composite    = repeater->composite.value_or(0);
      repeat.transform    = repeater->transform;
      repeat.startOpacity = repeater->startOpacity.tvalue(drawState.timeFrame, 100.0).value()/100.0;
      repeat.endOpacity   = repeater->endOpacity  .tvalue(drawState.timeFrame, 100.0).value()/100.0;

#if 0
      auto matrix = getTransformMatrix(drawState, repeat.transform);

      std::cerr << "copies: " << repeat.copies << "\n";
      std::cerr << "offset: " << repeat.offset << "\n";
      std::cerr << "composite: " << repeat.composite << "\n";
      std::cerr << "matrix: " << matrix << "\n";
#endif

      drawState.repeat = repeat;
    }
  }
  else {
    unhandledShape("???");
  }

  //---

  auto drawState1 = drawState;

  drawState1.merge.reset();
//drawState1.trim .reset();

  //---

#if 1
  for (auto it = shape->shapes().rbegin(); it != shape->shapes().rend(); ++it)
    drawShape(painter, drawState1, *it);
#else
  for (auto *shape : shape->shapes)
    drawShape(painter, drawState1, shape);
#endif

  //---

  drawMergeShapes(painter, drawState1);
}

void
CQLottie::
gradientFill(DrawState &drawState, const CLottieShape *shape)
{
  //unhandledShape("gradient fill");

  auto *gradientFill = shape->gradientFill();
  if (! gradientFill) return;

  auto startPoint = gradientFill->startPoint.tvalue(drawState.timeFrame, CPoint2D(0, 0));
  auto endPoint   = gradientFill->endPoint  .tvalue(drawState.timeFrame, CPoint2D(0, 0));

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
gradientStroke(DrawState &drawState, const CLottieShape *shape)
{
  //unhandledShape("gradient stroke");

  auto *gradientStroke = shape->gradientStroke();
  if (! gradientStroke) return;

  auto startPoint = gradientStroke->startPoint.tvalue(drawState.timeFrame, CPoint2D(0, 0));
  auto endPoint   = gradientStroke->endPoint  .tvalue(drawState.timeFrame, CPoint2D(0, 0));

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
drawEllipse(QPainter *painter, DrawState &drawState, const CLottieShape *shape)
{
  auto positionxy = shape->pos().tvalue(drawState.timeFrame, CLottie::XYVals());
  auto position   = positionxy.toPoint(CPoint2D(0, 0));

  auto sizexy = shape->size().tvalue(drawState.timeFrame, CLottie::XYVals());
  auto size   = sizexy.toPoint(CPoint2D(0, 0));

  //---

#if 0
  auto p1 = CPoint2D(position.x - size.x/2.0, position.y - size.y/2.0);
  auto p2 = CPoint2D(position.x + size.x/2.0, position.y + size.y/2.0);

  CBBox2D bbox(p1, p2);

  //---

  painter->save();

  //---

  auto pmatrix = displayRange_.getMatrix()*drawState.matrix;

  painter->setTransform(toQTransform(pmatrix));

  setPenBrush(painter, drawState, shape);

  painter->drawEllipse(toQRect(bbox));

  //---

  if (shape->selected()) {
    setSelectedPenBrush(painter);

    painter->drawEllipse(toQRect(bbox));
  }

  //---

  painter->restore();
#else
  auto a1 = -M_PI/2.0;
  auto a2 = a1 + 2.0*M_PI;

  CArcToBezier::BezierList beziers;
  CArcToBezier::ArcToBeziers(position.x, position.y, size.x/2.0, size.y/2.0, a1, a2, beziers);

  CBezierPath bezierPath(beziers);

  drawBezierPath(painter, drawState, shape, bezierPath);
#endif
}

void
CQLottie::
drawPath(QPainter *painter, DrawState &drawState, const CLottieShape *shape)
{
  CBezierPath bezierPath;
  pathToBezier(shape->path_, drawState, bezierPath);

  drawBezierPath(painter, drawState, shape, bezierPath);

  const_cast<CLottieShape *>(shape)->setBBox(bezierPath.bbox());
}

void
CQLottie::
drawBezierPath(QPainter *painter, DrawState &drawState, const CLottieShape *shape,
               CBezierPath &bezierPath)
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

  auto ppath = toQPath(bezierPath);

  //---

  if (drawState.merge) {
    drawState.merge->shape = shape;

    drawState.merge->paths.push_back(ppath);

    return;
  }

  //---

  if (drawState.fill.rule == 2)
    ppath.setFillRule(Qt::OddEvenFill);
  else
    ppath.setFillRule(Qt::WindingFill);

  //---

  painter->save();

  //---

#if 1
  auto pmatrix = displayRange_.getMatrix()*drawState.matrix;
#else
  auto pmatrix = displayRange_.getMatrix()*drawState.preMatrix;

  for (auto *tshape : drawState.transform.shapes)
    pmatrix = pmatrix*getShapeMatrix(drawState, tshape);
#endif

  painter->setTransform(toQTransform(pmatrix));

  setPenBrush(painter, drawState, shape);

  painter->drawPath(ppath);

  //---

  const_cast<CLottieShape *>(shape)->setBBox(bezierPath.bbox());

  //---

  if (shape->selected()) {
    setSelectedPenBrush(painter);

    painter->drawPath(ppath);

    if (shape->bbox().isSet()) {
      setBBoxPenBrush(painter);

      painter->drawRect(toQRect(shape->bbox()));
    }
  }

  //---

  painter->restore();
}

void
CQLottie::
pathToBezier(const CLottie::BezierProperty &path, const DrawState &drawState,
             CBezierPath &bezierPath) const
{
  const auto &points  = path.tvvalue(drawState.timeFrame, CLottie::PointList()).points;
  const auto &ipoints = path.tivalue(drawState.timeFrame, CLottie::PointList()).points;
  const auto &opoints = path.tovalue(drawState.timeFrame, CLottie::PointList()).points;
  auto        closed  = path.tclosed(drawState.timeFrame);

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
drawPolystar(QPainter *painter, DrawState &drawState, const CLottieShape *shape)
{
//unhandledShape("polystar");

  auto *polyStar = shape->polyStar();
  if (! polyStar) return;

  auto positionxy = polyStar->position.tvalue(drawState.timeFrame, CLottie::XYVals());
  auto position   = positionxy.toPoint(CPoint2D(0, 0));

  auto type = polyStar->type.value_or(0);

  if (type != 0)
    warnOnce(__LINE__, "type: " + std::to_string(type));

  auto numPoints = int(polyStar->points.tvalue(drawState.timeFrame, 0).value_or(0));

  auto innerRadius    = polyStar->innerRadius   .tvalue(drawState.timeFrame, 0).value_or(0);
  auto innerRoundness = polyStar->outerRoundness.tvalue(drawState.timeFrame, 0).value_or(0);
  auto outerRadius    = polyStar->outerRadius   .tvalue(drawState.timeFrame, 0).value_or(0);
  auto outerRoundness = polyStar->outerRoundness.tvalue(drawState.timeFrame, 0).value_or(0);

  auto rotation = CMathGen::DegToRad(polyStar->rotation.tvalue(drawState.timeFrame, 0).value_or(0));

#if 0
  if (innerRoundness != 0)
    warnOnce(__LINE__, "innerRoundness: " + std::to_string(innerRoundness));
  if (outerRoundness != 0)
    warnOnce(__LINE__, "outerRoundness: " + std::to_string(outerRoundness));
#endif

#if 0
  QPainterPath path;

  auto da = (numPoints > 0 ? 2.0*M_PI/(2*numPoints) : 0);

  for (int i = 0; i < numPoints; ++i) {
    auto a1 = 2*i*da + M_PI/2.0 + rotation;
    auto a2 = a1 + da;

    auto c1 = std::cos(a1); auto s1 = std::sin(a1);
    auto c2 = std::cos(a2); auto s2 = std::sin(a2);

    auto xo = position.x + c1*outerRadius;
    auto yo = position.y + s1*outerRadius;
    auto xi = position.x + c2*innerRadius;
    auto yi = position.y + s2*innerRadius;

    if (i == 0)
      path.moveTo(xo, yo);
    else
      path.lineTo(xo, yo);

    path.lineTo(xi, yi);
  }

  path.closeSubpath();
#else
  CBezierPath bezierPath;

#if 0
  auto da = (numPoints > 0 ? 2.0*M_PI/(2*numPoints) : 0);

  for (int i = 0; i < numPoints; ++i) {
    auto a1 = 2*i*da + M_PI/2.0 + rotation;
    auto a2 = a1 + da;

    auto c1 = std::cos(a1); auto s1 = std::sin(a1);
    auto c2 = std::cos(a2); auto s2 = std::sin(a2);

    auto xo = position.x + c1*outerRadius;
    auto yo = position.y + s1*outerRadius;
    auto xi = position.x + c2*innerRadius;
    auto yi = position.y + s2*innerRadius;

    if (i == 0)
      bezierPath.moveTo(CPoint2D(xo, yo));
    else
      bezierPath.lineTo(CPoint2D(xo, yo));

    bezierPath.lineTo(CPoint2D(xi, yi));
  }

  bezierPath.close();
#else
  bezierPath.addPolyStar(position, numPoints, innerRadius, outerRadius,
                         innerRoundness, outerRoundness, rotation);
#endif
#endif

  //---

#if 0
  painter->save();

  //---

#if 1
  auto pmatrix = displayRange_.getMatrix()*drawState.matrix;
#else
  auto pmatrix = displayRange_.getMatrix()*drawState.preMatrix;

  for (auto *tshape : drawState.transform.shapes)
    pmatrix = pmatrix*getShapeMatrix(drawState, tshape);
#endif

  painter->setTransform(toQTransform(pmatrix));

  setPenBrush(painter, drawState, shape);

  painter->drawPath(path);

  //---

  if (shape->selected()) {
    setSelectedPenBrush(painter);

    painter->drawPath(path);
  }

  //---

  painter->restore();
#else
  drawBezierPath(painter, drawState, shape, bezierPath);
#endif
}

void
CQLottie::
drawRectangle(QPainter *painter, DrawState &drawState, const CLottieShape *shape)
{
  auto positionxy = shape->pos().tvalue(drawState.timeFrame, CLottie::XYVals());
  auto position   = positionxy.toPoint(CPoint2D(0, 0));

  auto sizexy = shape->size().tvalue(drawState.timeFrame, CLottie::XYVals());
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

  drawBezierPath(painter, drawState, shape, bezierPath);

  //---

#if 0
  painter->save();

  //---

#if 1
  auto pmatrix = displayRange_.getMatrix()*drawState.matrix;
#else
  auto pmatrix = displayRange_.getMatrix()*drawState.preMatrix;

  for (auto *tshape : drawState.transform.shapes)
    pmatrix = pmatrix*getShapeMatrix(drawState, tshape);
#endif

  painter->setTransform(toQTransform(pmatrix));

  setPenBrush(painter, drawState, shape);

  painter->drawRect(toQRect(bbox));

  if (shape->selected()) {
    setSelectedPenBrush(painter);

    painter->drawRect(toQRect(bbox));
  }

  //---

  painter->restore();
#endif
}

void
CQLottie::
setPenBrush(QPainter *painter, const DrawState &drawState, const CLottieShape *shape)
{
  if (drawState.fillGradient.enabled) {
    painter->setBrush(drawState.fillGradient.gradient);
  }
  else {
    auto fillColor   = getFillColor  (drawState, shape, drawState.fill.color);
    auto fillOpacity = getFillOpacity(drawState, shape, drawState.fill.opacity);

    if (fillColor) {
      auto c = toQColor(fillColor.value());

      if (fillOpacity)
        c.setAlpha(int(255*(fillOpacity.value()/100.0)));

      painter->setBrush(c);
    }
    else
      painter->setBrush(Qt::NoBrush);
  }

  //---

  QPen pen;

  if (drawState.strokeGradient.enabled) {
    pen.setColor(Qt::black);

    if (drawState.strokeGradient.width)
      pen.setWidth(drawState.strokeGradient.width.value());

    if (drawState.strokeGradient.lineCap)
      pen.setCapStyle(toLineCap(drawState.strokeGradient.lineCap.value()));

    if (drawState.strokeGradient.lineJoin)
      pen.setJoinStyle(toLineJoin(drawState.strokeGradient.lineJoin.value()));

    if (drawState.strokeGradient.miterLimit)
      pen.setMiterLimit(drawState.strokeGradient.miterLimit.value());
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

  painter->setPen(pen);
}

void
CQLottie::
setSelectedPenBrush(QPainter *painter)
{
  painter->setBrush(QBrush(Qt::white, Qt::Dense6Pattern));
  painter->setPen  (Qt::red);
}

void
CQLottie::
setBBoxPenBrush(QPainter *painter)
{
  painter->setBrush(Qt::NoBrush);
  painter->setPen  (Qt::red);
}

//---

void
CQLottie::
mouseMove(const QPoint &pos)
{
  CPoint2D p;
  displayRange_.pixelToWindow(CPoint2D(pos.x(), pos.y()), p);

  statusLabel_->setText(QString("%1 %2").arg(p.x).arg(p.y));
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

  auto *pshape = shape->getParentShape();
  auto *player = shape->getParentLayer();

  if      (pshape)
    m = getShapeMatrix(drawState, pshape)*m;
  else if (player)
    m = getLayerMatrix(drawState, player)*m;

  return m;
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

  if (! o) {
    auto *pshape = shape->getParentShape();

    if (pshape)
      o = getFillOpacity(drawState, pshape, def);

    if (! o) {
      auto *player = shape->getParentLayer();

      if (player)
        o = getLayerOpacity(drawState, player, def);
    }
  }

  return (o ? o : def);
}

CQLottie::OptReal
CQLottie::
getLayerOpacity(const DrawState &drawState, const CLottieLayer *layer, const OptReal &def) const
{
  OptReal o;

  if (layer->transform())
    o = layer->transform()->opacity.tvalue(drawState.timeFrame);

  if (! o) {
    auto *player = layer->getParentLayer();

    if (player)
      o = getLayerOpacity(drawState, player, def);
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
  if (shape->stroke())
    return shape->stroke()->opacity.tvalue(drawState.timeFrame, 100.0).value();

  return def;
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

//---

CQLottieCanvas::
CQLottieCanvas(CQLottie *lottie) :
 lottie_(lottie)
{
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);
}

void
CQLottieCanvas::
resizeEvent(QResizeEvent *)
{
  lottie_->setPixelSize(width(), height());
}

void
CQLottieCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing);

  painter.fillRect(rect(), QColor(255, 255, 255));

  lottie_->draw(&painter);
}

void
CQLottieCanvas::
mouseMoveEvent(QMouseEvent *me)
{
  auto pos = me->pos();

  lottie_->mouseMove(pos);
}

void
CQLottieCanvas::
keyPressEvent(QKeyEvent *ke)
{
  auto key = ke->key();

  if      (key == Qt::Key_Plus) {
    lottie_->zoom(true);
  }
  else if (key == Qt::Key_Minus) {
    lottie_->zoom(false);
  }
  else if (key == Qt::Key_Left) {
    lottie_->scroll(-1, 0);
  }
  else if (key == Qt::Key_Right) {
    lottie_->scroll(1, 0);
  }
  else if (key == Qt::Key_Up) {
    lottie_->scroll(0, 1);
  }
  else if (key == Qt::Key_Down) {
    lottie_->scroll(0, -1);
  }

  update();
}

//---

CQLottieTree::
CQLottieTree(CQLottie *lottie) :
 lottie_(lottie)
{
  auto *layout = new QVBoxLayout(this);

  tree_ = new CQLottieTreeWidget(this);

  layout->addWidget(tree_);

  //---

  tree_->setColumnCount(2);

  tree_->setHeaderLabels(QStringList() << "Name" << "Value");

  //--

  tree_->setUniformRowHeights(true);

  tree_->setAlternatingRowColors(true);

  tree_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  //---

  tree_->setItemDelegate(new CQLottieTreeDelegate(this));

  //---

  tree_->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(tree_, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(customContextMenuSlot(const QPoint&)));

  //---

  auto *controlFrame  = new QFrame;
  auto *controlLayout = new QHBoxLayout(controlFrame);

  layout->addWidget(controlFrame);

  auto *transformButton     = new QPushButton("Transform");
  auto *hierTransformButton = new QPushButton("Hier Transform");
  auto *printButton         = new QPushButton("Print");

  connect(transformButton, SIGNAL(clicked()), this, SLOT(transformSlot()));
  connect(hierTransformButton, SIGNAL(clicked()), this, SLOT(hierTransformSlot()));
  connect(printButton, SIGNAL(clicked()), this, SLOT(printSlot()));

  controlLayout->addStretch(1);
  controlLayout->addWidget(transformButton);
  controlLayout->addWidget(hierTransformButton);
  controlLayout->addWidget(printButton);

  //---

  connectSlots(true);
}

void
CQLottieTree::
resizeEvent(QResizeEvent *)
{
  expandAll();
}

void
CQLottieTree::
load()
{
  connectSlots(false);

  //---

  tree_->clear();

  layerItem_ .clear();
  assetItem_ .clear();
  shapeItem_ .clear();
  effectItem_.clear();

  //---

  auto *root = lottie_->lottie()->root();

  rootItem_ = new CQLottieTreeRootItem(tree_, root);

  tree_->addTopLevelItem(rootItem_);

  for (auto *asset : root->assets()) {
    auto *item = createAssetItem(asset);

    item->setData(0, Qt::UserRole, asset->ind().value_or(-1));
  }

  for (auto *layer : root->layers()) {
    auto *item = createLayerItem(layer);

    item->setData(0, Qt::UserRole, layer->ind().value_or(-1));
  }

  //---

  expandAll();

  //---

  connectSlots(true);
}

QTreeWidgetItem *
CQLottieTree::
createLayerItem(CLottieLayer *layer)
{
  auto pl = layerItem_.find(layer);

  if (pl != layerItem_.end())
    return (*pl).second;

  auto name = QString::fromStdString(layer->name().value_or(""));

  auto *parentAsset = layer->getParentAsset();
  auto *parentLayer = layer->getParentLayer();

  QTreeWidgetItem *parentItem { nullptr };

  if      (parentAsset)
    parentItem = createAssetItem(parentAsset);
  else if (parentLayer)
    parentItem = createLayerItem(parentLayer);
  else
    parentItem = rootItem_;

  auto *item = new CQLottieTreeLayerItem(parentItem, layer);

  parentItem->addChild(item);

  layerItem_[layer] = item;

  //---

  auto *effect = layer->effect();

  if (effect) {
    auto *item = createEffectItem(effect);

    item->setData(0, Qt::UserRole, effect->ind().value_or(-1));
  }

  //---

  for (auto *shape : layer->shapes()) {
    auto *item = createShapeItem(shape);

    item->setData(0, Qt::UserRole, shape->ind().value_or(-1));
  }

  //---

  return item;
}

QTreeWidgetItem *
CQLottieTree::
createAssetItem(CLottieAsset *asset)
{
  auto pa = assetItem_.find(asset);

  if (pa != assetItem_.end())
    return (*pa).second;

  auto name = QString::fromStdString(asset->name().value_or(""));

  auto *item = new CQLottieTreeAssetItem(rootItem_, asset);

  rootItem_->addChild(item);

  assetItem_[asset] = item;

  //---

  for (auto *layer : asset->layers()) {
    auto *item = createLayerItem(layer);

    item->setData(0, Qt::UserRole, layer->ind().value_or(-1));
  }

  //---

  return item;
}

QTreeWidgetItem *
CQLottieTree::
createShapeItem(CLottieShape *shape)
{
  auto pl = shapeItem_.find(shape);

  if (pl != shapeItem_.end())
    return (*pl).second;

  auto name = QString::fromStdString(shape->name().value_or(""));

  auto *parentLayer = shape->getParentLayer();
  auto *parentShape = shape->getParentShape();

  QTreeWidgetItem *item = nullptr;

  if     (parentLayer) {
    auto *parentItem = createLayerItem(parentLayer);

    item = new CQLottieTreeShapeItem(parentItem, shape);

    parentItem->addChild(item);
  }
  else if (parentShape) {
    auto *parentItem = createShapeItem(parentShape);

    item = new CQLottieTreeShapeItem(parentItem, shape);

    parentItem->addChild(item);
  }

  assert(item);

  shapeItem_[shape] = item;

  //---

  for (auto *shape1 : shape->shapes()) {
    auto *item = createShapeItem(shape1);

    item->setData(0, Qt::UserRole, shape1->ind().value_or(-1));
  }

  //---

  return item;
}

QTreeWidgetItem *
CQLottieTree::
createEffectItem(CLottieEffect *effect)
{
  auto pe = effectItem_.find(effect);

  if (pe != effectItem_.end())
    return (*pe).second;

  auto name = QString::fromStdString(effect->name().value_or(""));

  auto *parentLayer = effect->getLayer();

  QTreeWidgetItem *item = nullptr;

  if (parentLayer) {
    auto *parentItem = createLayerItem(parentLayer);

    item = new CQLottieTreeEffectItem(parentItem, effect);

    parentItem->addChild(item);
  }

  assert(item);

  effectItem_[effect] = item;

  //---

  return item;
}

void
CQLottieTree::
connectSlots(bool b)
{
  if (b) {
    connect(tree_, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(itemClickedSlot(QTreeWidgetItem *, int)));
    connect(tree_, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(itemSelectedSlot(QTreeWidgetItem *, QTreeWidgetItem *)));
  }
  else {
    disconnect(tree_, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
               this, SLOT(itemClickedSlot(QTreeWidgetItem *, int)));
    disconnect(tree_, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
               this, SLOT(itemSelectedSlot(QTreeWidgetItem *, QTreeWidgetItem *)));
  }
}

void
CQLottieTree::
itemClickedSlot(QTreeWidgetItem * /*item*/, int /*column*/)
{
}

void
CQLottieTree::
itemSelectedSlot(QTreeWidgetItem *item, QTreeWidgetItem *)
{
  lottie_->lottie()->deselectAll();

  auto *objItem = dynamic_cast<CQLottieTreeObjectItem *>(item);
  if (! objItem) return;

  objItem->object()->setSelected(true);

  lottie_->objectTree()->setObject(objItem->object());

  lottie_->canvas()->update();
}

void
CQLottieTree::
customContextMenuSlot(const QPoint &pos)
{
  auto *menu = new QMenu;

  auto *expandAction   = new QAction("Expand All"  , menu);
  auto *collapseAction = new QAction("Collapse All", menu);

  connect(expandAction  , SIGNAL(triggered()), this, SLOT(expandAll()));
  connect(collapseAction, SIGNAL(triggered()), this, SLOT(collapseAll()));

  menu->addAction(expandAction);
  menu->addAction(collapseAction);

  auto mpos = tree_->viewport()->mapToGlobal(pos);

  menu->exec(mpos);

  delete menu;
}

void
CQLottieTree::
expandAll(const QModelIndex &ind)
{
  tree_->setExpanded(ind, true);

  for (int r = 0; r < tree_->model()->rowCount(ind); ++r) {
    auto ind1 = tree_->model()->index(r, 0, ind);

    expandAll(ind1);
  }

  if (! ind.parent().isValid()) {
    tree_->resizeColumnToContents(0);
    tree_->resizeColumnToContents(1);
  }
}

void
CQLottieTree::
collapseAll(const QModelIndex &ind)
{
  tree_->setExpanded(ind, false);

  for (int r = 0; r < tree_->model()->rowCount(ind); ++r) {
    auto ind1 = tree_->model()->index(r, 0, ind);

    collapseAll(ind1);
  }
}

void
CQLottieTree::
printSlot()
{
  auto selectedItems = tree_->selectedItems();

  for (auto *item : selectedItems) {
    auto *objectItem = dynamic_cast<CQLottieTreeObjectItem *>(item);
    if (! objectItem) continue;

    auto *obj = objectItem->object();

    obj->print();
  }
}

void
CQLottieTree::
transformSlot()
{
  CLottieUtil::TimeFrame timeFrame;
  lottie_->getTimeFrame(timeFrame);

  auto selectedItems = tree_->selectedItems();

  for (auto *item : selectedItems) {
    auto *objectItem = dynamic_cast<CQLottieTreeObjectItem *>(item);
    if (! objectItem) continue;

    auto *obj = objectItem->object();

    auto m = obj->calcTransform(timeFrame);

    std::cerr << m << "\n";
  }
}

void
CQLottieTree::
hierTransformSlot()
{
  CLottieUtil::TimeFrame timeFrame;
  lottie_->getTimeFrame(timeFrame);

  auto selectedItems = tree_->selectedItems();

  for (auto *item : selectedItems) {
    auto *objectItem = dynamic_cast<CQLottieTreeObjectItem *>(item);
    if (! objectItem) continue;

    auto *obj = objectItem->object();

    auto m = obj->calcHierTransform(timeFrame);

    std::cerr << m << "\n";
  }
}

QTreeWidgetItem *
CQLottieTree::
itemFromIndex(const QModelIndex &index) const
{
  QTreeWidgetItem *item;

  if (! index.parent().isValid())
    item = tree_->topLevelItem(index.row());
  else {
    auto *parent = itemFromIndex(index.parent());
    assert(parent);

    item = parent->child(index.row());
  }

  return item;
}

//---

CQLottieTreeWidget::
CQLottieTreeWidget(CQLottieTree *tree) :
 tree_(tree)
{
  setSelectionMode(QTreeWidget::SingleSelection);
}

//---

CQLottieTreeDelegate::
CQLottieTreeDelegate(CQLottieTree *tree) :
 QItemDelegate(tree), tree_(tree)
{
}

QWidget *
CQLottieTreeDelegate::
createEditor(QWidget * /*parent*/, const QStyleOptionViewItem &,
             const QModelIndex & /*index*/) const
{
  return nullptr;
}

//! get data to display in tree widget item
void
CQLottieTreeDelegate::
setEditorData(QWidget * /*w*/, const QModelIndex & /*index*/) const
{
}

//! store displayed tree widget item data in model
void
CQLottieTreeDelegate::
setModelData(QWidget * /*w*/, QAbstractItemModel *, const QModelIndex & /*index*/) const
{
}

void
CQLottieTreeDelegate::
updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const
{
  return QItemDelegate::updateEditorGeometry(editor, option, index);
}

QSize
CQLottieTreeDelegate::
sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  return QItemDelegate::sizeHint(option, index);
}

void
CQLottieTreeDelegate::
paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  return QItemDelegate::paint(painter, option, index);
}

//---

namespace {

QString objectLabelStr(CLottieObject *object) {
  auto objTypeName = QString::fromStdString(CLottieObject::typeName(object->objectType()));
  auto objName     = QString::fromStdString(object->name().value_or(""));
  auto typeName    = QString::fromStdString(object->type());

  QString indStr;

  if (object->ind())
    indStr = QString(" (#%1)").arg(*object->ind());

  return objTypeName + ": " + objName + " (" + typeName + ")" + indStr;
}

}

CQLottieTreeObjectItem::
CQLottieTreeObjectItem(QTreeWidget *parent, CLottieObject *object) :
 QTreeWidgetItem(parent, QStringList() << objectLabelStr(object)), object_(object)
{
}

CQLottieTreeObjectItem::
CQLottieTreeObjectItem(QTreeWidgetItem *parent, CLottieObject *object) :
 QTreeWidgetItem(parent, QStringList() << objectLabelStr(object)), object_(object)
{
}

//---

CQLottieTreeRootItem::
CQLottieTreeRootItem(QTreeWidget *parent, CLottieRoot *root) :
 CQLottieTreeObjectItem(parent, root), root_(root)
{
}

CQLottieTreeAssetItem::
CQLottieTreeAssetItem(QTreeWidgetItem *parent, CLottieAsset *asset) :
 CQLottieTreeObjectItem(parent, asset), asset_(asset)
{
}

CQLottieTreeLayerItem::
CQLottieTreeLayerItem(QTreeWidgetItem *parent, CLottieLayer *layer) :
 CQLottieTreeObjectItem(parent, layer), layer_(layer)
{
}

CQLottieTreeShapeItem::
CQLottieTreeShapeItem(QTreeWidgetItem *parent, CLottieShape *shape) :
 CQLottieTreeObjectItem(parent, shape), shape_(shape)
{
}

CQLottieTreeEffectItem::
CQLottieTreeEffectItem(QTreeWidgetItem *parent, CLottieEffect *effect) :
 CQLottieTreeObjectItem(parent, effect), effect_(effect)
{
}

//---

CQLottieObjectTree::
CQLottieObjectTree(CQLottie *lottie) :
 lottie_(lottie)
{
  auto *layout = new QVBoxLayout(this);

  tree_ = new CQLottieObjectTreeWidget(this);

  layout->addWidget(tree_);

  //---

  tree_->setColumnCount(2);

  tree_->setHeaderLabels(QStringList() << "Name" << "Value");

  //--

  tree_->setUniformRowHeights(true);

  tree_->setAlternatingRowColors(true);

  tree_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  //---

  tree_->setItemDelegate(new CQLottieObjectTreeDelegate(this));

  //---

  tree_->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(tree_, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(customContextMenuSlot(const QPoint&)));

  //---

  auto *controlFrame  = new QFrame;
  auto *controlLayout = new QHBoxLayout(controlFrame);

  layout->addWidget(controlFrame);

  auto *printButton = new QPushButton("Print");

  connect(printButton, SIGNAL(clicked()), this, SLOT(printSlot()));

  controlLayout->addStretch(1);
  controlLayout->addWidget(printButton);

  //---

  connectSlots(true);
}

void
CQLottieObjectTree::
connectSlots(bool b)
{
  if (b) {
    connect(tree_, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this, SLOT(itemClickedSlot(QTreeWidgetItem *, int)));
    connect(tree_, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(itemSelectedSlot(QTreeWidgetItem *, QTreeWidgetItem *)));
  }
  else {
    disconnect(tree_, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
               this, SLOT(itemClickedSlot(QTreeWidgetItem *, int)));
    disconnect(tree_, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
               this, SLOT(itemSelectedSlot(QTreeWidgetItem *, QTreeWidgetItem *)));
  }
}

void
CQLottieObjectTree::
setObject(CLottieObject *object)
{
  object_ = object;

  load();

  expandAll();
}

void
CQLottieObjectTree::
load()
{
  connectSlots(false);

  //---

  tree_->clear();

  //---

  // title
  auto title = objectLabelStr(object_);

  auto *item = new QTreeWidgetItem(QStringList() << title);

  tree_->addTopLevelItem(item);

  //---

  auto *root   = dynamic_cast<CLottieRoot   *>(object_);
  auto *asset  = dynamic_cast<CLottieAsset  *>(object_);
  auto *layer  = dynamic_cast<CLottieLayer  *>(object_);
  auto *shape  = dynamic_cast<CLottieShape  *>(object_);
  auto *effect = dynamic_cast<CLottieEffect *>(object_);

  struct PropName {
    QString                     name;
    CQLottieTreeValueItem::Type type { CQLottieTreeValueItem::Type::NONE };

    PropName() { }

    PropName(const QString &n, const CQLottieTreeValueItem::Type &t) :
     name(n), type(t) {
    }
  };

  std::vector<PropName> objPropNames = {
    { "name"  , CQLottieTreeValueItem::Type::STRING },
    { "type"  , CQLottieTreeValueItem::Type::STRING },
//  { "typeId", CQLottieTreeValueItem::Type::INTEGER },
    { "hidden", CQLottieTreeValueItem::Type::BOOL },
    { "ind"   , CQLottieTreeValueItem::Type::INTEGER }
  };

  if      (root) {
    std::vector<PropName> propNames = {
      { "frameRate" , CQLottieTreeValueItem::Type::REAL },
      { "frameStart", CQLottieTreeValueItem::Type::REAL },
      { "frameStop" , CQLottieTreeValueItem::Type::REAL },
      { "width"     , CQLottieTreeValueItem::Type::REAL },
      { "height"    , CQLottieTreeValueItem::Type::REAL },
    };

    auto addRootProp = [&](const PropName &propName) {
      auto *propItem = new CQLottieTreeRootValueItem(item, root, propName.name, propName.type);
      item->addChild(propItem);
    };

    for (const auto &propName : objPropNames) {
      if (propName.name == "ind" && ! object_->ind())
        continue;

      addRootProp(propName);
    }

    for (const auto &propName : propNames)
      addRootProp(propName);
  }
  else if (asset) {
    std::vector<PropName> propNames = {
      { "id", CQLottieTreeValueItem::Type::STRING },
    };

    auto addAssetProp = [&](const PropName &propName) {
      auto *propItem = new CQLottieTreeAssetValueItem(item, asset, propName.name, propName.type);
      item->addChild(propItem);
    };

    for (const auto &propName : objPropNames) {
      if (propName.name == "ind" && ! object_->ind())
        continue;

      addAssetProp(propName);
    }

    for (const auto &propName : propNames)
      addAssetProp(propName);
  }
  else if (layer) {
    std::vector<PropName> propNames = {
      { "typeId"   , CQLottieTreeValueItem::Type::STRING  },
      { "refId"    , CQLottieTreeValueItem::Type::STRING  },
      { "parentInd", CQLottieTreeValueItem::Type::INTEGER },
      { "opacity"  , CQLottieTreeValueItem::Type::REAL    },
      { "frameIn"  , CQLottieTreeValueItem::Type::INTEGER },
      { "frameOut" , CQLottieTreeValueItem::Type::INTEGER }
    };

    auto addLayerProp = [&](const PropName &propName) {
      auto *propItem = new CQLottieTreeLayerValueItem(item, layer, propName.name, propName.type);
      item->addChild(propItem);
    };

    for (const auto &propName : objPropNames) {
      if (propName.name == "ind" && ! object_->ind())
        continue;

      addLayerProp(propName);
    }

    for (const auto &propName : propNames) {
      if (propName.name == "parentInd" && ! layer->parentInd())
        continue;

      addLayerProp(propName);
    }
  }
  else if (shape) {
    std::vector<PropName> propNames = { };

    auto addShapeProp = [&](const PropName &propName) {
      auto *propItem = new CQLottieTreeShapeValueItem(item, shape, propName.name, propName.type);
      item->addChild(propItem);
    };

    for (const auto &propName : objPropNames) {
      if (propName.name == "ind" && ! object_->ind())
        continue;

      addShapeProp(propName);
    }

    if ((shape->fill  () && shape->fill  ()->color.isSet()) ||
        (shape->stroke() && shape->stroke()->color.isSet()) ||
        shape->color().isSet())
      addShapeProp(PropName("color", CQLottieTreeValueItem::Type::COLOR));

    if ((shape->fill     () && shape->fill     ()->opacity.isSet()) ||
        (shape->stroke   () && shape->stroke   ()->opacity.isSet()) ||
        (shape->transform() && shape->transform()->opacity.isSet()))
      addShapeProp(PropName("opacity", CQLottieTreeValueItem::Type::REAL));

    for (const auto &propName : propNames)
      addShapeProp(propName);

    if (shape->pos().isSet())
      addShapeProp(PropName("position", CQLottieTreeValueItem::Type::POSITION));

    if (shape->size().isSet())
      addShapeProp(PropName("size", CQLottieTreeValueItem::Type::SIZE));

    if (shape->trim()) {
      std::vector<PropName> trimPropNames = {
        { "trim.start"   , CQLottieTreeValueItem::Type::SCALAR },
        { "trim.end"     , CQLottieTreeValueItem::Type::SCALAR },
        { "trim.offset"  , CQLottieTreeValueItem::Type::SCALAR },
        { "trim.multiple", CQLottieTreeValueItem::Type::INTEGER }
      };

      for (const auto &propName : trimPropNames)
        addShapeProp(propName);
    }
  }
  else if (effect) {
    std::vector<PropName> propNames = {
      { "etype", CQLottieTreeValueItem::Type::INTEGER }
    };

    auto addEffectProp = [&](const PropName &propName) {
      auto *propItem = new CQLottieTreeEffectValueItem(item, effect, propName.name, propName.type);
      item->addChild(propItem);
    };

    for (const auto &propName : objPropNames) {
      if (propName.name == "ind" && ! object_->ind())
        continue;

      addEffectProp(propName);
    }

    for (const auto &propName : propNames)
      addEffectProp(propName);
  }

  //---

  connectSlots(true);
}

void
CQLottieObjectTree::
itemClickedSlot(QTreeWidgetItem *item, int column)
{
  if (column != 1)
    return;

  auto *valueItem = dynamic_cast<CQLottieTreeValueItem *>(item);
  if (! valueItem) return;

  if (valueItem->propType() == CQLottieTreeValueItem::Type::BOOL) {
    if (valueItem->propName() == "hidden") {
      valueItem->object()->setHidden(! valueItem->object()->hidden().value_or(false));

      auto ind = tree_->indexFromItem(item, column);

      tree_->update(ind);

      lottie_->canvas()->update();
    }
  }
}

void
CQLottieObjectTree::
itemSelectedSlot(QTreeWidgetItem *item, QTreeWidgetItem *)
{
  lottie_->lottie()->deselectAll();

  auto *valueItem = dynamic_cast<CQLottieTreeValueItem *>(item);
  if (! valueItem) return;

  valueItem->object()->setSelected(true);

  lottie_->canvas()->update();
}

void
CQLottieObjectTree::
customContextMenuSlot(const QPoint &pos)
{
  auto *menu = new QMenu;

  auto *expandAction   = new QAction("Expand All"  , menu);
  auto *collapseAction = new QAction("Collapse All", menu);

  connect(expandAction  , SIGNAL(triggered()), this, SLOT(expandAll()));
  connect(collapseAction, SIGNAL(triggered()), this, SLOT(collapseAll()));

  menu->addAction(expandAction);
  menu->addAction(collapseAction);

  auto mpos = tree_->viewport()->mapToGlobal(pos);

  menu->exec(mpos);

  delete menu;
}

void
CQLottieObjectTree::
expandAll(const QModelIndex &ind)
{
  tree_->setExpanded(ind, true);

  for (int r = 0; r < tree_->model()->rowCount(ind); ++r) {
    auto ind1 = tree_->model()->index(r, 0, ind);

    expandAll(ind1);
  }

  if (! ind.parent().isValid()) {
    tree_->resizeColumnToContents(0);
    tree_->resizeColumnToContents(1);
  }
}

void
CQLottieObjectTree::
collapseAll(const QModelIndex &ind)
{
  tree_->setExpanded(ind, false);

  for (int r = 0; r < tree_->model()->rowCount(ind); ++r) {
    auto ind1 = tree_->model()->index(r, 0, ind);

    collapseAll(ind1);
  }
}

void
CQLottieObjectTree::
printSlot()
{
  if (object_)
    object_->print();
}

QTreeWidgetItem *
CQLottieObjectTree::
itemFromIndex(const QModelIndex &index) const
{
  QTreeWidgetItem *item;

  if (! index.parent().isValid())
    item = tree_->topLevelItem(index.row());
  else {
    auto *parent = itemFromIndex(index.parent());
    assert(parent);

    item = parent->child(index.row());
  }

  return item;
}

//---

CQLottieObjectTreeWidget::
CQLottieObjectTreeWidget(CQLottieObjectTree *tree) :
 tree_(tree)
{
  setSelectionMode(QTreeWidget::SingleSelection);
}

//---

CQLottieObjectTreeDelegate::
CQLottieObjectTreeDelegate(CQLottieObjectTree *tree) :
 QItemDelegate(tree), tree_(tree)
{
}

QWidget *
CQLottieObjectTreeDelegate::
createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
  if (index.column() != 1)
    return nullptr;

  auto index1 = index.model()->index(index.row(), 0, index.parent());

  auto *item = tree_->itemFromIndex(index1);

  auto *valueItem = dynamic_cast<CQLottieTreeValueItem *>(item);
  if (! valueItem) return nullptr;

  QWidget *w { nullptr };

  if      (valueItem->propType() == CQLottieTreeValueItem::Type::BOOL) {
    auto *check = new QCheckBox(parent);

    check->setAutoFillBackground(true);

    connect(check, SIGNAL(stateChanged(int)), this, SLOT(updateValue()));

    w = check;
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::REAL) {
    auto *edit = new CQRealSpin(parent);

    edit->setAutoFillBackground(true);

    connect(edit, SIGNAL(editingFinished()), this, SLOT(updateValue()));

    w = edit;
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::STRING) {
    auto *edit = new QLineEdit(parent);

    edit->setAutoFillBackground(true);

    connect(edit, SIGNAL(editingFinished()), this, SLOT(updateValue()));

    w = edit;
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::COLOR) {
    auto *chooser = new CQColorChooser(parent);

    chooser->setAutoFillBackground(true);

    chooser->setStyles(CQColorChooser::Text | CQColorChooser::ColorButton);

    connect(chooser, SIGNAL(colorChanged(const QColor&)), this, SLOT(updateValue()));

    w = chooser;
  }

  if (! w)
    return nullptr;

  editIndex_ = index;

  return w;
}

void
CQLottieObjectTreeDelegate::
updateValue()
{
  auto *o = sender();

  setModelData(qobject_cast<QWidget *>(o), nullptr, editIndex_);
}

//! get data to display in tree widget item
void
CQLottieObjectTreeDelegate::
setEditorData(QWidget *w, const QModelIndex &index) const
{
  auto *item = tree_->itemFromIndex(index);

  auto *valueItem = dynamic_cast<CQLottieTreeValueItem *>(item);
  if (! valueItem) return;

//auto *rootItem  = dynamic_cast<CQLottieTreeRootValueItem  *>(valueItem);
//auto *assetItem = dynamic_cast<CQLottieTreeAssetValueItem *>(valueItem);
  auto *layerItem = dynamic_cast<CQLottieTreeLayerValueItem *>(valueItem);
  auto *shapeItem = dynamic_cast<CQLottieTreeShapeValueItem *>(valueItem);

  if      (valueItem->propType() == CQLottieTreeValueItem::Type::BOOL) {
    auto *check = qobject_cast<QCheckBox *>(w);

    if (valueItem->propName() == "hidden")
      check->setChecked(valueItem->object()->hidden().value_or(false));
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::INTEGER) {
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::REAL) {
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::STRING) {
    auto *edit = qobject_cast<QLineEdit *>(w);

    if (valueItem->propName() == "refId") {
      if (layerItem)
        edit->setText(QString::fromStdString(layerItem->layer()->refId().value_or("")));
    }
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::COLOR) {
    auto *chooser = qobject_cast<CQColorChooser *>(w);

    if (valueItem->propName() == "color") {
      if (shapeItem) {
        auto c = shapeItem->shape()->color().value(CRGBA(0, 0, 0, 0)).value();

        chooser->setColor(toQColor(c));
      }
    }
  }
}

//! store displayed tree widget item data in model
void
CQLottieObjectTreeDelegate::
setModelData(QWidget *w, QAbstractItemModel *, const QModelIndex &index) const
{
  auto *item = tree_->itemFromIndex(index);

  auto *valueItem = dynamic_cast<CQLottieTreeValueItem *>(item);
  if (! valueItem) return;

  auto *lottie = tree_->lottie();

//auto *rootItem  = dynamic_cast<CQLottieTreeRootValueItem  *>(valueItem);
//auto *assetItem = dynamic_cast<CQLottieTreeAssetValueItem *>(valueItem);
  auto *layerItem = dynamic_cast<CQLottieTreeLayerValueItem *>(valueItem);
  auto *shapeItem = dynamic_cast<CQLottieTreeShapeValueItem *>(valueItem);

  if      (valueItem->propType() == CQLottieTreeValueItem::Type::BOOL) {
    auto *check = qobject_cast<QCheckBox *>(w);

    if (valueItem->propName() == "hidden") {
      valueItem->object()->setHidden(check->isChecked());

      lottie->canvas()->update();
    }
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::INTEGER) {
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::REAL) {
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::STRING) {
    auto *edit = qobject_cast<QLineEdit *>(w);

    if (valueItem->propName() == "refId") {
      if (layerItem)
        layerItem->layer()->setRefId(edit->text().toStdString());
    }
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::COLOR) {
    auto *chooser = qobject_cast<CQColorChooser *>(w);

    if (valueItem->propName() == "color") {
      if (shapeItem)
        shapeItem->shape()->colorRef().setValue(toRGBA(chooser->color()));
    }
  }
}

void
CQLottieObjectTreeDelegate::
updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                     const QModelIndex &index) const
{
  return QItemDelegate::updateEditorGeometry(editor, option, index);
}

QSize
CQLottieObjectTreeDelegate::
sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  return QItemDelegate::sizeHint(option, index);
}

void
CQLottieObjectTreeDelegate::
paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if (index.column() == 1) {
    auto *item = tree_->itemFromIndex(index);

    auto *valueItem = dynamic_cast<CQLottieTreeValueItem *>(item);

    if (! valueItem)
      return QItemDelegate::paint(painter, option, index);

    //---

    auto *lottie = tree_->lottie();

    CLottieUtil::TimeFrame timeFrame;
    lottie->getTimeFrame(timeFrame);

    //---

    auto *rootItem   = dynamic_cast<CQLottieTreeRootValueItem   *>(valueItem);
    auto *assetItem  = dynamic_cast<CQLottieTreeAssetValueItem  *>(valueItem);
    auto *layerItem  = dynamic_cast<CQLottieTreeLayerValueItem  *>(valueItem);
    auto *shapeItem  = dynamic_cast<CQLottieTreeShapeValueItem  *>(valueItem);
    auto *effectItem = dynamic_cast<CQLottieTreeEffectValueItem *>(valueItem);

    auto *object = valueItem->object();
    auto *root   = (rootItem   ? rootItem  ->root  () : nullptr);
    auto *asset  = (assetItem  ? assetItem ->asset () : nullptr);
    auto *layer  = (layerItem  ? layerItem ->layer () : nullptr);
    auto *shape  = (shapeItem  ? shapeItem ->shape () : nullptr);
    auto *effect = (effectItem ? effectItem->effect() : nullptr);

    if      (valueItem->propType() == CQLottieTreeValueItem::Type::BOOL) {
      std::optional<bool> b;

      if (valueItem->propName() == "hidden")
        b = object->hidden().value_or(false);

      if (b)
        drawChecked(painter, option, b.value(), index);
      else
        drawString(painter, option, "<unset>", index);
    }
    else if (valueItem->propType() == CQLottieTreeValueItem::Type::INTEGER) {
      std::optional<int> i;

      if (valueItem->propName() == "ind") {
        if (object->ind())
          i = object->ind().value();
      }
      else {
        if      (layer) {
          if      (valueItem->propName() == "parentInd") {
            if (layer->parentInd())
              i = layer->parentInd().value();
          }
          else if (valueItem->propName() == "frameIn") {
            if (layer->frameIn())
              i = layer->frameIn().value();
          }
          else if (valueItem->propName() == "frameOut") {
            if (layer->frameOut())
              i = layer->frameOut().value();
          }
        }
        else if (shape) {
          if      (valueItem->propName() == "trim.multiple") {
            if (shape->trim() && shape->trim()->multiple)
              i = shape->trim()->multiple.value();
          }
        }
        else if (effect) {
          if (valueItem->propName() == "etype") {
            if (effect->type())
              i = effect->type().value();
          }
        }
      }

      if (i)
        drawString(painter, option, QString::number(i.value()), index);
      else
        drawString(painter, option, "<unset>", index);
    }
    else if (valueItem->propType() == CQLottieTreeValueItem::Type::REAL) {
      std::optional<double> r;

      if      (root) {
        if      (valueItem->propName() == "frameRate")
          r = root->frameRate();
        else if (valueItem->propName() == "frameStart")
          r = root->frameStart();
        else if (valueItem->propName() == "frameStop")
          r = root->frameStop();
        else if (valueItem->propName() == "width")
          r = root->width();
        else if (valueItem->propName() == "height")
          r = root->height();
      }
      else if (layer) {
        if (valueItem->propName() == "opacity") {
          if (layer->transform())
            r = layer->transform()->opacity.value(100.0);
        }
      }
      else if (shape) {
        if (valueItem->propName() == "opacity") {
          std::optional<double> opacity;

          if      (shape->fill() && shape->fill()->opacity.isSet())
            r = shape->fill()->opacity.value(100.0);
          else if (shape->stroke() && shape->stroke()->opacity.isSet())
            r = shape->stroke()->opacity.value(100.0);
          else if (shape->transform() && shape->transform()->opacity.isSet())
            r = shape->transform()->opacity.value(100.0);
        }
      }

      if (r)
        drawString(painter, option, QString::number(r.value()), index);
      else
        drawString(painter, option, "<unset>", index);
    }
    else if (valueItem->propType() == CQLottieTreeValueItem::Type::STRING) {
      std::optional<QString>     str;
      std::optional<std::string> cstr;

      if      (valueItem->propName() == "name") {
        if (object->name())
          cstr = object->name().value();
      }
      else if (valueItem->propName() == "type") {
        cstr = object->type();
      }
      else {
        if      (asset) {
          if (valueItem->propName() == "id")
            cstr = asset->id();
        }
        else if (layer) {
          if      (valueItem->propName() == "typeId") {
            if (layer->typeId()) {
              int typeId = layer->typeId().value();

              str = QString::fromStdString(CLottieLayer::typeIdName(typeId)) +
                       " (" + QString::number(typeId) + ")";
            }
          }
          else if (valueItem->propName() == "refId") {
            if (layer->precomp() && layer->precomp()->refId)
              cstr = layer->precomp()->refId.value();
            else if (layer->refId())
              cstr = layer->refId().value();
          }
        }
      }

      if (cstr)
        str = QString::fromStdString(cstr.value());

      if (str)
        drawString(painter, option, str.value(), index);
    }
    else if (valueItem->propType() == CQLottieTreeValueItem::Type::COLOR) {
      std::optional<CRGBA> c;

      if (shape) {
        if (valueItem->propName() == "color") {
          if      (shape->fill() && shape->fill()->color.isSet())
            c = shape->fill()->color.value(CRGBA(0, 0, 0, 0));
          else if (shape->stroke() && shape->stroke()->color.isSet())
            c = shape->stroke()->color.value(CRGBA(0, 0, 0, 0));
          else if (shape->color().isSet())
            c = shape->color().value(CRGBA(0, 0, 0, 0));
        }
      }

      if (c)
        drawColor(painter, option, toQColor(c.value()), index);
      else
        drawString(painter, option, "<unset>", index);
    }
    else if (valueItem->propType() == CQLottieTreeValueItem::Type::SCALAR) {
      std::optional<double> r;

      if (shape) {
        if      (valueItem->propName() == "trim.start") {
          if (shape->trim() && shape->trim()->start.isSet())
            r = shape->trim()->start.tvalue(timeFrame, 0.0);
        }
        else if (valueItem->propName() == "trim.end") {
          if (shape->trim() && shape->trim()->end.isSet())
            r = shape->trim()->end.tvalue(timeFrame, 0.0);
        }
        else if (valueItem->propName() == "trim.offset") {
          if (shape->trim() && shape->trim()->offset.isSet())
            r = shape->trim()->offset.tvalue(timeFrame, 0.0);
        }
      }

      if (r)
        drawString(painter, option, QString::number(r.value()), index);
      else
        drawString(painter, option, "<unset>", index);
    }
    else if (valueItem->propType() == CQLottieTreeValueItem::Type::POSITION) {
      std::optional<CPoint2D> p;

      if (shape) {
        if (valueItem->propName() == "position") {
          auto positionxy = shape->pos().tvalue(timeFrame, CLottie::XYVals());

          p = positionxy.toPoint(CPoint2D(0, 0));
        }
      }

      if (p)
        drawString(painter, option, QString("%1,%2").arg(p->x).arg(p->y), index);
      else
        drawString(painter, option, "<unset>", index);
    }
    else if (valueItem->propType() == CQLottieTreeValueItem::Type::SIZE) {
      std::optional<CPoint2D> p;

      if (shape) {
        if (valueItem->propName() == "size") {
          auto sizexy = shape->size().tvalue(timeFrame, CLottie::XYVals());

          p = sizexy.toPoint(CPoint2D(0, 0));
        }
      }

      if (p)
        drawString(painter, option, QString("%1,%2").arg(p->x).arg(p->y), index);
      else
        drawString(painter, option, "<unset>", index);
    }
    else
      return QItemDelegate::paint(painter, option, index);
  }
  else
    return QItemDelegate::paint(painter, option, index);
}

void
CQLottieObjectTreeDelegate::
drawChecked(QPainter *painter, const QStyleOptionViewItem &option,
            bool checked, const QModelIndex &index) const
{
  QItemDelegate::drawBackground(painter, option, index);

  auto checkState = (checked ? Qt::Checked : Qt::Unchecked);

  auto rect = option.rect;

  rect.setWidth(option.rect.height());

  rect.adjust(0, 1, -3, -2);

  QItemDelegate::drawCheck(painter, option, rect, checkState);

  QFontMetrics fm(painter->font());

  int x = rect.right() + 4;
//int y = rect.top() + fm.ascent();

  QRect rect1;

  rect1.setCoords(x, option.rect.top(), option.rect.right(), option.rect.bottom());

  //painter->drawText(x, y, (checked ? "true" : "false"));
  QItemDelegate::drawDisplay(painter, option, rect1, checked ? "true" : "false");
}

void
CQLottieObjectTreeDelegate::
drawColor(QPainter *painter, const QStyleOptionViewItem &option,
          const QColor &c, const QModelIndex &index) const
{
  QItemDelegate::drawBackground(painter, option, index);

  auto rect = option.rect;

  rect.setWidth(option.rect.height());

  rect.adjust(0, 1, -3, -2);

  painter->setBrush(QBrush(c));
  painter->setPen(QColor(Qt::black)); // TODO: contrast border

//painter->fillRect(rect, QBrush(c));
  painter->drawRect(rect);

  int x = rect.right() + 2;
//int y = rect.top() + fm.ascent();

  QRect rect1;

  rect1.setCoords(x, option.rect.top(), option.rect.right(), option.rect.bottom());

  QItemDelegate::drawDisplay(painter, option, rect1, c.name());
}

void
CQLottieObjectTreeDelegate::
drawString(QPainter *painter, const QStyleOptionViewItem &option,
           const QString &str, const QModelIndex &index) const
{
  QItemDelegate::drawBackground(painter, option, index);

  auto rect = option.rect;

  QItemDelegate::drawDisplay(painter, option, rect, str);
}

//---

CQLottieTreeValueItem::
CQLottieTreeValueItem(QTreeWidgetItem *parent, CLottieObject *object,
                      const QString &propName, const Type &propType) :
 QTreeWidgetItem(parent, QStringList() << propName), object_(object),
 propName_(propName), propType_(propType)
{
  setFlags(flags() | Qt::ItemIsEditable);
}

CQLottieTreeRootValueItem::
CQLottieTreeRootValueItem(QTreeWidgetItem *parent, CLottieRoot *root,
                          const QString &propName, const Type &propType) :
 CQLottieTreeValueItem(parent, root, propName, propType), root_(root)
{
}

CQLottieTreeAssetValueItem::
CQLottieTreeAssetValueItem(QTreeWidgetItem *parent, CLottieAsset *asset,
                           const QString &propName, const Type &propType) :
 CQLottieTreeValueItem(parent, asset, propName, propType), asset_(asset)
{
}

CQLottieTreeLayerValueItem::
CQLottieTreeLayerValueItem(QTreeWidgetItem *parent, CLottieLayer *layer,
                           const QString &propName, const Type &propType) :
 CQLottieTreeValueItem(parent, layer, propName, propType), layer_(layer)
{
}

CQLottieTreeShapeValueItem::
CQLottieTreeShapeValueItem(QTreeWidgetItem *parent, CLottieShape *shape,
                           const QString &propName, const Type &propType) :
 CQLottieTreeValueItem(parent, shape, propName, propType), shape_(shape)
{
}

CQLottieTreeEffectValueItem::
CQLottieTreeEffectValueItem(QTreeWidgetItem *parent, CLottieEffect *effect,
                            const QString &propName, const Type &propType) :
 CQLottieTreeValueItem(parent, effect, propName, propType), effect_(effect)
{
}
