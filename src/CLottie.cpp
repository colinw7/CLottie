#include <CLottie.h>
#include <CRGBName.h>

namespace {

std::string depthStr(uint depth) {
  std::string str;
  for (uint i = 0; i < depth; ++i)
    str += "  ";
  return str;
}

void unhandledName(const std::string &name, const CJson::ValueP &value) {
  std::cout << "Unhandled: " << name << "=" << *value << "\n";
}

void todoName(const std::string &msg, const std::string &name, const CJson::ValueP &value) {
  std::cout << "TODO: " << msg << " " << name << "=" << *value << "\n";
}

bool errorMsg(const std::string &prefix, const std::string &msg) {
  std::cerr << "Error: " << prefix << " : " << msg << "\n";
  return false;
}

bool errorValueMsg(const std::string &prefix, const std::string &msg, const CJson::ValueP &value) {
  std::cerr << "Error: " << prefix << " : " << msg << "(" << *value << ")\n";
  return false;
}

bool warnValueMsg(const std::string &prefix, const std::string &msg, const CJson::ValueP &value) {
  std::cerr << "Warning: " << prefix << " : " << msg << "(" << *value << ")\n";
  return false;
}

//---

template<typename T>
void printValue(const std::string &prefix, const std::string &n, const T &value) {
  std::cout << prefix << n << "=" << value << "\n";
}

template<typename T>
void optPrintValue(const std::string &prefix, const std::string &n, const T &value) {
  if (value)
    std::cout << prefix << n << "=" << *value << "\n";
}

template<typename T>
void printProperty(const std::string &prefix, const std::string &n, const T &value) {
  if (value.isSet()) {
    std::cout << prefix << n << "\n";
    value.print(prefix + "  ");
  }
}

}

//--

CLottie::
CLottie()
{
}

CLottie::
~CLottie()
{
  reset();
}

void
CLottie::
reset()
{
  delete root_;

  root_ = nullptr;

  for (auto &pa : assetIds_)
    delete pa.second;

  assets_  .clear();
  assetIds_.clear();

  for (auto *layer : layers_)
    delete layer;

  layers_  .clear();
  layerIds_.clear();

  for (auto *shape : shapes_)
    delete shape;

  shapes_  .clear();
  shapeIds_.clear();

  for (auto *marker : markers_)
    delete marker;

  markers_.clear();
}

bool
CLottie::
load(const std::string &file)
{
  CJson json;

  CJson::ValueP value;
  if (! json.loadFile(file, value))
    return errorMsg("", "json.loadFile");

  if (isDebug())
    std::cout << *value << "\n";

  reset();

  root_ = makeRoot();

  if (! readRoot("root", value))
    return errorMsg("", "readRoot");

  root_->buildLayerHier();

  if (isPrint()) {
    root_->printLayerHier();

    root_->printHier();
  }

  return true;
}

CLottieAsset *
CLottie::
getAssetById(const std::string &id) const
{
  auto pa = assetIds_.find(id);
  if (pa == assetIds_.end()) return nullptr;

  return (*pa).second;
}

CLottieLayer *
CLottie::
getLayerById(int id) const
{
  auto pl = layerIds_.find(id);
  if (pl == layerIds_.end()) return nullptr;

  return (*pl).second;
}

void
CLottie::
deselectAll()
{
  root_->setSelected(false);

  for (auto *layer : layers_)
    layer->setSelected(false);

  for (auto *shape : shapes_)
    shape->setSelected(false);

  for (auto *asset : assets_)
    asset->setSelected(false);
}

bool
CLottie::
readRoot(const std::string &msg, const CJson::ValueP &value)
{
  if (! value->isObject())
    return errorMsg(msg, "root is not an object");

  auto *obj = value->cast<CJson::Object>();

  CJson::Object::Names names;
  obj->getNames(names);

  for (const auto &name : names) {
    auto msg1 = msg + "/" + name;

    CJson::ValueP value1;
    obj->getNamedValue(name, value1);

    if (isDebug())
      std::cout << depthStr(obj->hier_depth()) << name << "=" << *value1 << "\n";

    if      (name == "nm") { // name ?
      root_->setName(valueToString(value1));
    }
    else if (name == "ver") { // version number
      todoName(msg1, name, value1);
    }
    else if (name == "v") { // version
      root_->setVersion(valueToString(value1));
    }
    else if (name == "fr") { // frame rate
      root_->setFrameRate(valueToReal(value1));
    }
    else if (name == "ip") { // frame start (in point)
      root_->setFrameStart(valueToReal(value1));
    }
    else if (name == "op") { // frame stop (out point)
      root_->setFrameStop(valueToReal(value1));
    }
    else if (name == "w") { // width
      root_->setWidth(valueToReal(value1));
    }
    else if (name == "h") { // height
      root_->setHeight(valueToReal(value1));
    }
    else if (name == "ddd") { // 3 dimensional
      root_->setThreeD(valueToBool(value1));
    }
    else if (name == "assets") { // assets
      if (! value1->isArray())
        return errorMsg(msg1, "assets is not an array");

      if (isDebug())
        std::cout << depthStr(value1->hier_depth()) << "Assets:\n";

      auto *assetArray = value1->cast<CJson::Array>();

      int i = 0;

      for (const auto &assetValue : assetArray->values()) {
        if (isDebug())
          std::cout << depthStr(assetValue->hier_depth()) << "[" << i << "]\n";

        auto *asset = makeAsset();

        if (! readAsset(msg1, assetValue, asset))
          return errorMsg(msg1, "readAsset");

        root_->addAsset(asset);

        asset->setParent(root_);

        auto ps = assetIds_.find(asset->id());

        if (ps != assetIds_.end())
          std::cerr << "Duplicate asset id " << asset->id() << "\n";

        assetIds_[asset->id()] = asset;

        assets_.push_back(asset);

        ++i;
      }
    }
    else if (name == "markers") { // markers
      if (! value1->isArray())
        return errorMsg(msg1, "makers is not an array");

      if (isDebug())
        std::cout << depthStr(value1->hier_depth()) << "Markers:\n";

      auto *markerArray = value1->cast<CJson::Array>();

      int i = 0;

      for (const auto &markerValue : markerArray->values()) {
        if (isDebug())
          std::cout << depthStr(markerValue->hier_depth()) << "[" << i << "]\n";

        auto *marker = makeMarker();

        marker->setParent(root_);

        if (! readMarker(msg1, markerValue, marker))
          return errorMsg(msg1, "readMarker");

        addMarker(marker);

        markers_.push_back(marker);

        ++i;
      }
    }
    else if (name == "slots") { // slot ids
      todoName(msg1, name, value1);
    }
    else if (name == "layers") {
      if (! value1->isArray())
        return errorMsg(msg1, "layers is not an array");

      if (isDebug())
        std::cout << depthStr(value1->hier_depth()) << "Layers:\n";

      auto *layerArray = value1->cast<CJson::Array>();

      int i = 0;

      for (const auto &layerValue : layerArray->values()) {
        if (isDebug())
          std::cout << depthStr(layerValue->hier_depth()) << "[" << i << "]\n";

        auto *layer = makeLayer();

        layer->setParent(root_);

        if (! readLayer(msg1, layerValue, layer))
          return errorMsg(msg1, "readLayer");

        addLayer(layer);

        layers_.push_back(layer);

        ++i;
      }
    }
    else if (name == "mn") { // match name
      root_->setMatchName(valueToString(value1));
    }
    else if (name == "meta") { // meta data
      // unhandledName(name, value1);
     }
    else {
      unhandledName(name, value1);
    }
  }

  return true;
}

bool
CLottie::
readLayer(const std::string &msg, const CJson::ValueP &iValue, CLottieLayer *layer)
{
  if (! iValue->isObject())
    return errorMsg(msg, "layer is not an object");

  auto *layerObj = iValue->cast<CJson::Object>();

  CJson::Object::Names names;
  layerObj->getNames(names);

  // get type first
  for (const auto &name : names) {
    if (name != "ty")
      continue;

    CJson::ValueP value1;
    layerObj->getNamedValue(name, value1);

    layer->setTypeId(valueToInt(value1));

    break;
  }

  for (const auto &name : names) {
    auto msg1 = msg + "/" + name;

    CJson::ValueP value1;
    layerObj->getNamedValue(name, value1);

    if (isDebug())
      std::cout << depthStr(layerObj->hier_depth()) << name << "=" << *value1 << "\n";

    if      (name == "nm") { // name
      layer->setName(valueToString(value1));
    }
    else if (name == "mn") { // match name
      layer->setMatchName(valueToString(value1));
    }
    else if (name == "ddd") { // 3d
      layer->setThreeD(valueToBool(value1));
    }
    else if (name == "hd") { // hidden
      layer->setHidden(value1->toBool());
    }
    else if (name == "ty") { // type
      // already done
      //layer->setTypeId(valueToInt(value1));
    }
    else if (name == "ind") { // index
      layer->setInd(valueToInt(value1));
    }
    else if (name == "parent") { // parent
      layer->setParentInd(valueToInt(value1));
    }
    else if (name == "sr") { // time stretch
      layer->setTimeStretch(valueToReal(value1));
    }
    else if (name == "ip") { // frame start (in point)
      layer->setFrameIn(valueToReal(value1));
    }
    else if (name == "op") { // frame stop (out point)
      layer->setFrameOut(valueToReal(value1));
    }
#if 0
    else if (name == "st") { // start time
      layer->setStartTime(valueToReal(value1));
    }
#endif

    // visual
    else if (name == "ks") { // transform
      auto *transform = layer->getTransform();

      if (! readTransform(msg1, value1, transform))
        return false;
    }
    else if (name == "ao") { // auto orient
      layer->setAutoOrient(valueToBool(value1));
    }
    else if (name == "tt") { // matte mode
      layer->setMatteMode(valueToInt(value1));
    }
    else if (name == "tp") { // matte parent
      layer->setMatteParent(valueToInt(value1));
    }
    else if (name == "td") { // matte target
      layer->setMatteTarget(valueToInt(value1));
    }
    else if (name == "hasMask") { // has mask
      layer->setHasMask(valueToBool(value1));
    }
    else if (name == "masksProperties") { // mask properties
      if (! readLayerMaskProperties(msg1, value1, layer))
        return false;
    }
    else if (name == "ef") { // layer effects
      auto *effect = layer->getEffect();

      if (! readEffect(msg1, value1, effect))
        return false;
    }
    else if (name == "mb") { // motion blur
      todoName(msg1, name, value1);
    }
    else if (name == "sy") { // layer style
      todoName(msg1, name, value1);
    }
    else if (name == "bm") { // blend mode
      layer->setBlendMode(valueToInt(value1));
    }
#if 0
    else if (name == "shapes") { // child shapes
      if (! readLayerShapes(msg1, value1, layer))
        return false;
    }
#endif
    else if (name == "cl") { // CSS class
      layer->setCss(valueToString(value1));
    }
    else if (name == "ln") { // Layer XML ID
      //unhandledName(name, value1);
    }
    else if (name == "tg") { // Layer XML tag name
      //unhandledName(name, value1);
    }
    else if (name == "cp") { // Collapse Transform (deprecated)
      unhandledName(name, value1);
    }
    else if (name == "ct") { // Collapse Transform
      unhandledName(name, value1);
    }
    else {
      auto typeId = layer->typeId().value_or(0);

      if      (typeId == 0) { // Precomposition
        auto *precomp = layer->getPrecomp();

        if      (name == "refId") { // asset reference
          precomp->refId = valueToString(value1);
        }
        else if (name == "w") { // width
          precomp->width = valueToReal(value1);
        }
        else if (name == "h") { // height
          precomp->height = valueToReal(value1);
        }
        else if (name == "st") { // start time (dup ?)
          precomp->startTime = valueToReal(value1);
        }
        else if (name == "tm") { // time remap
          if (! readScalarProperty(msg1, value1, precomp->timeRemap))
            return errorMsg(msg1, "readScalarProperty");
        }
        else {
          unhandledName(name, value1);
        }
      }
      else if (typeId == 1) { // Solid
        auto *solid = layer->getSolid();

        if      (name == "sw") { // solid width
          solid->width = valueToReal(value1);
        }
        else if (name == "sh") { // solid height
          solid->height = valueToReal(value1);
        }
        else if (name == "sc") { // solid color
          OptColor c;
          if (! readColor(msg1, value1, c))
            return errorMsg(msg1, "readColor");
          solid->color = c;
        }
        else if (name == "st") { // start time (dup ?)
          layer->setStartTime(valueToReal(value1));
        }
        else {
          unhandledName(name, value1);
        }
      }
      else if (typeId == 2) { // Image
        if      (name == "refId") { // image reference
          layer->setRefId(valueToString(value1));
        }
        else if (name == "st") { // start time (dup ?)
          layer->setStartTime(valueToReal(value1));
        }
        else {
          unhandledName(name, value1);
        }
      }
      else if (typeId == 3) { // Null
        if (name == "st") { // start time (dup ?)
          layer->setStartTime(valueToReal(value1));
        }
        else
          unhandledName(name, value1);
      }
      else if (typeId == 4) { // Shape
        if      (name == "shapes") { // child shapes
          if (! readLayerShapes(msg1, value1, layer))
            return false;
        }
        else if (name == "st") { // start time (dup ?)
          layer->setStartTime(valueToReal(value1));
        }
        else {
          unhandledName(name, value1);
        }
      }
      else {
        std::cerr << "Unexpected layer type '" << typeId << "'\n";
      }
    }
  }

  return true;
}

bool
CLottie::
readLayerMaskProperties(const std::string &msg, const CJson::ValueP &iValue, CLottieLayer *layer)
{
  auto *mask = layer->getMask();

  if (! iValue->isArray())
    return errorMsg(msg, "masksProperties is not an array");

  auto *mArray = iValue->cast<CJson::Array>();

  for (const auto &mValue : mArray->values()) {
    if (! mValue->isObject())
      return errorMsg(msg, "masksProperties array value is not an object");

    auto *mObj = mValue->cast<CJson::Object>();

    CJson::Object::Names mnames;
    mObj->getNames(mnames);

    for (const auto &mname : mnames) {
      auto msg2 = msg + "/" + mname;

      CJson::ValueP mValue1;
      mObj->getNamedValue(mname, mValue1);

      if      (mname == "mode") {
        mask->mode = mValue1->toString();
      }
      else if (mname == "o") {
        if (! readScalarProperty(msg2, mValue1, mask->opacity))
          return errorMsg(msg2, "readScalarProperty");
      }
      else if (mname == "pt") {
        if (! readBezierProperty(msg2, mValue1, mask->path))
          return errorMsg(msg2, "readBezierProperty");
      }
      else if (mname == "x") {
        if (! readScalarProperty(msg2, mValue1, mask->expand))
          return errorMsg(msg2, "readScalarProperty");
      }
      else if (mname == "inv") {
        mask->inverted = valueToBool(mValue1);
      }
      else if (mname == "nm") {
        mask->name = mValue1->toString();
      }
      else
        todoName(msg2, mname, mValue1);
    }
  }

  return true;
}

