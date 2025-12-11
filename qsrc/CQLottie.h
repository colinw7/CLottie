#ifndef CQLottie_H
#define CQLottie_H

#include <CLottie.h>
#include <CDisplayRange2D.h>
#include <CLineDash.h>

#include <QFrame>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QTransform>

#include <deque>

class CQLottieCanvas;
class CQLottieToolBar;
class CQLottieStatusBar;
class CQLottieSettings;
class CQLottieTimeLine;
class CQLottiePath;
class CQLottieTree;
class CQLottieLayer;
class CQLottieObjectTree;

class CQColorEdit;
class CBezierPath;

class QLabel;
class QTimer;
class QPainterPathStroker;

namespace CQLottieUtil {

inline QPointF toQPoint(const CPoint2D &point) {
  return QPointF(point.x, point.y);
}

inline CPoint2D toPoint(const QPointF &point) {
  return CPoint2D(point.x(), point.y());
}

inline QRectF toQRect(const CBBox2D &rect) {
  if (! rect.isSet()) return QRectF();
  return QRectF(toQPoint(rect.getLL()), toQPoint(rect.getUR())).normalized();
}

inline CBBox2D toBBox(const QRectF &rect) {
  return CBBox2D(toPoint(rect.topLeft()), toPoint(rect.bottomRight()));
}

inline CBBox2D transformBBox(const CMatrixStack2D &m, const CBBox2D &bbox) {
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

inline QRectF transformRect(const CMatrixStack2D &m, const QRectF &rect) {
  return toQRect(transformBBox(m, toBBox(rect)));
}

inline QColor toQColor(const CRGBA &color) {
  return QColor(color.getRedI(), color.getGreenI(), color.getBlueI(), color.getAlphaI());
}

inline Qt::PenCapStyle toLineCap(int lineCap) {
  switch (lineCap) {
    default:
    case 1: return Qt::FlatCap;
    case 2: return Qt::RoundCap;
    case 3: return Qt::SquareCap;
  }
}

inline Qt::PenJoinStyle toLineJoin(int lineJoin) {
  switch (lineJoin) {
    default:
    case 1: return Qt::MiterJoin;
    case 2: return Qt::RoundJoin;
    case 3: return Qt::BevelJoin;
  }
}

inline void penSetLineDash(QPen &pen, const CLineDash &dash) {
  auto num = dash.getNumLengths();

  if (num > 0) {
    auto w = pen.widthF();
    if (w < 0) w = 1.0;

    pen.setStyle(Qt::CustomDashLine);

    pen.setDashOffset(dash.getOffset()/w);

    QVector<qreal> dashes;

    for (int i = 0; i < int(num); ++i)
      dashes << dash.getLength(i)/w;

    if (num & 1)
      dashes << dash.getLength(0)/w;

    pen.setDashPattern(dashes);
  }
  else
    pen.setStyle(Qt::SolidLine);
}

QImage fillImage(const QImage &sourceImage, const CRGBA &color);

QImage alphaImage(const QImage &sourceImage, double a);

QImage applyDropShadow(const QImage &sourceImage, int blurRadius, const QColor &shadowColor,
                       const QPointF &offset);

}

//---

class CQLottie : public QWidget {
  Q_OBJECT

 public:
  using TimeFrame = CLottieUtil::TimeFrame;

 public:
  CQLottie();

  CLottie *lottie() const { return lottie_; }

  CQLottieCanvas *canvas() const { return canvas_; }

  CQLottieStatusBar *status() const { return status_; }

  CQLottieTimeLine *timeLine() const { return timeLine_; }

  CQLottiePath *path() const { return path_; }

  CQLottieTree *tree() const { return tree_; }

  CQLottieObjectTree *objectTree() const { return objectTree_; }

  //---

  void setDebug(bool b);
  void setPrint(bool b);

  bool isDoubleBuffer() const { return doubleBuffer_; }
  void setDoubleBuffer(bool b) { doubleBuffer_ = b; }

