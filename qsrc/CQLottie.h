#ifndef CQLottie_H
#define CQLottie_H

#include <CLottie.h>
#include <CDisplayRange2D.h>

#include <QFrame>

#include <deque>

class CQLottieCanvas;
class CQLottieToolBar;
class CQLottieSettings;
class CQLottieTree;
class CQLottieLayer;
class CQLottieObjectTree;

class CQColorEdit;
class CBezierPath;

class QLabel;
class QTimer;
class QPainterPathStroker;

class CQLottie : public QWidget {
  Q_OBJECT

 public:
  CQLottie();

  CLottie *lottie() const { return lottie_; }

  CQLottieCanvas *canvas() const { return canvas_; }

  CQLottieTree *tree() const { return tree_; }

  CQLottieObjectTree *objectTree() const { return objectTree_; }

  void setDebug(bool b);
  void setPrint(bool b);

  bool isDoubleBuffer() const { return doubleBuffer_; }
  void setDoubleBuffer(bool b) { doubleBuffer_ = b; }

  bool load(const std::string &filename);

  void setPixelSize(int w, int h);

  void draw(QPainter *painter, bool update);

  void zoom(bool zoomIn);
  void scroll(double dx, double dy);
  void zoomFull();

  void mouseMove(const QPoint &pos);

  void getTimeFrame(CLottieUtil::TimeFrame &timeFrame) const;

  const QColor &bgColor() const { return bgColor_; }
  void setBgColor(const QColor &c) { bgColor_ = c; }

  const QColor &selectedPenColor() const { return selectedPenColor_; }
  void setSelectedPenColor(const QColor &c) { selectedPenColor_ = c; }

  const QColor &selectedBrushColor() const { return selectedBrushColor_; }
  void setSelectedBrushColor(const QColor &c) { selectedBrushColor_ = c; }

  const QColor &bboxPenColor() const { return bboxPenColor_; }
  void setBBoxPenColor(const QColor &c) { bboxPenColor_ = c; }

  void updateAll();

 public Q_SLOTS:
  void loadSlot();

  void playSlot();
  void pauseSlot();
  void stepSlot();

 private Q_SLOTS:
  void tickSlot();

 private:
  using OptInt   = std::optional<int>;
  using OptReal  = std::optional<double>;
  using OptColor = std::optional<CRGBA>;

  using Layers = std::vector<CLottieLayer *>;

  struct DrawState {
    using Paths = std::vector<CBezierPath>;

    struct Fill {
      const CLottieShape* shape { nullptr };

      OptColor color;
      OptReal  opacity;
      int      rule  { 1 };
    };

    struct Stroke {
      const CLottieShape* shape { nullptr };

      OptColor color;
      OptReal  opacity;
      OptReal  width;
      OptInt   lineCap;
      OptInt   lineJoin;
      OptReal  miterLimit;
    };

    struct Gradient {
      bool            enabled { false };
      QLinearGradient gradient;
      OptReal         opacity;
      OptReal         width;
      OptInt          lineCap;
      OptInt          lineJoin;
      OptReal         miterLimit;
    };

    struct Transform {
      using Shapes = std::vector<const CLottieShape *>;

      Shapes shapes;
    };

    struct Trim {
      const CLottieShape* shape  { nullptr };
      double              start  { 0.0 };
      double              end    { 100.0 };
      double              offset { 0.0 };
      int                 mult   { 1 };
    };

    struct Merge {
      const CLottieShape* shape { nullptr };
      int                 mode  { 0 };
      Paths               paths;
    };

#if 0
    struct Repeat {
      int                 copies       { 0 };
      double              offset       { 0.0 };
      int                 composite    { 0 };
      CLottie::Transform* transform    { nullptr };
      double              startOpacity { 1.0 };
      double              endOpacity   { 1.0 };
    };
#endif

    //---

    QPainter *painter { nullptr };

