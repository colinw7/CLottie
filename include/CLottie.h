#ifndef CLottie_H
#define CLottie_H

#include <CJson.h>
#include <CMathUtil.h>
#include <CMathRound.h>
#include <CMatrixStack2D.h>
#include <CBezierPath.h>
#include <CBBox2D.h>
#include <CPoint2D.h>
#include <CStrUtil.h>
#include <CRGBA.h>
#include <CRGBName.h>
#include <CUtil.h>

#include <string>

namespace CLottieUtil {

template<typename T>
T mapValue(double t, const T &v1, const T &v2);

template<>
inline bool mapValue<bool>(double t, const bool &v1, const bool &v2) {
  return (t < 0.5 ? v1 : v2);
}

template<>
inline double mapValue<double>(double t, const double &v1, const double &v2) {
  return CMathUtil::map(t, 0.0, 1.0, v1, v2);
}

template<>
inline CRGBA mapValue<CRGBA>(double t, const CRGBA &v1, const CRGBA &v2) {
  return v1.blended(v2, t);
}

template<>
inline CPoint2D mapValue<CPoint2D>(double t, const CPoint2D &v1, const CPoint2D &v2) {
  return v1 + t*(v2 - v1);
}

//---

struct XYVals {
  XYVals() { }

  XYVals(const CPoint2D &p) {
    xvals.push_back(p.x);
    yvals.push_back(p.y);
  }

  size_t size() const { return std::min(xvals.size(), yvals.size()); }

  bool isSet() const { return ! xvals.empty() && ! yvals.empty(); }

  CPoint2D toPoint(const CPoint2D &def=CPoint2D()) const {
    if (! isSet())
      return def;

    return CPoint2D(xvals[0], yvals[0]);
  }

  friend std::ostream &operator<<(std::ostream &os, const XYVals &l) {
    os << "x=[";
    int i = 0;
    for (auto &x : l.xvals) {
      if (i > 0) os << " ";
      os << x;
      ++i;
    }
    os << "], y=[";
    i = 0;
    for (auto &y : l.yvals) {
      if (i > 0) os << " ";
      os << y;
      ++i;
    }
    os << "]";
    return os;
  }

  std::vector<double> xvals;
  std::vector<double> yvals;
};

template<typename T>
struct ValArrayT {
  ValArrayT() { }

  ValArrayT(const std::vector<T> &a) : vals(a) { }

  bool isSet() const { return ! vals.empty(); }

  friend std::ostream &operator<<(std::ostream &os, const ValArrayT &l) {
    os << "[";
    int i = 0;
    for (auto &v : l.vals) {
      if (i > 0) os << " ";
      os << v;
      ++i;
    }
    os << "]";
    return os;
  }

  std::vector<T> vals;
};

using RValArray = ValArrayT<double>;

template<>
inline XYVals mapValue<XYVals>(double t, const XYVals &v1, const XYVals &v2) {
  XYVals v;

  auto nx = std::max(v1.xvals.size(), v2.xvals.size());
  auto ny = std::max(v1.yvals.size(), v2.yvals.size());

  for (uint i = 0; i < nx; ++i) {
    auto x1 = (i < v1.xvals.size() ? v1.xvals[i] : 0.0);
    auto x2 = (i < v2.xvals.size() ? v2.xvals[i] : 0.0);

    v.xvals.push_back(CMathUtil::map(t, 0.0, 1.0, x1, x2));
  }

  for (uint i = 0; i < ny; ++i) {
    auto y1 = (i < v1.yvals.size() ? v1.yvals[i] : 0.0);
    auto y2 = (i < v2.yvals.size() ? v2.yvals[i] : 0.0);

    v.yvals.push_back(CMathUtil::map(t, 0.0, 1.0, y1, y2));
  }

  return v;
}

template<>
inline RValArray mapValue<RValArray>(double t, const RValArray &v1, const RValArray &v2) {
  RValArray v;

  auto n = std::min(v1.vals.size(), v2.vals.size());

  for (uint i = 0; i < n; ++i)
    v.vals.push_back(CMathUtil::map(t, 0.0, 1.0, v1.vals[i], v2.vals[i]));

  return v;
}

//---

using Points = std::vector<CPoint2D>;

struct PointList {
  PointList() { }

  PointList(const Points &p) : points(p) { }

  bool isEmpty() const { return points.empty(); }

  friend std::ostream &operator<<(std::ostream &os, const PointList &l) {
    int i = 0;
    for (auto &p : l.points) {
      if (i > 0) os << " ";
      os << p;
      ++i;
    }
    return os;
  }

  Points points;
};

template<>
inline Points mapValue<Points>(double t, const Points &v1, const Points &v2) {
  Points v;

  auto n = std::min(v1.size(), v2.size());

  for (uint i = 0; i < n; ++i) {
    auto x = CMathUtil::map(t, 0.0, 1.0, v1[i].x, v2[i].x);
    auto y = CMathUtil::map(t, 0.0, 1.0, v1[i].y, v2[i].y);

    v.push_back(CPoint2D(x, y));
  }

  return v;
}

struct TimeFrame {
  using OptReal = std::optional<double>;

  OptReal frameRate;
  OptReal frameStart;
  OptReal frameStop;

  double secs  { 0.0 };
  uint   frame { 0 };
  int    delta { 0 };

  double mapFrame() const {
    return CMathUtil::map(frame, frameStart.value_or(0.0), frameStop.value_or(1.0), 0.0, 1.0);
  }
};

//---

template<typename T>
bool stringToValue(const std::string &str, T &t);

template<typename T>
bool stringToValue(const std::string &str, std::optional<T> &t) {
  T t1;
  if (! stringToValue(str, t1))
    return false;
  t = t1;
  return true;
}

template<>
inline bool stringToValue<bool>(const std::string &str, bool &b) {
  int i;
  if (! CStrUtil::toInteger(str, &i))
    return false;
  b = i;
  return true;
}

template<>
inline bool stringToValue<int>(const std::string &str, int &i) {
  return CStrUtil::toInteger(str, &i);
}

template<>
inline bool stringToValue<double>(const std::string &str, double &r) {
  return CStrUtil::toReal(str, &r);
}

template<>
inline bool stringToValue<std::string>(const std::string &str, std::string &s) {
  s = str;
  return true;
}

template<>
inline bool stringToValue<CRGBA>(const std::string &str, CRGBA &rgba) {
  return CRGBName::toRGBA(str, rgba);
}

template<>
inline bool stringToValue<RValArray>(const std::string &, RValArray &) {
  return false;
}

template<typename T>
inline std::string valueToString(const T &value) {
  return CUtil::toString(value);
}

template<typename T>
inline std::string valueToString(const std::optional<T> &value) {
  return (value ? CUtil::toString(*value) : "");
}

template<>
inline std::string valueToString<CRGBA>(const CRGBA &rgba) {
  return rgba.stringEncode();
}

}

//---

struct CLottieFactory;
class  CLottie;
struct CLottieRoot;
struct CLottieLayer;
struct CLottieMarker;
struct CLottieAsset;
struct CLottieShape;
struct CLottieEffect;
struct CLottieEffectValue;

//---

class CLottieKeyFrame {
 public:
  using XYVals         = CLottieUtil::XYVals;
  using Interpolations = std::vector<std::string>;
  using OptPoint       = std::optional<CPoint2D>;
  using OptReal        = std::optional<double>;
  using OptBool        = std::optional<bool>;

 public:
  CLottieKeyFrame() { }

  virtual ~CLottieKeyFrame() { }

  //----

  const Interpolations &interpolation() const { return interpolation_; }
  void setInterpolation(const Interpolations &i) { interpolation_ = i; }

  const std::vector<XYVals> &ivalues() const { return ivalues_; }
  void setIValues(const std::vector<XYVals> &v) { ivalues_ = v; }

  const std::vector<XYVals> &ovalues() const { return ovalues_; }
  void setOValues(const std::vector<XYVals> &v) { ovalues_ = v; }

  const OptReal &timeFrame() const { return timeFrame_; }
  void setTimeFrame(const OptReal &v) { timeFrame_ = v; }

  const OptBool &isHold() const { return hold_; }
  void setHold(const OptBool &b) { hold_ = b; }

  const OptPoint &tangentIn() const { return tangentIn_; }
  void setTangentIn(const OptPoint &p) { tangentIn_ = p; }

  const OptPoint &tangentOut() const { return tangentOut_; }
  void setTangentOut(const OptPoint &p) { tangentOut_ = p; }

  //----

  virtual void print(const std::string &prefix="") const {
    auto printValue = [&](const std::string &n, auto value) {
      std::cout << prefix << n << "=" << value << "\n";
    };

    auto optPrintValue = [&](const std::string &n, const auto &value) {
      if (value)
        std::cout << prefix << n << "=" << *value << "\n";
    };

    if (! ivalues().empty()) {
      printValue("ivalues", "");

      for (auto &v : ivalues()) {
        std::cout << prefix << "  " << v << "\n";
      }
    }

    if (! ovalues().empty()) {
      printValue("ovalues", "");

      for (auto &v : ovalues()) {
        std::cout << prefix << "  " << v << "\n";
      }
    }

    if (! interpolation().empty()) {
      printValue("interpolation", "");

      for (auto &v : interpolation()) {
        std::cout << prefix << "  " << v << "\n";
      }
    }

    optPrintValue("tangentIn" , tangentIn());
    optPrintValue("tangentOut", tangentOut());

    optPrintValue("timeFrame", timeFrame());

    optPrintValue("hold", isHold());
  }

 private:
  Interpolations interpolation_;

  std::vector<XYVals> ivalues_;
  std::vector<XYVals> ovalues_;

  OptPoint tangentIn_;
  OptPoint tangentOut_;

  OptReal timeFrame_;

  OptBool hold_ { false };
};

//----

class CLottieVariant {
 public:
  enum class Type {
    NONE,
    BOOL,
    REAL,
    INTEGER,
    STRING,
    RGBA
  };

  CLottieVariant() { }
  CLottieVariant(const Type &t) : type_(t) { }

  virtual ~CLottieVariant() { }

  virtual bool hasValue() const = 0;

  virtual std::string value() const = 0;
  virtual bool setValue(const std::string &) const = 0;

  bool bvalue(bool def=false) const {
    if (! hasValue()) return def;
    return !!CStrUtil::toInteger(value());
  }

  int ivalue(int def=0) const {
    if (! hasValue()) return def;
    return int(CStrUtil::toInteger(value()));
  }

  double rvalue(double def=0.0) const {
    if (! hasValue()) return def;
    return CStrUtil::toReal(value());
  }

 private:
  Type type_ { Type::NONE };
};

template<typename T>
class CLottieVariantT : public CLottieVariant {
 public:
  CLottieVariantT(T *data) :
   data_(data) {
  }

  bool hasValue() const override {
    return (CLottieUtil::valueToString(*data_) != "");
  }

  std::string value() const override {
    return CLottieUtil::valueToString(*data_);
  }

  bool setValue(const std::string &str) const override {
    T value;
    if (! CLottieUtil::stringToValue(str, value))
      return false;
    *data_ = value;
    return true;
  }

 private:
  T *data_ { nullptr };
};

//---

class CLottieProperty {
 public:
  enum class Type {
    NONE,
    SCALAR,
    COLOR,
    ARRAY,
    VECTOR,
    POSITION,
    SPLIT_POSITION,
    BEZIER,
    SIZE
  };

  using TimeFrame = CLottieUtil::TimeFrame;
  using OptInt    = std::optional<int>;
  using OptReal   = std::optional<double>;
  using OptStr    = std::optional<std::string>;
  using OptBool   = std::optional<bool>;

  enum class FramePos {
    NONE,
    BEFORE,
    INSIDE,
    AFTER
  };

  struct FrameInd {
    FramePos pos { FramePos::NONE };
    int      ind { -1 };
  };

 public:
  static const char *typeName(const Type &type) {
    switch (type) {
      case Type::SCALAR        : return "Scalar";
      case Type::COLOR         : return "Color";
      case Type::ARRAY         : return "Array";
      case Type::VECTOR        : return "Vector";
      case Type::POSITION      : return "Position";
      case Type::SPLIT_POSITION: return "Split Position";
      case Type::BEZIER        : return "Bezier";
      case Type::SIZE          : return "Size";
      case Type::NONE          :
      default                  : return "<none>";
    }
  }

  CLottieProperty(const Type &t) :
   type_(t) {
  }

  virtual ~CLottieProperty() { }

  //---

  const Type &type() const { return type_; }

  void setLottie(CLottie *lottie) { lottie_ = lottie; }

  bool isAnimatedSet() const { return !!animated_; }

  bool isAnimated() const { return animated_.value_or(false); }
  void setAnimated(bool b) { animated_ = b; }

  const OptInt &index() const { return index_; }
  void setIndex(const OptInt &v) { index_ = v; }

  const OptStr &expression() const { return expression_; }
  void setExpression(const OptStr &v) { expression_ = v; }

  const OptStr &slot() const { return slot_; }
  void setSlot(const OptStr &v) { slot_ = v; }

