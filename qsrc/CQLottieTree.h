#ifndef CQLottieTree_H
#define CQLottieTree_H

#include <CQLottie.h>

#include <QItemDelegate>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class CQLottieTree;
class CQLottieObjectTree;
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

  void expandAll  (const QModelIndex &ind=QModelIndex());
  void collapseAll(const QModelIndex &ind=QModelIndex());

 private Q_SLOTS:
  void itemClickedSlot (QTreeWidgetItem *item, int column);
  void itemSelectedSlot(QTreeWidgetItem *, QTreeWidgetItem *);

  void customContextMenuSlot(const QPoint &pos);

  void expandAllSlot();
  void collapseAllSlot();

  void bboxSlot();
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
    SPLIT_POSITION,
    POSITION,
    SIZE,
    VECTOR,
    SCALAR,
    BEZIER
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