    CQLottieLayer *layer { nullptr };

    using TimeFrame = CLottieUtil::TimeFrame;

    TimeFrame timeFrame;

    CMatrix2D matrix { CMatrix2D::identity() };

    Fill fill;

    Stroke stroke;

    //---

#if 0
    std::vector<CDisplayRange2D> displayRanges;
#else
    CDisplayRange2D displayRange;
#endif

    Transform transform;

    Gradient fillGradient;
    Gradient strokeGradient;

    std::optional<Trim>  trim;
    std::optional<Merge> merge;

    std::deque<CLottieObject *> objects;

#if 0
    std::optional<Repeat> repeat;
#endif

    int frameDelta { 0 };

    OptInt    repeatInd;
//  OptReal   repeatOpacity;
//  CMatrix2D repeatMatrix;

    QPainterPathStroker *stroker { nullptr };

    CMatrix2D getDisplayMatrix() const {
#if 0
      CMatrix2D pmatrix = CMatrix2D::identity();

      for (const auto &displayRange : displayRanges)
        pmatrix = pmatrix*displayRange.getMatrix();

      return pmatrix;
#else
      return displayRange.getMatrix();
#endif
    }
  };

  void drawRoot(const DrawState &state, const CLottieRoot *root, bool update);

  void drawChildLayers(const DrawState &drawState, const Layers &childLayers, bool update);

  void drawLayer(const DrawState &state, CLottieLayer *layer, bool update);
  void drawShape(DrawState &state, CLottieShape *shape);

  void gradientFillShape  (DrawState &state, const CLottieShape *shape);
  void gradientStrokeShape(DrawState &state, const CLottieShape *shape);

  void drawLayerShapes(DrawState &drawState, const CLottieLayer *layer);

  void drawMergeShapes(DrawState &drawState);

  void drawAsset(const DrawState &drawState, CLottieAsset *asset);

  void drawPrecompLayer(const DrawState &drawState, const CLottieLayer *layer);

  void drawSolidLayer(const DrawState &drawState, const CLottieLayer *layer);

  void drawImageLayer(const DrawState &drawState, const CLottieLayer *layer);

  void drawEllipse  (DrawState &state, const CLottieShape *shape);
  void drawPath     (DrawState &state, const CLottieShape *shape);
  void drawPolystar (DrawState &state, const CLottieShape *shape);
  void drawRectangle(DrawState &state, const CLottieShape *shape);

  void drawBezierPath(DrawState &drawState, const CLottieShape *shape, CBezierPath &bezierPath);

  void pathToBezier(const CLottie::BezierProperty &path, const DrawState &drawState,
                    CBezierPath &bezierPath) const;

  void setPenBrush(DrawState &drawState, const CLottieShape *shape);

  void setSelectedPenBrush(QPainter *painter);
  void setBBoxPenBrush(QPainter *painter);

  CMatrix2D getLayerMatrix(const DrawState &drawState, const CLottieLayer *layer) const;
  CMatrix2D getShapeMatrix(const DrawState &drawState, const CLottieShape *shape) const;

  CMatrix2D calcRepeatMatrix(const DrawState &drawState, CLottieRepeater *repeater) const;

  CMatrix2D getTransformMatrix(const DrawState &drawState, CLottie::Transform *transform) const;

  OptColor getFillColor(const DrawState &drawState, const CLottieShape *shape,
                       const OptColor &def=OptColor()) const;
  OptReal getFillOpacity(const DrawState &drawState, const CLottieShape *shape,
                         const OptReal &def) const;

  OptReal getLayerOpacity(const DrawState &drawState, const CLottieLayer *layer,
                          const OptReal &def) const;

  OptColor getStrokeColor(const DrawState &drawState, const CLottieShape *shape,
                          const OptColor &def=OptColor()) const;
  OptReal getStrokeOpacity(const DrawState &drawState, const CLottieShape *shape,
                           const OptReal &def) const;