  bool isEqualScale() const { return equalScale_; }
  void setEqualScale(bool b);

  const QColor &bgColor() const { return bgColor_; }
  void setBgColor(const QColor &c) { bgColor_ = c; }

  bool isShowSelect() const { return showSelect_; }
  void setShowSelect(bool b) { showSelect_ = b; }

  const QColor &selectedPenColor() const { return selectedPenColor_; }
  void setSelectedPenColor(const QColor &c) { selectedPenColor_ = c; }

  const QColor &selectedBrushColor() const { return selectedBrushColor_; }
  void setSelectedBrushColor(const QColor &c) { selectedBrushColor_ = c; }

  bool isShowBBox() const { return showBBox_; }
  void setShowBBox(bool b) { showBBox_ = b; }

  const QColor &bboxPenColor() const { return bboxPenColor_; }
  void setBBoxPenColor(const QColor &c) { bboxPenColor_ = c; }

  //---

  bool load(const std::string &filename);

  void setPixelSize(int w, int h);

  void draw(QPainter *painter, bool update);

  void zoom(bool zoomIn);
  void scroll(double dx, double dy);
  void zoomTo(const CBBox2D &bbox);

  void mousePress(const QPoint &pos);
  void mouseMove(const QPoint &pos);

  void nextGeomShape();

  void selectInsideObject();

  void getTimeFrame(CLottieUtil::TimeFrame &timeFrame) const;

  void updateAll();

  //---

  void setTicks(int t);

  //---

  bool isShowTimeLine() const;
  bool isShowPath() const;

  //---

  CLottieAsset *getPrecompLayerAsset(const CLottieLayer *layer) const;
  CLottieAsset *getLayerAsset(const CLottieLayer *layer) const;

  QImage getAssetImage(CLottieAsset *asset, bool create=true) const;

  //---

  void toQPath(const CBezierPath &bezierPath, QPainterPath &path) const;
  void fromQPath(const QPainterPath &path, CBezierPath &bezierPath) const;

  QTransform toQTransform(const CMatrix2D &m) const;

 public Q_SLOTS:
  void loadSlot();

  void playSlot();
  void pauseSlot();
  void stepSlot();

  void zoomFull();

  void setShowTimeLine(bool b);
  void setShowPath(bool b);

 private Q_SLOTS:
  void tickSlot();

 private:
  void updateAnim();

 private:
  using OptInt   = std::optional<int>;
  using OptReal  = std::optional<double>;
  using OptColor = std::optional<CRGBA>;

  using Layers = std::vector<CLottieLayer *>;

  struct ObjectState {
    CLottieObject*              object { nullptr };
    std::deque<CLottieObject *> siblings;

    ObjectState() { }

    ObjectState(CLottieObject *o) { object = o; }
  };

  struct PathData {
    QTransform           transform;
    QPainterPath         path;
    QPen                 pen;
    QBrush               brush;
    QPainterPathStroker* stroker { nullptr };
    CMatrixStack2D       smatrix;
  };

  using PathDatas = std::vector<PathData>;

  struct MaskData {
    CLottieLayer::Mask* mask { nullptr };
    PathDatas           paths;
  };

  struct DrawState {
    using TimeFrame = CLottieUtil::TimeFrame;
    using Paths     = std::vector<CBezierPath>;

    // current fill (TODO: remove)
    struct Fill {
      const CLottieShape* shape { nullptr };

      OptColor color;
      OptReal  opacity;
      int      rule  { 1 };
    };

    // current stroke shape
    struct Stroke {
      const CLottieShape* shape { nullptr };
    };

    // current gradient shape
    struct Gradient {
      const CLottieShape* shape { nullptr };
    };

    // current trim paths
    struct Trim {
      const CLottieShape* shape  { nullptr };

      PathDatas paths;
    };

    // current merge paths
    struct Merge {
      const CLottieShape* shape { nullptr };

      Paths paths;
    };

