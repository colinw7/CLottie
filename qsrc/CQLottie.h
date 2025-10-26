#ifndef CQLottie_H
#define CQLottie_H

#include <CLottie.h>
#include <CDisplayRange2D.h>

#include <QFrame>
#include <QItemDelegate>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class CQLottieCanvas;
class CQLottieTree;
class CQLottieObjectTree;
class CBezierPath;

class QLabel;
class QTimer;

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

  bool load(const std::string &filename);

  void setPixelSize(int w, int h);

  void draw(QPainter *painter);

  void zoom(bool zoomIn);
  void scroll(double dx, double dy);

  void mouseMove(const QPoint &pos);

  void getTimeFrame(CLottieUtil::TimeFrame &timeFrame) const;

 private Q_SLOTS:
  void loadSlot();

  void playSlot();
  void pauseSlot();
  void stepSlot();

  void tickSlot();

 private:
  using OptInt   = std::optional<int>;
  using OptReal  = std::optional<double>;
  using OptColor = std::optional<CRGBA>;

  struct DrawState {
    using TimeFrame = CLottieUtil::TimeFrame;

    TimeFrame timeFrame;

    CMatrix2D preMatrix { CMatrix2D::identity() };
    CMatrix2D matrix    { CMatrix2D::identity() };

    struct Fill {
      const CLottieShape* shape { nullptr };

      OptColor color;
      OptReal  opacity;
      int      rule  { 1 };
    };

    Fill fill;

    struct Stroke {
      const CLottieShape* shape { nullptr };

      OptColor color;
      OptReal  opacity;
      OptReal  width;
      OptInt   lineCap;
      OptInt   lineJoin;
      OptReal  miterLimit;
    };

    Stroke stroke;

    struct Gradient {
      bool            enabled { false };
      QLinearGradient gradient;
      OptReal         opacity;
      OptReal         width;
      OptInt          lineCap;
      OptInt          lineJoin;
      OptReal         miterLimit;
    };

    Gradient fillGradient;
    Gradient strokeGradient;

    struct Transform {
      using Shapes = std::vector<const CLottieShape *>;

      Shapes shapes;
    };

    Transform transform;

    struct Trim {
      const CLottieShape* shape  { nullptr };
      double              start  { 0.0 };
      double              end    { 100.0 };
      double              offset { 0.0 };
      int                 mult   { 1 };
    };

    std::optional<Trim> trim;

    using Paths = std::vector<QPainterPath>;

    struct Merge {
      const CLottieShape* shape { nullptr };
      int                 mode  { 0 };
      Paths               paths;
    };

    std::optional<Merge> merge;

    struct Repeat {
      int                 copies       { 0 };
      double              offset       { 0.0 };
      int                 composite    { 0 };
      CLottie::Transform* transform    { nullptr };
      double              startOpacity { 1.0 };
      double              endOpacity   { 1.0 };
    };

    std::optional<Repeat> repeat;
  };

  void drawRoot (QPainter *painter, const CLottieRoot *root);
  void drawLayer(QPainter *painter, const DrawState &state, const CLottieLayer *layer);
  void drawShape(QPainter *painter, DrawState &state, const CLottieShape *shape);

  void gradientFill  (DrawState &state, const CLottieShape *shape);
  void gradientStroke(DrawState &state, const CLottieShape *shape);

  void drawLayerShapes(QPainter *painter, const DrawState &drawState, const CLottieLayer *layer);

  void drawMergeShapes(QPainter *painter, const DrawState &drawState);

  void drawAsset(QPainter *painter, const DrawState &drawState, const CLottieAsset *asset);

  void drawPrecompLayer(QPainter *painter, const DrawState &drawState, const CLottieLayer *layer);

  void drawSolidLayer(QPainter *painter, const DrawState &drawState, const CLottieLayer *layer);

  void drawImageLayer(QPainter *painter, const DrawState &drawState, const CLottieLayer *layer);

  void drawEllipse  (QPainter *painter, DrawState &state, const CLottieShape *shape);
  void drawPath     (QPainter *painter, DrawState &state, const CLottieShape *shape);
  void drawPolystar (QPainter *painter, DrawState &state, const CLottieShape *shape);
  void drawRectangle(QPainter *painter, DrawState &state, const CLottieShape *shape);

  void drawBezierPath(QPainter *painter, DrawState &drawState, const CLottieShape *shape,
                      CBezierPath &bezierPath);

  void pathToBezier(const CLottie::BezierProperty &path, const DrawState &drawState,
                    CBezierPath &bezierPath) const;

  void setPenBrush(QPainter *painter, const DrawState &drawState, const CLottieShape *shape);

  void setSelectedPenBrush(QPainter *painter);
  void setBBoxPenBrush(QPainter *painter);

  CMatrix2D getLayerMatrix(const DrawState &drawState, const CLottieLayer *layer) const;
  CMatrix2D getShapeMatrix(const DrawState &drawState, const CLottieShape *shape) const;

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

 private:
  QFrame*             toolbar_     { nullptr };
  CQLottieCanvas*     canvas_      { nullptr };
  QFrame*             status_      { nullptr };
  QLabel*             statusLabel_ { nullptr };
  QLabel*             ticksLabel_  { nullptr };
  CQLottieTree*       tree_        { nullptr };
  CQLottieObjectTree* objectTree_  { nullptr };

  CLottie* lottie_ { nullptr };

  CDisplayRange2D displayRange_;

  bool running_ { false };

  double fps_ { 30.0 };
  double dt_  { 0.0 };

  QTimer* timer_ { nullptr };
  uint    ticks_ { 0 };
  double  secs_  { 0.0 };
  uint    isecs_ { 0 };

  using AssetImage = std::map<std::string, QImage>;

  AssetImage assetImage_;
};