  //---

  virtual bool isSet() const = 0;

  virtual bool isTSet() const { return (numKeyFrames() > 0); }

  virtual size_t numKeyFrames() const = 0;

  virtual CLottieKeyFrame *keyFrame(uint i) const = 0;

  //---

  virtual std::string tvalueStr(const TimeFrame &frame) const = 0;

  virtual std::string minStr() const = 0;
  virtual std::string maxStr() const = 0;

  //---

  virtual bool setValueStr(const std::string &) { return false; }

  //---

  virtual FrameInd calcFrameInd(const TimeFrame &frame) const {
    FrameInd frameInd;

    auto nf = uint(numKeyFrames());

    if (nf == 0)
      return frameInd;

    frameInd.pos = FramePos::BEFORE;
    frameInd.ind = 0;

    auto rframe = double(frame.frame) + frame.delta;

    for (uint i = 0; i < nf; ++i) {
      auto *keyFrame1 = keyFrame(i);
      auto *keyFrame2 = (i < nf - 1 ? keyFrame(i + 1) : nullptr);

      auto frameStart = keyFrame1->timeFrame().value_or(0.0);

      OptReal frameStop;

      if (keyFrame2)
        frameStop = keyFrame2->timeFrame().value_or(0.0);

      if (rframe >= frameStart && frameStop && rframe < frameStop.value()) {
        frameInd.pos = FramePos::INSIDE;
        frameInd.ind = int(i);
        break;
      }

      if (frameStop && rframe >= frameStop.value()) {
        frameInd.pos = FramePos::AFTER;
        frameInd.ind = int(i);
      }
    }

    assert(frameInd.ind >= 0 && frameInd.ind <= int(nf - 1));

    return frameInd;
  }

  //---

  virtual void print(const std::string &prefix="") const {
    optPrintValue(prefix, "animated", animated_);

    optPrintValue(prefix, "index", index_);

    optPrintValue(prefix, "expression", expression_);

    optPrintValue(prefix, "slot", slot_);
  }

 protected:
  template<typename T>
  void optPrintValue(const std::string &prefix, const std::string &n,
                     const std::optional<T> &value) const {
    if (value)
      std::cout << prefix << n << "=" << *value << "\n";
  }

 protected:
  CLottie* lottie_ { nullptr };
  Type     type_;
  OptBool  animated_;
  OptInt   index_;
  OptStr   expression_;
  OptStr   slot_;
};

//---

template<typename T>
class CLottieKeyFrameT : public CLottieKeyFrame {
 public:
  using ValArray = CLottieUtil::ValArrayT<T>;

  ValArray startValue;
  ValArray endValue;

  void print(const std::string &prefix="") const override {
    CLottieKeyFrame::print(prefix);

    auto printValue = [&](const std::string &n, auto value) {
      std::cout << prefix << n << "=" << value << "\n";
    };

    if (startValue.isSet())
      printValue("startValue", startValue);

    if (endValue.isSet())
      printValue("endValue", endValue);
  }
};

//---

class CLottie {
 public:
  using TimeFrame = CLottieUtil::TimeFrame;

  using Points   = std::vector<CPoint2D>;
  using Colors   = std::vector<CRGBA>;
  using OptInt   = std::optional<int>;
  using OptReal  = std::optional<double>;
  using OptStr   = std::optional<std::string>;
  using OptBool  = std::optional<bool>;
  using OptPoint = std::optional<CPoint2D>;
  using OptColor = std::optional<CRGBA>;

  using Assets  = std::vector<CLottieAsset *>;
  using Layers  = std::vector<CLottieLayer *>;
  using Shapes  = std::vector<CLottieShape *>;
  using Markers = std::vector<CLottieMarker *>;
  using Effects = std::vector<CLottieEffect *>;

  struct RVals {
    RVals() { }

    RVals(const std::vector<double> &r) : vals(r) { }

    bool isSet() const { return ! vals.empty(); }

    friend std::ostream &operator<<(std::ostream &os, const RVals &l) {
      os << "[";
      int i = 0;
      for (auto &v : l.vals) {
        if (i > 0) os << " ";
        os << v;
        ++i;
      }
      os << "]";
      return os;
    }

    std::vector<double> vals;
  };

  using XYVals = CLottieUtil::XYVals;

  template<typename T>
  class PropertyT : public CLottieProperty {
   public:
    using KeyFrame = CLottieKeyFrameT<T>;
    using OptVal   = std::optional<T>;

   public:
    PropertyT(const Type &t) :
     CLottieProperty(t) {
    }

   ~PropertyT() override {
      for (auto *keyFrame : keyFrames)
        delete keyFrame;
    }

    bool isSet() const override {
      return (! values.empty() || ! keyFrames.empty());
    }

    //---

    size_t numKeyFrames() const override { return keyFrames.size(); }

    KeyFrame *keyFrame(uint i) const override { return keyFrames[i]; }

    //---

    virtual PropertyT *readValue(CLottie *, const CJson::ValueP &) const { return nullptr; }

    //---

    void print(const std::string &prefix="") const override {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

#if 0
      auto optPrintValue = [&](const std::string &n, const auto &value) {
        if (value)
          std::cout << prefix << n << "=" << *value << "\n";
      };
#endif

      CLottieProperty::print(prefix);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! keyFrames.empty()) {
        printValue("KeyFrames", "");

        int i = 0;

        for (auto *kf : keyFrames) {
          std::cout << prefix << " Frame [" << i << "]\n";

          kf->print(prefix + "  ");

          ++i;
        }
      }
    }

    //---

    std::string tvalueStr(const TimeFrame &frame) const override {
      auto value = tvalue(frame);
      if (! value) return "<none>";
      return CLottieUtil::valueToString(*value);
    }

    std::string minStr() const override {
      if (min_) return CLottieUtil::valueToString(*min_);
      return "";
    }

    std::string maxStr() const override {
      if (max_) return CLottieUtil::valueToString(*max_);
      return "";
    }

    //---

    bool setValueStr(const std::string &str) override {
      if (isAnimated())
        return false;

      T value;
      if (! CLottieUtil::stringToValue(str, value))
        return false;

      if (values.empty())
        values.push_back(value);
      else
        values[0] = value;

      return true;
    }

    //---

    OptVal value(const OptVal &def=OptVal()) const {
      if (values.empty())
        return def;

      return values[0];
    }

    OptVal tvalue(const TimeFrame &frame, const OptVal &def=OptVal()) const {
      if (slot_) {
        CJson::ValueP value;
        if (! lottie_->getSlotValue(slot_.value(), value)) {
          std::cerr << "No slot for " << slot_.value() << "\n";
          return def;
        }

        auto *prop = readValue(lottie_, value);

        if (! prop) {
          std::cerr << "Failed to read value for slot " << slot_.value() << "\n";
          return def;
        }

        auto res = prop->tvalue(frame, def);

        delete prop;

        return res;
      }

      if (isAnimated()) {
        if (! isTSet())
          return def;

        return keyFrameValue(frame, def);
      }
      else
        return value(def);
    }

    OptVal keyFrameValue(const TimeFrame &frame, const OptVal &def) const {
      auto frameInd = calcFrameInd(frame);
      if (frameInd.pos == FramePos::NONE) return def;

      auto nf = numKeyFrames();

      auto rframe = double(frame.frame) + frame.delta;

      auto *keyFrame1 = keyFrame(frameInd.ind);
      auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

      auto frameStart = keyFrame1->timeFrame().value_or(0.0);
      auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                     frame.frameStop.value_or(1.0));

