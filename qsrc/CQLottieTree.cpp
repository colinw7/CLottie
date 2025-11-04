#include <CQLottieTree.h>
#include <CQLottieCanvas.h>
#include <CQLottieTimeLine.h>

#include <CQRealSpin.h>
#include <CQColorChooser.h>

#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QPainter>

//---

namespace {

QColor toQColor(const CRGBA &color) {
  return QColor(color.getRedI(), color.getGreenI(), color.getBlueI(), color.getAlphaI());
}

CRGBA toRGBA(const QColor &color) {
  return CRGBA(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

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

  auto *bboxButton          = new QPushButton("BBox");
  auto *transformButton     = new QPushButton("Transform");
  auto *hierTransformButton = new QPushButton("Hier Transform");
  auto *printButton         = new QPushButton("Print");

  connect(bboxButton, SIGNAL(clicked()), this, SLOT(bboxSlot()));
  connect(transformButton, SIGNAL(clicked()), this, SLOT(transformSlot()));
  connect(hierTransformButton, SIGNAL(clicked()), this, SLOT(hierTransformSlot()));
  connect(printButton, SIGNAL(clicked()), this, SLOT(printSlot()));

  controlLayout->addStretch(1);
  controlLayout->addWidget(bboxButton);
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

  layerItemMap_ .clear();
  assetItemMap_ .clear();
  shapeItemMap_ .clear();
  effectItemMap_.clear();

  //---

  auto *root = lottie_->lottie()->root();

  rootItem_ = new CQLottieTreeRootItem(tree_, root);

  tree_->addTopLevelItem(rootItem_);

  for (auto *asset : root->assets()) {
    auto *item = createAssetItem(asset);

    item->setData(0, Qt::UserRole, asset->ind().value_or(-1));
  }

#if 0
  for (auto *layer : root->layers()) {
    auto *item = createLayerItem(layer);

    item->setData(0, Qt::UserRole, layer->ind().value_or(-1));
  }
#else
  for (auto *layer : root->childLayers()) {
    auto *item = createLayerItem(layer);

    item->setData(0, Qt::UserRole, layer->ind().value_or(-1));
  }
#endif

  //---

  expandAll();

  //---

  connectSlots(true);
}

void
CQLottieTree::
selectObject(CLottieObject *object)
{
  QTreeWidgetItem *item = nullptr;

  for (const auto &pl : layerItemMap_) {
    if (pl.first == object)
      item = pl.second;
  }

  for (const auto &ps : shapeItemMap_) {
    if (ps.first == object)
      item = ps.second;
  }

  if (item)
    tree_->setCurrentItem(item, 0, QItemSelectionModel::Clear | QItemSelectionModel::Select);
}

QTreeWidgetItem *
CQLottieTree::
createLayerItem(CLottieLayer *layer)
{
  auto pl = layerItemMap_.find(layer);

  if (pl != layerItemMap_.end())
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

  layerItemMap_[layer] = item;

  //---

  auto *effect = layer->effect();

  if (effect) {
    auto *item = createEffectItem(effect);

    item->setData(0, Qt::UserRole, effect->ind().value_or(-1));
  }

  //---

  for (auto *layer1 : layer->childLayers()) {
    auto *item = createLayerItem(layer1);

    item->setData(0, Qt::UserRole, layer1->ind().value_or(-1));
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
  auto pa = assetItemMap_.find(asset);

  if (pa != assetItemMap_.end())
    return (*pa).second;

  auto name = QString::fromStdString(asset->name().value_or(""));

  auto *item = new CQLottieTreeAssetItem(rootItem_, asset);

  rootItem_->addChild(item);

  assetItemMap_[asset] = item;

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
  auto pl = shapeItemMap_.find(shape);

  if (pl != shapeItemMap_.end())
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

  shapeItemMap_[shape] = item;

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
  auto pe = effectItemMap_.find(effect);

  if (pe != effectItemMap_.end())
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

  effectItemMap_[effect] = item;

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

  lottie_->canvas()->invalidate();
}

void
CQLottieTree::
customContextMenuSlot(const QPoint &pos)
{
  auto *menu = new QMenu;

  auto *expandAction   = new QAction("Expand All"  , menu);
  auto *collapseAction = new QAction("Collapse All", menu);

  connect(expandAction  , SIGNAL(triggered()), this, SLOT(expandAllSlot()));
  connect(collapseAction, SIGNAL(triggered()), this, SLOT(collapseAllSlot()));

  menu->addAction(expandAction);
  menu->addAction(collapseAction);

  auto mpos = tree_->viewport()->mapToGlobal(pos);

  menu->exec(mpos);

  delete menu;
}

void
CQLottieTree::
expandAllSlot()
{
  auto selectedItems = tree_->selectedItems();

  QModelIndex ind;

  if (! selectedItems.empty())
    ind = tree_->indexFromItem(selectedItems[0], 0);

  expandAll(ind);
}

void
CQLottieTree::
collapseAllSlot()
{
  auto selectedItems = tree_->selectedItems();

  QModelIndex ind;

  if (! selectedItems.empty())
    ind = tree_->indexFromItem(selectedItems[0], 0);

  collapseAll(ind);
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
bboxSlot()
{
  auto selectedItems = tree_->selectedItems();

  for (auto *item : selectedItems) {
    auto *objectItem = dynamic_cast<CQLottieTreeObjectItem *>(item);
    if (! objectItem) continue;

    auto *obj = objectItem->object();

    std::cerr << obj->bbox() << "\n";
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
  auto typeName    = QString::fromStdString(object->type().value_or(""));

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

  tree_->setColumnCount(3);

  tree_->setHeaderLabels(QStringList() << "Name" << "Value" << "Animated");

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

  auto *printButton    = new QPushButton("Print");
  auto *printAllButton = new QPushButton("Print All");

  connect(printButton, SIGNAL(clicked()), this, SLOT(printSlot()));
  connect(printAllButton, SIGNAL(clicked()), this, SLOT(printAllSlot()));

  controlLayout->addStretch(1);
  controlLayout->addWidget(printButton);
  controlLayout->addWidget(printAllButton);

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
    std::string                 cname;
    CQLottieTreeValueItem::Type type { CQLottieTreeValueItem::Type::NONE };

    PropName() { }

    PropName(const QString &n, const CQLottieTreeValueItem::Type &t) :
     name(n), cname(n.toStdString()), type(t) {
    }
  };

  auto isPropertyType = [&](const CQLottieTreeValueItem::Type &type) {
    switch (type) {
      case CQLottieTreeValueItem::Type::NONE:
      case CQLottieTreeValueItem::Type::BOOL:
      case CQLottieTreeValueItem::Type::INTEGER:
      case CQLottieTreeValueItem::Type::REAL:
      case CQLottieTreeValueItem::Type::STRING:
      case CQLottieTreeValueItem::Type::RGBA:
      default:
        return false;
      case CQLottieTreeValueItem::Type::SPLIT_POSITION:
      case CQLottieTreeValueItem::Type::POSITION:
      case CQLottieTreeValueItem::Type::COLOR:
      case CQLottieTreeValueItem::Type::SIZE:
      case CQLottieTreeValueItem::Type::VECTOR:
      case CQLottieTreeValueItem::Type::SCALAR:
      case CQLottieTreeValueItem::Type::BEZIER:
        return true;
    }
  };

  std::vector<PropName> objPropNames = {
    { "name"  , CQLottieTreeValueItem::Type::STRING  },
    { "type"  , CQLottieTreeValueItem::Type::STRING  },
//  { "typeId", CQLottieTreeValueItem::Type::INTEGER },
    { "hidden", CQLottieTreeValueItem::Type::BOOL    },
    { "ind"   , CQLottieTreeValueItem::Type::INTEGER }
  };

  if      (root) {
    std::vector<PropName> propNames = {
      { "version"   , CQLottieTreeValueItem::Type::STRING },
      { "matchName" , CQLottieTreeValueItem::Type::STRING },
      { "frameRate" , CQLottieTreeValueItem::Type::REAL   },
      { "frameStart", CQLottieTreeValueItem::Type::REAL   },
      { "frameStop" , CQLottieTreeValueItem::Type::REAL   },
      { "width"     , CQLottieTreeValueItem::Type::REAL   },
      { "height"    , CQLottieTreeValueItem::Type::REAL   },
    };

    auto addRootProp = [&](const PropName &propName) {
      auto *propItem = new CQLottieTreeRootValueItem(item, root, propName.name, propName.type);
      item->addChild(propItem);
    };

    for (const auto &propName : objPropNames) {
      if (propName.name == "name" && ! object_->name())
        continue;
      if (propName.name == "type" && ! object_->type())
        continue;
      if (propName.name == "ind" && ! object_->ind())
        continue;

      addRootProp(propName);
    }

    for (const auto &propName : propNames) {
      if (propName.name == "matchName" && ! root->matchName())
        continue;

      addRootProp(propName);
    }
  }
  else if (asset) {
    std::vector<PropName> propNames = {
      { "id"      , CQLottieTreeValueItem::Type::STRING },
      { "css"     , CQLottieTreeValueItem::Type::STRING },
      { "width"   , CQLottieTreeValueItem::Type::REAL   },
      { "height"  , CQLottieTreeValueItem::Type::REAL   },
      { "dir"     , CQLottieTreeValueItem::Type::STRING },
      { "path"    , CQLottieTreeValueItem::Type::STRING },
      { "embedded", CQLottieTreeValueItem::Type::BOOL   },
    };

    auto addAssetProp = [&](const PropName &propName) {
      auto *propItem = new CQLottieTreeAssetValueItem(item, asset, propName.name, propName.type);
      item->addChild(propItem);
    };

    for (const auto &propName : objPropNames) {
      if (propName.name == "name" && ! object_->name())
        continue;
      if (propName.name == "type" && ! object_->type())
        continue;
      if (propName.name == "ind" && ! object_->ind())
        continue;

      addAssetProp(propName);
    }

    for (const auto &propName : propNames) {
      if (propName.name == "css" && ! asset->css())
        continue;
      if (propName.name == "width" && ! asset->width())
        continue;
      if (propName.name == "height" && ! asset->height())
        continue;
      if (propName.name == "dir" && ! asset->dir())
        continue;
      if (propName.name == "path" && ! asset->path())
        continue;
      if (propName.name == "embedded" && ! asset->embedded())
        continue;

      addAssetProp(propName);
    }
  }
  else if (layer) {
    std::vector<PropName> propNames = {
      { "enabled"    , CQLottieTreeValueItem::Type::BOOL    },
      { "typeId"     , CQLottieTreeValueItem::Type::STRING  },
      { "refId"      , CQLottieTreeValueItem::Type::STRING  },
      { "parentInd"  , CQLottieTreeValueItem::Type::INTEGER },
      { "autoOrient" , CQLottieTreeValueItem::Type::INTEGER },
      { "blendMode"  , CQLottieTreeValueItem::Type::INTEGER },
      { "width"      , CQLottieTreeValueItem::Type::INTEGER },
      { "height"     , CQLottieTreeValueItem::Type::INTEGER },
      { "frameIn"    , CQLottieTreeValueItem::Type::INTEGER },
      { "frameOut"   , CQLottieTreeValueItem::Type::INTEGER },
      { "startTime"  , CQLottieTreeValueItem::Type::REAL    },
      { "timeStretch", CQLottieTreeValueItem::Type::REAL    },
      { "matteMode"  , CQLottieTreeValueItem::Type::INTEGER },
      { "matteParent", CQLottieTreeValueItem::Type::INTEGER },
      { "matteTarget", CQLottieTreeValueItem::Type::INTEGER }
    };

    auto addLayerProp = [&](const PropName &propName) {
      CLottieProperty *prop = nullptr;
      if (isPropertyType(propName.type)) {
        prop = layer->getProperty(propName.cname);
        if (! prop || ! prop->isSet())
          return;
      }

      auto *propItem = new CQLottieTreeLayerValueItem(item, layer, propName.name, propName.type);
      item->addChild(propItem);

      propItem->setProperty(prop);
    };

    for (const auto &propName : objPropNames) {
      if (propName.name == "name" && ! object_->name())
        continue;
      if (propName.name == "type" && ! object_->type())
        continue;
      if (propName.name == "ind" && ! object_->ind())
        continue;

      addLayerProp(propName);
    }

    for (const auto &propName : propNames) {
      if (propName.name == "refId" && ! layer->refId())
        continue;
      if (propName.name == "parentInd" && ! layer->parentInd())
        continue;
      if (propName.name == "matteMode" && ! layer->matteMode())
        continue;
      if (propName.name == "matteParent" && ! layer->matteParent())
        continue;
      if (propName.name == "matteTarget" && ! layer->matteTarget())
        continue;
      if (propName.name == "width" && ! layer->width())
        continue;
      if (propName.name == "height" && ! layer->height())
        continue;
      if (propName.name == "startTime" && ! layer->startTime())
        continue;

      addLayerProp(propName);
    }

    if (layer->transform()) {
      std::vector<PropName> transformPropNames = {
        { "transform.anchorPoint", CQLottieTreeValueItem::Type::POSITION },
        { "transform.position"   , CQLottieTreeValueItem::Type::SPLIT_POSITION },
        { "transform.rotation"   , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.scale"      , CQLottieTreeValueItem::Type::VECTOR },
        { "transform.opacity"    , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.skew"       , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.skewAxis"   , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.x_rotation" , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.y_rotation" , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.z_rotation" , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.orientation", CQLottieTreeValueItem::Type::VECTOR },
      };

      for (const auto &propName : transformPropNames) {
#if 0
        if (propName.name == "transform.x_rotation" && ! layer->transform()->x_rotation.isSet())
          continue;
        if (propName.name == "transform.y_rotation" && ! layer->transform()->y_rotation.isSet())
          continue;
        if (propName.name == "transform.z_rotation" && ! layer->transform()->z_rotation.isSet())
          continue;
        if (propName.name == "transform.orientation" && ! layer->transform()->orientation.isSet())
          continue;
        if (propName.name == "transform.skew" && ! layer->transform()->skew.isSet())
          continue;
        if (propName.name == "transform.skewAxis" && ! layer->transform()->skewAxis.isSet())
          continue;
#endif

        addLayerProp(propName);
      }
    }

    if (layer->mask()) {
      std::vector<PropName> maskPropNames = {
        { "mask.mode"    , CQLottieTreeValueItem::Type::STRING },
        { "mask.opacity" , CQLottieTreeValueItem::Type::SCALAR },
        { "mask.path"    , CQLottieTreeValueItem::Type::BEZIER },
        { "mask.expand"  , CQLottieTreeValueItem::Type::SCALAR },
        { "mask.inverted", CQLottieTreeValueItem::Type::BOOL   },
        { "mask.name"    , CQLottieTreeValueItem::Type::STRING }
      };

      for (const auto &propName : maskPropNames) {
        addLayerProp(propName);
      }
    }

    if (layer->effect()) {
      std::vector<PropName> effectPropNames = {
        { "effect.type"         , CQLottieTreeValueItem::Type::INTEGER },
        { "effect.match"        , CQLottieTreeValueItem::Type::STRING  },
        { "effect.index"        , CQLottieTreeValueItem::Type::INTEGER },
        { "effect.numProperties", CQLottieTreeValueItem::Type::INTEGER },
        { "effect.enabled"      , CQLottieTreeValueItem::Type::INTEGER }
      };

      for (const auto &propName : effectPropNames) {
        addLayerProp(propName);
      }
    }

    if (layer->solid()) {
      std::vector<PropName> solidPropNames = {
        { "solid.width" , CQLottieTreeValueItem::Type::REAL },
        { "solid.height", CQLottieTreeValueItem::Type::REAL },
        { "solid.color" , CQLottieTreeValueItem::Type::RGBA }
      };

      for (const auto &propName : solidPropNames) {
        addLayerProp(propName);
      }
    }

    if (layer->precomp()) {
      std::vector<PropName> precompPropNames = {
        { "precomp.refId"    , CQLottieTreeValueItem::Type::STRING },
        { "precomp.width"    , CQLottieTreeValueItem::Type::REAL   },
        { "precomp.height"   , CQLottieTreeValueItem::Type::REAL   },
        { "precomp.startTime", CQLottieTreeValueItem::Type::REAL   },
        { "precomp.timeRemap", CQLottieTreeValueItem::Type::SCALAR }
      };

      for (const auto &propName : precompPropNames) {
        if (propName.name == "precomp.timeRemap" && ! layer->precomp()->timeRemap.isSet())
          continue;

        addLayerProp(propName);
      }
    }
  }
  else if (shape) {
    std::vector<PropName> propNames = {
      { "longName" , CQLottieTreeValueItem::Type::STRING },
      { "index"    , CQLottieTreeValueItem::Type::INTEGER },
      { "direction", CQLottieTreeValueItem::Type::INTEGER },
    };

    auto addShapeProp = [&](const PropName &propName) {
      CLottieProperty *prop = nullptr;
      if (isPropertyType(propName.type)) {
        prop = shape->getProperty(propName.cname);
        if (! prop || ! prop->isSet())
          return;
      }

      auto *propItem = new CQLottieTreeShapeValueItem(item, shape, propName.name, propName.type);
      item->addChild(propItem);

      propItem->setProperty(prop);
    };

    for (const auto &propName : objPropNames) {
      if (propName.name == "name" && ! object_->name())
        continue;
      if (propName.name == "type" && ! object_->type())
        continue;
      if (propName.name == "ind" && ! object_->ind())
        continue;

      addShapeProp(propName);
    }

    for (const auto &propName : propNames) {
      if (propName.name == "longName" && ! shape->longName())
        continue;
      if (propName.name == "index" && ! shape->index())
        continue;
      if (propName.name == "direction" && ! shape->direction())
        continue;

      addShapeProp(propName);
    }

    if (shape->pos().isSet()) {
      addShapeProp(PropName("position", CQLottieTreeValueItem::Type::POSITION));
    }

    if (shape->size().isSet()) {
      addShapeProp(PropName("size", CQLottieTreeValueItem::Type::SIZE));
    }

    if (shape->color().isSet()) {
      addShapeProp(PropName("color", CQLottieTreeValueItem::Type::COLOR));
    }

    if (shape->path().isSet()) {
      addShapeProp(PropName("path", CQLottieTreeValueItem::Type::BEZIER));
    }

    if (shape->transform()) {
      std::vector<PropName> transformPropNames = {
        { "transform.anchorPoint", CQLottieTreeValueItem::Type::POSITION },
        { "transform.position"   , CQLottieTreeValueItem::Type::SPLIT_POSITION },
        { "transform.rotation"   , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.scale"      , CQLottieTreeValueItem::Type::VECTOR },
        { "transform.opacity"    , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.skew"       , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.skewAxis"   , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.x_rotation" , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.y_rotation" , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.z_rotation" , CQLottieTreeValueItem::Type::SCALAR },
        { "transform.orientation", CQLottieTreeValueItem::Type::VECTOR },
      };

      for (const auto &propName : transformPropNames) {
        if (propName.name == "transform.x_rotation" && ! shape->transform()->x_rotation.isSet())
          continue;
        if (propName.name == "transform.y_rotation" && ! shape->transform()->y_rotation.isSet())
          continue;
        if (propName.name == "transform.z_rotation" && ! shape->transform()->z_rotation.isSet())
          continue;
        if (propName.name == "transform.orientation" && ! shape->transform()->orientation.isSet())
          continue;
        if (propName.name == "transform.skew" && ! shape->transform()->skew.isSet())
          continue;
        if (propName.name == "transform.skewAxis" && ! shape->transform()->skewAxis.isSet())
          continue;

        addShapeProp(propName);
      }
    }

    if (shape->rectangle()) {
      std::vector<PropName> rectanglePropNames = {
        { "rectangle.roundness", CQLottieTreeValueItem::Type::SCALAR },
      };

      for (const auto &propName : rectanglePropNames) {
        addShapeProp(propName);
      }
    }

    if (shape->repeater()) {
      std::vector<PropName> repeaterPropNames = {
        { "repeater.copies"      , CQLottieTreeValueItem::Type::SCALAR },
        { "repeater.offset"      , CQLottieTreeValueItem::Type::SCALAR },
        { "repeater.composite"   , CQLottieTreeValueItem::Type::INTEGER },
        { "repeater.startOpacity", CQLottieTreeValueItem::Type::SCALAR },
        { "repeater.endOpacity"  , CQLottieTreeValueItem::Type::SCALAR },

        { "repeater.transform.anchorPoint", CQLottieTreeValueItem::Type::POSITION },
        { "repeater.transform.position"   , CQLottieTreeValueItem::Type::SPLIT_POSITION },
        { "repeater.transform.rotation"   , CQLottieTreeValueItem::Type::SCALAR },
        { "repeater.transform.scale"      , CQLottieTreeValueItem::Type::VECTOR },
      };

      for (const auto &propName : repeaterPropNames) {
        if (propName.name == "repeater.startOpacity" && ! shape->repeater()->startOpacity.isSet())
          continue;
        if (propName.name == "repeater.endOpacity" && ! shape->repeater()->endOpacity.isSet())
          continue;

        addShapeProp(propName);
      }
    }

    if (shape->stroke()) {
      std::vector<PropName> strokePropNames = {
        { "stroke.color"         , CQLottieTreeValueItem::Type::COLOR   },
        { "stroke.opacity"       , CQLottieTreeValueItem::Type::SCALAR  },
        { "stroke.width"         , CQLottieTreeValueItem::Type::SCALAR  },
        { "stroke.lineCap"       , CQLottieTreeValueItem::Type::INTEGER },
        { "stroke.lineJoin"      , CQLottieTreeValueItem::Type::INTEGER },
        { "stroke.miterLimit"    , CQLottieTreeValueItem::Type::REAL    },
        { "stroke.miterLimitAnim", CQLottieTreeValueItem::Type::SCALAR  },
        { "stroke.dash.type"     , CQLottieTreeValueItem::Type::STRING  },
        { "stroke.dash.name"     , CQLottieTreeValueItem::Type::STRING  },
        { "stroke.dash.value"    , CQLottieTreeValueItem::Type::SCALAR  },
        { "stroke.blendMode"     , CQLottieTreeValueItem::Type::INTEGER }
      };

      for (const auto &propName : strokePropNames) {
        if (propName.name == "stroke.miterLimit" && ! shape->stroke()->miterLimit)
          continue;
        if (propName.name == "stroke.miterLimitAnim" && ! shape->stroke()->miterLimitAnim.isSet())
          continue;
        if (propName.name == "stroke.dash.type" && ! shape->stroke()->dash.type)
          continue;
        if (propName.name == "stroke.dash.name" && ! shape->stroke()->dash.name)
          continue;
        if (propName.name == "stroke.dash.value" && ! shape->stroke()->dash.value.isSet())
          continue;
        if (propName.name == "stroke.blendMode" && ! shape->stroke()->blendMode)
          continue;

        addShapeProp(propName);
      }
    }

    if (shape->fill()) {
      std::vector<PropName> fillPropNames = {
        { "fill.color"      , CQLottieTreeValueItem::Type::COLOR   },
        { "fill.opacity"    , CQLottieTreeValueItem::Type::SCALAR  },
        { "fill.fillRule"   , CQLottieTreeValueItem::Type::INTEGER },
        { "fill.blendMode"  , CQLottieTreeValueItem::Type::INTEGER },
        { "fill.fillEnabled", CQLottieTreeValueItem::Type::BOOL    }
      };

      for (const auto &propName : fillPropNames) {
        if (propName.name == "fill.blendMode" && ! shape->fill()->blendMode)
          continue;
        if (propName.name == "fill.fillEnabled" && ! shape->fill()->fillEnabled)
          continue;

        addShapeProp(propName);
      }
    }

    if (shape->group()) {
      std::vector<PropName> groupPropNames = {
        { "group.color"        , CQLottieTreeValueItem::Type::COLOR   },
        { "group.opacity"      , CQLottieTreeValueItem::Type::SCALAR  },
        { "group.numProperties", CQLottieTreeValueItem::Type::INTEGER },
        { "group.blendMode"    , CQLottieTreeValueItem::Type::INTEGER }
      };

      for (const auto &propName : groupPropNames) {
        if (propName.name == "group.color" && ! shape->group()->color.isSet())
          continue;
        if (propName.name == "group.opacity" && ! shape->group()->opacity.isSet())
          continue;
        if (propName.name == "group.blendMode" && ! shape->group()->blendMode)
          continue;

        addShapeProp(propName);
      }
    }

    if (shape->gradientFill()) {
      std::vector<PropName> fillPropNames = {
        { "gradientFill.color"          , CQLottieTreeValueItem::Type::COLOR   },
        { "gradientFill.opacity"        , CQLottieTreeValueItem::Type::SCALAR  },
        { "gradientFill.type"           , CQLottieTreeValueItem::Type::INTEGER },
        { "gradientFill.stopCount"      , CQLottieTreeValueItem::Type::INTEGER },
        { "gradientFill.index"          , CQLottieTreeValueItem::Type::INTEGER },
        { "gradientFill.startPoint"     , CQLottieTreeValueItem::Type::VECTOR  },
        { "gradientFill.endPoint"       , CQLottieTreeValueItem::Type::VECTOR  },
        { "gradientFill.highlightLength", CQLottieTreeValueItem::Type::SCALAR  },
        { "gradientFill.highlightAngle" , CQLottieTreeValueItem::Type::SCALAR  },
        { "gradientFill.fillRule"       , CQLottieTreeValueItem::Type::INTEGER },
        { "gradientFill.blendMode"      , CQLottieTreeValueItem::Type::INTEGER }
//      { "gradientFill.colors"         , CQLottieTreeValueItem::Type::ARRAY   }
      };

      for (const auto &propName : fillPropNames) {
        if (propName.name == "gradientFill.color" && ! shape->gradientFill()->color.isSet())
          continue;
        if (propName.name == "gradientFill.index" && ! shape->gradientFill()->index)
          continue;
        if (propName.name == "gradientFill.highlightLength" &&
            ! shape->gradientFill()->highlightLength.isSet())
          continue;
        if (propName.name == "gradientFill.highlightAngle" &&
            ! shape->gradientFill()->highlightAngle.isSet())
          continue;
        if (propName.name == "gradientFill.blendMode" && ! shape->gradientFill()->blendMode)
          continue;

        addShapeProp(propName);
      }
    }

    if (shape->gradientStroke()) {
      std::vector<PropName> strokePropNames = {
        { "gradientStroke.opacity"   , CQLottieTreeValueItem::Type::SCALAR  },
        { "gradientStroke.type"      , CQLottieTreeValueItem::Type::INTEGER },
        { "gradientStroke.stopCount" , CQLottieTreeValueItem::Type::INTEGER },
        { "gradientStroke.index"     , CQLottieTreeValueItem::Type::INTEGER },
        { "gradientStroke.startPoint", CQLottieTreeValueItem::Type::VECTOR  },
        { "gradientStroke.endPoint"  , CQLottieTreeValueItem::Type::VECTOR  },
        { "gradientStroke.width"     , CQLottieTreeValueItem::Type::SCALAR  },
        { "gradientStroke.lineCap"   , CQLottieTreeValueItem::Type::INTEGER },
        { "gradientStroke.lineJoin"  , CQLottieTreeValueItem::Type::INTEGER },
        { "gradientStroke.miterLimit", CQLottieTreeValueItem::Type::REAL    },
        { "gradientStroke.dash.type" , CQLottieTreeValueItem::Type::STRING  },
        { "gradientStroke.dash.name" , CQLottieTreeValueItem::Type::STRING  },
        { "gradientStroke.dash.value", CQLottieTreeValueItem::Type::SCALAR  },
//      { "gradientStroke.colors"    , CQLottieTreeValueItem::Type::ARRAY   }
      };

      for (const auto &propName : strokePropNames) {
        if (propName.name == "gradientStroke.opacity" &&
            ! shape->gradientStroke()->opacity.isSet())
          continue;
        if (propName.name == "gradientStroke.type" && ! shape->gradientStroke()->type)
          continue;
        if (propName.name == "gradientStroke.index" && ! shape->gradientStroke()->index)
          continue;
        if (propName.name == "gradientStroke.miterLimit" &&
            ! shape->gradientStroke()->miterLimit)
          continue;
        if (propName.name == "gradientStroke.dash.type" &&
            ! shape->gradientStroke()->dash.type)
          continue;
        if (propName.name == "gradientStroke.dash.name" &&
            ! shape->gradientStroke()->dash.name)
          continue;
        if (propName.name == "gradientStroke.dash.value" &&
            ! shape->gradientStroke()->dash.value.isSet())
          continue;

        addShapeProp(propName);
      }
    }

    if (shape->trim()) {
      std::vector<PropName> trimPropNames = {
        { "trim.start"   , CQLottieTreeValueItem::Type::SCALAR },
        { "trim.end"     , CQLottieTreeValueItem::Type::SCALAR },
        { "trim.offset"  , CQLottieTreeValueItem::Type::SCALAR },
        { "trim.multiple", CQLottieTreeValueItem::Type::INTEGER }
      };

      for (const auto &propName : trimPropNames) {
        addShapeProp(propName);
      }
    }

    if (shape->polyStar()) {
      std::vector<PropName> starPropNames = {
        { "polystar.type"          , CQLottieTreeValueItem::Type::INTEGER },
        { "polystar.position"      , CQLottieTreeValueItem::Type::POSITION },
        { "polystar.innerRadius"   , CQLottieTreeValueItem::Type::SCALAR },
        { "polystar.innerRoundness", CQLottieTreeValueItem::Type::SCALAR },
        { "polystar.outerRadius"   , CQLottieTreeValueItem::Type::SCALAR },
        { "polystar.outerRoundness", CQLottieTreeValueItem::Type::SCALAR },
        { "polystar.rotation"      , CQLottieTreeValueItem::Type::SCALAR },
        { "polystar.points"        , CQLottieTreeValueItem::Type::SCALAR },
      };

      for (const auto &propName : starPropNames) {
        addShapeProp(propName);
      }
    }

    if (shape->merge()) {
      std::vector<PropName> mergePropNames = {
        { "merge.mode", CQLottieTreeValueItem::Type::INTEGER },
      };

      for (const auto &propName : mergePropNames) {
        addShapeProp(propName);
      }
    }

    if (shape->rounded()) {
      std::vector<PropName> reoundedPropNames = {
        { "rounded.roundness", CQLottieTreeValueItem::Type::SCALAR },
      };

      for (const auto &propName : reoundedPropNames) {
        addShapeProp(propName);
      }
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
      if (propName.name == "name" && ! object_->name())
        continue;
      if (propName.name == "type" && ! object_->type())
        continue;
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

  auto *layerItem = dynamic_cast<CQLottieTreeLayerValueItem *>(valueItem);

  auto *object = valueItem->object();
  auto *layer  = (layerItem ? layerItem->layer() : nullptr);

  auto ind = tree_->indexFromItem(item, column);

  if (valueItem->propType() == CQLottieTreeValueItem::Type::BOOL) {
    if (valueItem->propName() == "hidden") {
      object->setHidden(! object->hidden().value_or(false));

      lottie_->canvas()->invalidate();
    }
    else {
      if (layer) {
        if (valueItem->propName() == "enabled") {
          auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);

          qlayer->setEnabled(! qlayer->isEnabled());

          lottie_->canvas()->invalidate();
        }
      }
    }
  }

  tree_->update(ind);
}

void
CQLottieObjectTree::
itemSelectedSlot(QTreeWidgetItem *item, QTreeWidgetItem *)
{
  auto *valueItem = dynamic_cast<CQLottieTreeValueItem *>(item);
  if (! valueItem) return;

  auto *prop = valueItem->property();

  lottie_->timeLine()->setProperty(const_cast<CLottieProperty *>(prop));
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
    tree_->resizeColumnToContents(2);
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
  auto selectedItems = tree_->selectedItems();

  for (auto *item : selectedItems) {
    auto *valueItem = dynamic_cast<CQLottieTreeValueItem *>(item);
    if (! valueItem) continue;

    auto *prop = valueItem->property();

    if (prop)
      prop->print();
  }
}

void
CQLottieObjectTree::
printAllSlot()
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

  auto *object = valueItem->object();
  auto *layer  = (layerItem ? layerItem->layer() : nullptr);
  auto *shape  = (shapeItem ? shapeItem->shape() : nullptr);

  if      (valueItem->propType() == CQLottieTreeValueItem::Type::BOOL) {
    auto *check = qobject_cast<QCheckBox *>(w);

    if (valueItem->propName() == "hidden")
      check->setChecked(object->hidden().value_or(false));
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::INTEGER) {
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::REAL) {
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::STRING) {
    auto *edit = qobject_cast<QLineEdit *>(w);

    if (valueItem->propName() == "refId") {
      if (layer)
        edit->setText(QString::fromStdString(layer->refId().value_or("")));
    }
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::COLOR) {
    auto *chooser = qobject_cast<CQColorChooser *>(w);

    if (valueItem->propName() == "color") {
      if (shape) {
        auto c = shape->color().value(CRGBA(0, 0, 0, 0)).value();

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

  auto *object = valueItem->object();
  auto *layer  = (layerItem ? layerItem->layer() : nullptr);
  auto *shape  = (shapeItem ? shapeItem->shape() : nullptr);

  if      (valueItem->propType() == CQLottieTreeValueItem::Type::BOOL) {
    auto *check = qobject_cast<QCheckBox *>(w);

    if (valueItem->propName() == "hidden") {
      object->setHidden(check->isChecked());

      lottie->canvas()->invalidate();
    }
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::INTEGER) {
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::REAL) {
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::STRING) {
    auto *edit = qobject_cast<QLineEdit *>(w);

    if (valueItem->propName() == "refId") {
      if (layer)
        layer->setRefId(edit->text().toStdString());
    }
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::COLOR) {
    auto *chooser = qobject_cast<CQColorChooser *>(w);

    if (valueItem->propName() == "color") {
      if (shape)
        shape->colorRef().setValue(toRGBA(chooser->color()));
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
  auto s =  QItemDelegate::sizeHint(option, index);

  if      (index.column() == 1)
    s.setWidth(option.fontMetrics.horizontalAdvance("XXXXXXXXXXXX`"));
  else if (index.column() == 2)
    s.setWidth(option.fontMetrics.horizontalAdvance("Animated"));

  return s;
}

void
CQLottieObjectTreeDelegate::
paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if (index.column() == 0)
    return QItemDelegate::paint(painter, option, index);

  //---

  auto *item      = tree_->itemFromIndex(index);
  auto *valueItem = dynamic_cast<CQLottieTreeValueItem *>(item);

  if (! valueItem)
    return QItemDelegate::paint(painter, option, index);

  //---

  auto *property = valueItem->property();

  bool animated = false;

  if (property)
    animated = property->isAnimated();

  if (index.column() == 2) {
    if (animated)
      drawChecked(painter, option, true, index);

    return;
  }

  //---

  auto *lottie = tree_->lottie();

  CLottieUtil::TimeFrame timeFrame;
  lottie->getTimeFrame(timeFrame);

  if (property) {
    auto str = property->tvalueStr(timeFrame);

    valueItem->setToolTip(1, QString::fromStdString(str));
  }

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

    if (valueItem->propName() == "hidden") {
      b = object->hidden().value_or(false);
    }
    else {
      if      (layer) {
        if (valueItem->propName() == "enabled") {
          auto *qlayer = dynamic_cast<CQLottieLayer *>(layer);

          if (qlayer)
            b = qlayer->isEnabled();
        }
      }
      else if (shape) {
        if (valueItem->propName() == "fill.fillEnabled") {
          if (shape->fill() && shape->fill()->fillEnabled)
            b = shape->fill()->fillEnabled.value();
        }
      }
    }

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
        else if (valueItem->propName() == "width") {
          if (layer->width())
            i = layer->width().value();
        }
        else if (valueItem->propName() == "height") {
          if (layer->height())
            i = layer->height().value();
        }
        else if (valueItem->propName() == "frameIn") {
          if (layer->frameIn())
            i = layer->frameIn().value();
        }
        else if (valueItem->propName() == "frameOut") {
          if (layer->frameOut())
            i = layer->frameOut().value();
        }
        else if (valueItem->propName() == "autoOrient") {
          if (layer->autoOrient())
            i = layer->autoOrient().value();
        }
        else if (valueItem->propName() == "blendMode") {
          if (layer->blendMode())
            i = layer->blendMode().value();
        }
        else if (valueItem->propName() == "matteMode") {
          if (layer->matteMode())
            i = layer->matteMode().value();
        }
        else if (valueItem->propName() == "matteParent") {
          if (layer->matteParent())
            i = layer->matteParent().value();
        }
        else if (valueItem->propName() == "matteTarget") {
          if (layer->matteTarget())
            i = layer->matteTarget().value();
        }
        else if (valueItem->propName() == "effect.type") {
          if (layer->effect() && layer->effect()->type())
            i = layer->effect()->type().value();
        }
        else if (valueItem->propName() == "effect.index") {
          if (layer->effect() && layer->effect()->index())
            i = layer->effect()->index().value();
        }
        else if (valueItem->propName() == "effect.numProperties") {
          if (layer->effect() && layer->effect()->numProperties())
            i = layer->effect()->numProperties().value();
        }
        else if (valueItem->propName() == "effect.enabled") {
          if (layer->effect() && layer->effect()->enabled())
            i = layer->effect()->enabled().value();
        }
      }
      else if (shape) {
        if      (valueItem->propName() == "index") {
          if (shape->index())
            i = shape->index().value();
        }
        else if (valueItem->propName() == "direction") {
          if (shape->direction())
            i = shape->direction().value();
        }
        if      (valueItem->propName() == "fill.fillRule") {
          if (shape->fill() && shape->fill()->fillRule)
            i = shape->fill()->fillRule.value();
        }
        else if (valueItem->propName() == "fill.blendMode") {
          if (shape->fill() && shape->fill()->blendMode)
            i = shape->fill()->blendMode.value();
        }
        else if (valueItem->propName() == "group.numProperties") {
          if (shape->group() && shape->group()->numProperties)
            i = shape->group()->numProperties.value();
        }
        else if (valueItem->propName() == "group.blendMode") {
          if (shape->group() && shape->group()->blendMode)
            i = shape->group()->blendMode.value();
        }
        else if (valueItem->propName() == "trim.multiple") {
          if (shape->trim() && shape->trim()->multiple)
            i = shape->trim()->multiple.value();
        }
        else if (valueItem->propName() == "merge.mode") {
          if (shape->merge() && shape->merge()->mode)
            i = shape->merge()->mode.value();
        }
        else if (valueItem->propName() == "stroke.lineCap") {
          if (shape->stroke() && shape->stroke()->lineCap)
            i = shape->stroke()->lineCap.value();
        }
        else if (valueItem->propName() == "stroke.lineJoin") {
          if (shape->stroke() && shape->stroke()->lineJoin)
            i = shape->stroke()->lineJoin.value();
        }
        else if (valueItem->propName() == "stroke.blendMode") {
          if (shape->stroke() && shape->stroke()->blendMode)
            i = shape->stroke()->blendMode.value();
        }
        else if (valueItem->propName() == "gradientFill.type") {
          if (shape->gradientFill() && shape->gradientFill()->type)
            i = shape->gradientFill()->type.value();
        }
        else if (valueItem->propName() == "gradientFill.stopCount") {
          if (shape->gradientFill() && shape->gradientFill()->stopCount)
            i = shape->gradientFill()->stopCount.value();
        }
        else if (valueItem->propName() == "gradientFill.index") {
          if (shape->gradientFill() && shape->gradientFill()->index)
            i = shape->gradientFill()->index.value();
        }
        else if (valueItem->propName() == "gradientFill.fillRule") {
          if (shape->gradientFill() && shape->gradientFill()->fillRule)
            i = shape->gradientFill()->fillRule.value();
        }
        else if (valueItem->propName() == "gradientFill.blendMode") {
          if (shape->gradientFill() && shape->gradientFill()->blendMode)
            i = shape->gradientFill()->blendMode.value();
        }
        else if (valueItem->propName() == "gradientStroke.type") {
          if (shape->gradientStroke() && shape->gradientStroke()->type)
            i = shape->gradientStroke()->type.value();
        }
        else if (valueItem->propName() == "gradientStroke.stopCount") {
          if (shape->gradientStroke() && shape->gradientStroke()->stopCount)
            i = shape->gradientStroke()->stopCount.value();
        }
        else if (valueItem->propName() == "gradientStroke.index") {
          if (shape->gradientStroke() && shape->gradientStroke()->index)
            i = shape->gradientStroke()->index.value();
        }
        else if (valueItem->propName() == "gradientStroke.lineCap") {
          if (shape->gradientStroke() && shape->gradientStroke()->lineCap)
            i = shape->gradientStroke()->lineCap.value();
        }
        else if (valueItem->propName() == "gradientStroke.lineJoin") {
          if (shape->gradientStroke() && shape->gradientStroke()->lineJoin)
            i = shape->gradientStroke()->lineJoin.value();
        }
        else if (valueItem->propName() == "repeater.composite") {
          if (shape->repeater() && shape->repeater()->composite)
            i = shape->repeater()->composite.value();
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
      if      (valueItem->propName() == "startTime") {
        if (layer->startTime())
          r = layer->startTime().value();
      }
      else if (valueItem->propName() == "timeStretch") {
        if (layer->timeStretch())
          r = layer->timeStretch().value();
      }
      else if (valueItem->propName() == "precomp.width") {
        if (layer->precomp() && layer->precomp()->width)
          r = layer->precomp()->width.value();
      }
      else if (valueItem->propName() == "precomp.height") {
        if (layer->precomp() && layer->precomp()->height)
          r = layer->precomp()->height.value();
      }
      else if (valueItem->propName() == "precomp.startTime") {
        if (layer->precomp() && layer->precomp()->startTime)
          r = layer->precomp()->startTime.value();
      }
      else if (valueItem->propName() == "solid.width") {
        if (layer->solid() && layer->solid()->width)
          r = layer->solid()->width.value();
      }
      else if (valueItem->propName() == "solid.height") {
        if (layer->solid() && layer->solid()->height)
          r = layer->solid()->height.value();
      }
    }
    else if (shape) {
      if      (valueItem->propName() == "stroke.miterLimit") {
        if (shape->stroke() && shape->stroke()->miterLimit)
          r = shape->stroke()->miterLimit.value();
      }
      else if (valueItem->propName() == "gradientStroke.miterLimit") {
        if (shape->gradientStroke() && shape->gradientStroke()->miterLimit)
          r = shape->gradientStroke()->miterLimit.value();
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
      if (object->type())
        cstr = object->type().value();
    }
    else {
      if      (root) {
        if      (valueItem->propName() == "version") {
          if (root->version())
            cstr = root->version().value();
        }
        else if (valueItem->propName() == "matchName") {
          if (root->matchName())
            cstr = root->matchName().value();
        }
      }
      else if (asset) {
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
          if (layer->refId())
            cstr = layer->refId().value();
        }
        else if (valueItem->propName() == "effect.match") {
          if (layer->effect() && layer->effect()->match())
            cstr = layer->effect()->match().value();
        }
        else if (valueItem->propName() == "precomp.refId") {
          if (layer->precomp() && layer->precomp()->refId)
            cstr = layer->precomp()->refId.value();
        }
      }
      else if (shape) {
        if      (valueItem->propName() == "longName") {
          if (shape->longName())
            cstr = shape->longName().value();
        }
        else if (valueItem->propName() == "stroke.dash.type") {
          if (shape->stroke() && shape->stroke()->dash.type)
            cstr = shape->stroke()->dash.type.value();
        }
        else if (valueItem->propName() == "stroke.dash.name") {
          if (shape->stroke() && shape->stroke()->dash.name)
            cstr = shape->stroke()->dash.name.value();
        }
        else if (valueItem->propName() == "gradientStroke.dash.type") {
          if (shape->gradientStroke() && shape->gradientStroke()->dash.type)
            cstr = shape->gradientStroke()->dash.type.value();
        }
        else if (valueItem->propName() == "gradientStroke.dash.name") {
          if (shape->gradientStroke() && shape->gradientStroke()->dash.name)
            cstr = shape->gradientStroke()->dash.name.value();
        }
      }
    }

    if (cstr)
      str = QString::fromStdString(cstr.value());

    if (str)
      drawString(painter, option, str.value(), index);
    else
      drawString(painter, option, "<unset>", index);
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::RGBA) {
    std::optional<CRGBA> c;

    if (layer) {
      if (valueItem->propName() == "solid.color") {
        if (layer->solid() && layer->solid()->color)
          c = layer->solid()->color.value();
      }
    }

    if (c)
      drawColor(painter, option, toQColor(c.value()), index);
    else
      drawString(painter, option, "<unset>", index);
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::COLOR) {
    std::optional<CRGBA> c;

    if (shape) {
      if      (valueItem->propName() == "color") {
        if (shape->color().isSet())
          c = shape->color().tvalue(timeFrame);
      }
      else if (valueItem->propName() == "fill.color") {
        if (shape->fill() && shape->fill()->color.isSet())
          c = shape->fill()->color.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "stroke.color") {
        if (shape->stroke() && shape->stroke()->color.isSet())
          c = shape->stroke()->color.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "group.color") {
        if (shape->group() && shape->group()->color.isSet())
          c = shape->group()->color.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientFill.color") {
        if (shape->gradientFill() && shape->gradientFill()->color.isSet())
          c = shape->gradientFill()->color.tvalue(timeFrame);
      }
    }

    if (c)
      drawColor(painter, option, toQColor(c.value()), index);
    else
      drawString(painter, option, "<unset>", index);
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::SPLIT_POSITION) {
    std::optional<CPoint2D> p;

    if      (layer) {
      if      (valueItem->propName() == "transform.position") {
        if (layer->transform())
          p = layer->transform()->position.tvalue(timeFrame);
      }
    }
    else if (shape) {
      if      (valueItem->propName() == "transform.position") {
        if (shape->transform() && shape->transform()->position.isSet())
          p = shape->transform()->position.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "repeater.transform.position") {
        if (shape->repeater() && shape->repeater()->transform &&
            shape->repeater()->transform->position.isSet())
          p = shape->repeater()->transform->position.tvalue(timeFrame);
      }
    }

    if (p)
      drawString(painter, option, QString("%1,%2").arg(p->x).arg(p->y), index);
    else
      drawString(painter, option, "<unset>", index);
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::POSITION) {
    std::optional<CLottie::XYVals> xy;

    if      (layer) {
      if (valueItem->propName() == "transform.anchorPoint") {
        if (layer->transform() && layer->transform()->anchorPoint.isSet())
          xy = layer->transform()->anchorPoint.tvalue(timeFrame);
      }
    }
    else if (shape) {
      if      (valueItem->propName() == "position")
        xy = shape->pos().tvalue(timeFrame, CLottie::XYVals());
      else if (valueItem->propName() == "transform.anchorPoint") {
        if (shape->transform() && shape->transform()->anchorPoint.isSet())
          xy = shape->transform()->anchorPoint.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "repeater.transform.anchorPoint") {
        if (shape->repeater() && shape->repeater()->transform &&
            shape->repeater()->transform->anchorPoint.isSet())
          xy = shape->repeater()->transform->anchorPoint.tvalue(timeFrame);
      }
    }

    if (xy) {
      auto p = xy->toPoint(CPoint2D(0, 0));

      drawString(painter, option, QString("%1,%2").arg(p.x).arg(p.y), index);
    }
    else
      drawString(painter, option, "<unset>", index);
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::SIZE) {
    std::optional<CPoint2D> p;

    if (shape) {
      if (valueItem->propName() == "size") {
        auto sizexy = shape->size().tvalue(timeFrame, CLottie::XYVals()).value();

        p = sizexy.toPoint(CPoint2D(0, 0));
      }
    }

    if (p)
      drawString(painter, option, QString("%1,%2").arg(p->x).arg(p->y), index);
    else
      drawString(painter, option, "<unset>", index);
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::VECTOR) {
    std::optional<CPoint2D> p;

    if      (layer) {
      if      (valueItem->propName() == "transform.scale") {
        if (layer->transform() && layer->transform()->scale.isSet())
          p = layer->transform()->scale.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.orientation") {
        if (layer->transform() && layer->transform()->orientation.isSet())
          p = layer->transform()->orientation.tvalue(timeFrame);
      }
    }
    else if (shape) {
      if      (valueItem->propName() == "transform.scale") {
        if (shape->transform() && shape->transform()->scale.isSet())
          p = shape->transform()->scale.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.orientation") {
        if (shape->transform() && shape->transform()->orientation.isSet())
          p = shape->transform()->orientation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientFill.startPoint") {
        if (shape->gradientFill() && shape->gradientFill()->startPoint.isSet())
          p = shape->gradientFill()->startPoint.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientFill.endPoint") {
        if (shape->gradientFill() && shape->gradientFill()->endPoint.isSet())
          p = shape->gradientFill()->endPoint.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientStroke.startPoint") {
        if (shape->gradientStroke() && shape->gradientStroke()->startPoint.isSet()) {
          animated = shape->gradientStroke()->startPoint.isAnimated();

          p = shape->gradientStroke()->startPoint.tvalue(timeFrame);
        }
      }
      else if (valueItem->propName() == "gradientStroke.endPoint") {
        if (shape->gradientStroke() && shape->gradientStroke()->endPoint.isSet())
          p = shape->gradientStroke()->endPoint.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "repeater.transform.scale") {
        if (shape->repeater() && shape->repeater()->transform &&
            shape->repeater()->transform->scale.isSet())
          p = shape->repeater()->transform->scale.tvalue(timeFrame);
      }
    }

    if (p)
      drawString(painter, option, QString("%1,%2").arg(p->x).arg(p->y), index);
    else
      drawString(painter, option, "<unset>", index);
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::SCALAR) {
    std::optional<double> r;

    if      (layer) {
      if      (valueItem->propName() == "transform.rotation") {
        if (layer->transform() && layer->transform()->rotation.isSet())
          r = layer->transform()->rotation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.opacity") {
        if (layer->transform() && layer->transform()->opacity.isSet())
          r = layer->transform()->opacity.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.skew") {
        if (layer->transform() && layer->transform()->skew.isSet())
          r = layer->transform()->skew.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.skewAxis") {
        if (layer->transform() && layer->transform()->skewAxis.isSet())
          r = layer->transform()->skewAxis.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.x_rotation") {
        if (layer->transform() && layer->transform()->x_rotation.isSet())
          r = layer->transform()->x_rotation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.y_rotation") {
        if (layer->transform() && layer->transform()->y_rotation.isSet())
          r = layer->transform()->y_rotation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.z_rotation") {
        if (layer->transform() && layer->transform()->z_rotation.isSet())
          r = layer->transform()->z_rotation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "precomp.timeRemap") {
        if (layer->precomp() && layer->precomp()->timeRemap.isSet())
          r = layer->precomp()->timeRemap.tvalue(timeFrame);
      }
    }
    else if (shape) {
      if      (valueItem->propName() == "fill.opacity") {
        if (shape->fill() && shape->fill()->opacity.isSet())
          r = shape->fill()->opacity.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "stroke.opacity") {
        if (shape->stroke() && shape->stroke()->opacity.isSet())
          r = shape->stroke()->opacity.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "stroke.width") {
        if (shape->stroke() && shape->stroke()->width.isSet())
          r = shape->stroke()->width.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "stroke.miterLimitAnim") {
        if (shape->stroke() && shape->stroke()->miterLimitAnim.isSet())
          r = shape->stroke()->miterLimitAnim.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "stroke.dash.value") {
        if (shape->stroke() && shape->stroke()->dash.value.isSet())
          r = shape->stroke()->dash.value.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "group.opacity") {
        if (shape->group() && shape->group()->opacity.isSet())
          r = shape->group()->opacity.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.rotation") {
        if (shape->transform() && shape->transform()->rotation.isSet())
          r = shape->transform()->rotation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.opacity") {
        if (shape->transform() && shape->transform()->opacity.isSet())
          r = shape->transform()->opacity.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.skew") {
        if (shape->transform() && shape->transform()->skew.isSet())
          r = shape->transform()->skew.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.skewAxis") {
        if (shape->transform() && shape->transform()->skewAxis.isSet())
          r = shape->transform()->skewAxis.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.x_rotation") {
        if (shape->transform() && shape->transform()->x_rotation.isSet())
          r = shape->transform()->x_rotation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.y_rotation") {
        if (shape->transform() && shape->transform()->y_rotation.isSet())
          r = shape->transform()->y_rotation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "transform.z_rotation") {
        if (shape->transform() && shape->transform()->z_rotation.isSet())
          r = shape->transform()->z_rotation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "trim.start") {
        if (shape->trim() && shape->trim()->start.isSet())
          r = shape->trim()->start.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "trim.end") {
        if (shape->trim() && shape->trim()->end.isSet())
          r = shape->trim()->end.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "trim.offset") {
        if (shape->trim() && shape->trim()->offset.isSet())
          r = shape->trim()->offset.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientFill.highlightLength") {
        if (shape->gradientFill() && shape->gradientFill()->highlightLength.isSet())
          r = shape->gradientFill()->highlightLength.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientFill.highlightAngle") {
        if (shape->gradientFill() && shape->gradientFill()->highlightAngle.isSet())
          r = shape->gradientFill()->highlightAngle.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientFill.opacity") {
        if (shape->gradientFill() && shape->gradientFill()->opacity.isSet())
          r = shape->gradientFill()->opacity.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientStroke.opacity") {
        if (shape->gradientStroke() && shape->gradientStroke()->opacity.isSet())
          r = shape->gradientStroke()->opacity.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientStroke.width") {
        if (shape->gradientStroke() && shape->gradientStroke()->width.isSet())
          r = shape->gradientStroke()->width.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "gradientStroke.dash.value") {
        if (shape->gradientStroke() && shape->gradientStroke()->dash.value.isSet())
          r = shape->gradientStroke()->dash.value.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "rectangle.roundness") {
        if (shape->rectangle() && shape->rectangle()->roundness.isSet())
          r = shape->rectangle()->roundness.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "repeater.copies") {
        if (shape->repeater() && shape->repeater()->copies.isSet())
          r = shape->repeater()->copies.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "repeater.offset") {
        if (shape->repeater() && shape->repeater()->offset.isSet())
          r = shape->repeater()->offset.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "repeater.startOpacity") {
        if (shape->repeater() && shape->repeater()->startOpacity.isSet())
          r = shape->repeater()->startOpacity.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "repeater.endOpacity") {
        if (shape->repeater() && shape->repeater()->endOpacity.isSet())
          r = shape->repeater()->endOpacity.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "repeater.transform.rotation") {
        if (shape->repeater() && shape->repeater()->transform &&
            shape->repeater()->transform->rotation.isSet())
          r = shape->repeater()->transform->rotation.tvalue(timeFrame);
      }
      else if (valueItem->propName() == "rounded.roundness") {
        if (shape->rounded() && shape->rounded()->roundness.isSet())
          r = shape->rounded()->roundness.tvalue(timeFrame);
      }
    }

    if (r)
      drawString(painter, option, QString::number(*r), index);
    else
      drawString(painter, option, "<unset>", index);
  }
  else if (valueItem->propType() == CQLottieTreeValueItem::Type::BEZIER) {
    std::optional<std::string> str;

    if (shape) {
      if      (valueItem->propName() == "path") {
        if (shape->path().isSet())
          str = shape->path().tvalueStr(timeFrame);
      }
    }

    if (str)
      drawString(painter, option, QString::fromStdString(*str), index);
    else
      drawString(painter, option, "<unset>", index);

    valueItem->setToolTip(1, QString::fromStdString(str.value_or("")));
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