bool
CLottie::
readLayerShapes(const std::string &msg, const CJson::ValueP &iValue, CLottieLayer *layer)
{
  if (! iValue->isArray())
    return errorMsg(msg, "shapes is not an array");

  if (isDebug())
    std::cout << depthStr(iValue->hier_depth()) << "Shapes:\n";

  auto *shapeArray = iValue->cast<CJson::Array>();

  int i = 0;

  for (const auto &shapeValue : shapeArray->values()) {
    if (isDebug())
      std::cout << depthStr(shapeValue->hier_depth()) << "[" << i << "]\n";

    auto *shape = makeShape();

    if (! readShape(msg, shapeValue, shape))
      return errorMsg(msg, "readShape");

    shape->setParent(layer);

    layer->addShape(shape);

    addShape(shape);

    shapes_.push_back(shape);

    ++i;
  }

  return true;
}

void
CLottie::
addLayer(CLottieLayer *layer)
{
  root_->addLayer(layer);

  if (layer->ind()) {
    auto ind = layer->ind().value();

    auto pl = layerIds_.find(ind);

    if (pl != layerIds_.end())
      std::cerr << "Duplicate layer ind " << ind << "\n";

    layerIds_[ind] = layer;
  }
}

bool
CLottie::
readMarker(const std::string &msg, const CJson::ValueP &iValue, CLottieMarker *)
{
  todoName(msg, "marker", iValue);

  return true;
}

void
CLottie::
addMarker(CLottieMarker *)
{
}

bool
CLottie::
readEffect(const std::string &msg, const CJson::ValueP &iValue, CLottieEffect *effect)
{
  if (! iValue->isArray())
    return errorMsg(msg, "layer effects is not an array");

  auto *efArray = iValue->cast<CJson::Array>();

  for (const auto &efValue : efArray->values()) {
    if (! efValue->isObject())
      return errorMsg(msg, "layer effects array value is not an object");

    auto *efObj = efValue->cast<CJson::Object>();

    CJson::Object::Names efnames;
    efObj->getNames(efnames);

    for (const auto &efname : efnames) {
      if (efname == "ty") {
        CJson::ValueP efValue1;
        efObj->getNamedValue(efname, efValue1);

        effect->setType(valueToInt(efValue1));

        break;
      }
    }

    for (const auto &efname : efnames) {
      auto msg1 = msg + "/" + efname + "(" + std::to_string(effect->type().value_or(0)) + ")";

      CJson::ValueP efValue1;
      efObj->getNamedValue(efname, efValue1);

      if      (efname == "ty") {
        // already done
        //effect->setType(valueToInt(efValue1));
      }
      else if (efname == "nm") {
        effect->setName(valueToString(efValue1));
      }
      else if (efname == "mn") {
        effect->setMatch(valueToString(efValue1));
      }
      else if (efname == "ix") {
        effect->setIndex(valueToInt(efValue1));
      }
      else if (efname == "ef") {
        auto *effect1 = makeEffectValue();

        effect1->parent = effect;

        if (! readEffectValue(msg1, efValue1, effect1))
          return false;

        effect->addValue(effect1);
      }
      else {
        auto etype = effect->type().value_or(0);

        if      (etype == 0) { // slider
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 1) { // angle
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 2) { // color
          if (efname == "v")
            todoName(msg1, efname, efValue1);
          else
            todoName(msg1, efname, efValue1);
        }
        else if (etype == 3) { // point
          if (efname == "v")
            todoName(msg1, efname, efValue1);
          else
            todoName(msg1, efname, efValue1);
        }
        else if (etype == 4) { // checkbox
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 5) { // custom
          if      (efname == "np") {
            effect->setNumProperties(valueToInt(efValue1));
          }
          else if (efname == "en") {
            effect->setEnabled(valueToInt(efValue1));
          }
          else if (efname == "v") {
            todoName(msg1, efname, efValue1);
          }
          else
            todoName(msg1, efname, efValue1);
        }
        else if (etype == 6) { // ignored
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 7) { // drop down
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 10) { // layer
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 20) { // tint
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 21) { // fill
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 22) { // stroke
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 23) { // tritone
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 24) { // pro levels
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 25) { // drop shadow
          if      (efname == "np") {
            effect->setNumProperties(valueToInt(efValue1));
          }
          else if (efname == "en") {
            effect->setEnabled(valueToInt(efValue1));
          }
          else
            todoName(msg1, efname, efValue1);
        }
        else if (etype == 26) { // radial wipe
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 27) { // displacement map
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 28) { // set matte
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 29) { // gaussian blur
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 30) { // twirl
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 31) { // mesh warp
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 32) { // wavy
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 33) { // spherize
          todoName(msg1, efname, efValue1);
        }
        else if (etype == 34) { // puppet
          todoName(msg1, efname, efValue1);
        }
        else {
          todoName(msg1, efname, efValue1);
        }
      }
    }
  }

  return true;
}

bool
CLottie::
readEffectValue(const std::string &msg, const CJson::ValueP &iValue,
                CLottieEffectValue *effectValue) const
{
  if (! iValue->isArray())
    return errorMsg(msg, "layer effects value is not an array");

  auto *efArray = iValue->cast<CJson::Array>();

  for (const auto &efValue : efArray->values()) {
    if (! efValue->isObject())
      return errorMsg(msg, "layer effects value array value is not an object");

    auto *efObj = efValue->cast<CJson::Object>();

    CJson::Object::Names efnames;
    efObj->getNames(efnames);

    for (const auto &efname : efnames) {
      if (efname == "ty") {
        CJson::ValueP efValue1;
        efObj->getNamedValue(efname, efValue1);

        effectValue->type = valueToInt(efValue1);

        break;
      }
    }

    for (const auto &efname : efnames) {
      auto msg1 = msg + "/" + efname + "(" + std::to_string(effectValue->type.value_or(0)) + ")";

      CJson::ValueP efValue1;
      efObj->getNamedValue(efname, efValue1);

      if      (efname == "ty") {
        // already done
        //effectValue->type = valueToInt(efValue1);
      }
      else if (efname == "nm") {
        effectValue->name = valueToString(efValue1);
      }
      else if (efname == "mn") {
        effectValue->match = valueToString(efValue1);
      }
      else if (efname == "ix") {
        effectValue->index = valueToInt(efValue1);
      }
      else {
        if      (effectValue->type == 0) { // slider
          if (efname == "v") {
            if (! readScalarProperty(msg1, efValue1, effectValue->scalar))
              return errorMsg(msg1, "readScalarProperty");
          }
          else
            todoName(msg1, efname, efValue1);
        }
        else if (effectValue->type == 1) { // angle
          if (efname == "v") {
            if (! readScalarProperty(msg1, efValue1, effectValue->scalar))
              return errorMsg(msg1, "readScalarProperty");
          }
          else
            todoName(msg1, efname, efValue1);
        }
        else if (effectValue->type == 2) { // color
          if (efname == "v") {
            if (! readColorProperty(msg1, efValue1, effectValue->color))
              return errorMsg(msg1, "readColorProperty");
          }
          else
            todoName(msg1, efname, efValue1);
        }
        else if (effectValue->type == 3) { // point
          if (efname == "v") {
            if (! readVectorProperty(msg1, efValue1, effectValue->point))
              (void) errorMsg(msg1, "readVectorProperty");
          }
          else
            todoName(msg1, efname, efValue1);
        }
        else if (effectValue->type == 7) { // drop down
          if (efname == "v") {
            if (! readScalarProperty(msg1, efValue1, effectValue->scalar))
              return errorMsg(msg1, "readScalarProperty");
          }
          else
            todoName(msg1, efname, efValue1);
        }
        else if (effectValue->type == 10) { // layer
          if (efname == "v") {
            if (! readScalarProperty(msg1, efValue1, effectValue->scalar))
              return errorMsg(msg1, "readScalarProperty");
          }
          else
            todoName(msg1, efname, efValue1);
        }
        else {
          todoName(msg1, efname, efValue1);
        }
      }
    }
  }

  return true;
}