      if (keyFrame1->startValue.vals.empty() || keyFrame1->endValue.vals.empty()) {
        if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
          --frameInd.ind;

          keyFrame1 = keyFrame(frameInd.ind);
        }
      }

      if (keyFrame1->startValue.vals.empty() && keyFrame1->endValue.vals.empty())
        return def;

      OptVal v1, v2;

      if (! keyFrame1->startValue.vals.empty())
        v1 = keyFrame1->startValue.vals[0];

      if      (! keyFrame1->endValue.vals.empty())
        v2 = keyFrame1->endValue.vals[0];
      else if (! keyFrame2->startValue.vals.empty())
        v2 = keyFrame2->startValue.vals[0];

      if (frameInd.pos == FramePos::INSIDE) {
        if (v1 && v2) {
          auto dt = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

          return CLottieUtil::mapValue(dt, *v1, *v2);
        }
        else if (v1)
          return v1;
        else if (v2)
          return v2;
      }
      else if (frameInd.pos == FramePos::BEFORE) {
        if (v1)
          return v1;
      }
      else if (frameInd.pos == FramePos::AFTER) {
        if (v2)
          return v2;
      }

      return def;
    }

    void setValue(const T &v) {
      if (values.empty())
        values.push_back(v);
      else
        values[0] = v;
    }

   public:
    std::vector<T>          values;
    std::vector<KeyFrame *> keyFrames;

   protected:
    std::optional<T> min_;
    std::optional<T> max_;
  };

  //---

  class ScalarProperty : public PropertyT<double> {
   public:
    ScalarProperty(double min=0.0, double max=100.0) :
     PropertyT(Type::SCALAR) {
      min_ = min;
      max_ = max;
    }

    ScalarProperty *readValue(CLottie *lottie, const CJson::ValueP &ivalue) const override {
      auto *scalar = new ScalarProperty(min_.value_or(0.0), max_.value_or(100.0));
      if (! lottie->readScalarProperty("", ivalue, *scalar)) {
        delete scalar;
        return nullptr;
      }
      return scalar;
    }
  };

  //---

  class SplitPositionProperty : public CLottieProperty {
   public:
    using KeyFrame = CLottieKeyFrameT<CPoint2D>;
    using OptVal   = std::optional<CPoint2D>;

   public:
    SplitPositionProperty() :
     CLottieProperty(Type::SPLIT_POSITION) {
    }

   ~SplitPositionProperty() override {
      for (auto *keyFrame : keyFrames)
        delete keyFrame;
    }

    bool isSet() const override {
      if (split().value_or(false))
        return x.isSet() && y.isSet();

      return (! values.empty() || ! keyFrames.empty());
    }

    //---

    const OptBool &split() const { return split_; }
    void setSplit(const OptBool &v) { split_ = v; }

    const OptInt &length() const { return length_; }
    void setLength(const OptInt &i) { length_ = i; }

    //---

    size_t numKeyFrames() const override { return keyFrames.size(); }

    KeyFrame *keyFrame(uint i) const override { return keyFrames[i]; }

    //---

    void print(const std::string &prefix="") const override {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      CLottieProperty::print(prefix);

      if (split().value_or(false)) {
        x.print(prefix);
        y.print(prefix);
      }
      else {
        if (! values.empty()) {
          printValue("values", "");

          for (auto &v : values) {
            std::cout << prefix << "  " << v << "\n";
          }
        }

        if (! keyFrames.empty()) {
          printValue("KeyFrames", "");

          int i = 0;

          for (auto *kf : keyFrames) {
            std::cout << prefix << " Frame [" << i << "]\n";

            kf->print(prefix + "  ");

            ++i;
          }
        }
      }
    }

    //---

    std::string tvalueStr(const TimeFrame &frame) const override {
      if (split().value_or(false)) {
        return "(" + x.tvalueStr(frame) + " " + y.tvalueStr(frame) + ")";
      }
      else {
        auto value = tvalue(frame);
        if (! value) return "<none>";
        return CLottieUtil::valueToString(*value);
      }
    }

    std::string minStr() const override {
      return "";
    }

    std::string maxStr() const override {
      return "";
    }

    //---

    OptVal value(const OptVal &def=OptVal()) const {
      if (split().value_or(false)) {
        auto v1 = x.value();
        auto v2 = y.value();

        if (v1 && v2)
          return CPoint2D(*v1, *v2);
      }
      else {
        if (! values.empty())
          return values[0];
      }

      return def;
    }

    OptVal tvalue(const TimeFrame &frame, const OptVal &def=OptVal()) const {
      if (split().value_or(false)) {
        OptReal v1, v2;

        if (x.isAnimated())
          v1 = x.tvalue(frame);
        else
          v1 = x.value();

        if (y.isAnimated())
          v2 = y.tvalue(frame);
        else
          v2 = y.value();

        if (v1 && v2)
          return CPoint2D(*v1, *v2);
      }
      else {
        if (isAnimated()) {
          if (isTSet())
            return keyFrameValue(frame, def);
        }
        else
          return value(def);
      }

      return def;
    }

    OptVal keyFrameValue(const TimeFrame &frame, const OptVal &def) const {
      auto frameInd = calcFrameInd(frame);
      if (frameInd.pos == FramePos::NONE) return def;

      auto nf = numKeyFrames();

      auto rframe = double(frame.frame) + frame.delta;

      auto *keyFrame1 = keyFrame(frameInd.ind);
      auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

      auto frameStart = keyFrame1->timeFrame().value_or(0.0);
      auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                     frame.frameStop.value_or(1.0));

      if (keyFrame1->startValue.vals.empty() || keyFrame1->endValue.vals.empty()) {
        if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
          --frameInd.ind;

          keyFrame1 = keyFrame(frameInd.ind);
        }
      }

      if (keyFrame1->startValue.vals.empty() && keyFrame1->endValue.vals.empty())
        return def;

      OptVal v1, v4;

      if (! keyFrame1->startValue.vals.empty())
        v1 = keyFrame1->startValue.vals[0];

      if      (! keyFrame1->endValue.vals.empty())
        v4 = keyFrame1->endValue.vals[0];
      else if (! keyFrame2->startValue.vals.empty())
        v4 = keyFrame2->startValue.vals[0];

      if (frameInd.pos == FramePos::INSIDE) {
        if (v1 && v4) {
          auto t = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

          auto v2 = *v1 + keyFrame1->tangentOut().value_or(CPoint2D(0, 0));
          auto v3 = *v4 + keyFrame1->tangentIn ().value_or(CPoint2D(0, 0));

          CBezierPath bezierPath;
          bezierPath.moveTo(*v1);
          bezierPath.cubicTo(v2, v3, *v4);

          if (bezierPath.arcLength() > 0.0)
            return bezierPath.calc(t);
          else
            return CLottieUtil::mapValue(t, *v1, *v4);
        }
        else if (v1)
          return v1;
        else if (v4)
          return v4;
      }
      else if (frameInd.pos == FramePos::BEFORE) {
        if (v1)
          return v1;
      }
      else if (frameInd.pos == FramePos::AFTER) {
        if (v4)
          return v4;
      }

      return def;
    }

    //---

    double pathGradient(const TimeFrame &frame) const {
      auto frameInd = calcFrameInd(frame);
      if (frameInd.pos == FramePos::NONE) return 0.0;

      auto nf = numKeyFrames();

      auto rframe = double(frame.frame) + frame.delta;

      auto *keyFrame1 = keyFrame(frameInd.ind);
      auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

      auto frameStart = keyFrame1->timeFrame().value_or(0.0);
      auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                     frame.frameStop.value_or(1.0));

      if (keyFrame1->startValue.vals.empty() || keyFrame1->endValue.vals.empty()) {
        if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
          --frameInd.ind;

          keyFrame1 = keyFrame(frameInd.ind);
        }
      }

      if (keyFrame1->startValue.vals.empty() && keyFrame1->endValue.vals.empty())
        return 0.0;

      auto v1 = keyFrame1->startValue.vals[0];
      auto v4 = keyFrame1->endValue  .vals[0];

      auto v2 = v1 + keyFrame1->tangentOut().value_or(CPoint2D(0, 0));
      auto v3 = v4 + keyFrame1->tangentIn ().value_or(CPoint2D(0, 0));

      CBezierPath bezierPath;
      bezierPath.moveTo(v1);
      bezierPath.cubicTo(v2, v3, v4);

      auto t = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

      return CMathGen::RadToDeg(bezierPath.gradient(t));
    }

    CBezierPath path() const {
      CBezierPath bezierPath;

      int i = 0;

      for (auto *kf : keyFrames) {
        if (kf->startValue.vals.empty())
          continue;

        auto v1 = kf->startValue.vals[0];
        auto v2 = v1 + kf->tangentOut().value_or(CPoint2D(0, 0));
        auto v4 = kf->endValue  .vals[0];
        auto v3 = v4 + kf->tangentIn ().value_or(CPoint2D(0, 0));

        if (i == 0)
          bezierPath.moveTo(v1);
#if 0
        else
          bezierPath.lineTo(v1);
#endif

#if 1
        bezierPath.cubicTo(v2, v3, v4);
#else
        bezierPath.lineTo(v2);
        bezierPath.lineTo(v3);
        bezierPath.lineTo(v4);
#endif

        ++i;
      }

      return bezierPath;
    }

   public:
    std::vector<CPoint2D>   values;
    std::vector<KeyFrame *> keyFrames;

   private:
    OptBool split_;

   public:
    ScalarProperty x;
    ScalarProperty y;

   private:
    OptInt length_;
  };

  class VectorProperty : public CLottieProperty {
   public:
    using KeyFrame = CLottieKeyFrameT<CPoint2D>;
    using OptVal   = std::optional<CPoint2D>;

    //---

   public:
    VectorProperty() :
     CLottieProperty(Type::VECTOR) {
    }

   ~VectorProperty() override {
      for (auto *keyFrame : keyFrames)
        delete keyFrame;
    }

    bool isSet() const override {
      return (! values.empty() || ! keyFrames.empty());
    }

    //---

    const OptInt &length() const { return length_; }
    void setLength(const OptInt &i) { length_ = i; }

    //---

    size_t numKeyFrames() const override { return keyFrames.size(); }

    KeyFrame *keyFrame(uint i) const override { return keyFrames[i]; }

    //---

    void print(const std::string &prefix="") const override {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      CLottieProperty::print(prefix);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! keyFrames.empty()) {
        printValue("KeyFrames", "");

        int i = 0;

        for (auto *kf : keyFrames) {
          std::cout << prefix << " Frame [" << i << "]\n";

          kf->print(prefix + "  ");

          ++i;
        }
      }
    }

    //---

    std::string tvalueStr(const TimeFrame &frame) const override {
      auto value = tvalue(frame);
      if (! value) return "<none>";
      return CLottieUtil::valueToString(*value);
    }

    std::string minStr() const override {
      return "";
    }

    std::string maxStr() const override {
      return "";
    }

    //---

    OptVal value(const OptVal &def=OptVal()) const {
      if (values.empty())
        return def;

      return values[0];
    }

    OptVal tvalue(const TimeFrame &frame, const OptVal &def=OptVal()) const {
      if (slot_) {
        CJson::ValueP value;
        if (! lottie_->getSlotValue(slot_.value(), value)) {
          std::cerr << "No slot for " << slot_.value() << "\n";
          return def;
        }

        auto *prop = readValue(lottie_, value);

        if (! prop) {
          std::cerr << "Failed to read value for slot " << slot_.value() << "\n";
          return def;
        }

        auto res = prop->tvalue(frame, def);

        delete prop;

        return res;
      }

      if (isAnimated()) {
        if (! isTSet())
          return def;

        return keyFrameValue(frame, def);
      }
      else
        return value(def);
    }

    OptVal keyFrameValue(const TimeFrame &frame, const OptVal &def) const {
      auto frameInd = calcFrameInd(frame);
      if (frameInd.pos == FramePos::NONE) return def;

      auto nf = numKeyFrames();

      auto rframe = double(frame.frame) + frame.delta;

      auto *keyFrame1 = keyFrame(frameInd.ind);
      auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

      auto frameStart = keyFrame1->timeFrame().value_or(0.0);
      auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                     frame.frameStop.value_or(1.0));

      if (keyFrame1->startValue.vals.empty() || keyFrame1->endValue.vals.empty()) {
        if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
          --frameInd.ind;

          keyFrame1 = keyFrame(frameInd.ind);
        }
      }

      if (keyFrame1->startValue.vals.empty() && keyFrame1->endValue.vals.empty())
        return def;

      OptVal v1, v2;

      if (! keyFrame1->startValue.vals.empty())
        v1 = keyFrame1->startValue.vals[0];

      if      (! keyFrame1->endValue.vals.empty())
        v2 = keyFrame1->endValue.vals[0];
      else if (! keyFrame2->startValue.vals.empty())
        v2 = keyFrame2->startValue.vals[0];

      if (frameInd.pos == FramePos::INSIDE) {
        if (v1 && v2) {
          auto dt = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

          return CLottieUtil::mapValue(dt, *v1, *v2);
        }
        else if (v1)
          return v1;
        else if (v2)
          return v2;
      }
      else if (frameInd.pos == FramePos::BEFORE) {
        if (v1)
          return v1;
      }
      else if (frameInd.pos == FramePos::AFTER) {
        if (v2)
          return v2;
      }

      return def;
    }

    //---

    VectorProperty *readValue(CLottie *lottie, const CJson::ValueP &ivalue) const {
      auto *vector = new VectorProperty();
      if (! lottie->readVectorProperty("", ivalue, *vector)) {
        delete vector;
        return nullptr;
      }
      return vector;
    }

   public:
    std::vector<CPoint2D>   values;
    std::vector<KeyFrame *> keyFrames;

   private:
    OptInt length_;
  };

  //---

  class PositionProperty : public CLottieProperty {
   public:
    using KeyFrame = CLottieKeyFrameT<XYVals>;
    using OptVal   = std::optional<XYVals>;

   public:
    PositionProperty() :
     CLottieProperty(Type::POSITION) {
    }

   ~PositionProperty() override {
      for (auto *keyFrame : keyFrames)
        delete keyFrame;
    }

    bool isSet() const override {
      return (! values.empty() || ! keyFrames.empty());
    }

    //---

    const OptInt &length() const { return length_; }
    void setLength(const OptInt &i) { length_ = i; }

    //---

    size_t numKeyFrames() const override { return keyFrames.size(); }

    KeyFrame *keyFrame(uint i) const override { return keyFrames[i]; }

    //---

    void print(const std::string &prefix="") const override {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      CLottieProperty::print(prefix);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! keyFrames.empty()) {
        printValue("KeyFrames", "");

        int i = 0;

        for (auto *kf : keyFrames) {
          std::cout << prefix << " Frame [" << i << "]\n";

          kf->print(prefix + "  ");

          ++i;
        }
      }
    }

    //---

    std::string tvalueStr(const TimeFrame &frame) const override {
      auto value = tvalue(frame);
      if (! value) return "<none>";
      return CLottieUtil::valueToString(*value);
    }

    std::string minStr() const override {
      return "";
    }

    std::string maxStr() const override {
      return "";
    }

    //---

    OptVal value(const OptVal &def=OptVal()) const {
      if (values.empty())
        return def;

      return values[0];
    }

    OptVal tvalue(const TimeFrame &frame, const OptVal &def=OptVal()) const {
      if (isAnimated()) {
        if (! isTSet())
          return def;

        return keyFrameValue(frame, def);
      }
      else
        return value(def);
    }

    OptVal keyFrameValue(const TimeFrame &frame, const OptVal &def) const {
      auto frameInd = calcFrameInd(frame);
      if (frameInd.pos == FramePos::NONE) return def;

      auto nf = numKeyFrames();

      auto rframe = double(frame.frame) + frame.delta;

      auto *keyFrame1 = keyFrame(frameInd.ind);
      auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

      auto frameStart = keyFrame1->timeFrame().value_or(0.0);
      auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                     frame.frameStop.value_or(1.0));

      if (keyFrame1->startValue.vals.empty() || keyFrame1->endValue.vals.empty()) {
        if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
          --frameInd.ind;

          keyFrame1 = keyFrame(frameInd.ind);
        }
      }

      if (keyFrame1->startValue.vals.empty() && keyFrame1->endValue.vals.empty())
        return def;

      OptVal v1, v2;

      if (! keyFrame1->startValue.vals.empty())
        v1 = keyFrame1->startValue.vals[0];

      if      (! keyFrame1->endValue.vals.empty())
        v2 = keyFrame1->endValue.vals[0];
      else if (! keyFrame2->startValue.vals.empty())
        v2 = keyFrame2->startValue.vals[0];

      if (frameInd.pos == FramePos::INSIDE) {
        if (v1 && v2) {
          auto dt = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

          return CLottieUtil::mapValue(dt, *v1, *v2);
        }
        else if (v1)
          return v1;
        else if (v2)
          return v2;
      }
      else if (frameInd.pos == FramePos::BEFORE) {
        if (v1)
          return v1;
      }
      else if (frameInd.pos == FramePos::AFTER) {
        if (v2)
          return v2;
      }

      return def;
    }

   public:
    std::vector<XYVals>     values;
    std::vector<KeyFrame *> keyFrames;

   private:
    OptInt length_;
  };

  //---

  class SizeProperty : public CLottieProperty {
   public:
    using KeyFrame = CLottieKeyFrameT<XYVals>;
    using OptVal   = std::optional<XYVals>;

   public:
    SizeProperty() :
     CLottieProperty(Type::SIZE) {
    }

   ~SizeProperty() override {
      for (auto *keyFrame : keyFrames)
        delete keyFrame;
    }

    bool isSet() const override {
      return (! values.empty() || ! keyFrames.empty());
    }

    //---

    size_t numKeyFrames() const override { return keyFrames.size(); }

    KeyFrame *keyFrame(uint i) const override { return keyFrames[i]; }

    //---

    void print(const std::string &prefix="") const override {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      CLottieProperty::print(prefix);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! keyFrames.empty()) {
        printValue("KeyFrames", "");

        int i = 0;

        for (auto *kf : keyFrames) {
          std::cout << prefix << " Frame [" << i << "]\n";

          kf->print(prefix + "  ");

          ++i;
        }
      }
    }

    //---

    std::string tvalueStr(const TimeFrame &frame) const override {
      auto value = tvalue(frame);
      if (! value) return "<none>";
      return CLottieUtil::valueToString(*value);
    }

    std::string minStr() const override {
      return "";
    }

    std::string maxStr() const override {
      return "";
    }

    //---

    OptVal value(const OptVal &def=OptVal()) const {
      if (values.empty())
        return def;

      return values[0];
    }

    OptVal tvalue(const TimeFrame &frame, const OptVal &def=OptVal()) const {
      if (isAnimated()) {
        if (! isTSet())
          return def;

        return keyFrameValue(frame, def);
      }
      else
        return value(def);
    }

    OptVal keyFrameValue(const TimeFrame &frame, const OptVal &def) const {
      auto frameInd = calcFrameInd(frame);
      if (frameInd.pos == FramePos::NONE) return def;

      auto nf = numKeyFrames();

      auto rframe = double(frame.frame) + frame.delta;

      auto *keyFrame1 = keyFrame(frameInd.ind);
      auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

      auto frameStart = keyFrame1->timeFrame().value_or(0.0);
      auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                     frame.frameStop.value_or(1.0));

      if (keyFrame1->startValue.vals.empty() || keyFrame1->endValue.vals.empty()) {
        if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
          --frameInd.ind;

          keyFrame1 = keyFrame(frameInd.ind);
        }
      }

      if (keyFrame1->startValue.vals.empty() && keyFrame1->endValue.vals.empty())
        return def;

      OptVal v1, v2;

      if (! keyFrame1->startValue.vals.empty())
        v1 = keyFrame1->startValue.vals[0];

      if      (! keyFrame1->endValue.vals.empty())
        v2 = keyFrame1->endValue.vals[0];
      else if (! keyFrame2->startValue.vals.empty())
        v2 = keyFrame2->startValue.vals[0];

      if (frameInd.pos == FramePos::INSIDE) {
        if (v1 && v2) {
          auto dt = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

          return CLottieUtil::mapValue(dt, *v1, *v2);
        }
        else if (v1)
          return v1;
        else if (v2)
          return v2;
      }
      else if (frameInd.pos == FramePos::BEFORE) {
        if (v1)
          return v1;
      }
      else if (frameInd.pos == FramePos::AFTER) {
        if (v2)
          return v2;
      }

      return def;
    }

   public:
    std::vector<XYVals>     values;
    std::vector<KeyFrame *> keyFrames;
  };

  //---

  using PointList = CLottieUtil::PointList;

  class BezierPathData {
   public:
    void print(const std::string &prefix="") const {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      printValue("closed", closed);

      if (! ipoints.empty()) {
        printValue("ipoints", "");

        for (auto &p : ipoints) {
          std::cout << prefix << "  " << p << "\n";
        }
      }

      if (! opoints.empty()) {
        printValue("opoints", "");

        for (auto &p : opoints) {
          std::cout << prefix << "  " << p << "\n";
        }
      }

      if (! vpoints.empty()) {
        printValue("vpoints", "");

        for (auto &p : vpoints) {
          std::cout << prefix << "  " << p << "\n";
        }
      }
    }

   public:
    std::vector<CPoint2D> ipoints;
    std::vector<CPoint2D> opoints;
    std::vector<CPoint2D> vpoints;
    bool                  closed { false };
  };

  class BezierKeyFrame : public CLottieKeyFrame {
   public:
    BezierKeyFrame() { }

   ~BezierKeyFrame() {
      delete ipathData;
      delete epathData;
    }

    OptBool         closed;
    PointList       values;
    PointList       ivalues;
    PointList       ovalues;
    PointList       vvalues;
    Interpolations  interpolation;
    BezierPathData* ipathData { nullptr };
    BezierPathData* epathData { nullptr };
  };

  class BezierProperty : public CLottieProperty {
   public:
    using OptVal   = std::optional<PointList>;
    using KeyFrame = BezierKeyFrame;

    using Interpolations = std::vector<std::string>;

   public:
    BezierProperty() :
     CLottieProperty(Type::BEZIER) {
    }

   ~BezierProperty() override {
      for (auto *keyFrame : keyFrames)
        delete keyFrame;
    }

    //---

    size_t numKeyFrames() const override { return keyFrames.size(); }

    KeyFrame *keyFrame(uint i) const override { return const_cast<KeyFrame *>(keyFrames[i]); }

    //---

    void print(const std::string &prefix="") const override {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      auto optPrintValue = [&](const std::string &n, const auto &value) {
        if (value)
          std::cout << prefix << n << "=" << *value << "\n";
      };

      CLottieProperty::print(prefix);

      for (auto *keyFrame : keyFrames) {
        optPrintValue("closed", keyFrame->closed);

        if (! keyFrame->interpolation.empty()) {
          std::cout << prefix << "interpolation [";

          for (const auto &i : keyFrame->interpolation)
            std::cout << " " << i;

          std::cout << "]\n";
        }

        optPrintValue("timeFrame", keyFrame->timeFrame());

        printValue("values", keyFrame->values);

        printValue("ivalues", keyFrame->ivalues);
        printValue("ovalues", keyFrame->ovalues);
        printValue("vvalues", keyFrame->vvalues);

        if (keyFrame->ipathData) {
          printValue("ipathData", "");

          keyFrame->ipathData->print(prefix + "  ");
        }

        if (keyFrame->epathData) {
          printValue("epathData", "");

          keyFrame->epathData->print(prefix + "  ");
        }
      }
    }

    //---

    bool isSet() const override { return ! keyFrames.empty(); }

    bool isTSet() const override {
      return ! keyFrames.empty() && ! keyFrames[0]->ivalues.isEmpty(); }

    bool isISet() const { return ! keyFrames.empty()  && ! keyFrames[0]->ivalues.isEmpty(); }
    bool isOSet() const { return ! keyFrames.empty()  && ! keyFrames[0]->ovalues.isEmpty(); }
    bool isVSet() const { return ! keyFrames.empty()  && ! keyFrames[0]->vvalues.isEmpty(); }

    //---

    std::string tvalueStr(const TimeFrame &frame) const override {
      auto points  = tvvalue(frame, CLottie::PointList() )->points;
      auto ipoints = tivalue(frame, CLottie::PointList() )->points;
      auto opoints = tovalue(frame, CLottie::PointList() )->points;
      auto closed  = tclosed(frame).value_or(false);

      CBezierPath bezierPath;

      bezierPath.clear();

      auto n = points.size();
      if (n == 0) return "";

      if (ipoints.size() != n || opoints.size() != n)
        return "";

      auto p1 = points[0];

      bezierPath.moveTo(p1);

      for (size_t i = 1; i < n; ++i) {
        auto p2 = points[i - 1] + opoints[i - 1];
        auto p3 = points[i    ] + ipoints[i    ];
        auto p4 = points[i    ];

        bezierPath.cubicTo(p2, p3, p4);
      }

      if (closed) {
        auto p2 = points[n - 1] + opoints[n - 1];
        auto p3 = points[0    ] + ipoints[0    ];
        auto p4 = points[0    ];

        bezierPath.cubicTo(p2, p3, p4);

        bezierPath.setClosed(true);
      }

      return bezierPath.toString();
    }

    std::string minStr() const override {
      return "";
    }

    std::string maxStr() const override {
      return "";
    }

    //---

    OptVal ivalue(const OptVal &def=OptVal()) const {
      if (! isISet())
        return def;

      return keyFrames[0]->ivalues;
    }

    OptVal ovalue(const OptVal &def=OptVal()) const {
      if (! isOSet())
        return def;

      return keyFrames[0]->ovalues;
    }

    OptVal vvalue(const OptVal &def=OptVal()) const {
      if (! isVSet())
        return def;

      return keyFrames[0]->vvalues;
    }

    OptVal tivalue(const TimeFrame &frame, const OptVal &def=OptVal()) const {
      if (isAnimated()) {
        if (! isTSet())
          return ivalue(def);

#if 1
        auto frameInd = calcFrameInd(frame);
        if (frameInd.pos == FramePos::NONE) return def;

        auto nf = numKeyFrames();

        auto rframe = double(frame.frame) + frame.delta;

        auto *keyFrame1 = keyFrame(frameInd.ind);
        auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

        auto frameStart = keyFrame1->timeFrame().value_or(0.0);
        auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                       frame.frameStop.value_or(1.0));

        auto hasPathData = [&](BezierPathData *pathData) {
          return (pathData && ! pathData->ipoints.empty());
        };

        if (! hasPathData(keyFrame1->ipathData) || ! hasPathData(keyFrame1->epathData)) {
          if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
            --frameInd.ind;

            keyFrame1 = keyFrame(frameInd.ind);
          }
        }

        if (! hasPathData(keyFrame1->ipathData) && ! hasPathData(keyFrame1->epathData))
          return def;

        std::optional<Points> v1, v2;

        if (hasPathData(keyFrame1->ipathData))
          v1 = keyFrame1->ipathData->ipoints;

        if (hasPathData(keyFrame1->epathData))
          v2 = keyFrame1->epathData->ipoints;

        if (frameInd.pos == FramePos::INSIDE) {
          if (v1 && v2) {
            auto dt = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

            return CLottieUtil::mapValue(dt, *v1, *v2);
          }
          else if (v1)
            return v1;
          else if (v2)
            return v2;
        }
        else if (frameInd.pos == FramePos::BEFORE) {
          if (v1)
            return v1;
        }
        else if (frameInd.pos == FramePos::AFTER) {
          if (v2)
            return v2;
        }

        return def;
#else
        auto t1 = int(CMathRound::RoundDown(frame.secs) % keyFrames.size());
        auto t2 = (t1 + 1) % keyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        const auto &v1 = keyFrames[t1]->ipathData->ipoints;
        const auto &v2 = keyFrames[t2]->epathData->ipoints;

        return CLottieUtil::mapValue(dt, v1, v2);
#endif
      }
      else
        return ivalue(def);
    }

    OptVal tovalue(const TimeFrame &frame, const OptVal &def=OptVal()) const {
      if (isAnimated()) {
        if (! isTSet())
          return ovalue(def);

#if 1
        auto frameInd = calcFrameInd(frame);
        if (frameInd.pos == FramePos::NONE) return def;

        auto nf = numKeyFrames();

        auto rframe = double(frame.frame) + frame.delta;

        auto *keyFrame1 = keyFrame(frameInd.ind);
        auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

        auto frameStart = keyFrame1->timeFrame().value_or(0.0);
        auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                       frame.frameStop.value_or(1.0));

        auto hasPathData = [&](BezierPathData *pathData) {
          return (pathData && ! pathData->vpoints.empty());
        };

        if (! hasPathData(keyFrame1->ipathData) || ! hasPathData(keyFrame1->epathData)) {
          if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
            --frameInd.ind;

            keyFrame1 = keyFrame(frameInd.ind);
          }
        }

        if (! hasPathData(keyFrame1->ipathData) && ! hasPathData(keyFrame1->epathData))
          return def;

        std::optional<Points> v1, v2;

        if (hasPathData(keyFrame1->ipathData))
          v1 = keyFrame1->ipathData->opoints;

        if (hasPathData(keyFrame1->epathData))
          v2 = keyFrame1->epathData->opoints;

        if (frameInd.pos == FramePos::INSIDE) {
          if (v1 && v2) {
            auto dt = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

            return CLottieUtil::mapValue(dt, *v1, *v2);
          }
          else if (v1)
            return v1;
          else if (v2)
            return v2;
        }
        else if (frameInd.pos == FramePos::BEFORE) {
          if (v1)
            return v1;
        }
        else if (frameInd.pos == FramePos::AFTER) {
          if (v2)
            return v2;
        }

        return def;