//---

class CQLottieCanvas : public QWidget {
  Q_OBJECT

 public:
  CQLottieCanvas(CQLottie *lottie);

  void resizeEvent(QResizeEvent *) override;

  void paintEvent(QPaintEvent *) override;

  void mouseMoveEvent(QMouseEvent *) override;

  void keyPressEvent(QKeyEvent *) override;

  QSize sizeHint() const override { return QSize(1600, 1600); }

 private:
  CQLottie* lottie_ { nullptr };
};

//---

class CQLottieTreeWidget;

class CQLottieTree : public QFrame {
  Q_OBJECT

 public:
  CQLottieTree(CQLottie *lottie);

  CQLottie *lottie() const { return lottie_; }

  CQLottieTreeWidget *tree() const { return tree_; }

  void resizeEvent(QResizeEvent *) override;

  void load();

  QSize sizeHint() const override { return QSize(600, 1600); }

  QTreeWidgetItem *itemFromIndex(const QModelIndex &index) const;

 private:
  void connectSlots(bool b);

  QTreeWidgetItem *createRootItem  (CLottieRoot   *root  );
  QTreeWidgetItem *createAssetItem (CLottieAsset  *asset );
  QTreeWidgetItem *createLayerItem (CLottieLayer  *layer );
  QTreeWidgetItem *createShapeItem (CLottieShape  *layer );
  QTreeWidgetItem *createEffectItem(CLottieEffect *effect);

 private Q_SLOTS:
  void itemClickedSlot (QTreeWidgetItem *item, int column);
  void itemSelectedSlot(QTreeWidgetItem *, QTreeWidgetItem *);

  void customContextMenuSlot(const QPoint &pos);

  void expandAll  (const QModelIndex &ind=QModelIndex());
  void collapseAll(const QModelIndex &ind=QModelIndex());

  void transformSlot();
  void hierTransformSlot();
  void printSlot();

 private:
  CQLottie*           lottie_ { nullptr };
  CQLottieTreeWidget* tree_   { nullptr };