  double getRepeatOpacity(const DrawState &drawState, CLottieRepeater *repeater) const;

  QImage matteLayerImage(CQLottieLayer *layer, CQLottieLayer *clipLayer) const;

 private:
  CQLottieToolBar*    toolbar_     { nullptr };
  CQLottieSettings*   settings_    { nullptr };
  CQLottieCanvas*     canvas_      { nullptr };
  QFrame*             status_      { nullptr };
  QLabel*             statusLabel_ { nullptr };
  QLabel*             ticksLabel_  { nullptr };
  CQLottieTree*       tree_        { nullptr };
  CQLottieObjectTree* objectTree_  { nullptr };

  CLottie* lottie_ { nullptr };

  CDisplayRange2D displayRange_;

  bool doubleBuffer_ { false };

  bool running_ { false };

  double fps_ { 30.0 };
  double dt_  { 0.0 };

  QTimer* timer_ { nullptr };
  uint    ticks_ { 0 };
  double  secs_  { 0.0 };
  uint    isecs_ { 0 };

  using AssetImage = std::map<std::string, QImage>;

  AssetImage assetImage_;

  QColor bgColor_            { Qt::white };
  QColor selectedPenColor_   { Qt::red };
  QColor selectedBrushColor_ { Qt::white };
  QColor bboxPenColor_       { Qt::red };
};

//---

class CQLottieCanvas : public QWidget {
  Q_OBJECT

 public:
  CQLottieCanvas(CQLottie *lottie);

  void invalidate();

  void resizeEvent(QResizeEvent *) override;

  void paintEvent(QPaintEvent *) override;

  void mouseMoveEvent(QMouseEvent *) override;

  void keyPressEvent(QKeyEvent *) override;

  QSize sizeHint() const override { return QSize(1600, 1600); }

 private:
  CQLottie* lottie_ { nullptr };

  bool needsUpdate_ { true };
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

  CQLottieLayer *matteLayer() const { return matteLayer_; }
  void setMatteLayer(CQLottieLayer *l) { matteLayer_ = l; }

  void resize(int w, int h);

  QPainter *painter();

  const QImage &image() const { return image_; }

  void clear();

 private:
  CQLottie *lottie_ { nullptr };

  int w_ { 0 };
  int h_ { 0 };

  bool enabled_      { true };
  bool changed_      { false };
  bool doubleBuffer_ { false };

  CQLottieLayer* matteLayer_ { nullptr };

  QImage    image_;
  QPainter* painter_ { nullptr };
};

class CQLottieShape : public CLottieShape {
 public:
  CQLottieShape(CQLottie *lottie) :
   CLottieShape(lottie->lottie()), lottie_(lottie) {
  }

 private:
  CQLottie *lottie_ { nullptr };
};

//---

class CQLottieToolBar : public QFrame {
  Q_OBJECT

 public:
  CQLottieToolBar(CQLottie *lottie);

 private Q_SLOTS:
  void loadSlot();

  void playSlot();
  void pauseSlot();
  void stepSlot();

 private:
  CQLottie *lottie_ { nullptr };
};

//---

class CQLottieSettings : public QFrame {
  Q_OBJECT

 public:
  CQLottieSettings(CQLottie *lottie);

 private Q_SLOTS:
  void bgFillSlot(const QColor &);
  void selectedFillSlot(const QColor &);
  void selectedStrokeSlot(const QColor &);
  void bboxStrokeSlot(const QColor &);

 private:
  void connectSlots(bool b);
  void updateWidgets();

 private:
  CQLottie *lottie_ { nullptr };

  CQColorEdit *bgFillEdit_       { nullptr };
  CQColorEdit *selectFillEdit_   { nullptr };
  CQColorEdit *selectStrokeEdit_ { nullptr };
  CQColorEdit *bboxStrokeEdit_   { nullptr };
};

#endif