#else
        auto t1 = int(CMathRound::RoundDown(frame.secs) % keyFrames.size());
        auto t2 = (t1 + 1) % keyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        const auto &v1 = keyFrames[t1]->ipathData->opoints;
        const auto &v2 = keyFrames[t2]->epathData->opoints;

        return CLottieUtil::mapValue(dt, v1, v2);
#endif
      }
      else
        return ovalue(def);
    }

    OptVal tvvalue(const TimeFrame &frame, const OptVal &def=OptVal()) const {
      if (isAnimated()) {
        if (! isTSet())
          return vvalue(def);

#if 1
        auto frameInd = calcFrameInd(frame);
        if (frameInd.pos == FramePos::NONE) return def;

        auto nf = numKeyFrames();

        auto rframe = double(frame.frame) + frame.delta;

        auto *keyFrame1 = keyFrame(frameInd.ind);
        auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

        auto frameStart = keyFrame1->timeFrame().value_or(0.0);
        auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                       frame.frameStop.value_or(1.0));

        auto hasPathData = [&](BezierPathData *pathData) {
          return (pathData && ! pathData->vpoints.empty());
        };

        if (! hasPathData(keyFrame1->ipathData) || ! hasPathData(keyFrame1->epathData)) {
          if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
            --frameInd.ind;

            keyFrame1 = keyFrame(frameInd.ind);
          }
        }

        if (! hasPathData(keyFrame1->ipathData) && ! hasPathData(keyFrame1->epathData))
          return def;

        std::optional<Points> v1, v2;

        if (hasPathData(keyFrame1->ipathData))
          v1 = keyFrame1->ipathData->vpoints;

        if (hasPathData(keyFrame1->epathData))
          v2 = keyFrame1->epathData->vpoints;

        if (frameInd.pos == FramePos::INSIDE) {
          if (v1 && v2) {
            auto dt = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

            return CLottieUtil::mapValue(dt, *v1, *v2);
          }
          else if (v1)
            return v1;
          else if (v2)
            return v2;
        }
        else if (frameInd.pos == FramePos::BEFORE) {
          if (v1)
            return v1;
        }
        else if (frameInd.pos == FramePos::AFTER) {
          if (v2)
            return v2;
        }

        return def;