  using AssetItem  = std::map<CLottieAsset  *, QTreeWidgetItem *>;
  using LayerItem  = std::map<CLottieLayer  *, QTreeWidgetItem *>;
  using ShapeItem  = std::map<CLottieShape  *, QTreeWidgetItem *>;
  using EffectItem = std::map<CLottieEffect *, QTreeWidgetItem *>;

  QTreeWidgetItem *rootItem_ { nullptr };

  AssetItem  assetItem_;
  LayerItem  layerItem_;
  ShapeItem  shapeItem_;
  EffectItem effectItem_;
};

//---

class CQLottieTreeWidget : public QTreeWidget {
  Q_OBJECT

 public:
  CQLottieTreeWidget(CQLottieTree *tree);

  QModelIndex indexFromItem(const QTreeWidgetItem *item, int column=0) const {
    return QTreeWidget::indexFromItem(item, column);
  }

 private:
  CQLottieTree* tree_ { nullptr };
};

//---

class CQLottieTreeDelegate : public QItemDelegate {
  Q_OBJECT

 public:
  CQLottieTreeDelegate(CQLottieTree *lottie);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const override;

  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;

 private:
  CQLottieTree* tree_ { nullptr };
};

//---

class CQLottieTreeObjectItem : public QTreeWidgetItem {
 public:
  CQLottieTreeObjectItem(QTreeWidget *parent, CLottieObject *object);
  CQLottieTreeObjectItem(QTreeWidgetItem *parent, CLottieObject *object);

  virtual ~CQLottieTreeObjectItem() { }

  CLottieObject *object() const { return object_; }

 private:
  CLottieObject* object_ { nullptr };
};

class CQLottieTreeRootItem : public CQLottieTreeObjectItem {
 public:
  CQLottieTreeRootItem(QTreeWidget *parent, CLottieRoot *root);

  CLottieRoot *root() const { return root_; }

 private:
  CLottieRoot* root_ { nullptr };
};

class CQLottieTreeAssetItem : public CQLottieTreeObjectItem {
 public:
  CQLottieTreeAssetItem(QTreeWidgetItem *parent, CLottieAsset *asset);

  CLottieAsset *asset() const { return asset_; }

 private:
  CLottieAsset* asset_ { nullptr };
};

class CQLottieTreeLayerItem : public CQLottieTreeObjectItem {
 public:
  CQLottieTreeLayerItem(QTreeWidgetItem *parent, CLottieLayer *layer);

  CLottieLayer *layer() const { return layer_; }

 private:
  CLottieLayer* layer_ { nullptr };
};

class CQLottieTreeShapeItem : public CQLottieTreeObjectItem {
 public:
  CQLottieTreeShapeItem(QTreeWidgetItem *parent, CLottieShape *shape);

  CLottieShape *shape() const { return shape_; }

 private:
  CLottieShape* shape_ { nullptr };
};

class CQLottieTreeEffectItem : public CQLottieTreeObjectItem {
 public:
  CQLottieTreeEffectItem(QTreeWidgetItem *parent, CLottieEffect *effect);

  CLottieEffect *effect() const { return effect_; }

 private:
  CLottieEffect* effect_ { nullptr };
};

class CQLottieTreeValueItem : public QTreeWidgetItem {
 public:
  enum class Type {
    NONE,
    BOOL,
    INTEGER,
    REAL,
    STRING,
    COLOR,
    POSITION,
    SIZE,
    SCALAR
  };

 public:
  CQLottieTreeValueItem(QTreeWidgetItem *parent, CLottieObject *object,
                        const QString &propName, const Type &propType);
  virtual ~CQLottieTreeValueItem() { }

  const QString& propName() const { return propName_; }
  const Type&    propType() const { return propType_; }

  CLottieObject *object() const { return object_; }

 protected:
  CLottieObject* object_ { nullptr };

  QString propName_;
  Type    propType_ { Type::NONE };
};

class CQLottieTreeRootValueItem : public CQLottieTreeValueItem {
 public:
  CQLottieTreeRootValueItem(QTreeWidgetItem *parent, CLottieRoot *root,
                            const QString &propName, const Type &propType);