    // current rounded
    struct Rounded {
      const CLottieShape* shape { nullptr };
    };

    //---

    QPainter *painter { nullptr };

    CQLottieLayer *layer { nullptr };

    TimeFrame timeFrame;

    //---

    Fill     fill;
    Stroke   stroke;
    Gradient fillGradient;
    Gradient strokeGradient;
    Trim     trim;
    Merge    merge;
    Rounded  rounded;

    MaskData* maskData { nullptr };

    //---

    CDisplayRange2D displayRange;

    std::deque<ObjectState> objects;

    int frameDelta { 0 };

    QPainterPathStroker *stroker { nullptr };

    CMatrix2D getDisplayMatrix() const { return displayRange.getMatrix(); }
  };

  struct PenBrush {
    QPen   pen;
    QBrush brush;

    QPainterPathStroker *stroker { nullptr };

    OptReal   strokeWidth;
    OptInt    strokeLineCap;
    OptInt    strokeLineJoin;
    CLineDash strokeLineDash;
    OptReal   strokeMiterLimit;
  };

  struct RectData {
    QRectF rect;
  };

 private:
  void printAssetLayers() const;

  void drawRoot(const DrawState &state, const CLottieRoot *root, bool update);

  void drawChildLayers(const DrawState &drawState, const Layers &childLayers, bool update);

  void drawLayer(const DrawState &state, CLottieLayer *layer, bool update);
  void drawShape(DrawState &state, CLottieShape *shape);

  void maskLayer(const DrawState &drawState, MaskData *maskData, CLottieLayer *layer);

//void gradientFillShape  (DrawState &state, const CLottieShape *shape);
//void gradientStrokeShape(DrawState &state, const CLottieShape *shape);

  void drawLayerShapes(DrawState &drawState, const CLottieLayer *layer);

  void drawMergeShapes(DrawState &drawState, const CLottieShape *mergeShape);
  void drawTrimShapes (DrawState &drawState, const CLottieShape *trimShape);

  void drawAsset(const DrawState &drawState, CLottieAsset *asset);

  void drawPrecompLayer(const DrawState &drawState, CLottieLayer *layer);

  void drawSolidLayer(const DrawState &drawState, const CLottieLayer *layer);

  void drawImageLayer(const DrawState &drawState, const CLottieLayer *layer);

  void drawEllipse  (DrawState &state, const CLottieShape *shape);
  void drawPath     (DrawState &state, const CLottieShape *shape);
  void drawPolystar (DrawState &state, const CLottieShape *shape);
  void drawRectangle(DrawState &state, const CLottieShape *shape);

  void drawBezierPath(DrawState &drawState, const CLottieShape *shape, CBezierPath &bezierPath);

  CBezierPath trimPath(DrawState &drawState, const CLottieShape *trimShape,
                       const CBezierPath &bezierPath) const;

  CLottieShape *getDrawMergeShape(const DrawState &drawState) const;
  CLottieShape *getDrawTrimShape (const DrawState &drawState) const;

  QPainterPath drawPathData(QPainter *painter, const PathData &pathData) const;
  QPainterPath drawPathDataPath(QPainter *painter, const PathData &pathData,
                                const QPainterPath &ppath) const;

  void addSelectedRect(QPainter *painter, const QRectF &rect);
  void addSelectedPath(QPainter *painter, const QPainterPath &path);

  void addBBoxRect(QPainter *painter, const QRectF &rect);

 public:
  QGradient calcGradientFill(const TimeFrame &timeFrame,
                             CLottieShape::GradientFill *gradientFill) const;

  QGradient calcGradientStroke(const TimeFrame &timeFrame,
                               CLottieShape::GradientStroke *gradientStroke) const;

  CBezierPath getEllipsePath(const TimeFrame &timeFrame, const CLottieShape *shape) const;
  CBezierPath getPolyStarPath(const TimeFrame &timeFrame, const CLottieShape *shape) const;
  CBezierPath getRectanglePath(const TimeFrame &timeFrame, const CLottieShape *shape) const;