#else
        auto t1 = int(CMathRound::RoundDown(frame.secs) % keyFrames.size());
        auto t2 = (t1 + 1) % keyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        const auto &v1 = keyFrames[t1]->ipathData->vpoints;
        const auto &v2 = keyFrames[t2]->epathData->vpoints;

        return CLottieUtil::mapValue(dt, v1, v2);
#endif
      }
      else
        return vvalue(def);
    }

    OptBool tclosed(const TimeFrame &frame, const OptBool &def=OptBool()) const {
      if (isAnimated()) {
        if (! isTSet())
          return def;

#if 1
        auto frameInd = calcFrameInd(frame);
        if (frameInd.pos == FramePos::NONE) return def;

        auto nf = numKeyFrames();

        auto rframe = double(frame.frame) + frame.delta;

        auto *keyFrame1 = keyFrame(frameInd.ind);
        auto *keyFrame2 = (frameInd.ind < int(nf - 1) ? keyFrame(frameInd.ind + 1) : nullptr);

        auto frameStart = keyFrame1->timeFrame().value_or(0.0);
        auto frameStop  = (keyFrame2 ? keyFrame2->timeFrame().value_or(0.0) :
                                       frame.frameStop.value_or(1.0));

        auto hasPathData = [&](BezierPathData *pathData) {
          return (pathData && pathData->closed);
        };

        if (! hasPathData(keyFrame1->ipathData) || ! hasPathData(keyFrame1->epathData)) {
          if (frameInd.ind > 0 && frameInd.ind == int(nf - 1)) {
            --frameInd.ind;

            keyFrame1 = keyFrame(frameInd.ind);
          }
        }

        if (! hasPathData(keyFrame1->ipathData) && ! hasPathData(keyFrame1->epathData))
          return def;

        std::optional<bool> v1, v2;

        if (hasPathData(keyFrame1->ipathData))
          v1 = keyFrame1->ipathData->closed;

        if (hasPathData(keyFrame1->epathData))
          v2 = keyFrame1->epathData->closed;

        if (frameInd.pos == FramePos::INSIDE) {
          if (v1 && v2) {
            auto dt = CMathUtil::map(rframe, frameStart, frameStop, 0.0, 1.0);

            return CLottieUtil::mapValue(dt, *v1, *v2);
          }
          else if (v1)
            return v1;
          else if (v2)
            return v2;
        }
        else if (frameInd.pos == FramePos::BEFORE) {
          if (v1)
            return v1;
        }
        else if (frameInd.pos == FramePos::AFTER) {
          if (v2)
            return v2;
        }

        return def;
#else
        auto t1 = int(CMathRound::RoundDown(frame.secs) % keyFrames.size());

        auto v1 = keyFrames[t1]->closed.value_or(false);

        return v1;
#endif
      }
      else {
        if (keyFrames.empty())
          return def;

        return keyFrames[0]->closed.value_or(false);
      }
    }

    FrameInd calcFrameInd(const TimeFrame &frame) const override {
      FrameInd frameInd;

      auto nf = uint(numKeyFrames());

      if (nf == 0)
        return frameInd;

      frameInd.pos = FramePos::BEFORE;
      frameInd.ind = 0;

      auto rframe = double(frame.frame) + frame.delta;

      for (uint i = 0; i < nf; ++i) {
        auto *keyFrame1 = keyFrame(i);
        auto *keyFrame2 = (i < nf - 1 ? keyFrame(i + 1) : nullptr);

        auto frameStart = keyFrame1->timeFrame().value_or(0.0);

        OptReal frameStop;

        if (keyFrame2)
          frameStop = keyFrame2->timeFrame().value_or(0.0);

        if (rframe >= frameStart && frameStop && rframe < frameStop.value()) {
          frameInd.pos = FramePos::INSIDE;
          frameInd.ind = int(i);
          break;
        }

        if (frameStop && rframe >= frameStop.value()) {
          frameInd.pos = FramePos::AFTER;
          frameInd.ind = int(i);
        }
      }

      assert(frameInd.ind >= 0 && frameInd.ind <= int(nf - 1));

      return frameInd;
    }

   public:
    std::vector<KeyFrame *> keyFrames;
  };

  //---

  class ColorProperty : public PropertyT<CRGBA> {
   public:
    ColorProperty() :
     PropertyT(Type::COLOR) {
    }
  };

  //---

  class ArrayProperty : public PropertyT<CLottieUtil::RValArray> {
   public:
    ArrayProperty() :
     PropertyT(Type::ARRAY) {
    }
  };

  // transform
  struct Transform {
    PositionProperty      anchorPoint;
    SplitPositionProperty position;
    ScalarProperty        rotation { 0, 360 };
    VectorProperty        scale;
    ScalarProperty        opacity;
    ScalarProperty        skew;
    ScalarProperty        skewAxis;
    ScalarProperty        x_rotation { 0, 360 };
    ScalarProperty        y_rotation { 0, 360 };
    ScalarProperty        z_rotation { 0, 360 };
    VectorProperty        orientation;

    void* repeater { nullptr };

    void print(const std::string &prefix="") const;
  };

  // dash
  struct DashData {
    OptStr         type;
    OptStr         name;
    ScalarProperty value;
  };

  struct Dash {
    using Datas = std::vector<DashData>;

    std::map<std::string, Datas> data;

    std::vector<ScalarProperty> offset;
    std::vector<ScalarProperty> dash;
    std::vector<ScalarProperty> gap;
  };

  struct StyleData {
    OptInt type;
    OptStr name;

    ColorProperty  color;
    ScalarProperty size;
  };

  struct Slot {
    ScalarProperty rotation { 0, 360 };
    VectorProperty scale;
    ScalarProperty opacity;
  };

 public:
  CLottie();
 ~CLottie();

  //---

  bool isDebug() const { return debug_; }
  void setDebug(bool b) { debug_ = b; }

  bool isPrint() const { return print_; }
  void setPrint(bool b) { print_ = b; }

  bool isStats() const { return stats_; }
  void setStats(bool b) { stats_ = b; }

  bool isQuiet() const { return quiet_; }
  void setQuiet(bool b) { quiet_ = b; }

  //---

  CLottieFactory *factory() const { return factory_; }
  void setFactory(CLottieFactory *f) { factory_ = f; }

  bool load(const std::string &file);

  void buildLayerHier();
  void printLayerHier() const;

  void printHier() const;

  void printStats() const;

  void reset();

  //---

  CLottieRoot *root() const { return root_; }

  CLottieAsset *getAssetById(const std::string &id) const;

  void addLayerId(CLottieLayer *layer);

  CLottieLayer *getLayerById(int id) const;

  //---

  CLottieAsset *getLayerAsset(const CLottieLayer *layer) const;

  //---

  const Layers &layers() const { return layers_; }
  const Shapes &shapes() const { return shapes_; }

  //---

  void deselectAll();

  CMatrixStack2D getTransformMatrix(const TimeFrame &timeFrame, CLottie::Transform *transform,
                                    bool autoOrient=false) const;

  CMatrixStack2D getRepeaterMatrix(const TimeFrame &timeFrame,
                                   CLottie::Transform *transform, double f) const;

  CBezierPath getPositionPath(CLottie::Transform *transform) const;

  //---

  bool pathToBezier(const CLottie::BezierProperty &path, const TimeFrame &timeFrame,
                    CBezierPath &bezierPath) const;

  //---

  CLottieEffect *makeEffect();

 private:
  bool readRoot(const std::string &msg, const CJson::ValueP &value);

  bool readLayer (const std::string &msg, const CJson::ValueP &ivalue, CLottieLayer *layer);
  bool readMarker(const std::string &msg, const CJson::ValueP &ivalue, CLottieMarker *marker);
  bool readShape (const std::string &msg, const CJson::ValueP &ivalue, CLottieShape *shape);
  bool readAsset (const std::string &msg, const CJson::ValueP &ivalue, CLottieAsset *asset);

  bool readLayerMaskProperties(const std::string &msg, const CJson::ValueP &iValue,
                               CLottieLayer *layer);
  bool readLayerShapes(const std::string &msg, const CJson::ValueP &iValue, CLottieLayer *layer);

  bool readSlots(const std::string &msg, const CJson::ValueP &iValue);

  bool getSlotValue(const std::string &name, CJson::ValueP &value) const;

  bool readSplitPositionProperty(const std::string &msg, const CJson::ValueP &ivalue,
                                 SplitPositionProperty &position) const;
  bool readVectorProperty       (const std::string &msg, const CJson::ValueP &ivalue,
                                 VectorProperty &vector) const;

  bool readPositionProperty(const std::string &msg, const CJson::ValueP &ivalue,
                            PositionProperty &position) const;
  bool readSizeProperty    (const std::string &msg, const CJson::ValueP &ivalue,
                            SizeProperty &size) const;
  bool readColorProperty   (const std::string &msg, const CJson::ValueP &ivalue,
                            ColorProperty &color) const;
  bool readArrayProperty   (const std::string &msg, const CJson::ValueP &ivalue,
                            ArrayProperty &color) const;
  bool readBezierProperty  (const std::string &msg, const CJson::ValueP &ivalue,
                            BezierProperty &bezier) const;
  bool readScalarProperty  (const std::string &msg, const CJson::ValueP &ivalue,
                            ScalarProperty &scalar) const;

  bool readTransform(const std::string &msg1, CJson::ValueP &value1, Transform *transform) const;

  bool readEffect(const std::string &msg, const CJson::ValueP &iValue, CLottieEffect *effect);
  bool readEffectValues(const std::string &msg, const CJson::ValueP &iValue,
                        std::vector<CLottieEffectValue *> &effectValues);

  bool readStyleData(const std::string &msg, const CJson::ValueP &iValue, StyleData *styleData);

  bool getKeyFrameValues(const std::string &xyMsg, const std::string &name, CJson::ValueP &xyValue,
                         std::vector<XYVals> &xyValues) const;

  bool readPointList(const std::string &msg, const CJson::ValueP &iValue,
                     std::vector<CPoint2D> &points) const;

  bool readDash(const std::string &msg, const CJson::ValueP &iValue, Dash &dash) const;

  bool readVector(const std::string &msg, const CJson::ValueP &iValue, CPoint2D &p) const;
  bool readColor (const std::string &msg, const CJson::ValueP &iValue, OptColor &c) const;

  bool readStrings(const std::string &msg, const CJson::ValueP &iValue,
                   std::vector<std::string> &strs) const;
  bool readNumbers(const std::string &msg, const CJson::ValueP &iValue,
                   std::vector<double> &numbers) const;

  void addLayer (CLottieLayer  *layer);
  void addMarker(CLottieMarker *marker);
  void addShape (CLottieShape  *shape);

  //---

  std::string valueToString(const CJson::ValueP &value, const std::string &def="") const;
  double      valueToReal  (const CJson::ValueP &value, double def=0.0) const;
  int         valueToInt   (const CJson::ValueP &value, int def=0) const;
  bool        valueToBool  (const CJson::ValueP &value, bool def=false) const;

  CLottieRoot   *makeRoot  ();
  CLottieLayer  *makeLayer ();
  CLottieShape  *makeShape ();
  CLottieAsset  *makeAsset ();
  CLottieMarker *makeMarker();

  CLottieEffectValue *makeEffectValue();

  //---

  void unhandledName(const std::string &name, const CJson::ValueP &value) const;
  void todoName(const std::string &msg, const std::string &name, const CJson::ValueP &value) const;

  bool warnValueMsg(const std::string &prefix, const std::string &msg,
                    const CJson::ValueP &value) const;

 private:
  bool debug_ { false };
  bool print_ { false };
  bool stats_ { false };
  bool quiet_ { false };

  CLottieFactory *factory_ { nullptr };

  CLottieRoot* root_ { nullptr };

  using AssetMap = std::map<std::string, CLottieAsset *>;
  using LayerMap = std::map<int        , CLottieLayer *>;
  using ShapeMap = std::map<int        , CLottieShape *>;

  AssetMap assetIds_;
  Assets   assets_;

  LayerMap layerIds_;
  Layers   layers_;

  ShapeMap shapeIds_;
  Shapes   shapes_;

  Markers markers_;

  Effects effects_;

  using Slots = std::map<std::string, CJson::ValueP>;

  Slots slots_;

  using LayoutTypeCount = std::map<int, int>;
  using ShapeTypeCount  = std::map<int, int>;
  using EffectTypeCount = std::map<int, int>;

  struct StatsData {
    LayoutTypeCount layerTypeCount;
    ShapeTypeCount  shapeTypeCount;
    EffectTypeCount effectTypeCount;
    int             layerMatteCount { 0 };
    int             layerMaskCount  { 0 };
  };

  StatsData statsData_;
};