  CLottieRoot *root() const { return root_; }

 private:
  CLottieRoot* root_ { nullptr };
};

class CQLottieTreeAssetValueItem : public CQLottieTreeValueItem {
 public:
  CQLottieTreeAssetValueItem(QTreeWidgetItem *parent, CLottieAsset *asset,
                             const QString &propName, const Type &propType);

  CLottieAsset *asset() const { return asset_; }

 private:
  CLottieAsset* asset_ { nullptr };
};

class CQLottieTreeLayerValueItem : public CQLottieTreeValueItem {
 public:
  CQLottieTreeLayerValueItem(QTreeWidgetItem *parent, CLottieLayer *layer,
                             const QString &propName, const Type &propType);

  CLottieLayer *layer() const { return layer_; }

 private:
  CLottieLayer* layer_ { nullptr };
};

class CQLottieTreeShapeValueItem : public CQLottieTreeValueItem {
 public:
  CQLottieTreeShapeValueItem(QTreeWidgetItem *parent, CLottieShape *shape,
                             const QString &propName, const Type &propType);

  CLottieShape *shape() const { return shape_; }

 private:
  CLottieShape* shape_ { nullptr };
};

class CQLottieTreeEffectValueItem : public CQLottieTreeValueItem {
 public:
  CQLottieTreeEffectValueItem(QTreeWidgetItem *parent, CLottieEffect *effect,
                              const QString &propName, const Type &propType);

  CLottieEffect *effect() const { return effect_; }

 private:
  CLottieEffect* effect_ { nullptr };
};

//---

class CQLottieObjectTreeWidget;

class CQLottieObjectTree : public QFrame {
  Q_OBJECT

 public:
  CQLottieObjectTree(CQLottie *lottie);

  CQLottie *lottie() const { return lottie_; }

  CQLottieObjectTreeWidget *tree() const { return tree_; }

  void setObject(CLottieObject *object);

  QSize sizeHint() const override { return QSize(600, 1600); }

  QTreeWidgetItem *itemFromIndex(const QModelIndex &index) const;

 private:
  void load();

  void connectSlots(bool b);

 private Q_SLOTS:
  void itemClickedSlot (QTreeWidgetItem *item, int column);
  void itemSelectedSlot(QTreeWidgetItem *, QTreeWidgetItem *);

  void customContextMenuSlot(const QPoint &pos);

  void expandAll  (const QModelIndex &ind=QModelIndex());
  void collapseAll(const QModelIndex &ind=QModelIndex());

  void printSlot();

 private:
  CQLottie*                 lottie_ { nullptr };
  CQLottieObjectTreeWidget* tree_   { nullptr };
  CLottieObject*            object_ { nullptr };
};

//---

class CQLottieObjectTreeWidget : public QTreeWidget {
  Q_OBJECT

 public:
  CQLottieObjectTreeWidget(CQLottieObjectTree *tree);

  QModelIndex indexFromItem(const QTreeWidgetItem *item, int column=0) const {
    return QTreeWidget::indexFromItem(item, column);
  }

 private:
  CQLottieObjectTree* tree_ { nullptr };
};

//---

class CQLottieObjectTreeDelegate : public QItemDelegate {
  Q_OBJECT

 public:
  CQLottieObjectTreeDelegate(CQLottieObjectTree *lottie);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const override;

  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;

  void drawChecked(QPainter *painter, const QStyleOptionViewItem &option,
                   bool checked, const QModelIndex &index) const;
  void drawColor(QPainter *painter, const QStyleOptionViewItem &option,
                 const QColor &c, const QModelIndex &index) const;
  void drawString(QPainter *painter, const QStyleOptionViewItem &option,
                  const QString &str, const QModelIndex &index) const;

 private Q_SLOTS:
  void updateValue();

 private:
  CQLottieObjectTree* tree_ { nullptr };

  mutable QModelIndex editIndex_;
};

//---

#endif