bool
CLottie::
readShape(const std::string &msg, const CJson::ValueP &iValue, CLottieShape *shape)
{
  if (! iValue->isObject())
    return errorMsg(msg, "shape is not an object");

  auto *shapeObj = iValue->cast<CJson::Object>();

  CJson::Object::Names names;
  shapeObj->getNames(names);

  // get type first
  for (const auto &name : names) {
    if (name != "ty")
      continue;

    CJson::ValueP value1;
    shapeObj->getNamedValue(name, value1);

    shape->setType(valueToString(value1));

    break;
  }

  // get remaining values
  for (const auto &name : names) {
    auto msg1 = msg + "/" + name;

    CJson::ValueP value1;
    shapeObj->getNamedValue(name, value1);

    if (isDebug())
      std::cout << depthStr(shapeObj->hier_depth()) << name << "=" << *value1 << "\n";

    if      (name == "nm") { // name
      shape->setName(valueToString(value1));
    }
    else if (name == "hd") { // hidden
      shape->setHidden(value1->toBool());
    }
    else if (name == "ty") { // type
      // already done
      //shape->setType(valueToString(value1));
    }
    else if (name == "mn") { // match name
      shape->setLongName(valueToString(value1));
    }
    else if (name == "cix") { // property index
      //todoName(msg1, name, value1);
    }
    else if (name == "ix") { // index
      shape->setIndex(valueToInt(value1));
    }
    else if (name == "ind") {
      shape->setInd(valueToInt(value1));
    }
    else {
      auto type = shape->type().value_or("");

      // ellipse
      if      (type == "el") {
        if      (name == "p") { // position
          if (! readPositionProperty(msg1, value1, shape->pos_))
            return errorMsg(msg1, "readPositionProperty");
        }
        else if (name == "s") { // size
          if (! readSizeProperty(msg1, value1, shape->size_))
            return errorMsg(msg1, "readSizeProperty");
        }
        else if (name == "c") { // color
          if (! readColorProperty(msg1, value1, shape->color_))
            return errorMsg(msg1, "readColorProperty");
        }
#if 0
        else if (name == "o") { // opacity
          auto *transform = shape->getTransform();

          if (! readScalarProperty(msg1, value1, transform->opacity))
            return errorMsg(msg1, "readScalarProperty");
        }
#endif
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // fill
      else if (type == "fl") {
        auto *fill = shape->getFill();

        if      (name == "c") { // color
          if (! readColorProperty(msg1, value1, fill->color))
            return errorMsg(msg1, "readColorProperty");
        }
        else if (name == "o") { // opacity
          if (! readScalarProperty(msg1, value1, fill->opacity))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "r") { // fill rule
          fill->fillRule = valueToInt(value1);
        }
        else if (name == "bm") { // blend mode
          fill->blendMode = valueToInt(value1);
        }
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else if (name == "fillEnabled") { // blend mode
          fill->fillEnabled = valueToBool(value1);
        }
        else
          unhandledName(name, value1);
      }
      // gradient fill
      else if (type == "gf") {
        auto *gradientFill = shape->getGradientFill();

        if      (name == "c") { // color
          if (! readColorProperty(msg1, value1, gradientFill->color))
            return errorMsg(msg1, "readColorProperty");
        }
        else if (name == "o") { // opacity
          if (! readScalarProperty(msg1, value1, gradientFill->opacity))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "r") { // fill rule
          gradientFill->fillRule = valueToInt(value1);
        }
        else if (name == "bm") { // blend mode
          gradientFill->blendMode = valueToInt(value1);
        }
        else if (name == "g") { // gradient fill colors
          if (! value1->isObject())
            return errorMsg(msg1, "gradient fill colors not an object");

          auto *gObj = value1->cast<CJson::Object>();

          CJson::Object::Names gnames;
          gObj->getNames(gnames);

          for (const auto &gname : gnames) {
            auto msg2 = msg1 + "/" + gname;

            CJson::ValueP gvalue;
            gObj->getNamedValue(gname, gvalue);

            if (isDebug())
              std::cout << depthStr(gObj->hier_depth()) << gname << "=" << *gvalue << "\n";

            if      (gname == "p") { // color stop count
              gradientFill->stopCount = valueToInt(gvalue);
            }
            else if (gname == "k") {
              if (! gvalue->isObject())
                return errorMsg(msg1, "gradient fill colors k value not an object");

              auto *kObj = gvalue->cast<CJson::Object>();

              CJson::Object::Names knames;
              kObj->getNames(knames);

              for (const auto &kname : knames) {
                auto msg3 = msg2 + "/" + kname;

                CJson::ValueP kvalue;
                kObj->getNamedValue(kname, kvalue);

                if (isDebug())
                  std::cout << depthStr(kObj->hier_depth()) << kname << "=" << *kvalue << "\n";

                if      (kname == "a") {
                  gradientFill->colors.animated = valueToBool(kvalue);
                }
                else if (kname == "k") {
                  if (! kvalue->isArray())
                    return errorMsg(msg1, "k is not an array");

                  auto *kArray = kvalue->cast<CJson::Array>();

                  if (! gradientFill->colors.animated && kArray->size() > 0 &&
                      kArray->at(0)->isObject())
                    gradientFill->colors.animated = true;

                  if (gradientFill->colors.animated.value_or(false)) {
                    for (const auto &kvalue1 : kArray->values()) {
                      if (! kvalue1->isObject())
                        return errorMsg(msg1, "gradient fill colors k value not an object");

                      ArrayProperty::KeyFrame keyFrame;

                      auto *kObj1 = kvalue1->cast<CJson::Object>();

                      CJson::Object::Names knames1;
                      kObj1->getNames(knames1);

                      for (const auto &kname1 : knames1) {
                        auto msg4 = msg3 + "/" + kname1;

                        CJson::ValueP kvalue2;
                        kObj1->getNamedValue(kname1, kvalue2);

                        if      (kname1 == "i") { // input
                          getKeyFrameValues(msg2, kname1, kvalue2, keyFrame.ivalues);
                        }
                        else if (kname1 == "o") { // output
                          getKeyFrameValues(msg2, kname1, kvalue2, keyFrame.ovalues);
                        }
                        else if (kname1 == "s") { // start value
                          std::vector<double> startValue;
                          if (! readNumbers(msg2, kvalue2, startValue))
                            return errorMsg(msg2, "readNumbers");
                          keyFrame.startValue.vals.push_back(startValue);
                        }
                        else if (kname1 == "e") { // end value
                          std::vector<double> endValue;
                          if (! readNumbers(msg2, kvalue2, endValue))
                            return errorMsg(msg2, "readNumbers");
                          keyFrame.endValue.vals.push_back(endValue);
                        }
                        else if (kname1 == "n") { // interpolation key
                          if (! readStrings(msg2, kvalue2, keyFrame.interpolation))
                            return errorMsg(msg2, "readStrings");
                        }
                        else if (kname1 == "t") { // ???
                          keyFrame.timeFrame = valueToReal(kvalue2);
                        }
                        else
                          unhandledName(kname1, kvalue2);
                      }

                      gradientFill->colors.keyFrames.push_back(keyFrame);
                    }
                  }
                  else {
                    std::vector<double> numbers;
                    if (! readNumbers(msg3, kvalue, numbers))
                      return errorMsg(msg3, "readNumbers");
                    gradientFill->colors.values.push_back(numbers);
                  }
                }
                else if (kname == "ix") {
                  gradientFill->colors.index = valueToInt(kvalue);
                }
                else
                  unhandledName(kname, kvalue);
              }
            }
            else if (name == "ix") {
              gradientFill->index = valueToInt(gvalue);
            }
            else
              unhandledName(gname, gvalue);
          }
        }
        else if (name == "s") { // start point
          if (! readVectorProperty(msg1, value1, gradientFill->startPoint))
            return errorMsg(msg1, "readVectorProperty");
        }
        else if (name == "e") { // end point
          if (! readVectorProperty(msg1, value1, gradientFill->endPoint))
            return errorMsg(msg1, "readVectorProperty");
        }
        else if (name == "t") { // gradient type
          gradientFill->type = valueToInt(value1);
        }
        else if (name == "h") { // highlight length
          if (! readScalarProperty(msg1, value1, gradientFill->highlightLength))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "a") { // highlight angle
          if (! readScalarProperty(msg1, value1, gradientFill->highlightAngle))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // group
      else if (type == "gr") {
        auto *group = shape->getGroup();

        if      (name == "np") { // number of properties
          group->numProperties = valueToInt(value1);
        }
        else if (name == "it") { // shapes (children)
          if (! value1->isArray())
            return errorMsg(msg1, "it is not an array");

          if (isDebug())
            std::cout << depthStr(value1->hier_depth()) << "Shapes:\n";

          auto *itArray = value1->cast<CJson::Array>();

          int i = 0;

          for (const auto &itValue : itArray->values()) {
            if (isDebug())
              std::cout << depthStr(itValue->hier_depth()) << "[" << i << "]\n";

            auto *shape1 = makeShape();

            if (! readShape(msg1, itValue, shape1))
              return errorMsg(msg1, "readShape");

            shape1->setParent(shape);

            shape->addShape(shape1);

            addShape(shape);

            shapes_.push_back(shape1);

            ++i;
          }
        }
        else if (name == "c") { // color
          if (! readColorProperty(msg1, value1, group->color))
            return errorMsg(msg1, "readColorProperty");
        }
        else if (name == "o") { // opacity
          if (! readScalarProperty(msg1, value1, group->opacity))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "bm") { // blend mode
          group->blendMode = valueToInt(value1);
        }
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // gradient stroke
      else if (type == "gs") {
        auto *gradientStroke = shape->getGradientStroke();

        if      (name == "o") { // opacity
          if (! readScalarProperty(msg1, value1, gradientStroke->opacity))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "w") { // stroke width
          if (! readScalarProperty(msg1, value1, gradientStroke->width))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "g") { // colors
          if (! value1->isObject())
            return errorMsg(msg1, "gradient stroke colors not an object");

          auto *gObj = value1->cast<CJson::Object>();

          CJson::Object::Names gnames;
          gObj->getNames(gnames);

          for (const auto &gname : gnames) {
            auto msg2 = msg1 + "/" + gname;

            CJson::ValueP gvalue;
            gObj->getNamedValue(gname, gvalue);

            if (isDebug())
              std::cout << depthStr(gObj->hier_depth()) << gname << "=" << *gvalue << "\n";

            if      (gname == "p") { // color stop count
              gradientStroke->stopCount = valueToInt(gvalue);
            }
            else if (gname == "k") {
              if (! gvalue->isObject())
                return errorMsg(msg1, "gradient stroke colors k value not an object");

              auto *kObj = gvalue->cast<CJson::Object>();

              CJson::Object::Names knames;
              kObj->getNames(knames);

              for (const auto &kname : knames) {
                auto msg3 = msg2 + "/" + kname;

                CJson::ValueP kvalue;
                kObj->getNamedValue(kname, kvalue);

                if (isDebug())
                  std::cout << depthStr(kObj->hier_depth()) << kname << "=" << *kvalue << "\n";

                if      (kname == "a") {
                  gradientStroke->colors.animated = valueToBool(kvalue);
                }
                else if (kname == "k") {
                  if (! kvalue->isArray())
                    return errorMsg(msg1, "k is not an array");

                  auto *kArray = kvalue->cast<CJson::Array>();

                  if (! gradientStroke->colors.animated && kArray->size() > 0 &&
                      kArray->at(0)->isObject())
                    gradientStroke->colors.animated = true;

                  if (gradientStroke->colors.animated.value_or(false)) {
                    for (const auto &kvalue1 : kArray->values()) {
                      if (! kvalue1->isObject())
                        return errorMsg(msg1, "gradient stroke colors k value not an object");

                      ArrayProperty::KeyFrame keyFrame;

                      auto *kObj1 = kvalue1->cast<CJson::Object>();

                      CJson::Object::Names knames1;
                      kObj1->getNames(knames1);

                      for (const auto &kname1 : knames1) {
                        auto msg4 = msg3 + "/" + kname1;

                        CJson::ValueP kvalue2;
                        kObj1->getNamedValue(kname1, kvalue2);

                        if      (kname1 == "i") { // input
                          getKeyFrameValues(msg2, kname1, kvalue2, keyFrame.ivalues);
                        }
                        else if (kname1 == "o") { // output
                          getKeyFrameValues(msg2, kname1, kvalue2, keyFrame.ovalues);
                        }
                        else if (kname1 == "s") { // start value
                          std::vector<double> startValue;
                          if (! readNumbers(msg2, kvalue2, startValue))
                            return errorMsg(msg2, "readNumbers");
                          keyFrame.startValue.vals.push_back(startValue);
                        }
                        else if (kname1 == "e") { // end value
                          std::vector<double> endValue;
                          if (! readNumbers(msg2, kvalue2, endValue))
                            return errorMsg(msg2, "readVector");
                          keyFrame.endValue.vals.push_back(endValue);
                        }
                        else if (kname1 == "n") { // interpolation key
                          if (! readStrings(msg2, kvalue2, keyFrame.interpolation))
                            return errorMsg(msg2, "readStrings");
                        }
                        else if (kname1 == "t") { // ???
                          keyFrame.timeFrame = valueToReal(kvalue2);
                        }
                        else
                          unhandledName(kname1, kvalue2);
                      }

                      gradientStroke->colors.keyFrames.push_back(keyFrame);
                    }
                  }
                  else {
                    std::vector<double> numbers;
                    if (! readNumbers(msg3, kvalue, numbers))
                      return errorMsg(msg3, "readNumbers");
                    gradientStroke->colors.values.push_back(numbers);
                  }
                }
                else if (kname == "ix") {
                  gradientStroke->colors.index = valueToInt(kvalue);
                }
                else
                  unhandledName(kname, kvalue);
              }
            }
            else if (name == "ix") {
              gradientStroke->index = valueToInt(gvalue);
            }
            else
              unhandledName(gname, gvalue);
          }
        }
        else if (name == "s") { // start point
          if (! readVectorProperty(msg1, value1, gradientStroke->endPoint))
            return errorMsg(msg1, "readVectorProperty");
        }
        else if (name == "e") { // end point
          if (! readVectorProperty(msg1, value1, gradientStroke->endPoint))
            return errorMsg(msg1, "readVectorProperty");
        }
        else if (name == "t") {
          gradientStroke->type = valueToInt(value1);
        }
        else if (name == "lc") {
          gradientStroke->lineCap = valueToInt(value1);
        }
        else if (name == "lj") {
          gradientStroke->lineJoin = valueToInt(value1);
        }
        else if (name == "ml") {
          gradientStroke->miterLimit = valueToReal(value1);
        }
        else if (name == "d") { // direction
          Dash dash;
          if (! readDash(msg1, value1, gradientStroke->dash))
            return errorMsg(msg1, "readDash");
        }
        else
          unhandledName(name, value1);
      }
      // merge path
      else if (type == "mm") {
        auto *merge = shape->getMerge();

        if      (name == "mm") { // merge mode
          merge->mode = valueToInt(value1);
        }
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // no style
      else if (type == "no") {
        unhandledName(name, value1);
      }
      // offset path
      else if (type == "op") {
        unhandledName(name, value1);
      }
      // path
      else if (type == "sh") {
        if      (name == "ks") { // bezier path
          if (! readBezierProperty(msg1, value1, shape->path_))
            return errorMsg(msg1, "readBezierProperty");
        }
        else if (name == "c") { // color
          if (! readColorProperty(msg1, value1, shape->color_))
            return errorMsg(msg1, "readColorProperty");
        }
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // polystar
      else if (type == "sr") {
        auto *polyStar = shape->getPolyStar();

        if      (name == "p") { // position
          if (! readPositionProperty(msg1, value1, polyStar->position))
            return errorMsg(msg1, "readPositionProperty");
        }
        else if (name == "or") { // outer radius
          if (! readScalarProperty(msg1, value1, polyStar->outerRadius))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "os") { // outer roundness
          if (! readScalarProperty(msg1, value1, polyStar->outerRoundness))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "r") { // rotation
          if (! readScalarProperty(msg1, value1, polyStar->rotation))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "pt") { // points
          if (! readScalarProperty(msg1, value1, polyStar->points))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "sy") { // star type
          polyStar->type = valueToInt(value1);
        }
        else if (name == "ir") { // inner radius
          if (! readScalarProperty(msg1, value1, polyStar->innerRadius))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "is") { // inner roundness
          if (! readScalarProperty(msg1, value1, polyStar->innerRoundness))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "c") { // color
          if (! readColorProperty(msg1, value1, shape->color_))
            return errorMsg(msg1, "readColorProperty");
        }
#if 0
        else if (name == "o") { // opacity
          auto *transform = shape->getTransform();

          if (! readScalarProperty(msg1, value1, transform->opacity))
            return errorMsg(msg1, "readScalarProperty");
        }
#endif
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // pucker bloat
      else if (type == "pb") {
        unhandledName(name, value1);
      }
      // rectangle
      else if (type == "rc") {
        auto *rectangle = shape->getRectangle();

        if      (name == "p") { // position
          if (! readPositionProperty(msg1, value1, shape->pos_))
            return errorMsg(msg1, "readPositionProperty");
        }
        else if (name == "s") { // size
          if (! readSizeProperty(msg1, value1, shape->size_))
            return errorMsg(msg1, "readSizeProperty");
        }
        else if (name == "r") { // roundness (fill rule for fill shape ?)
          if (! readScalarProperty(msg1, value1, rectangle->roundness))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "c") { // color
          if (! readColorProperty(msg1, value1, shape->color_))
            return errorMsg(msg1, "readColorProperty");
        }
#if 0
        else if (name == "o") { // opacity
          auto *transform = shape->getTransform();

          if (! readScalarProperty(msg1, value1, transform->opacity))
            return errorMsg(msg1, "readScalarProperty");
        }
#endif
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // repeater
      else if (type == "rp") {
        auto *repeater = shape->getRepeater();

        if      (name == "c") { // copies
          if (! readScalarProperty(msg1, value1, repeater->copies))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "o") { // offset
          if (! readScalarProperty(msg1, value1, repeater->offset))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "m") { // composite
          repeater->composite = valueToInt(value1);
        }
        else if (name == "tr") { // transform
          if (! repeater->transform)
            repeater->transform = new Transform;

          repeater->transform->repeater = repeater;

          if (! readTransform(msg1, value1, repeater->transform))
            return false;
        }
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // rounded corners
      else if (type == "rd") {
        auto *rounded = shape->getRounded();

        if      (name == "r") {
          if (! readScalarProperty(msg1, value1, rounded->roundness))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // stroke
      else if (type == "st") {
        auto *stroke = shape->getStroke();

        if      (name == "c") { // color
          if (! readColorProperty(msg1, value1, stroke->color))
            return errorMsg(msg1, "readColorProperty");
        }
        else if (name == "o") { // opacity
          if (! readScalarProperty(msg1, value1, stroke->opacity))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "w") { // stroke width
          if (! readScalarProperty(msg1, value1, stroke->width))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "lc") { // line cap
          stroke->lineCap = valueToInt(value1);
        }
        else if (name == "lj") { // line join
          stroke->lineJoin = valueToInt(value1);
        }
        else if (name == "ml") { // miter limit
          stroke->miterLimit = valueToReal(value1);
        }
        else if (name == "ml2") { // miter limit (animate)
          if (! readScalarProperty(msg1, value1, stroke->miterLimitAnim))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "bm") { // blend mode
          stroke->blendMode = valueToInt(value1);
        }
        else if (name == "d") { // dash
          if (! readDash(msg1, value1, stroke->dash))
            return errorMsg(msg1, "readDash");
        }
        else
          unhandledName(name, value1);
      }
      // transform
      else if (type == "tr") {
        auto *transform = shape->getTransform();

        if      (name == "a") { // anchor point
          if (! readPositionProperty(msg1, value1, transform->anchorPoint))
            return errorMsg(msg1, "readPositionProperty");
        }
        else if (name == "p") { // position
          if (! readSplitPositionProperty(msg1, value1, transform->position))
            return errorMsg(msg1, "readSplitPositionProperty");
        }
        else if (name == "r") { // rotation
          if (! readScalarProperty(msg1, value1, transform->rotation))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "s") { // scale
          if (! readVectorProperty(msg1, value1, transform->scale))
            return errorMsg(msg1, "readVectorProperty");
        }
        else if (name == "sk") {
          if (! readScalarProperty(msg1, value1, transform->skew))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "sa") {
          if (! readScalarProperty(msg1, value1, transform->skewAxis))
            return errorMsg(msg1, "readScalarProperty");
        }
#if 0
        else if (name == "c") { // color
          if (! readColorProperty(msg1, value1, transform->color_))
            return errorMsg(msg1, "readColorProperty");
        }
#endif
        else if (name == "o") { // opacity
          if (! readScalarProperty(msg1, value1, transform->opacity))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // trim path
      else if (type == "tm") {
        auto *trim = shape->getTrim();

        if      (name == "s") { // start
          if (! readScalarProperty(msg1, value1, trim->start))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "e") { // start
          if (! readScalarProperty(msg1, value1, trim->end))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "o") { // offset
          if (! readScalarProperty(msg1, value1, trim->offset))
            return errorMsg(msg1, "readScalarProperty");
        }
        else if (name == "m") { // multiple
          trim->multiple = valueToInt(value1);
        }
        else if (name == "d") { // direction
          shape->setDirection(valueToInt(value1));
        }
        else
          unhandledName(name, value1);
      }
      // twist
      else if (type == "tw") {
        unhandledName(name, value1);
      }
      // zig zag
      else if (type == "zz") {
        unhandledName(name, value1);
      }
      else {
        std::cerr << "Unexpected shape type '" << type << "'\n";
      }
    }
  }

  return true;
}

void
CLottie::
addShape(CLottieShape *shape)
{
//shape->parent->shapes.push_back(shape);

  if (shape->ind()) {
    auto ind = shape->ind().value();

    auto pl = shapeIds_.find(ind);

    if (pl != shapeIds_.end())
      std::cerr << "Duplicate shape ind " << ind << "\n";

    shapeIds_[ind] = shape;
  }
}

bool
CLottie::
readAsset(const std::string &msg, const CJson::ValueP &iValue, CLottieAsset *asset)
{
  if (! iValue->isObject())
    return errorMsg(msg, "asset is object");

  auto *assetObj = iValue->cast<CJson::Object>();

  CJson::Object::Names names;
  assetObj->getNames(names);

  for (const auto &name : names) {
    auto msg1 = msg + "/" + name;

    CJson::ValueP value1;
    assetObj->getNamedValue(name, value1);

    if (isDebug())
      std::cout << depthStr(assetObj->hier_depth()) << name << "=" << *value1 << "\n";

    if      (name == "id") { // id
      asset->setId(valueToString(value1));
    }
    else if (name == "w") { // width
      asset->setWidth(valueToReal(value1));
    }
    else if (name == "h") { // height
      asset->setHeight(valueToReal(value1));
    }
    else if (name == "u") { // File path
      asset->setDir(valueToString(value1));
    }
    else if (name == "p") { // File name
      asset->setPath(valueToString(value1));
    }
    else if (name == "cl") { // CSS class
      asset->setCss(valueToString(value1));
    }
    else if (name == "e") { // Embedded
      asset->setEmbedded(valueToBool(value1));
    }
    else if (name == "layers") {
      if (! value1->isArray())
        return errorMsg(msg1, "layers is not an array");

      if (isDebug())
        std::cout << depthStr(value1->hier_depth()) << "Layers:\n";

      auto *layerArray = value1->cast<CJson::Array>();

      int i = 0;

      for (const auto &layerValue : layerArray->values()) {
        if (isDebug())
          std::cout << depthStr(layerValue->hier_depth()) << "[" << i << "]\n";

        auto *layer = makeLayer();

        if (! readLayer(msg1, layerValue, layer))
          return errorMsg(msg1, "readLayer");

        asset->addLayer(layer);

        layer->setParent(asset);

        layers_.push_back(layer);

        ++i;
      }
    }
    else if (name == "t") { // type
      unhandledName(name, value1);
    }
    else
      unhandledName(name, value1);
  }

  return true;
}

bool
CLottie::
readSplitPositionProperty(const std::string &msg, const CJson::ValueP &ivalue,
                          SplitPositionProperty &position) const
{
  if (! ivalue->isObject())
    return errorMsg(msg, "split position is object");

  auto *sObj = ivalue->cast<CJson::Object>();

  CJson::Object::Names names;
  sObj->getNames(names);

  for (const auto &name1 : names) {
    auto msg1 = msg + "/" + name1;

    CJson::ValueP value2;
    sObj->getNamedValue(name1, value2);

    if (isDebug())
      std::cout << depthStr(sObj->hier_depth()) << name1 << "=" << *value2 << "\n";

    if      (name1 == "a") {
      position.animated = valueToBool(value2);
    }
    else if (name1 == "s") {
      position.split = valueToBool(value2);
    }
    else if (name1 == "k") {
      if (! value2->isArray())
        return errorMsg(msg1, "k is not an array");

      auto *kArray2 = value2->cast<CJson::Array>();

      if (! position.animated && kArray2->size() > 0 && kArray2->at(0)->isObject())
        position.animated = true;

      if (position.animated.value_or(false)) {
        for (const auto &kValue : kArray2->values()) {
          if (! kValue->isObject())
            return errorMsg(msg1, "k value is not an object");

          SplitPositionProperty::KeyFrame keyFrame;

          auto *kObj3 = kValue->cast<CJson::Object>();

          CJson::Object::Names names2;
          kObj3->getNames(names2);

          for (const auto &name2 : names2) {
            auto msg2 = msg1 + "/" + name2;

            CJson::ValueP value3;
            kObj3->getNamedValue(name2, value3);

            if (isDebug())
              std::cout << depthStr(kObj3->hier_depth()) << name2 << "=" << *value3 << "\n";

            if      (name2 == "i") { // input
              getKeyFrameValues(msg2, name2, value3, keyFrame.ivalues);
            }
            else if (name2 == "o") { // output
              getKeyFrameValues(msg2, name2, value3, keyFrame.ovalues);
            }
            else if (name2 == "s") { // start value
              CPoint2D startValue;
              if (! readVector(msg2, value3, startValue))
                return errorMsg(msg2, "readVector");
              keyFrame.startValue.vals.push_back(startValue);
            }
            else if (name2 == "e") { // end value
              CPoint2D endValue;
              if (! readVector(msg2, value3, endValue))
                return errorMsg(msg2, "readVector");
              keyFrame.endValue.vals.push_back(endValue);
            }
            else if (name2 == "n") { // interpolation key
              if (! readStrings(msg2, value3, keyFrame.interpolation))
                return errorMsg(msg2, "readStrings");
            }
            else if (name2 == "h") { // hold
              keyFrame.hold = valueToInt(value3);
            }
            else if (name2 == "ti") { // Value in Tangent
              CPoint2D tangentIn;
              if (! readVector(msg2, value3, tangentIn))
                return errorMsg(msg2, "readVector");
              keyFrame.tangentIn = tangentIn;
            }
            else if (name2 == "to") { // Value Out Tangent
              CPoint2D tangentOut;
              if (! readVector(msg2, value3, tangentOut))
                return errorMsg(msg2, "readVector");
              keyFrame.tangentOut = tangentOut;
            }
            else if (name2 == "t") { // ???
              keyFrame.timeFrame = valueToReal(value3);
            }
            else
              unhandledName(name2, value3);
          }

          position.keyFrames.push_back(keyFrame);
        }
      }
      else {
        CPoint2D p;
        if (! readVector(msg1, value2, p))
          return false;

        position.values.push_back(p);
      }
    }
    else if (name1 == "x") {
      if (value2->isString()) { // expression
        warnValueMsg(msg1, "x is expression", value2);
      }
      else {
        if (! readScalarProperty(msg1, value2, position.x))
          return errorMsg(msg1, "readScalarProperty");
      }
    }
    else if (name1 == "y") {
      if (! readScalarProperty(msg1, value2, position.y))
        return errorMsg(msg1, "readScalarProperty");
    }
    else if (name1 == "ix") {
      position.index = valueToInt(value2);
    }
    else if (name1 == "l") {
      position.length = valueToInt(value2);
    }
    else
      unhandledName(name1, value2);
  }

  return true;
}

bool
CLottie::
readVectorProperty(const std::string &msg, const CJson::ValueP &ivalue,
                   VectorProperty &vector) const
{
  if (! ivalue->isObject())
    return errorMsg(msg, "vector is object");

  auto *sObj = ivalue->cast<CJson::Object>();

  CJson::Object::Names names;
  sObj->getNames(names);

  for (const auto &name1 : names) {
    auto msg1 = msg + "/" + name1;

    CJson::ValueP value2;
    sObj->getNamedValue(name1, value2);

    if (isDebug())
      std::cout << depthStr(sObj->hier_depth()) << name1 << "=" << *value2 << "\n";

    if      (name1 == "a") {
      vector.animated = valueToBool(value2);
    }
    else if (name1 == "k") {
      if (value2->isArray()) {
        auto *kArray2 = value2->cast<CJson::Array>();

        if (! vector.animated && kArray2->size() > 0 && kArray2->at(0)->isObject())
          vector.animated = true;

        if (vector.animated.value_or(false)) {
          for (const auto &kValue : kArray2->values()) {
            if (! kValue->isObject())
              return errorMsg(msg1, "k value is not an object");

            VectorProperty::KeyFrame keyFrame;

            auto *kObj3 = kValue->cast<CJson::Object>();

            CJson::Object::Names names2;
            kObj3->getNames(names2);

            for (const auto &name2 : names2) {
              auto msg2 = msg1 + "/" + name2;

              CJson::ValueP value3;
              kObj3->getNamedValue(name2, value3);

              if (isDebug())
                std::cout << depthStr(kObj3->hier_depth()) << name2 << "=" << *value3 << "\n";

              if      (name2 == "i") { // input
                getKeyFrameValues(msg2, name2, value3, keyFrame.ivalues);
              }
              else if (name2 == "o") { // output
                getKeyFrameValues(msg2, name2, value3, keyFrame.ovalues);
              }
              else if (name2 == "s") { // start value
                CPoint2D startValue;
                if (! readVector(msg2, value3, startValue))
                  return errorMsg(msg2, "readVector");
                keyFrame.startValue.vals.push_back(startValue);
              }
              else if (name2 == "e") { // end value
                CPoint2D endValue;
                if (! readVector(msg2, value3, endValue))
                  return errorMsg(msg2, "readVector");
                keyFrame.endValue.vals.push_back(endValue);
              }
              else if (name2 == "n") { // interpolation key
                if (! readStrings(msg2, value3, keyFrame.interpolation))
                  return errorMsg(msg2, "readStrings");
              }
              else if (name2 == "h") { // hold
                keyFrame.hold = valueToInt(value3);
              }
              else if (name2 == "ti") { // Value in Tangent
                CPoint2D tangentIn;
                if (! readVector(msg2, value3, tangentIn))
                  return errorMsg(msg2, "readVector");
                keyFrame.tangentIn = tangentIn;
              }
              else if (name2 == "to") { // Value Out Tangent
                CPoint2D tangentOut;
                if (! readVector(msg2, value3, tangentOut))
                  return errorMsg(msg2, "readVector");
                keyFrame.tangentOut = tangentOut;
              }
              else if (name2 == "t") { // ???
                keyFrame.timeFrame = valueToReal(value3);
              }
              else
                unhandledName(name2, value3);
            }

            vector.keyFrames.push_back(keyFrame);
          }
        }
        else {
          CPoint2D p;
          if (! readVector(msg1, value2, p))
            return false;

          vector.values.push_back(p);
        }
      }
      else {
        auto r = valueToReal(value2);

        vector.values.push_back(CPoint2D(r, r));
      }
    }
    else if (name1 == "ix") {
      vector.index = valueToInt(value2);
    }
    else if (name1 == "l") {
      vector.length = valueToInt(value2);
    }
    else
      unhandledName(name1, value2);
  }

  return true;
}

bool
CLottie::
readPositionProperty(const std::string &msg, const CJson::ValueP &ivalue,
                     PositionProperty &position) const
{
  if (! ivalue->isObject())
    return errorMsg(msg, "position is object");

  auto *sObj = ivalue->cast<CJson::Object>();

  CJson::Object::Names names;
  sObj->getNames(names);

  for (const auto &name1 : names) {
    auto msg1 = msg + "/" + name1;

    CJson::ValueP value2;
    sObj->getNamedValue(name1, value2);

    if (isDebug())
      std::cout << depthStr(sObj->hier_depth()) << name1 << "=" << *value2 << "\n";

    if      (name1 == "a") {
      position.animated = valueToBool(value2);
    }
    else if (name1 == "k") {
      if (! value2->isArray())
        return errorMsg(msg1, "k is not an array");

      auto *kArray2 = value2->cast<CJson::Array>();

      if (position.animated.value_or(false)) {
        for (const auto &kValue : kArray2->values()) {
          if (! kValue->isObject())
            return errorMsg(msg1, "k value is not an object");

          PositionProperty::KeyFrame keyFrame;

          auto *kObj3 = kValue->cast<CJson::Object>();

          CJson::Object::Names names2;
          kObj3->getNames(names2);

          for (const auto &name2 : names2) {
            auto msg2 = msg1 + "/" + name2;

            CJson::ValueP value3;
            kObj3->getNamedValue(name2, value3);

            if (isDebug())
              std::cout << depthStr(kObj3->hier_depth()) << name2 << "=" << *value3 << "\n";

            if      (name2 == "i") { // input
              getKeyFrameValues(msg2, name2, value3, keyFrame.ivalues);
            }
            else if (name2 == "o") { // output
              getKeyFrameValues(msg2, name2, value3, keyFrame.ovalues);
            }
            else if (name2 == "s") { // start value
              CPoint2D startValue;
              if (! readVector(msg2, value3, startValue))
                return errorMsg(msg2, "readVector");
              keyFrame.startValue.vals.push_back(startValue);
            }
            else if (name2 == "e") { // end value
              CPoint2D endValue;
              if (! readVector(msg2, value3, endValue))
                return errorMsg(msg2, "readVector");
              keyFrame.endValue.vals.push_back(endValue);
            }
            else if (name2 == "n") { // interpolation key
              if (! readStrings(msg2, value3, keyFrame.interpolation))
                return errorMsg(msg2, "readStrings");
            }
            else if (name2 == "h") { // hold
              keyFrame.hold = valueToInt(value3);
            }
            else if (name2 == "ti") { // Value in Tangent
              CPoint2D tangentIn;
              if (! readVector(msg2, value3, tangentIn))
                return errorMsg(msg2, "readVector");
              keyFrame.tangentIn = tangentIn;
            }
            else if (name2 == "to") { // Value Out Tangent
              CPoint2D tangentOut;
              if (! readVector(msg2, value3, tangentOut))
                return errorMsg(msg2, "readVector");
              keyFrame.tangentOut = tangentOut;
            }
            else if (name2 == "t") { // ???
              keyFrame.timeFrame = valueToReal(value3);
            }
            else
              unhandledName(name2, value3);
          }

          position.keyFrames.push_back(keyFrame);
        }
      }
      else {
        CPoint2D p;
        if (! readVector(msg1, value2, p))
          return false;

        XYVals xyvals;

        xyvals.xvals.push_back(p.x);
        xyvals.yvals.push_back(p.y);

        position.values.push_back(xyvals);
      }
    }
    else if (name1 == "ix") {
      position.index = valueToInt(value2);
    }
    else if (name1 == "l") {
      position.length = valueToInt(value2);
    }
    else
      unhandledName(name1, value2);
  }

  return true;
}

bool
CLottie::
readSizeProperty(const std::string &msg, const CJson::ValueP &ivalue, SizeProperty &size) const
{
  if (! ivalue->isObject())
    return errorMsg(msg, "size is object");

  auto *sObj = ivalue->cast<CJson::Object>();

  CJson::Object::Names names;
  sObj->getNames(names);

  for (const auto &name1 : names) {
    auto msg1 = msg + "/" + name1;

    CJson::ValueP value2;
    sObj->getNamedValue(name1, value2);

    if (isDebug())
      std::cout << depthStr(sObj->hier_depth()) << name1 << "=" << *value2 << "\n";

    if      (name1 == "a") {
      size.animated = valueToBool(value2);
    }
    else if (name1 == "k") {
      if (! value2->isArray())
        return errorMsg(msg1, "k is not an array");

      auto *kArray2 = value2->cast<CJson::Array>();

      if (size.animated.value_or(false)) {
        for (const auto &kValue : kArray2->values()) {
          if (! kValue->isObject())
            return errorMsg(msg1, "k value is not an object");

          SizeProperty::KeyFrame keyFrame;

          auto *kObj3 = kValue->cast<CJson::Object>();

          CJson::Object::Names names2;
          kObj3->getNames(names2);

          for (const auto &name2 : names2) {
            auto msg2 = msg1 + "/" + name2;

            CJson::ValueP value3;
            kObj3->getNamedValue(name2, value3);

            if (isDebug())
              std::cout << depthStr(kObj3->hier_depth()) << name2 << "=" << *value3 << "\n";

            if      (name2 == "i") { // input
              getKeyFrameValues(msg2, name2, value3, keyFrame.ivalues);
            }
            else if (name2 == "o") { // output
              getKeyFrameValues(msg2, name2, value3, keyFrame.ovalues);
            }
            else if (name2 == "s") { // start value
              CPoint2D startValue;
              if (! readVector(msg2, value3, startValue))
                return errorMsg(msg2, "readVector");
              keyFrame.startValue.vals.push_back(startValue);
            }
            else if (name2 == "e") { // end value
              CPoint2D endValue;
              if (! readVector(msg2, value3, endValue))
                return errorMsg(msg2, "readVector");
              keyFrame.endValue.vals.push_back(endValue);
            }
            else if (name2 == "n") { // interpolation key
              if (! readStrings(msg2, value3, keyFrame.interpolation))
                return errorMsg(msg2, "readStrings");
            }
            else if (name2 == "h") { // hold
              keyFrame.hold = valueToInt(value3);
            }
            else if (name2 == "ti") { // Value in Tangent
              CPoint2D tangentIn;
              if (! readVector(msg2, value3, tangentIn))
                return errorMsg(msg2, "readVector");
              keyFrame.tangentIn = tangentIn;
            }
            else if (name2 == "to") { // Value Out Tangent
              CPoint2D tangentOut;
              if (! readVector(msg2, value3, tangentOut))
                return errorMsg(msg2, "readVector");
              keyFrame.tangentOut = tangentOut;
            }
            else if (name2 == "t") { // ???
              keyFrame.timeFrame = valueToReal(value3);
            }
            else
              unhandledName(name2, value3);
          }

          size.keyFrames.push_back(keyFrame);
        }
      }
      else {
        CPoint2D p;
        if (! readVector(msg1, value2, p))
          return false;

        XYVals xyvals;

        xyvals.xvals.push_back(p.x);
        xyvals.yvals.push_back(p.y);

        size.values.push_back(xyvals);
      }
    }
    else if (name1 == "ix") {
      size.index = valueToInt(value2);
    }
    else
      unhandledName(name1, value2);
  }

  return true;
}

bool
CLottie::
readColorProperty(const std::string &msg, const CJson::ValueP &ivalue, ColorProperty &color) const
{
  if (! ivalue->isObject())
    return errorMsg(msg, "color is object");

  auto *sObj = ivalue->cast<CJson::Object>();

  CJson::Object::Names names;
  sObj->getNames(names);

  for (const auto &name1 : names) {
    auto msg1 = msg + "/" + name1;

    CJson::ValueP value2;
    sObj->getNamedValue(name1, value2);

    if (isDebug())
      std::cout << depthStr(sObj->hier_depth()) << name1 << "=" << *value2 << "\n";

    if      (name1 == "a") {
      color.animated = valueToBool(value2);
    }
    else if (name1 == "k") {
      if (! value2->isArray())
        return errorMsg(msg1, "k is not an array");

      auto *kArray2 = value2->cast<CJson::Array>();

      if (color.animated.value_or(false)) {
        for (const auto &kValue : kArray2->values()) {
          if (! kValue->isObject())
            return errorMsg(msg1, "k value is not an object");

          ColorProperty::KeyFrame keyFrame;

          auto *kObj3 = kValue->cast<CJson::Object>();

          CJson::Object::Names names2;
          kObj3->getNames(names2);

          for (const auto &name2 : names2) {
            auto msg2 = msg1 + "/" + name2;

            CJson::ValueP value3;
            kObj3->getNamedValue(name2, value3);

            if (isDebug())
              std::cout << depthStr(kObj3->hier_depth()) << name2 << "=" << *value3 << "\n";

            if      (name2 == "i") { // input
              getKeyFrameValues(msg2, name2, value3, keyFrame.ivalues);
            }
            else if (name2 == "o") { // output
              getKeyFrameValues(msg2, name2, value3, keyFrame.ovalues);
            }
            else if (name2 == "s") { // start value
#if 0
              std::vector<double> numbers;
              if (! readNumbers(msg2, value3, numbers))
                return errorMsg(msg2, "readNumbers");
              for (const auto &n : numbers)
                keyFrame.startValue.vals.push_back(CRGBA(n, n, n)); // TODO
#else
              OptColor c;
              if (! readColor(msg2, value3, c))
                return errorMsg(msg2, "readColor");
              if (c)
                keyFrame.startValue.vals.push_back(c.value());
#endif
            }
            else if (name2 == "e") { // end value
#if 0
              std::vector<double> numbers;
              if (! readNumbers(msg2, value3, numbers))
                return errorMsg(msg2, "readNumbers");
              for (const auto &n : numbers)
                keyFrame.startValue.vals.push_back(CRGBA(n, n, n)); // TODO
#else
              OptColor c;
              if (! readColor(msg2, value3, c))
                return errorMsg(msg2, "readColor");
              if (c)
                keyFrame.endValue.vals.push_back(c.value());
#endif
            }
            else if (name2 == "n") { // interpolation key
              if (! readStrings(msg2, value3, keyFrame.interpolation))
                return errorMsg(msg2, "readStrings");
            }
            else if (name2 == "h") { // hold
              keyFrame.hold = valueToInt(value3);
            }
            else if (name2 == "ti") { // Value in Tangent
              CPoint2D tangentIn;
              if (! readVector(msg2, value3, tangentIn))
                return errorMsg(msg2, "readVector");
              keyFrame.tangentIn = tangentIn;
            }
            else if (name2 == "to") { // Value Out Tangent
              CPoint2D tangentOut;
              if (! readVector(msg2, value3, tangentOut))
                return errorMsg(msg2, "readVector");
              keyFrame.tangentOut = tangentOut;
            }
            else if (name2 == "t") { // ???
              keyFrame.timeFrame = valueToReal(value3);
            }
            else
              unhandledName(name2, value3);
          }

          color.keyFrames.push_back(keyFrame);
        }
      }
      else {
        OptColor c;
        if (! readColor(msg1, value2, c))
          return errorMsg(msg1, "readColor");

        if (c)
          color.values.push_back(c.value());
      }
    }
    else if (name1 == "ix") {
      color.index = valueToInt(value2);
    }
    else
      unhandledName(name1, value2);
  }

  return true;
}

bool
CLottie::
readBezierProperty(const std::string &msg, const CJson::ValueP &ivalue,
                   BezierProperty &bezier) const
{
  if (! ivalue->isObject())
    return errorMsg(msg, "position is object");

  auto *sObj = ivalue->cast<CJson::Object>();

  CJson::Object::Names names;
  sObj->getNames(names);

  for (const auto &name1 : names) {
    auto msg1 = msg + "/" + name1;

    CJson::ValueP value2;
    sObj->getNamedValue(name1, value2);

    if (isDebug())
      std::cout << depthStr(sObj->hier_depth()) << name1 << "=" << *value2 << "\n";

    if      (name1 == "a") {
      bezier.animated = valueToBool(value2);
    }
    else if (name1 == "k") {
      auto readBezier = [&](const CJson::ValueP &bvalue) {
        if (! bvalue->isObject())
          return errorMsg(msg1, "k is object");

        auto *kObj = bvalue->cast<CJson::Object>();

        CJson::Object::Names names2;
        kObj->getNames(names2);

        for (const auto &name2 : names2) {
          auto msg2 = msg1 + "/" + name2;

          CJson::ValueP value3;
          kObj->getNamedValue(name2, value3);

          if (isDebug())
            std::cout << depthStr(kObj->hier_depth()) << name2 << "=" << *value3 << "\n";

          if      (name2 == "i") { // In Tangents
            if (value3->isArray()) {
              auto *kArray3 = value3->cast<CJson::Array>();

              if (bezier.animated.value_or(false)) {
                for (const auto &kValue3 : kArray3->values()) {
                  std::vector<CPoint2D> points;
                  if (! readPointList(msg1, kValue3, points))
                    return false;

                  bezier.ivalues.push_back(points);
                }
              }
              else {
                std::vector<CPoint2D> points;
                if (! readPointList(msg1, value3, points))
                  return false;

                bezier.ivalues.push_back(points);
              }
            }
            else {
              if (! value3->isObject())
                return errorMsg(msg2, "i value is not an object");

              auto *kObj4 = value3->cast<CJson::Object>();

              CJson::Object::Names names3;
              kObj4->getNames(names3);

              CPoint2D p;

              for (const auto &name3 : names3) {
                auto msg3 = msg2 + "/" + name3;

                CJson::ValueP value4;
                kObj4->getNamedValue(name3, value4);

                if (isDebug())
                  std::cout << depthStr(kObj4->hier_depth()) << name3 << "=" << *value4 << "\n";

                if      (name3 == "x") {
                  p.x = valueToReal(value4);
                }
                else if (name3 == "y") {
                  p.y = valueToReal(value4);
                }
                else {
                  unhandledName(name3, value4);
                }
              }

              std::vector<CPoint2D> points;

              points.push_back(p);

              bezier.ivalues.push_back(points);
            }
          }
          else if (name2 == "o") { // Out Tangents
            if (value3->isArray()) {
              auto *kArray3 = value3->cast<CJson::Array>();

              if (bezier.animated.value_or(false)) {
                for (const auto &kValue3 : kArray3->values()) {
                  std::vector<CPoint2D> points;
                  if (! readPointList(msg1, kValue3, points))
                    return false;

                  bezier.ovalues.push_back(points);
                }
              }
              else {
                std::vector<CPoint2D> points;
                if (! readPointList(msg1, value3, points))
                  return false;

                bezier.ovalues.push_back(points);
              }
            }
            else {
              if (! value3->isObject())
                return errorMsg(msg2, "o value is not an object");

              auto *kObj4 = value3->cast<CJson::Object>();

              CJson::Object::Names names3;
              kObj4->getNames(names3);

              CPoint2D p;

              for (const auto &name3 : names3) {
                auto msg3 = msg2 + "/" + name3;

                CJson::ValueP value4;
                kObj4->getNamedValue(name3, value4);

                if (isDebug())
                  std::cout << depthStr(kObj4->hier_depth()) << name3 << "=" << *value4 << "\n";

                if      (name3 == "x") {
                  p.x = valueToReal(value4);
                }
                else if (name3 == "y") {
                  p.y = valueToReal(value4);
                }
                else {
                  unhandledName(name3, value4);
                }
              }

              std::vector<CPoint2D> points;

              points.push_back(p);

              bezier.ovalues.push_back(points);
            }
          }
          else if (name2 == "v") { // Vertices
            if (! value3->isArray())
              return errorMsg(msg1, "v is not an array");

            auto *kArray3 = value3->cast<CJson::Array>();

            if (bezier.animated.value_or(false)) {
              for (const auto &kValue3 : kArray3->values()) {
                std::vector<CPoint2D> points;
                if (! readPointList(msg1, kValue3, points))
                  return false;

                bezier.vvalues.push_back(points);
              }
            }
            else {
              std::vector<CPoint2D> points;
              if (! readPointList(msg1, value3, points))
                return false;

              bezier.vvalues.push_back(points);
            }
          }
          else if (name2 == "c") { // close
            bezier.closed = value3->toBool();
          }
          else if (name2 == "n") { // interpolation key
            if (! readStrings(msg2, value3, bezier.interpolation))
              return errorMsg(msg2, "readStrings");
          }
          else if (name2 == "t") { // ???
            bezier.timeFrame = valueToReal(value3);
          }
          else if (name2 == "s") { // ???
            auto *kArray3 = value3->cast<CJson::Array>();

            BezierProperty::KeyFrame keyFrame;

            for (const auto &kValue4 : kArray3->values()) {
              if (! kValue4->isObject())
                return errorMsg(msg1, "k value is not an object");

              auto *kObj4 = kValue4->cast<CJson::Object>();

              CJson::Object::Names names4;
              kObj4->getNames(names4);

              for (const auto &name4 : names4) {
                auto msg3 = msg2 + "/" + name4;

                CJson::ValueP value5;
                kObj4->getNamedValue(name4, value5);

                if      (name4 == "i") {
                  if (! readPointList(msg1, value5, keyFrame.ipoints))
                    return false;
                }
                else if (name4 == "o") {
                  if (! readPointList(msg1, value5, keyFrame.opoints))
                    return false;
                }
                else if (name4 == "v") {
                  std::vector<CPoint2D> points;
                  if (! readPointList(msg1, value5, keyFrame.vpoints))
                    return false;
                }
                else if (name4 == "c") {
                  keyFrame.closed = value5->toBool();
                }
                else
                  unhandledName(name4, value5);
              }
            }

            bezier.ikeyFrames.push_back(keyFrame);
          }
          else if (name2 == "e") { // ???
            auto *kArray3 = value3->cast<CJson::Array>();

            BezierProperty::KeyFrame keyFrame;

            for (const auto &kValue4 : kArray3->values()) {
              if (! kValue4->isObject())
                return errorMsg(msg1, "k value is not an object");

              auto *kObj4 = kValue4->cast<CJson::Object>();

              CJson::Object::Names names4;
              kObj4->getNames(names4);

              for (const auto &name4 : names4) {
                auto msg3 = msg2 + "/" + name4;

                CJson::ValueP value5;
                kObj4->getNamedValue(name4, value5);

                if      (name4 == "i") {
                  if (! readPointList(msg1, value5, keyFrame.ipoints))
                    return false;
                }
                else if (name4 == "o") {
                  if (! readPointList(msg1, value5, keyFrame.opoints))
                    return false;
                }
                else if (name4 == "v") {
                  std::vector<CPoint2D> points;
                  if (! readPointList(msg1, value5, keyFrame.vpoints))
                    return false;
                }
                else if (name4 == "c") {
                  keyFrame.closed = value5->toBool();
                }
                else
                  unhandledName(name4, value5);
              }
            }

            bezier.ekeyFrames.push_back(keyFrame);
          }
          else
            unhandledName(name2, value3);
        }

        return true;
      };

      if (value2->isArray()) {
        auto *kArray2 = value2->cast<CJson::Array>();

        for (const auto &kValue : kArray2->values()) {
          if (! readBezier(kValue))
            return false;
        }
      }
      else {
        if (! readBezier(value2))
          return false;
      }
    }
    else if (name1 == "ix") {
      bezier.index = valueToInt(value2);
    }
    else if (name1 == "x") {
      bezier.expression = valueToString(value2);
    }
    else
      unhandledName(name1, value2);
  }

  return true;
}

bool
CLottie::
readScalarProperty(const std::string &msg, const CJson::ValueP &iValue,
                   ScalarProperty &scalar) const
{
  if (! iValue->isObject()) {
    scalar.animated = false;

    auto r = valueToReal(iValue);

    scalar.values.push_back(r);

    return true;
  }

  auto *sObj = iValue->cast<CJson::Object>();

  CJson::Object::Names names;
  sObj->getNames(names);

  for (const auto &name1 : names) {
    auto msg1 = msg + "/" + name1;

    CJson::ValueP value2;
    sObj->getNamedValue(name1, value2);

    if (isDebug())
      std::cout << depthStr(sObj->hier_depth()) << name1 << "=" << *value2 << "\n";

    if      (name1 == "a") {
      scalar.animated = valueToBool(value2);
    }
    else if (name1 == "k") {
      if (! scalar.animated && value2->isArray())
        scalar.animated = true;

      if (scalar.animated.value_or(false)) {
        if (! value2->isArray())
          return errorMsg(msg1, "k is not an array");

        auto *kArray2 = value2->cast<CJson::Array>();

        for (const auto &kValue : kArray2->values()) {
          if (! kValue->isObject())
            return errorMsg(msg1, "k value is not an object");

          ScalarProperty::KeyFrame keyFrame;

          auto *kObj3 = kValue->cast<CJson::Object>();

          CJson::Object::Names names2;
          kObj3->getNames(names2);

          for (const auto &name2 : names2) {
            auto msg2 = msg1 + "/" + name2;

            CJson::ValueP value3;
            kObj3->getNamedValue(name2, value3);

            if (isDebug())
              std::cout << depthStr(kObj3->hier_depth()) << name2 << "=" << *value3 << "\n";

            if      (name2 == "i") { // input
              getKeyFrameValues(msg2, name2, value3, keyFrame.ivalues);
            }
            else if (name2 == "o") { // output
              getKeyFrameValues(msg2, name2, value3, keyFrame.ovalues);
            }
            else if (name2 == "s") { // start value
              std::vector<double> numbers;
              if (! readNumbers(msg2, value3, numbers))
                return errorMsg(msg2, "readNumbers");
              keyFrame.startValue = numbers;
            }
            else if (name2 == "e") { // end value
              std::vector<double> numbers;
              if (! readNumbers(msg2, value3, numbers))
                return errorMsg(msg2, "readNumbers");
              keyFrame.endValue = numbers;
            }
            else if (name2 == "n") { // interpolation key
              if (! readStrings(msg2, value3, keyFrame.interpolation))
                return errorMsg(msg2, "readStrings");
            }
            else if (name2 == "h") { // hold
              keyFrame.hold = valueToInt(value3);
            }
            else if (name2 == "t") { // ???
              keyFrame.timeFrame = valueToReal(value3);
            }
            else
              unhandledName(name2, value3);
          }

          scalar.keyFrames.push_back(keyFrame);
        }
      }
      else {
        auto r = valueToReal(value2);

        scalar.values.push_back(r);
      }
    }
    else if (name1 == "ix") {
      scalar.index = valueToInt(value2);
    }
    else if (name1 == "x") {
      scalar.expression = valueToString(value2);
    }
    else
      unhandledName(name1, value2);
  }

  return true;
}

bool
CLottie::
readTransform(const std::string &msg1, CJson::ValueP &value1, Transform *transform) const
{
  if (! value1->isObject())
    return errorMsg(msg1, "transform is not an object");

  auto *ksObj = value1->cast<CJson::Object>();

  CJson::Object::Names names;
  ksObj->getNames(names);

  for (const auto &name1 : names) {
    auto msg2 = msg1 + "/" + name1;

    CJson::ValueP value2;
    ksObj->getNamedValue(name1, value2);

    if (isDebug())
      std::cout << depthStr(ksObj->hier_depth()) << name1 << "=" << *value2 << "\n";

    if      (name1 == "a") { // anchor point
      if (! readPositionProperty(msg2, value2, transform->anchorPoint))
        return errorMsg(msg2, "readPositionProperty");
    }
    else if (name1 == "p") { // position
      if (! readSplitPositionProperty(msg2, value2, transform->position))
        return errorMsg(msg2, "readSplitPositionProperty");
    }
    else if (name1 == "r") { // rotation
      if (! readScalarProperty(msg2, value2, transform->rotation))
        return errorMsg(msg2, "readScalarProperty");
    }
    else if (name1 == "s") { // scale (percent 100 = no scale)
      if (! readVectorProperty(msg2, value2, transform->scale))
        return errorMsg(msg2, "readVectorProperty");
    }
    else if (name1 == "o") { // opacity
      if (! readScalarProperty(msg2, value2, transform->opacity))
        return errorMsg(msg2, "readScalarProperty");
    }
    else if (name1 == "sk") { // skew
      if (! readScalarProperty(msg2, value2, transform->skew))
        return errorMsg(msg2, "readScalarProperty");
    }
    else if (name1 == "sa") { // skew axis
      if (! readScalarProperty(msg2, value2, transform->skewAxis))
        return errorMsg(msg2, "readScalarProperty");
    }
    else if (name1 == "rx") { // rotate x axis
     if (! readScalarProperty(msg2, value2, transform->x_rotation))
        return errorMsg(msg2, "readScalarProperty");
    }
    else if (name1 == "ry") { // rotate y axis
     if (! readScalarProperty(msg2, value2, transform->y_rotation))
        return errorMsg(msg2, "readScalarProperty");
    }
    else if (name1 == "rz") { // rotate z axis
     if (! readScalarProperty(msg2, value2, transform->z_rotation))
        return errorMsg(msg2, "readScalarProperty");
    }
    else if (name1 == "or") { // orientation
     if (! readVectorProperty(msg2, value2, transform->orientation))
        return errorMsg(msg2, "readVectorProperty");
    }
    else if (transform->repeater) {
      if      (name1 == "ty") {
        // TODO: skip ?
      }
      else if (name1 == "nm") {
        // TODO: skip ?
      }
      else if (name1 == "so") {
        auto *repeater = static_cast<CLottieRepeater *>(transform->repeater);

        if (! readScalarProperty(msg2, value2, repeater->startOpacity))
          return errorMsg(msg2, "readScalarProperty");
      }
      else if (name1 == "eo") {
        auto *repeater = static_cast<CLottieRepeater *>(transform->repeater);

        if (! readScalarProperty(msg2, value2, repeater->endOpacity))
          return errorMsg(msg2, "readScalarProperty");
      }
      else
        unhandledName(name1, value2);
    }
    else
      unhandledName(name1, value2);
  }

  return true;
}

bool
CLottie::
getKeyFrameValues(const std::string &xyMsg, const std::string &name, CJson::ValueP &xyValue,
                  std::vector<XYVals> &xyValues) const
{
  if (! xyValue->isObject())
    return errorValueMsg(xyMsg, name + " value is not an object", xyValue);

  auto *kObj4 = xyValue->cast<CJson::Object>();

  CJson::Object::Names names3;
  kObj4->getNames(names3);

  XYVals xyvals;

  for (const auto &name3 : names3) {
    auto msg3 = xyMsg + "/" + name3;

    CJson::ValueP value4;
    kObj4->getNamedValue(name3, value4);

    if (isDebug())
      std::cout << depthStr(kObj4->hier_depth()) << name3 << "=" << *value4 << "\n";

    if      (name3 == "x") {
      if (value4->isArray()) {
        auto *array4 = value4->cast<CJson::Array>();

        for (const auto &value5 : array4->values())
          xyvals.xvals.push_back(valueToReal(value5));
      }
      else {
        xyvals.xvals.push_back(valueToReal(value4));
      }
    }
    else if (name3 == "y") {
      if (value4->isArray()) {
        auto *array4 = value4->cast<CJson::Array>();

        for (const auto &value5 : array4->values())
          xyvals.yvals.push_back(valueToReal(value5));
      }
      else {
        xyvals.yvals.push_back(valueToReal(value4));
      }
    }
    else {
      unhandledName(name3, value4);
    }
  }

  xyValues.push_back(xyvals);

  return true;
}

bool
CLottie::
readDash(const std::string &msg, const CJson::ValueP &iValue, Dash &dash) const
{
  if (! iValue->isArray())
    return errorMsg(msg, "dash is not an array");

  auto *dArray = iValue->cast<CJson::Array>();

  for (const auto &dvalue : dArray->values()) {
    if (! dvalue->isObject())
      return errorMsg(msg, "dash array value is not an object");

    auto *dobj = dvalue->cast<CJson::Object>();

    CJson::Object::Names dnames;
    dobj->getNames(dnames);

    for (const auto &dname : dnames) {
      auto msg2 = msg + "/" + dname;

      CJson::ValueP dvalue1;
      dobj->getNamedValue(dname, dvalue1);

      if      (dname == "n") { // dash type
        dash.type = valueToString(dvalue1);
      }
      else if (dname == "nm") {
        dash.name = valueToString(dvalue1);
      }
      else if (dname == "v") { // length
        if (! readScalarProperty(msg2, dvalue1, dash.value))
          return errorMsg(msg2, "readScalarProperty");
      }
      else
        unhandledName(dname, dvalue1);
    }
  }

  return true;
}

bool
CLottie::
readPointList(const std::string &msg, const CJson::ValueP &iValue,
              std::vector<CPoint2D> &points) const
{
  if (! iValue->isArray())
    return errorValueMsg(msg, "point list is not an array", iValue);

  auto *iArray = iValue->cast<CJson::Array>();

  for (const auto &iValue1 : iArray->values()) {
    if (! iValue1->isArray())
      return errorValueMsg(msg, "point list value is not an array", iValue);

    auto *iArray1 = iValue1->cast<CJson::Array>();

    if (iArray1->size() < 2)
      return errorValueMsg(msg, "point list value array less than 2", iValue);

    CPoint2D p;

    p.x = valueToReal(iArray1->indexValue(0));
    p.y = valueToReal(iArray1->indexValue(1));

    points.push_back(p);
  }

  return true;
}

bool
CLottie::
readVector(const std::string &msg, const CJson::ValueP &iValue, CPoint2D &p) const
{
  if (! iValue->isArray())
    return errorValueMsg(msg, "vector value is not an array", iValue);

  auto *iArray = iValue->cast<CJson::Array>();

  if (iArray->size() < 2)
    return errorValueMsg(msg, "vector array size less than 2", iValue);

  p.x = valueToReal(iArray->indexValue(0));
  p.y = valueToReal(iArray->indexValue(1));

  return true;
}

bool
CLottie::
readColor(const std::string &msg, const CJson::ValueP &iValue, OptColor &c) const
{
  if (iValue->isString()) {
    auto str = valueToString(iValue);

    if (str != "") {
      double r, g, b, a;
      if (! CRGBName::lookup(str, &r, &g, &b, &a))
        return errorValueMsg(msg, "color string invalid", iValue);

      c = CRGBA(r, g, b, a);
    }

    return true;
  }

  if (! iValue->isArray())
    return errorValueMsg(msg, "color value is not an array", iValue);

  auto *iArray = iValue->cast<CJson::Array>();

  if (iArray->size() < 3)
    return errorValueMsg(msg, "color array size less than 3", iValue);

  auto r = valueToReal(iArray->indexValue(0));
  auto g = valueToReal(iArray->indexValue(1));
  auto b = valueToReal(iArray->indexValue(2));
  auto a = 1.0;

  if (iArray->size() >= 4)
    a = valueToReal(iArray->indexValue(3));

  c = CRGBA(r, g, b, a);

  return true;
}

bool
CLottie::
readStrings(const std::string &msg, const CJson::ValueP &iValue,
            std::vector<std::string> &strs) const
{
  if     (iValue->isArray()) {
    auto *iArray = iValue->cast<CJson::Array>();

    for (uint i = 0; i < iArray->size(); ++i) {
      auto str = iArray->indexValue(i)->toString();

      strs.push_back(str);
    }
  }
  else if (iValue->isString()) {
    auto str = valueToString(iValue);

    strs.push_back(str);
  }
  else
    return errorValueMsg(msg, "value is not an array", iValue);

  return true;
}

bool
CLottie::
readNumbers(const std::string &, const CJson::ValueP &iValue,
            std::vector<double> &numbers) const
{
  if (iValue->isArray()) {
    auto *iArray = iValue->cast<CJson::Array>();

    for (uint i = 0; i < iArray->size(); ++i) {
      auto r = valueToReal(iArray->indexValue(i));

      numbers.push_back(r);
    }
  }
  else {
    auto r = valueToReal(iValue);

    numbers.push_back(r);
  }

  return true;
}

std::string
CLottie::
valueToString(const CJson::ValueP &value, const std::string &def) const
{
  if (! value->isString()) {
    warnValueMsg("", "value not a string", value);
    return def;
  }

  return value->toString();
}

double
CLottie::
valueToReal(const CJson::ValueP &value, double def) const
{
  if (! value->isNumber()) {
    warnValueMsg("", "value not a real", value);
    return def;
  }

  return value->toNumber();
}

int
CLottie::
valueToInt(const CJson::ValueP &value, int def) const
{
  if (! value->isNumber()) {
    warnValueMsg("", "value not an int", value);
    return def;
  }

  return int(valueToReal(value));
}

bool
CLottie::
valueToBool(const CJson::ValueP &value, bool def) const
{
  if (value->isBool())
    return value->toBool();

  if (! value->isNumber()) {
    warnValueMsg("", "value not a bool", value);
    return def;
  }

  return (valueToReal(value) != 0.0);
}

CMatrix2D
CLottie::
getTransformMatrix(const TimeFrame &timeFrame, CLottie::Transform *transform) const
{
  // Translate by −a
  // Scale by s/100
  // If sk != 0:
  //   Rotate by −sa
  //   Skew x by tan(−sk)
  //   Rotate by sa
  // Rotate by −r
  // Translate by p

  if (! transform)
    return CMatrix2D::identity();

  // TODO: autoOrient adds frame angle ?

  auto apxy = transform->anchorPoint.tvalue(timeFrame, CPoint2D(0, 0)).value();
  auto ap   = (apxy.isSet() ? CPoint2D(apxy.xvals[0], apxy.yvals[0]) : CPoint2D(0, 0));

  auto s = transform->scale   .tvalue(timeFrame, CPoint2D(100, 100)).value();
  auto r = transform->rotation.tvalue(timeFrame, 0).value();
  auto p = transform->position.tvalue(timeFrame, CPoint2D(0, 0)).value();

  auto m1 = CMatrix2D::translation(-ap.x, -ap.y);
  auto m2 = CMatrix2D::scale(s.x/100.0, s.y/100.0);
  auto m3 = CMatrix2D::rotation(CMathGen::DegToRad(r));
  auto m4 = CMatrix2D::translation(p.x, p.y);

  return m4*m3*m2*m1;
}

CMatrix2D
CLottie::
getRepeaterMatrix(const TimeFrame &timeFrame, CLottie::Transform *transform, double f) const
{
  auto apxy = transform->anchorPoint.tvalue(timeFrame, CPoint2D(0, 0)).value();
  auto ap   = (apxy.isSet() ? CPoint2D(apxy.xvals[0], apxy.yvals[0]) : CPoint2D(0, 0));

  auto s = transform->scale   .tvalue(timeFrame, CPoint2D(100, 100)).value();
  auto r = transform->rotation.tvalue(timeFrame, 0).value();
  auto p = transform->position.tvalue(timeFrame, CPoint2D(0, 0)).value();

  auto m1 = CMatrix2D::translation(-ap.x, -ap.y);
  auto m2 = CMatrix2D::rotation(CMathGen::DegToRad(r)*f); // additive
  auto m3 = CMatrix2D::scale(std::pow(s.x/100.0, f), std::pow(s.y/100.0, f)); // multiplicative
  auto m4 = CMatrix2D::translation(ap.x, ap.y);
  auto m5 = CMatrix2D::translation(p.x*f, p.y*f);  // additive

  return m5*m4*m3*m2*m1;
}

CLottieRoot *
CLottie::
makeRoot()
{
  return new CLottieRoot(this);
}

CLottieLayer *
CLottie::
makeLayer()
{
  if (factory_)
    return factory_->makeLayer(this);

  return new CLottieLayer(this);
}

CLottieShape *
CLottie::
makeShape()
{
  if (factory_)
    return factory_->makeShape(this);

  return new CLottieShape(this);
}

CLottieAsset *
CLottie::
makeAsset()
{
  if (factory_)
    return factory_->makeAsset(this);

  return new CLottieAsset(this);
}

CLottieMarker *
CLottie::
makeMarker()
{
  return new CLottieMarker(this);
}

CLottieEffect *
CLottie::
makeEffect()
{
  return new CLottieEffect(this);
}

CLottieEffectValue *
CLottie::
makeEffectValue()
{
  return new CLottieEffectValue(this);
}

//---

CLottieRoot::
CLottieRoot(CLottie *l) :
 CLottieObject(l, Type::ROOT)
{
}

CLottieRoot::
~CLottieRoot()
{
}

void
CLottieRoot::
buildLayerHier()
{
  for (auto *layer : layers_) {
    auto *player = layer->getParentLayer();

    if (! player) {
      childLayers_.push_back(layer);
      continue;
    }

    player->addChildLayer(layer);
  }
}

void
CLottieRoot::
printLayerHier() const
{
  for (auto *layer : childLayers_) {
    std::cerr << layer->name().value_or("") << "\n";

    layer->printLayerHier("  ");
  }
}

void
CLottieRoot::
printI(const std::string &prefix, bool hier) const
{
  CLottieObject::printI(prefix, hier);

  //---

  printValue(prefix, "version", version_);

  optPrintValue(prefix, "frameRate" , timeFrame_.frameRate);
  optPrintValue(prefix, "frameStart", timeFrame_.frameStart);
  optPrintValue(prefix, "frameStop" , timeFrame_.frameStop);

  optPrintValue(prefix, "width" , width_);
  optPrintValue(prefix, "height", height_);

  optPrintValue(prefix, "three_d", threeD_);

  //---

  if (! hier) return;

  if (! assets_.empty()) {
    std::cout << "Assets\n";

    int i = 0;

    for (const auto &asset : assets_) {
      std::cout << " [Asset " << i << "]\n";

      asset->printI("  ", hier);

      ++i;
    }
  }

  if (! layers_.empty()) {
    std::cout << "Layers\n";

    int i = 0;

    for (auto *layer : layers_) {
      std::cout << "[Layer " << i << "]\n";

      layer->printI("  ", hier);

      ++i;
    }
  }
}

//---

CLottieAsset::
CLottieAsset(CLottie *l) :
 CLottieObject(l, Type::ASSET)
{
}

CLottieAsset::
~CLottieAsset()
{
}

CLottieRoot *
CLottieAsset::
getRoot() const
{
  if (parent_ && parent_->objectType() == Type::ROOT)
    return dynamic_cast<CLottieRoot *>(parent_);

  return nullptr;
}

void
CLottieAsset::
printI(const std::string &prefix, bool hier) const
{
  CLottieObject::printI(prefix, hier);

  //---

  printValue(prefix, "id", id_);

  optPrintValue(prefix, "css", css_);

  optPrintValue(prefix, "width" , width_);
  optPrintValue(prefix, "height", height_);

  optPrintValue(prefix, "dir"     , dir_);
  optPrintValue(prefix, "path"    , path_);
  optPrintValue(prefix, "embedded", embedded_);

  //---

  if (! hier) return;

  if (! layers_.empty()) {
    std::cout << prefix << "Layers\n";

    int i = 0;

    for (auto *layer : layers_) {
      std::cout << prefix << "[Layer " << i << "]\n";

      layer->printI(prefix + "  ", hier);

      ++i;
    }
  }
}

//---

CLottieLayer::
CLottieLayer(CLottie *l) :
 CLottieObject(l, Type::LAYER)
{
}

CLottieLayer::
~CLottieLayer()
{
  delete transform_;

  delete mask_;
  delete effect_;
  delete solid_;
  delete precomp_;
}

CLottieRoot *
CLottieLayer::
getRoot() const
{
  if (parent_ && parent_->objectType() == Type::ROOT)
    return dynamic_cast<CLottieRoot *>(parent_);

  auto *layer = getParentLayer();

  return (layer ? layer->getRoot() : nullptr);
}

CMatrix2D
CLottieLayer::
calcTransform(const TimeFrame &timeFrame) const
{
  if (! transform_)
    return CMatrix2D::identity();

  return lottie_->getTransformMatrix(timeFrame, transform_);
}

CMatrix2D
CLottieLayer::
calcHierTransform(const TimeFrame &timeFrame) const
{
  auto m = calcTransform(timeFrame);

  auto *player = getParentLayer();

  if (player)
    m = player->calcHierTransform(timeFrame)*m;

  return m;
}

//---

CLottieRepeater *
CLottieLayer::
calcRepeater() const
{
  auto *repeaterShape = getRepeaterShape();

  auto *repeater = (repeaterShape ? repeaterShape->repeater() : nullptr);

  if (repeater)
    return repeater;

  auto *player = getParentLayer();

  if (player)
    return player->calcRepeater();

  return nullptr;
}

CLottieShape *
CLottieLayer::
getRepeaterShape() const
{
  for (auto *shape : shapes()) {
    if (shape->type() == "rp")
      return shape;
  }

  return nullptr;
}

//---

CLottieLayer *
CLottieLayer::
getParentLayer() const
{
  return (parentInd_ ? lottie_->getLayerById(*parentInd_) : nullptr);
}

CLottieAsset *
CLottieLayer::
getParentAsset() const
{
  if (parent_ && parent_->objectType() == Type::ASSET)
    return dynamic_cast<CLottieAsset *>(parent_);

  return nullptr;
}

void
CLottieLayer::
printLayerHier(const std::string &prefix) const
{
  for (auto *layer : childLayers_) {
    std::cerr << prefix << layer->name().value_or("") << "\n";

    layer->printLayerHier(prefix + "  ");
  }
}

void
CLottieLayer::
printI(const std::string &prefix, bool hier) const
{
  CLottieObject::printI(prefix, hier);

  //---

  optPrintValue(prefix, "matchName", matchName_);
  optPrintValue(prefix, "css"      , css_);

  optPrintValue(prefix, "three_d"   , threeD_);
  optPrintValue(prefix, "autoOrient", autoOrient_);
  optPrintValue(prefix, "blendMode" , blendMode_);

  optPrintValue(prefix, "matteMode"  , matteMode_);
  optPrintValue(prefix, "matteParent", matteParent_);
  optPrintValue(prefix, "matteTarget", matteTarget_);
  optPrintValue(prefix, "hasMask"    , hasMask_);

  optPrintValue(prefix, "parentInd", parentInd_);

  optPrintValue(prefix, "width" , width_);
  optPrintValue(prefix, "height", height_);

  optPrintValue(prefix, "frameIn"    , frameIn_);
  optPrintValue(prefix, "frameOut"   , frameOut_);
  optPrintValue(prefix, "startTime"  , startTime_);
  optPrintValue(prefix, "timeStretch", timeStretch_);

  optPrintValue(prefix, "refId", refId_);

  if (transform_)
    transform_->print(prefix);

  if (mask_)
    mask_->print(prefix);

  if (effect_) {
    std::cout << prefix << "Effect\n";

    auto prefix1 = prefix + "  ";

    effect_->printI(prefix1, hier);
  }

  if (solid_)
    solid_->print(prefix);

  if (precomp_)
    precomp_->print(prefix);

  //---

  if (! hier) return;

  if (! shapes().empty()) {
    std::cout << prefix << "Shapes\n";

    int i = 0;

    for (auto *shape : shapes()) {
      std::cout << prefix << "[Shape " << i << "]\n";

      shape->printI(prefix + "  ", hier);

      ++i;
    }
  }
}

const char *
CLottieLayer::
typeIdName(int t)
{
  switch (t) {
    case  0: return "Precomposition Layer";
    case  1: return "Solid Layer";
    case  2: return "Image Layer";
    case  3: return "Null Layer";
    case  4: return "Shape Layer";
    case  5: return "Text Layer";
    case  6: return "Audio Layer";
    case 13: return "Camera Layer";
    case 15: return "Data Layer";
    default: return "Unknown Layer";
  }
}

//---

CLottieMarker::
CLottieMarker(CLottie *l) :
 CLottieObject(l, Type::MARKER)
{
}

CLottieMarker::
~CLottieMarker()
{
}

CLottieRoot *
CLottieMarker::
getRoot() const
{
  if (! parent_ || parent_->objectType() != Type::ROOT)
    return dynamic_cast<CLottieRoot *>(parent_);

  return nullptr;
}

void
CLottieMarker::
printI(const std::string &prefix, bool hier) const
{
  CLottieObject::printI(prefix, hier);
}

//---

CLottieEffect::
CLottieEffect(CLottie *l) :
 CLottieObject(l, Type::EFFECT)
{
}

CLottieEffect::
~CLottieEffect()
{
}

CLottieRoot *
CLottieEffect::
getRoot() const
{
  auto *layer = getLayer();
  if (! layer) return nullptr;

  return layer->getRoot();
}

CLottieLayer *
CLottieEffect::
getLayer() const
{
  if (parent_ && parent_->objectType() == Type::LAYER)
    return dynamic_cast<CLottieLayer *>(parent_);

  return nullptr;
}

void
CLottieEffect::
printI(const std::string &prefix, bool hier) const
{
  CLottieObject::printI(prefix, hier);

  //---

  optPrintValue(prefix, "type" , type_);
  optPrintValue(prefix, "name" , name_);
  optPrintValue(prefix, "match", match_);
  optPrintValue(prefix, "index", index_);

  optPrintValue(prefix, "numProperties", numProperties_);
  optPrintValue(prefix, "enabled"      , enabled_);

  for (auto *c : children_)
    c->print(prefix);
}

//---

CLottieShape::
CLottieShape(CLottie *l) :
 CLottieObject(l, Type::SHAPE)
{
}

CLottieShape::
~CLottieShape()
{
  delete transform_;

  delete stroke_;
  delete fill_;

  delete group_;
  delete rectangle_;
  delete repeater_;
  delete gradientFill_;
  delete gradientStroke_;
  delete trim_;
  delete polyStar_;
  delete merge_;
  delete rounded_;
}

CLottieRoot *
CLottieShape::
getRoot() const
{
  auto *layer = getParentLayer();

  return layer->getRoot();
}

//---

CLottieLayer *
CLottieShape::
getHierParentLayer() const
{
  auto *layer = getParentLayer();

  if (! layer) {
    auto *pshape = getParentShape();

    if (pshape)
      layer = pshape->getHierParentLayer();
  }

  return layer;
}

CLottieLayer *
CLottieShape::
getParentLayer() const
{
  if (parent_ && parent_->objectType() == Type::LAYER)
    return dynamic_cast<CLottieLayer *>(parent_);

  return nullptr;
}

CLottieShape *
CLottieShape::
getParentShape() const
{
  if (parent_ && parent_->objectType() == Type::SHAPE)
    return dynamic_cast<CLottieShape *>(parent_);

  return nullptr;
}

//---

CMatrix2D
CLottieShape::
calcTransform(const TimeFrame &timeFrame) const
{
  if (! transform_)
    return CMatrix2D::identity();

  return lottie_->getTransformMatrix(timeFrame, transform_);
}

#if 0
CMatrix2D
CLottieShape::
calcHierTransform(const TimeFrame &timeFrame) const
{
  auto m = calcTransform(timeFrame);

  auto *pshape = getParentShape();
  auto *player = getParentLayer();

  if      (pshape)
    m = pshape->calcHierTransform(timeFrame)*m;
  else if (player)
    m = player->calcHierTransform(timeFrame)*m;

  return m;
}
#endif

CMatrix2D
CLottieShape::
calcHierTransform(const TimeFrame &timeFrame) const
{
  auto *pshape = getParentShape();
  auto *player = getParentLayer();

  CLottieShape *transformShape = nullptr;

  if (pshape)
    transformShape = pshape->getTransformShape();

  CMatrix2D m;

  if (transformShape)
    m = transformShape->calcTransform(timeFrame);
  else
    m = calcTransform(timeFrame);

  if      (pshape)
    m = pshape->calcHierTransform(timeFrame)*m;
  else if (player)
    m = player->calcHierTransform(timeFrame)*m;

  return m;
}

CLottieShape *
CLottieShape::
getTransformShape() const
{
  for (auto *shape : shapes()) {
    if (shape->type() == "tr")
      return shape;
  }

  return nullptr;
}

//---

CLottieShape::Fill *
CLottieShape::
calcFill() const
{
  if (type() == "fl")
    return const_cast<CLottieShape::Fill *>(fill());

  auto *pshape = getParentShape();
  if (! pshape) return nullptr;

  auto fillShape = pshape->getFillShape();

  return (fillShape ? fillShape->fill() : nullptr);
}

CLottieShape *
CLottieShape::
getFillShape() const
{
  for (auto *shape : shapes()) {
    if (shape->type() == "fl")
      return shape;
  }

  return nullptr;
}

//---

CLottieRepeater *
CLottieShape::
calcRepeater() const
{
  if (type() == "rp")
    return const_cast<CLottieRepeater *>(repeater());

  auto *repeaterShape = getRepeaterShape();

  auto *repeater = (repeaterShape ? repeaterShape->repeater() : nullptr);

  if (repeater)
    return repeater;

  auto *pshape = getParentShape();

  if (pshape)
    return pshape->calcRepeater();

  auto *player = getParentLayer();

  return (player ? player->calcRepeater() : nullptr);
}

CLottieShape *
CLottieShape::
getRepeaterShape() const
{
  for (auto *shape : shapes()) {
    if (shape->type() == "rp")
      return shape;
  }

  return nullptr;
}

//---

CLottieShape::Merge *
CLottieShape::
calcHierMerge() const
{
  auto *mergeShape = calcHierMergeShape();

  return (mergeShape ? mergeShape->merge() : nullptr);
}

CLottieShape *
CLottieShape::
calcHierMergeShape() const
{
  if (type() == "mm" && ! hidden().value_or(false))
    return const_cast<CLottieShape*>(this);

  auto *pshape = getParentShape();
  if (! pshape) return nullptr;

  auto *mergeShape = pshape->getMergeShape();

  if (mergeShape)
    return mergeShape;

  return pshape->calcHierMergeShape();
}

CLottieShape *
CLottieShape::
getMergeShape() const
{
  for (auto *shape : shapes()) {
    if (shape->type() == "mm" && ! shape->hidden().value_or(false))
      return shape;
  }

  return nullptr;
}

//---

void
CLottieShape::
printI(const std::string &prefix, bool hier) const
{
  CLottieObject::printI(prefix, hier);

  //---

  optPrintValue(prefix, "longName", longName_);

  optPrintValue(prefix, "index"    , index_);
  optPrintValue(prefix, "direction", direction_);

  if (transform_)
    transform_->print(prefix);

  printProperty(prefix, "pos"  , pos_);
  printProperty(prefix, "size" , size_);
  printProperty(prefix, "color", color_);
  printProperty(prefix, "path" , path_);

  //---

  if (stroke_)
    stroke_->print(prefix);

  if (fill_)
    fill_->print(prefix);

  //---

  if (group_)
    group_->print(prefix);

  if (rectangle_)
    rectangle_->print(prefix);

  if (repeater_)
    repeater_->print(prefix);

  if (gradientFill_)
    gradientFill_->print(prefix);

  if (gradientStroke_)
    gradientStroke_->print(prefix);

  if (trim_)
    trim_->print(prefix);

  if (polyStar_)
    polyStar_->print(prefix);

  if (merge_)
    merge_->print(prefix);

  if (rounded_)
    rounded_->print(prefix);

  //---

  if (! hier) return;

  if (! shapes().empty()) {
    int i = 0;

    for (auto *shape : shapes()) {
      std::cout << prefix << "[Shape " << i << "]\n";

      shape->printI(prefix + "  ", hier);

      ++i;
    }
  }
}

//---

CLottieObject::
CLottieObject(CLottie *l, const Type &t) :
 lottie_(l), objectType_(t)
{
}

CLottieObject::
~CLottieObject()
{
}

bool
CLottieObject::
isHierSelected() const
{
  if (selected())
    return true;

  if (parent())
    return parent()->isHierSelected();

  return false;
}

void
CLottieObject::
debugPrint() const
{
  printHier("");
}

void
CLottieObject::
printI(const std::string &prefix, bool /*hier*/) const
{
  optPrintValue(prefix, "name", name_);
  optPrintValue(prefix, "type", type_);

  if (typeId_)
    std::cout << prefix << "typeId=" <<
      CLottieLayer::typeIdName(*typeId_) << "(" << *typeId_ << ")\n";

  printValue(prefix, "selected", selected_);

  optPrintValue(prefix, "hidden", hidden_);

  optPrintValue(prefix, "ind", ind_);
}

//---

void
CLottieEffectValue::
print(const std::string &prefix) const
{
  std::cout << prefix << "Effect Value\n";

  auto prefix1 = prefix + "  ";

  optPrintValue(prefix1, "type" , type);
  optPrintValue(prefix1, "name" , name);
  optPrintValue(prefix1, "match", match);
  optPrintValue(prefix1, "index", index);

  optPrintValue(prefix1, "ivalue", ivalue);

  printProperty(prefix1, "scalar", scalar);
  printProperty(prefix1, "color" , color);
  printProperty(prefix1, "point" , point);
}

//---

void
CLottieLayer::Mask::
print(const std::string &prefix) const
{
  std::cout << prefix << "Mask\n";

  auto prefix1 = prefix + "  ";

  optPrintValue(prefix1, "mode"    , mode);
  printProperty(prefix1, "opacity" , opacity);
  printProperty(prefix1, "path"    , path);
  printProperty(prefix1, "expand"  , expand);
  optPrintValue(prefix1, "inverted", inverted);
  optPrintValue(prefix1, "name"    , name);
}

void
CLottieLayer::Solid::
print(const std::string &prefix) const
{
  std::cout << prefix << "Solid\n";

  auto prefix1 = prefix + "  ";

  optPrintValue(prefix1, "width" , width);
  optPrintValue(prefix1, "height", height);
  optPrintValue(prefix1, "color" , color);
}

void
CLottieLayer::Precomp::
print(const std::string &prefix) const
{
  std::cout << prefix << "Precomp\n";

  auto prefix1 = prefix + "  ";

  optPrintValue(prefix1, "refId"    , refId);
  optPrintValue(prefix1, "width"    , width);
  optPrintValue(prefix1, "height"   , height);
  optPrintValue(prefix1, "startTime", startTime);
  printProperty(prefix1, "timeRemap", timeRemap);
}

//---

void
CLottieShape::Transform::
print(const std::string &prefix) const
{
  std::cout << prefix << "Transform\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "anchorPoint", anchorPoint);
  printProperty(prefix1, "position"   , position);
  printProperty(prefix1, "rotation"   , rotation);
  printProperty(prefix1, "scale"      , scale);
  printProperty(prefix1, "opacity"    , opacity);
  printProperty(prefix1, "skew"       , skew);
  printProperty(prefix1, "skewAxis"   , skewAxis);
  printProperty(prefix1, "x_rotation" , x_rotation);
  printProperty(prefix1, "y_rotation" , y_rotation);
  printProperty(prefix1, "z_rotation" , z_rotation);
  printProperty(prefix1, "orientation", orientation);
}

void
CLottieShape::Group::
print(const std::string &prefix) const
{
  std::cout << prefix << "Group\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "color"        , color);
  printProperty(prefix1, "opacity"      , opacity);
  optPrintValue(prefix1, "numProperties", numProperties);
  optPrintValue(prefix1, "blendMode"    , blendMode);
}

void
CLottieShape::Rectangle::
print(const std::string &prefix) const
{
  std::cout << prefix << "Rectangle\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "roundness", roundness);
}

void
CLottieRepeater::
print(const std::string &prefix) const
{
  std::cout << prefix << "Repeater\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "copies"   , copies   );
  printProperty(prefix1, "offset"   , offset   );
  optPrintValue(prefix1, "composite", composite);

  if (transform)
    transform->print(prefix1);

  printProperty(prefix1, "startOpacity", startOpacity);
  printProperty(prefix1, "endOpacity"  , endOpacity);
}

void
CLottieShape::Stroke::
print(const std::string &prefix) const
{
  std::cout << prefix << "Stroke\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "color"         , color);
  printProperty(prefix1, "opacity"       , opacity);
  printProperty(prefix1, "width"         , width);
  optPrintValue(prefix1, "lineCap"       , lineCap);
  optPrintValue(prefix1, "lineJoin"      , lineJoin);
  optPrintValue(prefix1, "miterLimit"    , miterLimit);
  printProperty(prefix1, "miterLimitAnim", miterLimitAnim);
  optPrintValue(prefix1, "dashType"      , dash.type);
  optPrintValue(prefix1, "dashName"      , dash.name);
  printProperty(prefix1, "dashValue"     , dash.value);
  optPrintValue(prefix1, "blendMode"     , blendMode);
}

void
CLottieShape::Fill::
print(const std::string &prefix) const
{
  std::cout << prefix << "Fill\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "color"    , color);
  printProperty(prefix1, "opacity"  , opacity);
  optPrintValue(prefix1, "fillRule" , fillRule );
  optPrintValue(prefix1, "blendMode", blendMode);
}

void
CLottieShape::GradientFill::
print(const std::string &prefix) const
{
  std::cout << prefix << "Gradient Fill\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "color"          , color);
  printProperty(prefix1, "opacity"        , opacity);
  optPrintValue(prefix1, "type"           , type);
  optPrintValue(prefix1, "stopCount"      , stopCount);
  optPrintValue(prefix1, "index"          , index);
  printProperty(prefix1, "startPoint"     , startPoint);
  printProperty(prefix1, "endPoint"       , endPoint);
  printProperty(prefix1, "highlightLength", highlightLength);
  printProperty(prefix1, "highlightAngle" , highlightAngle);
  optPrintValue(prefix1, "fillRule"       , fillRule);
  optPrintValue(prefix1, "blendMode"      , blendMode);
  printProperty(prefix1, "colors"         , colors);
}

void
CLottieShape::GradientStroke::
print(const std::string &prefix) const
{
  std::cout << prefix << "Gradient Stroke\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "opacity"   , opacity);
  optPrintValue(prefix1, "type"      , type);
  optPrintValue(prefix1, "stopCount" , stopCount);
  optPrintValue(prefix1, "index"     , index);
  printProperty(prefix1, "startPoint", startPoint);
  printProperty(prefix1, "endPoint"  , endPoint);
  printProperty(prefix1, "width"     , width);
  optPrintValue(prefix1, "lineCap"   , lineCap);
  optPrintValue(prefix1, "lineJoin"  , lineJoin);
  optPrintValue(prefix1, "miterLimit", miterLimit);
  printProperty(prefix1, "colors"    , colors);
}

void
CLottieShape::Trim::
print(const std::string &prefix) const
{
  std::cout << prefix << "Trim\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "start"   , start);
  printProperty(prefix1, "end"     , end);
  printProperty(prefix1, "offset"  , offset);
  optPrintValue(prefix1, "multiple", multiple);
}

void
CLottieShape::PolyStar::
print(const std::string &prefix) const
{
  std::cout << prefix << "PolyStar\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "position"      , position      );
  printProperty(prefix1, "outerRadius"   , outerRadius   );
  printProperty(prefix1, "outerRoundness", outerRoundness);
  printProperty(prefix1, "rotation"      , rotation      );
  printProperty(prefix1, "points"        , points        );
  optPrintValue(prefix1, "type"          , type          );
  printProperty(prefix1, "innerRadius"   , innerRadius   );
  printProperty(prefix1, "innerRoundness", innerRoundness);
}

void
CLottieShape::Merge::
print(const std::string &prefix) const
{
  std::cout << prefix << "Merge\n";

  auto prefix1 = prefix + "  ";

  optPrintValue(prefix1, "mode", mode);
}

void
CLottieShape::Rounded::
print(const std::string &prefix) const
{
  std::cout << prefix << "Rounded\n";

  auto prefix1 = prefix + "  ";

  printProperty(prefix1, "roundness", roundness);
}