//---

class CLottieFactory {
 public:
  CLottieFactory() { }

  virtual ~CLottieFactory() { }

  virtual CLottieLayer *makeLayer(CLottie *) = 0;
  virtual CLottieShape *makeShape(CLottie *) = 0;
  virtual CLottieAsset *makeAsset(CLottie *) = 0;
};

//---

class CLottieObject {
 public:
  using OptReal  = std::optional<double>;
  using OptInt   = std::optional<int>;
  using OptBool  = std::optional<bool>;
  using OptStr   = std::optional<std::string>;
  using OptColor = std::optional<CRGBA>;

  using Transform = CLottie::Transform;

  using TimeFrame = CLottieUtil::TimeFrame;

  using PositionProperty = CLottie::PositionProperty;
  using SizeProperty     = CLottie::SizeProperty;
  using VectorProperty   = CLottie::VectorProperty;
  using BezierProperty   = CLottie::BezierProperty;
  using ColorProperty    = CLottie::ColorProperty;
  using ArrayProperty    = CLottie::ArrayProperty;
  using ScalarProperty   = CLottie::ScalarProperty;

  //---

  enum class Type {
    NONE,
    ROOT,
    LAYER,
    SHAPE,
    ASSET,
    MARKER,
    EFFECT,
    REPEATER
  };

  enum class LayerType {
    UNKNOWN = -1,
    PRECOMPOSITION = 0,
    SOLID = 1,
    IMAGE = 2,
    NUL = 3,
    SHAPE = 4,
    TEXT = 5,
    AUDIO = 6,
    CAMERA = 13,
    DATA = 15,
  };

  enum class ShapeType {
    NONE,
    ELLIPSE,
    FILL,
    GRADIENT_FILL,
    GROUP,
    GRADIENT_STROKE,
    MERGE,
    OFFSET_PATH,
    PATH,
    POLYSTAR,
    PUCKER_BLOAT,
    RECTANGLE,
    REPEATER,
    ROUNDED,
    STROKE,
    TRANSFORM,
    TRIM,
    TWIST,
    ZIGZAG
  };

 public:
  static std::string typeName(const Type &type) {
    switch (type) {
      case Type::ROOT    : return "Root";
      case Type::LAYER   : return "Layer";
      case Type::SHAPE   : return "Shape";
      case Type::ASSET   : return "Asset";
      case Type::MARKER  : return "Marker";
      case Type::EFFECT  : return "Effect";
      case Type::REPEATER: return "Repeater";
      default:             return "None";
    }
  };

  CLottieObject(CLottie *l, const Type &t);

  virtual ~CLottieObject();

  //---

  CLottie* lottie() const { return lottie_; }

  const Type &objectType() const { return objectType_; }

  const OptStr &name() const { return name_; }
  void setName(const OptStr &s) { name_ = s; }

  const OptStr &type() const { return type_; }
  void setType(const OptStr &s) { type_ = s; }

  const OptInt &typeId() const { return typeId_; }
  void setTypeId(const OptInt &v) { typeId_ = v; }

  bool selected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  const OptBool &isHidden() const { return hidden_; }
  void setHidden(const OptBool &v) { hidden_ = v; }

  const OptInt &ind() const { return ind_; }
  void setInd(const OptInt &v) { ind_ = v; }

  CLottieObject *parent() const { return parent_; }
  void setParent(CLottieObject *v) { parent_ = v; }

  const OptStr &css() const { return css_; }
  void setCss(const OptStr &v) { css_ = v; }

  //---

  const CBBox2D &bbox() const { return bbox_; }
  void setBBox(const CBBox2D &v) { bbox_ = v; }

  //---

  const CJson::ValueP &jsonValue() const { return jsonValue_; }
  void setJsonValue(const CJson::ValueP &v) { jsonValue_ = v; }

  //---

  std::string hierName() const;

  bool isHierSelected() const;

  //---

  virtual CLottieRoot *getRoot() const = 0;

  virtual CMatrixStack2D calcTransform(const TimeFrame &) const = 0;

  virtual CMatrixStack2D calcHierTransform(const TimeFrame &timeFrame) const {
    return calcTransform(timeFrame);
  }

  //---

  virtual CLottieVariant *getVariant(const std::string &name) {
    if      (name == "hidden") {
      return new CLottieVariantT<OptBool>(&hidden_);
    }
    else if (name == "css") {
      return new CLottieVariantT<OptStr>(&css_);
    }
    else {
      std::cerr << "No variant of name '" << name << "'\n";
      return nullptr;
    }
  }

  virtual CLottieProperty *getProperty(const std::string &name) const {
    std::cerr << "No property of name '" << name << "'\n";
    return nullptr;
  }

  //---

  void debugPrint() const;

  void printHier(const std::string &prefix="") const { printI(prefix, true); }
  void print(const std::string &prefix="") const { printI(prefix, false); }

  virtual void printI(const std::string &prefix, bool hier) const;

 protected:
  CLottie* lottie_ { nullptr };

  Type objectType_ { Type::NONE };

  OptStr         name_;
  OptStr         type_;
  OptInt         typeId_;
  bool           selected_ { false };
  OptBool        hidden_;
  OptInt         ind_;
  CLottieObject* parent_ { nullptr };
  OptStr         css_;

  CBBox2D bbox_;

  CJson::ValueP jsonValue_;
};

//---

class CLottieRoot : public CLottieObject {
 public:
  using Layers = std::vector<CLottieLayer *>;
  using Assets = std::vector<CLottieAsset *>;

 public:
  CLottieRoot(CLottie *l);
 ~CLottieRoot() override;

  //---

  const OptStr &version() const { return version_; }
  void setVersion(const OptStr &s) { version_ = s; }

  const OptStr &matchName() const { return matchName_; }
  void setMatchName(const OptStr &v) { matchName_ = v; }

  //---

  const OptReal &frameRate() const { return timeFrame_.frameRate; }
  void setFrameRate(const OptReal &r) { timeFrame_.frameRate = r; }

  const OptReal &frameStart() const { return timeFrame_.frameStart; }
  void setFrameStart(const OptReal &r) { timeFrame_.frameStart = r; }

  const OptReal &frameStop() const { return timeFrame_.frameStop; }
  void setFrameStop(const OptReal &r) { timeFrame_.frameStop = r; }

  //---

  const OptReal &width() const { return width_; }
  void setWidth(const OptReal &v) { width_ = v; }

  const OptReal &height() const { return height_; }
  void setHeight(const OptReal &v) { height_ = v; }

  const OptBool &threeD() const { return threeD_; }
  void setThreeD(const OptBool &v) { threeD_ = v; }

  //---

  const Layers &layers() const { return layers_; }
  const Assets &assets() const { return assets_; }

  void addLayer(CLottieLayer *layer) { layers_.push_back(layer); }
  void addAsset(CLottieAsset *asset) { assets_.push_back(asset); }

  const Layers &childLayers() const { return childLayers_; }
  void addChildLayer(CLottieLayer *layer) { childLayers_.push_back(layer); }

  //---

  CLottieRoot *getRoot() const override { return const_cast<CLottieRoot *>(this); }

  CMatrixStack2D calcTransform(const TimeFrame &) const override { return CMatrixStack2D(); }

//void buildLayerHier();
  void printLayerHier() const;

  //---

  CLottieVariant *getVariant(const std::string &name) override {
    if      (name == "frameStart") {
      return new CLottieVariantT<OptReal>(&timeFrame_.frameStart);
    }
    else if (name == "frameStop") {
      return new CLottieVariantT<OptReal>(&timeFrame_.frameStop);
    }
    else
      return CLottieObject::getVariant(name);
  }

  //---

  void printI(const std::string &prefix, bool hier) const override;

 private:
  OptStr version_;

  OptStr matchName_;

  TimeFrame timeFrame_; // frameRate, frameStart, frameStop

  OptReal width_;
  OptReal height_;

  OptBool threeD_;

  Assets assets_;
  Layers layers_;

  Layers childLayers_;
};

//---

class CLottieAsset : public CLottieObject {
 public:
  using Layers = std::vector<CLottieLayer *>;

 public:
  CLottieAsset(CLottie *l);
 ~CLottieAsset() override;

  //---

  const OptStr &id() const { return id_; }
  void setId(const OptStr &s) { id_ = s; }

  const OptReal &width() const { return width_; }
  void setWidth(const OptReal &v) { width_ = v; }

  const OptReal &height() const { return height_; }
  void setHeight(const OptReal &v) { height_ = v; }

  const OptStr &dir() const { return dir_; }
  void setDir(const OptStr &v) { dir_ = v; }

  const OptStr &path() const { return path_; }
  void setPath(const OptStr &v) { path_ = v; }

  const OptBool &embedded() const { return embedded_; }
  void setEmbedded(const OptBool &v) { embedded_ = v; }

  const Layers &layers() const { return layers_; }

  bool hasLayer(CLottieLayer *layer) const;
  void addLayer(CLottieLayer *layer);

  CLottieLayer *getLayerById(int id) const;

  //---

  CLottieRoot *getRoot() const override;

  const Layers &childLayers() const { return childLayers_; }
  void addChildLayer(CLottieLayer *layer) { childLayers_.push_back(layer); }

  //---

  CMatrixStack2D calcTransform(const TimeFrame &) const override { return CMatrixStack2D(); }

  void printI(const std::string &prefix, bool hier) const override;

 private:
  void addLayerId(CLottieLayer *layer);

 private:
  using LayerMap = std::map<int, CLottieLayer *>;

  OptStr  id_;
  OptReal width_;
  OptReal height_;

  OptStr  dir_;
  OptStr  path_;
  OptBool embedded_;

  Layers layers_;

  Layers childLayers_;

  LayerMap layerIds_;
};

//---

// repeater
struct CLottieRepeater : public CLottieObject {
 public:
  CLottieRepeater(CLottieShape *shape);
 ~CLottieRepeater() override;

  CLottieRoot *getRoot() const override;

  CMatrixStack2D calcTransform(const TimeFrame &) const override { return CMatrixStack2D(); }

  void printI(const std::string &prefix, bool hier) const override;

 public:
  using ScalarProperty = CLottie::ScalarProperty;
  using Transform      = CLottie::Transform;
  using OptInt         = std::optional<int>;

  CLottieShape *shape_ { nullptr };

  ScalarProperty copies;
  ScalarProperty offset;
  OptInt         composite;
  Transform*     transform { nullptr };

  ScalarProperty startOpacity;
  ScalarProperty endOpacity;

  OptInt ind;
};

//---

class CLottieEffect : public CLottieObject {
 public:
  using EffectValues = std::vector<CLottieEffectValue *>;

 public:
  CLottieEffect(CLottie *lottie);
 ~CLottieEffect() override;

  const OptInt &type() const { return type_; }
  void setType(const OptInt &v) { type_ = v; }

  const OptStr &match() const { return match_; }
  void setMatch(const OptStr &v) { match_ = v; }

  const OptInt &index() const { return index_; }
  void setIndex(const OptInt &v) { index_ = v; }

  const OptInt &numProperties() const { return numProperties_; }
  void setNumProperties(const OptInt &v) { numProperties_ = v; }

  const OptInt &enabled() const { return enabled_; }
  void setEnabled(const OptInt &v) { enabled_ = v; }

  CLottieRoot *getRoot() const override;

  CMatrixStack2D calcTransform(const TimeFrame &) const override { return CMatrixStack2D(); }

  CLottieLayer *getLayer() const;

  const EffectValues &values() const { return values_; }

  void addValue(CLottieEffectValue *value) {
    values_.push_back(value);
  }

  void printI(const std::string &prefix, bool hier) const override;

 private:
  using OptInt = std::optional<int>;
  using OptStr = std::optional<std::string>;

  OptInt type_;
  OptStr match_;
  OptInt index_;

  OptInt numProperties_;
  OptInt enabled_;

  EffectValues values_;
};

class CLottieEffectValue {
 public:
  CLottieEffectValue(CLottie *lottie) :
   lottie_(lottie) {
  }

 public:
  void print(const std::string &prefix="") const;

 public:
  using OptInt  = std::optional<int>;
  using OptReal = std::optional<double>;
  using OptStr  = std::optional<std::string>;

  using OptScalarProperty = std::optional<CLottie::ScalarProperty>;
  using OptColorProperty  = std::optional<CLottie::ColorProperty>;
  using OptVectorProperty = std::optional<CLottie::VectorProperty>;

  CLottie* lottie_ { nullptr };

  CLottieEffect* parent { nullptr };

  OptInt type;
  OptStr name;
  OptStr match;
  OptInt index;