 private:
  void setPathData(const DrawState &drawState, const QPainterPath &path,
                   const CMatrixStack2D &smatrix, PathData &pathData) const;

  void pathToBezier(const CLottie::BezierProperty &path, const DrawState &drawState,
                    CBezierPath &bezierPath) const;

  void setPenBrush(DrawState &drawState, const CLottieShape *shape);

  void calcPenBrush(const DrawState &drawState, const CLottieShape *shape,
                    PenBrush &penBrush) const;

  QPainterPathStroker *makeStroker() const;

  void setSelectedPenBrush(QPainter *painter);
  void setBBoxPenBrush(QPainter *painter);

  CMatrixStack2D getLayerMatrix(const DrawState &drawState, const CLottieLayer *layer) const;
  CMatrixStack2D getShapeMatrix(const DrawState &drawState, const CLottieShape *shape) const;

  CMatrixStack2D calcRepeatMatrix(const DrawState &drawState, CLottieRepeater *repeater) const;

  CMatrixStack2D getLayerTransformMatrix(const DrawState &drawState,
                                         const CLottieLayer *layer) const;
  CMatrixStack2D getShapeTransformMatrix(const DrawState &drawState,
                                         const CLottieShape *shape) const;

  OptColor getHierFillColor(const DrawState &drawState, const CLottieShape *drawShape,
                            const OptColor &def=OptColor()) const;
  OptReal getHierFillOpacity(const DrawState &drawState, const CLottieShape *shape,
                             const OptReal &def) const;

  OptColor getFillColor(const TimeFrame &timeFrame, const CLottieShape *shape,
                        const OptColor &def=OptColor()) const;
  OptReal getFillOpacity(const TimeFrame &timeFrame, const CLottieShape *shape,
                         const OptReal &def=OptReal()) const;

  CLottieShape *getDrawFillShape(const DrawState &drawState) const;

  OptReal getHierLayerOpacity(const DrawState &drawState, const CLottieLayer *layer,
                              const OptReal &def=OptReal()) const;
  OptReal getLayerOpacity(const DrawState &drawState, const CLottieLayer *layer,
                          const OptReal &def=OptReal()) const;

  OptColor getHierStrokeColor(const DrawState &drawState, const CLottieShape *shape,
                              const OptColor &def=OptColor()) const;
  OptReal getHierStrokeOpacity(const DrawState &drawState, const CLottieShape *shape,
                               const OptReal &def=OptReal()) const;

  CLottieShape *getDrawStrokeShape(const DrawState &drawState) const;

  OptColor getStrokeColor(const TimeFrame &timeFrame, const CLottieShape *shape,
                          const OptColor &def=OptColor()) const;
  OptReal getStrokeOpacity(const TimeFrame &timeFrame, const CLottieShape *shape,
                           const OptReal &def) const;

  double getRepeatOpacity(const DrawState &drawState, CLottieRepeater *repeater) const;

  QImage matteLayerImage(CQLottieLayer *layer, CQLottieLayer *clipLayer, int matteMode) const;

  CLottieShape *getDrawGradientFillShape  (const DrawState &drawState) const;
  CLottieShape *getDrawGradientStrokeShape(const DrawState &drawState) const;

  std::string hierName(CLottieObject *object, const DrawState &drawState) const;

  void drawPainterPath(QPainter *painter, const QPainterPath &path) const;

 private:
  CQLottieToolBar*    toolbar_     { nullptr };
  CQLottieSettings*   settings_    { nullptr };
  CQLottieCanvas*     canvas_      { nullptr };
  CQLottieStatusBar*  status_      { nullptr };
  CQLottieTimeLine*   timeLine_    { nullptr };
  CQLottiePath*       path_        { nullptr };
  CQLottieTree*       tree_        { nullptr };
  CQLottieObjectTree* objectTree_  { nullptr };