  OptInt            ivalue;
  OptScalarProperty scalar;
  OptColorProperty  color;
  OptVectorProperty point;
  OptReal           number;
};

//---

class CLottieLayer : public CLottieObject {
 public:
  using StyleData = CLottie::StyleData;

  using Layers = std::vector<CLottieLayer *>;
  using Shapes = std::vector<CLottieShape *>;

  struct Mask {
    OptStr         mode;
    ScalarProperty opacity;
    BezierProperty path;
    ScalarProperty expand;
    OptBool        inverted;
    OptStr         name;

    void print(const std::string &prefix="") const;
  };

  struct Solid {
    OptReal  width;
    OptReal  height;
    OptColor color;

    void print(const std::string &prefix="") const;
  };

  struct Precomp {
    OptStr         refId;
    OptReal        width;
    OptReal        height;
    OptReal        startTime;
    ScalarProperty timeRemap;

    void print(const std::string &prefix="") const;
  };

 public:
  static const char *typeIdName(int t);

  //---

  CLottieLayer(CLottie *l);
 ~CLottieLayer() override;

  //---

  CLottieLayer *getParentLayer() const;
  CLottieAsset *getParentAsset() const;

  //---

  const LayerType &layerType() const { return layerType_; }
  void setLayerType(const LayerType &t) { layerType_ = t; }

  const OptBool &threeD() const { return threeD_; }
  void setThreeD(const OptBool &v) { threeD_ = v; }

  const OptBool &autoOrient() const { return autoOrient_; }
  void setAutoOrient(const OptBool &v) { autoOrient_ = v; }

  const OptInt &blendMode() const { return blendMode_; }
  void setBlendMode(const OptInt &v) { blendMode_ = v; }

  const OptInt &matteMode() const { return matteMode_; }
  void setMatteMode(const OptInt &v) { matteMode_ = v; }

  const OptInt &matteParent() const { return matteParent_; }
  void setMatteParent(const OptInt &v) { matteParent_ = v; }

  const OptInt &matteTarget() const { return matteTarget_; }
  void setMatteTarget(const OptInt &v) { matteTarget_ = v; }

  const OptBool &hasMask() const { return hasMask_; }
  void setHasMask(const OptBool &v) { hasMask_ = v; }

  const OptStr &refId() const { return refId_; }
  void setRefId(const OptStr &v) { refId_ = v; }

  const OptInt &parentInd() const { return parentInd_; }
  void setParentInd(const OptInt &v) { parentInd_ = v; }

  const OptReal &width() const { return width_; }
  void setWidth(const OptReal &v) { width_ = v; }

  const OptReal &height() const { return height_; }
  void setHeight(const OptReal &v) { height_ = v; }

  const OptStr &matchName() const { return matchName_; }
  void setMatchName(const OptStr &v) { matchName_ = v; }

  const OptReal &frameIn() const { return frameIn_; }
  void setFrameIn(const OptReal &v) { frameIn_ = v; }

  const OptReal &frameOut() const { return frameOut_; }
  void setFrameOut(const OptReal &v) { frameOut_ = v; }

  const OptReal &startTime() const { return startTime_; }
  void setStartTime(const OptReal &v) { startTime_ = v; }

  const OptReal &timeStretch() const { return timeStretch_; }
  void setTimeStretch(const OptReal &v) { timeStretch_ = v; }

  const OptInt &collapseTransform() const { return collapseTransform_; }
  void setCollapseTransform(const OptInt &v) { collapseTransform_ = v; }

  //---

  Transform *transform() const { return transform_; }

  Transform *getTransform() {
    if (! transform_)
      transform_ = new Transform;
    return transform_;
  }

  Mask *mask() const { return mask_; }

  Mask *getMask() {
    if (! mask_)
      mask_ = new Mask;
    return mask_;
  }

  CLottieEffect *effect() const { return effect_; }
  CLottieEffect *getEffect();

  StyleData *styleData() const { return styleData_; }
  StyleData *getStyleData();

  Solid *solid() const { return solid_; }

  Solid *getSolid() {
    if (! solid_)
      solid_ = new Solid;
    return solid_;
  }

  Precomp *precomp() const { return precomp_; }

  Precomp *getPrecomp() {
    if (! precomp_)
      precomp_ = new Precomp;
    return precomp_;
  }

  const Shapes &shapes() const { return shapes_; }

  void addShape(CLottieShape *shape) { shapes_.push_back(shape); }

  //---

  const Layers &childLayers() const { return childLayers_; }
  void addChildLayer(CLottieLayer *layer) { childLayers_.push_back(layer); }

  //---

  CLottieRoot *getRoot() const override;

  CMatrixStack2D calcTransform(const TimeFrame &) const override;
  CMatrixStack2D calcHierTransform(const TimeFrame &timeFrame) const override;

  CLottieRepeater *calcRepeater() const;

  CLottieShape *getRepeaterShape      () const;
  CLottieShape *getTransformShape     () const;
  CLottieShape *getFillShape          () const;
  CLottieShape *getStrokeShape        () const;
  CLottieShape *getGradientFillShape  () const;
  CLottieShape *getGradientStrokeShape() const;
  CLottieShape *getMergeShape         () const;
  CLottieShape *getGeomShape          () const;

  CLottieShape *getShapeOfType(const ShapeType &type, bool hidden=false) const;

  //---

  CLottieVariant *getVariant(const std::string &name) override {
    if      (name == "frameIn") {
      return new CLottieVariantT<OptReal>(&frameIn_);
    }
    else if (name == "frameOut") {
      return new CLottieVariantT<OptReal>(&frameOut_);
    }
    else if (name == "matteMode") {
      return new CLottieVariantT<OptInt>(&matteMode_);
    }
    else if (name == "mask.mode") {
      if (! mask()) return nullptr;
      return new CLottieVariantT<OptStr>(&mask_->mode);
    }
    else if (name == "mask.inverted") {
      if (! mask()) return nullptr;
      return new CLottieVariantT<OptBool>(&mask_->inverted);
    }
    else
      return CLottieObject::getVariant(name);
  }

  //---

  CLottieProperty *getProperty(const std::string &name) const override {
    if      (name == "transform.anchorPoint") {
      if (! transform()) return nullptr;
      return &transform()->anchorPoint;
    }
    else if (name == "transform.position") {
      if (! transform()) return nullptr;
      return &transform()->position;
    }
    else if (name == "transform.rotation") {
      if (! transform()) return nullptr;
      return &transform()->rotation;
    }
    else if (name == "transform.scale") {
      if (! transform()) return nullptr;
      return &transform()->scale;
    }
    else if (name == "transform.opacity") {
      if (! transform()) return nullptr;
      return &transform()->opacity;
    }
    else if (name == "transform.skew") {
      if (! transform()) return nullptr;
      return &transform()->skew;
    }
    else if (name == "transform.skewAxis") {
      if (! transform()) return nullptr;
      return &transform()->skewAxis;
    }
    else if (name == "transform.x_rotation") {
      if (! transform()) return nullptr;
      return &transform()->x_rotation;
    }
    else if (name == "transform.y_rotation") {
      if (! transform()) return nullptr;
      return &transform()->y_rotation;
    }
    else if (name == "transform.z_rotation") {
      if (! transform()) return nullptr;
      return &transform()->z_rotation;
    }
    else if (name == "transform.orientation") {
      if (! transform()) return nullptr;
      return &transform()->orientation;
    }
    else if (name == "precomp.timeRemap") {
      if (! precomp()) return nullptr;
      return &precomp()->timeRemap;
    }
    else if (name == "mask.opacity") {
      if (! mask()) return nullptr;
      return &mask()->opacity;
    }
    else if (name == "mask.path") {
      if (! mask()) return nullptr;
      return &mask()->path;
    }
    else if (name == "mask.expand") {
      if (! mask()) return nullptr;
      return &mask()->expand;
    }
    else
      return CLottieObject::getProperty(name);
  }

  //---

  void printLayerHier(const std::string &prefix) const;

  void printI(const std::string &prefix, bool hier) const override;

  //---

 private:
  LayerType layerType_ { LayerType::UNKNOWN };

  OptStr matchName_;

  OptBool threeD_;
  OptBool autoOrient_;
  OptInt  blendMode_;

  OptInt  matteMode_;
  OptInt  matteParent_;
  OptInt  matteTarget_;
  OptBool hasMask_;

  OptStr refId_;

  OptInt parentInd_;

  OptReal width_;
  OptReal height_;

  OptReal frameIn_;
  OptReal frameOut_;
  OptReal startTime_;
  OptReal timeStretch_;

  OptInt collapseTransform_;

  // transform
  Transform *transform_ { nullptr };

  Mask*          mask_      { nullptr };
  CLottieEffect* effect_    { nullptr };
  StyleData*     styleData_ { nullptr };
  Solid*         solid_     { nullptr };
  Precomp*       precomp_   { nullptr };

  Shapes shapes_;

  Layers childLayers_;
};

//---

class CLottieMarker : public CLottieObject {
 public:
  CLottieMarker(CLottie *l);
 ~CLottieMarker() override;

  //---

  CLottieRoot *getRoot() const override;

  CMatrixStack2D calcTransform(const TimeFrame &) const override { return CMatrixStack2D(); }

  void printI(const std::string &prefix, bool hier) const override;
};

//---

class CLottieShape : public CLottieObject {
 public:
  using Shapes = std::vector<CLottieShape *>;
  using Dash   = CLottie::Dash;

  // rectangle
  struct Rectangle {
    ScalarProperty roundness { 0, 100 };

    void print(const std::string &prefix="") const;
  };

  // stroke
  struct Stroke {
    ColorProperty  color;
    ScalarProperty opacity;
    ScalarProperty width;
    OptInt         lineCap;
    OptInt         lineJoin;
    OptReal        miterLimit;
    ScalarProperty miterLimitAnim;
    Dash           dash;
    OptInt         blendMode;

    void print(const std::string &prefix="") const;
  };

  // fill
  struct Fill {
    ColorProperty  color;
    ScalarProperty opacity;
    OptInt         fillRule;
    OptInt         blendMode;
    OptBool        fillEnabled;

    void print(const std::string &prefix="") const;
  };

  struct Group {
    ColorProperty  color;
    ScalarProperty opacity;
    OptInt         numProperties;
    OptInt         blendMode;

    void print(const std::string &prefix="") const;
  };

  struct GradientBase {
    ScalarProperty opacity;
    OptInt         type;
    OptInt         stopCount;
    OptInt         index;
    VectorProperty startPoint;
    VectorProperty endPoint;
    ArrayProperty  colors;
  };

  struct GradientFill : GradientBase {
    ColorProperty  color;
    ScalarProperty highlightLength;
    ScalarProperty highlightAngle;
    OptInt         fillRule;
    OptInt         blendMode;

    void print(const std::string &prefix="") const;
  };

  struct GradientStroke : GradientBase {
    ScalarProperty width;
    OptInt         lineCap;
    OptInt         lineJoin;
    OptReal        miterLimit;
    Dash           dash;
    OptInt         fillRule;
    ColorProperty  color;

    void print(const std::string &prefix="") const;
  };

  struct Trim {
    ScalarProperty start;
    ScalarProperty end;
    ScalarProperty offset;
    OptInt         multiple;

    void print(const std::string &prefix="") const;
  };

  struct PolyStar {
    OptInt           type;
    PositionProperty position;
    ScalarProperty   innerRadius;
    ScalarProperty   innerRoundness { 0, 100 };
    ScalarProperty   outerRadius;
    ScalarProperty   outerRoundness { 0, 100 };
    ScalarProperty   rotation { 0, 360 };
    ScalarProperty   points;

    void print(const std::string &prefix="") const;
  };

  struct Merge {
    OptInt mode;

    void print(const std::string &prefix="") const;
  };

  struct Rounded {
    ScalarProperty roundness { 0, 100 };

    void print(const std::string &prefix="") const;
  };

  //---

  CLottieShape(CLottie *l);
 ~CLottieShape() override;

  //---

  const ShapeType &shapeType() const { return shapeType_; }
  void setShapeType(const ShapeType &t) { shapeType_ = t; }

  //---

  CLottieRoot *getRoot() const override;

  CLottieLayer *getParentLayer() const;
  CLottieShape *getParentShape() const;

  CLottieLayer *getHierParentLayer() const;

  const Shapes &shapes() const { return shapes_; }

  void addShape(CLottieShape *shape) { shapes_.push_back(shape); }

  //---

  CMatrixStack2D calcTransform(const TimeFrame &) const override;
  CMatrixStack2D calcHierTransform(const TimeFrame &timeFrame) const override;

  Fill *calcFill() const;

  CLottieRepeater *calcRepeater() const;

  Merge *calcHierMerge() const;
  CLottieShape *calcHierMergeShape() const;

  CLottieShape *getRepeaterShape      () const;
  CLottieShape *getTransformShape     () const;
  CLottieShape *getFillShape          () const;
  CLottieShape *getStrokeShape        () const;
  CLottieShape *getGradientFillShape  () const;
  CLottieShape *getGradientStrokeShape() const;
  CLottieShape *getMergeShape         () const;
  CLottieShape *getGeomShape          () const;

  CLottieShape *getShapeOfType(const ShapeType &type, bool hidden=false) const;

  bool isGeomShape() const;

  static const char *shapeTypeName(const ShapeType &shapeType);

  //---

  const OptStr &longName() const { return longName_; }
  void setLongName(const OptStr &v) { longName_ = v; }

  const OptInt &index() const { return index_; }
  void setIndex(const OptInt &v) { index_ = v; }

  const OptInt &direction() const { return direction_; }
  void setDirection(const OptInt &v) { direction_ = v; }

  const PositionProperty &pos() const { return pos_; }
  void setPos(const PositionProperty &v) { pos_ = v; }

  const SizeProperty &size() const { return size_; }
  void setSize(const SizeProperty &v) { size_ = v; }

  const ColorProperty &color() const { return color_; }
  ColorProperty &colorRef() { return color_; }
  void setColor(const ColorProperty &v) { color_ = v; }

  const BezierProperty &path() const { return path_; }
  void setPath(const BezierProperty &v) { path_ = v; }

  //---

  Transform *transform() const { return transform_; }

  Transform *getTransform() {
    if (! transform_)
      transform_ = new Transform;
    return transform_;
  }

  Stroke* stroke() const { return stroke_; }

  Stroke *getStroke() {
    if (! stroke_)
      stroke_ = new Stroke;
    return stroke_;
  }

  Fill* fill() const { return fill_; }

  Fill *getFill() {
    if (! fill_)
      fill_ = new Fill;
    return fill_;
  }

  Group* group() const { return group_; }

  Group *getGroup() {
    if (! group_)
      group_ = new Group;
    return group_;
  }

  Rectangle *rectangle() const { return rectangle_; }

  Rectangle *getRectangle() {
    if (! rectangle_)
      rectangle_ = new Rectangle;
    return rectangle_;
  }

  CLottieRepeater *repeater() const { return repeater_; }

  CLottieRepeater *getRepeater() {
    if (! repeater_)
      repeater_ = new CLottieRepeater(this);
    return repeater_;
  }

  GradientFill *gradientFill() const { return gradientFill_; }

  GradientFill *getGradientFill() {
    if (! gradientFill_)
      gradientFill_ = new GradientFill;
    return gradientFill_;
  }

  GradientStroke *gradientStroke() const { return gradientStroke_; }

  GradientStroke *getGradientStroke() {
    if (! gradientStroke_)
      gradientStroke_ = new GradientStroke;
    return gradientStroke_;
  }

  Trim *trim() const { return trim_; }

  Trim *getTrim() {
    if (! trim_)
      trim_ = new Trim;
    return trim_;
  }

  PolyStar *polyStar() const { return polyStar_; }

  PolyStar *getPolyStar() {
    if (! polyStar_)
      polyStar_ = new PolyStar;
    return polyStar_;
  }

  Merge* merge() const { return merge_; }

  Merge *getMerge() {
    if (! merge_)
      merge_ = new Merge;
    return merge_;
  }

  Rounded* rounded() const { return rounded_; }

  Rounded *getRounded() {
    if (! rounded_)
      rounded_ = new Rounded;
    return rounded_;
  }

  //---

  CLottieVariant *getVariant(const std::string &name) override {
    if      (name == "stroke.lineCap") {
      if (stroke_)
        return new CLottieVariantT<OptInt>(&stroke_->lineCap);
      else
        return nullptr;
    }
    else if (name == "stroke.lineJoin") {
      if (stroke_)
        return new CLottieVariantT<OptInt>(&stroke_->lineJoin);
      else
        return nullptr;
    }
    else if (name == "stroke.miterLimit") {
      if (stroke_)
        return new CLottieVariantT<OptReal>(&stroke_->miterLimit);
      else
        return nullptr;
    }
    else if (name == "fill.fillRule") {
      if (fill_)
        return new CLottieVariantT<OptInt>(&fill_->fillRule);
      else
        return nullptr;
    }
    else if (name == "fill.blendMode") {
      if (fill_)
        return new CLottieVariantT<OptInt>(&fill_->blendMode);
      else
        return nullptr;
    }
    else
      return CLottieObject::getVariant(name);
  }

  //---

  CLottieProperty *getProperty(const std::string &name) const override {
    if      (name == "position") {
      return const_cast<PositionProperty *>(&pos_);
    }
    else if (name == "size") {
      return const_cast<SizeProperty *>(&size_);
    }
    else if (name == "color") {
      return const_cast<ColorProperty *>(&color_);
    }
    else if (name == "path") {
      return const_cast<BezierProperty *>(&path_);
    }
    else if (name == "transform.anchorPoint") {
      if (! transform()) return nullptr;
      return &transform()->anchorPoint;
    }
    else if (name == "transform.position") {
      if (! transform()) return nullptr;
      return &transform()->position;
    }
    else if (name == "transform.rotation") {
      if (! transform()) return nullptr;
      return &transform()->rotation;
    }
    else if (name == "transform.scale") {
      if (! transform()) return nullptr;
      return &transform()->scale;
    }
    else if (name == "transform.opacity") {
      if (! transform()) return nullptr;
      return &transform()->opacity;
    }
    else if (name == "transform.skew") {
      if (! transform()) return nullptr;
      return &transform()->skew;
    }
    else if (name == "transform.skewAxis") {
      if (! transform()) return nullptr;
      return &transform()->skewAxis;
    }
    else if (name == "transform.x_rotation") {
      if (! transform()) return nullptr;
      return &transform()->x_rotation;
    }
    else if (name == "transform.y_rotation") {
      if (! transform()) return nullptr;
      return &transform()->y_rotation;
    }
    else if (name == "transform.z_rotation") {
      if (! transform()) return nullptr;
      return &transform()->z_rotation;
    }
    else if (name == "transform.orientation") {
      if (! transform()) return nullptr;
      return &transform()->orientation;
    }
    else if (name == "fill.color") {
      if (! fill()) return nullptr;
      return &fill()->color;
    }
    else if (name == "fill.opacity") {
      if (! fill()) return nullptr;
      return &fill()->opacity;
    }
    else if (name == "stroke.color") {
      if (! stroke()) return nullptr;
      return &stroke()->color;
    }
    else if (name == "stroke.opacity") {
      if (! stroke()) return nullptr;
      return &stroke()->opacity;
    }
    else if (name == "stroke.width") {
      if (! stroke()) return nullptr;
      return &stroke()->width;
    }
    else if (name == "stroke.miterLimitAnim") {
      if (! stroke()) return nullptr;
      return &stroke()->miterLimitAnim;
    }
    else if (name == "stroke.dash.offset") {
      if (! stroke() || stroke()->dash.offset.empty()) return nullptr;
      return &stroke()->dash.offset[0];
    }
    else if (name == "stroke.dash.dash") {
      if (! stroke() || stroke()->dash.dash.empty()) return nullptr;
      return &stroke()->dash.dash[0];
    }
    else if (name == "stroke.dash.gap") {
      if (! stroke() || stroke()->dash.gap.empty()) return nullptr;
      return &stroke()->dash.gap[0];
    }
    else if (name == "group.color") {
      if (! group()) return nullptr;
      return &group()->color;
    }
    else if (name == "group.opacity") {
      if (! group()) return nullptr;
      return &group()->opacity;
    }
    else if (name == "gradientFill.color") {
      if (! gradientFill()) return nullptr;
      return &gradientFill()->color;
    }
    else if (name == "gradientFill.opacity") {
      if (! gradientFill()) return nullptr;
      return &gradientFill()->opacity;
    }
    else if (name == "gradientFill.startPoint") {
      if (! gradientFill()) return nullptr;
      return &gradientFill()->startPoint;
    }
    else if (name == "gradientFill.endPoint") {
      if (! gradientFill()) return nullptr;
      return &gradientFill()->endPoint;
    }
    else if (name == "gradientFill.highlightLength") {
      if (! gradientFill()) return nullptr;
      return &gradientFill()->highlightLength;
    }
    else if (name == "gradientFill.highlightAngle") {
      if (! gradientFill()) return nullptr;
      return &gradientFill()->highlightAngle;
    }
    else if (name == "gradientStroke.opacity") {
      if (! gradientStroke()) return nullptr;
      return &gradientStroke()->opacity;
    }
    else if (name == "gradientStroke.startPoint") {
      if (! gradientStroke()) return nullptr;
      return &gradientStroke()->startPoint;
    }
    else if (name == "gradientStroke.endPoint") {
      if (! gradientStroke()) return nullptr;
      return &gradientStroke()->endPoint;
    }
    else if (name == "gradientStroke.width") {
      if (! gradientStroke()) return nullptr;
      return &gradientStroke()->width;
    }
    else if (name == "gradientStroke.dash.offset") {
      if (! gradientStroke() || gradientStroke()->dash.offset.empty()) return nullptr;
      return &gradientStroke()->dash.offset[0];
    }
    else if (name == "gradientStroke.dash.dash") {
      if (! gradientStroke() || gradientStroke()->dash.dash.empty()) return nullptr;
      return &gradientStroke()->dash.dash[0];
    }
    else if (name == "gradientStroke.dash.gap") {
      if (! gradientStroke() || gradientStroke()->dash.gap.empty()) return nullptr;
      return &gradientStroke()->dash.gap[0];
    }
    else if (name == "repeater.copies") {
      if (! repeater()) return nullptr;
      return &repeater()->copies;
    }
    else if (name == "repeater.offset") {
      if (! repeater()) return nullptr;
      return &repeater()->offset;
    }
    else if (name == "repeater.startOpacity") {
      if (! repeater()) return nullptr;
      return &repeater()->startOpacity;
    }
    else if (name == "repeater.endOpacity") {
      if (! repeater()) return nullptr;
      return &repeater()->endOpacity;
    }
    else if (name =="repeater.transform.anchorPoint") {
      if (! repeater() || ! repeater()->transform) return nullptr;
      return &repeater()->transform->anchorPoint;
    }
    else if (name == "repeater.transform.position") {
      if (! repeater() || ! repeater()->transform) return nullptr;
      return &repeater()->transform->position;
    }
    else if (name == "repeater.transform.rotation") {
      if (! repeater() || ! repeater()->transform) return nullptr;
      return &repeater()->transform->rotation;
    }
    else if (name == "repeater.transform.scale") {
      if (! repeater() || ! repeater()->transform) return nullptr;
      return &repeater()->transform->scale;
    }
    else if (name == "rectangle.roundness") {
      if (! rectangle()) return nullptr;
      return &rectangle()->roundness;
    }
    else if (name == "trim.start") {
      if (! trim()) return nullptr;
      return &trim()->start;
    }
    else if (name == "trim.end") {
      if (! trim()) return nullptr;
      return &trim()->end;
    }
    else if (name == "trim.offset") {
      if (! trim()) return nullptr;
      return &trim()->offset;
    }
    else if (name == "polystar.position") {
      if (! polyStar()) return nullptr;
      return &polyStar()->position;
    }
    else if (name == "polystar.innerRadius") {
      if (! polyStar()) return nullptr;
      return &polyStar()->innerRadius;
    }
    else if (name == "polystar.innerRoundness") {
      if (! polyStar()) return nullptr;
      return &polyStar()->innerRoundness;
    }
    else if (name == "polystar.outerRadius") {
      if (! polyStar()) return nullptr;
      return &polyStar()->outerRadius;
    }
    else if (name == "polystar.outerRoundness") {
      if (! polyStar()) return nullptr;
      return &polyStar()->outerRoundness;
    }
    else if (name == "polystar.rotation") {
      if (! polyStar()) return nullptr;
      return &polyStar()->rotation;
    }
    else if (name == "polystar.points") {
      if (! polyStar()) return nullptr;
      return &polyStar()->points;
    }
    else if (name == "rounded.roundness") {
      if (! rounded()) return nullptr;
      return &rounded()->roundness;
    }
    else
      return CLottieObject::getProperty(name);
  }

  //---

//CBezierPath getBezierPath() const;

  //---

  void printI(const std::string &prefix, bool hier) const override;

 private:
  OptStr longName_;
  OptInt index_;
  OptInt direction_;

  // transform
  Transform *transform_ { nullptr };

 public:
  ShapeType shapeType_ { ShapeType::NONE };

  // style
  PositionProperty pos_;
  SizeProperty     size_;
  ColorProperty    color_;
  BezierProperty   path_;

 private:
  Stroke* stroke_ { nullptr };
  Fill*   fill_   { nullptr };

  Group*           group_          { nullptr };
  Rectangle*       rectangle_      { nullptr };
  CLottieRepeater* repeater_       { nullptr };
  GradientFill*    gradientFill_   { nullptr };
  GradientStroke*  gradientStroke_ { nullptr };
  Trim*            trim_           { nullptr };
  PolyStar*        polyStar_       { nullptr };
  Merge*           merge_          { nullptr };
  Rounded*         rounded_        { nullptr };

  //---

  Shapes shapes_;
};

#endif