  CLottie* lottie_ { nullptr };

  CDisplayRange2D displayRange_;

  bool doubleBuffer_ { false };
  bool equalScale_   { true };

  bool running_ { false };

  int w_ { 100 };
  int h_ { 100 };

  double fps_ { 30.0 };
  double dt_  { 0.0 };

  QTimer* timer_ { nullptr };
  uint    ticks_ { 0 };
  double  secs_  { 0.0 };
  uint    isecs_ { 0 };

  struct ImageData {
    QImage image;
    int    width  { 0 };
    int    height { 0 };
  };

  using AssetImage = std::map<std::string, ImageData>;

  AssetImage assetImage_;

  QColor bgColor_            { Qt::white };
  bool   showSelect_         { true };
  QColor selectedPenColor_   { Qt::red };
  QColor selectedBrushColor_ { Qt::white };
  bool   showBBox_           { false };
  QColor bboxPenColor_       { Qt::red };

  CLottieObject *insideObject_ { nullptr };

  std::vector<PathData> selectedPaths_;
  std::vector<RectData> bboxRects_;

  mutable std::vector<QPainterPathStroker *> strokers_;

  using AssetLayers = std::map<CLottieAsset *, Layers>;

  AssetLayers assetLayers_;
};

//---

class CQLottieAsset : public CLottieAsset {
 public:
  CQLottieAsset(CQLottie *lottie) :
   CLottieAsset(lottie->lottie()), lottie_(lottie) {
  }

 private:
  CQLottie *lottie_ { nullptr };
};

//---

class CQLottieLayer : public CLottieLayer {
 public:
  struct ImagePainter {
    QImage    image;
    QPainter* painter { nullptr };
  };

 public:
  CQLottieLayer(CQLottie *lottie);
 ~CQLottieLayer() override;

  bool isEnabled() const { return enabled_; }
  void setEnabled(bool b) { enabled_ = b; }

  bool isChanged() const { return changed_; }
  void setChanged(bool b) { changed_ = b; }

  bool isDoubleBuffer() const { return doubleBuffer_; }
  void setDoubleBuffer(bool b) { doubleBuffer_ = b; }

  int width () const { return w_; }
  int height() const { return h_; }

  CQLottieLayer *matteTargetLayer() const { return matteTargetLayer_; }
  void setMatteTargetLayer(CQLottieLayer *l) { matteTargetLayer_ = l; }

  CQLottieLayer *matteModeLayer() const { return matteModeLayer_; }
  void setMatteModeLayer(CQLottieLayer *l) { matteModeLayer_ = l; }

  const QImage &matteImage() const { return matteImage_; }
  void setMatteImage(const QImage &v) { matteImage_ = v; }

  const QImage &effectImage() const { return effectImage_; }
  void setEffectImage(const QImage &v) { effectImage_ = v; }

  const std::string &hierName() const { return hierName_; }
  void setHierName(const std::string &s) { hierName_ = s; }

  void resize(int w, int h);

  QPainter *painter();

  void createImagePainter(ImagePainter &imagePainter) const;

  const QImage &image() const { return imagePainter_.image; }
  void setImage(const QImage &i) { imagePainter_.image = i; }

  void clear();

 private:
  CQLottie *lottie_ { nullptr };

  int w_ { 0 };
  int h_ { 0 };

  bool enabled_      { true };
  bool changed_      { false };
  bool doubleBuffer_ { false };

  CQLottieLayer* matteTargetLayer_ { nullptr };
  CQLottieLayer* matteModeLayer_   { nullptr };
  QImage         matteImage_;

  QImage effectImage_;

  ImagePainter imagePainter_;

  std::string hierName_;
};

//---

class CQLottieShape : public CLottieShape {
 public:
  CQLottieShape(CQLottie *lottie) :
   CLottieShape(lottie->lottie()), lottie_(lottie) {
  }

 private:
  CQLottie *lottie_ { nullptr };
};

#endif
