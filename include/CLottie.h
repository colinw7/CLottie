#ifndef CLottie_H
#define CLottie_H

#include <CJson.h>
#include <CMathUtil.h>
#include <CMathRound.h>
#include <CMatrix2D.h>
#include <CBBox2D.h>
#include <CPoint2D.h>
#include <CRGBA.h>

#include <string>

namespace CLottieUtil {

template<typename T>
T mapValue(double t, const T &v1, const T &v2);

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

  bool isSet() const { return ! xvals.empty() && ! yvals.empty(); }

  CPoint2D toPoint(const CPoint2D &def) const {
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
struct ValArray {
  ValArray() { }

  ValArray(const std::vector<T> &a) : vals(a) { }

  bool isSet() const { return ! vals.empty(); }

  friend std::ostream &operator<<(std::ostream &os, const ValArray &l) {
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

using RValArray = ValArray<double>;

template<typename T>
struct KeyFrameT {
  using OptPoint = std::optional<CPoint2D>;
  using OptReal  = std::optional<double>;

  std::vector<XYVals> ivalues;
  std::vector<XYVals> ovalues;

  ValArray<T> startValue;
  ValArray<T> endValue;

  std::vector<std::string> interpolation;

  bool hold { false };

  OptPoint tangentIn;
  OptPoint tangentOut;

  OptReal timeFrame;

  void print(const std::string &prefix="") const {
    auto printValue = [&](const std::string &n, auto value) {
      std::cout << prefix << n << "=" << value << "\n";
    };

    auto optPrintValue = [&](const std::string &n, const auto &value) {
      if (value)
        std::cout << prefix << n << "=" << *value << "\n";
    };

    if (! ivalues.empty()) {
      printValue("ivalues", "");

      for (auto &v : ivalues) {
        std::cout << prefix << "  " << v << "\n";
      }
    }

    if (! ovalues.empty()) {
      printValue("ovalues", "");

      for (auto &v : ovalues) {
        std::cout << prefix << "  " << v << "\n";
      }
    }

    if (! interpolation.empty()) {
      printValue("interpolation", "");

      for (auto &v : interpolation) {
        std::cout << prefix << "  " << v << "\n";
      }
    }

    if (startValue.isSet())
      printValue("startValue", startValue);

    if (endValue.isSet())
      printValue("endValue", endValue);

    optPrintValue("tangentIn" , tangentIn );
    optPrintValue("tangentOut", tangentOut);

    optPrintValue("timeFrame", timeFrame);
  }
};

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

  auto n = std::min(v.vals.size(), v.vals.size());

  for (uint i = 0; i < n; ++i)
    v.vals.push_back(CMathUtil::map(t, 0.0, 1.0, v1.vals[i], v2.vals[i]));

  return v;
}

//---

using Points = std::vector<CPoint2D>;

struct PointList {
  PointList() { }

  PointList(const Points &p) : points(p) { }

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
  double frameRate  { 0.0 };
  double frameStart { 0.0 };
  double frameStop  { 0.0 };
  double secs       { 0.0 };
  uint   frame      { 0 };
};

}

//---

struct CLottieRoot;
struct CLottieLayer;
struct CLottieMarker;
struct CLottieAsset;
struct CLottieShape;
struct CLottieEffect;
struct CLottieEffectValue;

//---

class CLottie {
 public:
  using TimeFrame = CLottieUtil::TimeFrame;

  using Points   = std::vector<CPoint2D>;
  using Colors   = std::vector<CRGBA>;
  using OptBool  = std::optional<bool>;
  using OptInt   = std::optional<int>;
  using OptReal  = std::optional<double>;
  using OptStr   = std::optional<std::string>;
  using OptPoint = std::optional<CPoint2D>;
  using OptColor = std::optional<CRGBA>;

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

  struct Property {
    OptBool animated;
    OptInt  index;
    OptStr  expression;

    void print(const std::string &prefix="") const {
      auto optPrintValue = [&](const std::string &n, const auto &value) {
        if (value)
          std::cout << prefix << n << "=" << *value << "\n";
      };

      optPrintValue("animated", animated);

      optPrintValue("index", index);

      optPrintValue("expression", expression);
    }
  };

  using XYVals = CLottieUtil::XYVals;

  template<typename T>
  struct PropertyT : Property {
    using KeyFrame = CLottieUtil::KeyFrameT<T>;
    using OptVal   = std::optional<T>;

    std::vector<T>        values;
    std::vector<KeyFrame> keyFrames;

    bool isSet() const {
      return (! values.empty() || ! keyFrames.empty());
    }

    bool isTSet() const {
      return ! keyFrames.empty();
    }

    void print(const std::string &prefix="") const {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

#if 0
      auto optPrintValue = [&](const std::string &n, const auto &value) {
        if (value)
          std::cout << prefix << n << "=" << *value << "\n";
      };
#endif

      Property::print(prefix);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! keyFrames.empty()) {
        printValue("KeyFrames", "");

        int i = 0;

        for (const auto &kf : keyFrames) {
          std::cout << prefix << " Frame [" << i << "]\n";

          kf.print(prefix + "  ");

          ++i;
        }
      }
    }

    OptVal value(const OptVal &def) const {
      if (values.empty())
        return def;

      return values[0];
    }

#if 0
    std::vector<T> values() const {
      return values;
    }
#endif

    OptVal tvalue(const TimeFrame &frame, const OptVal &def=OptVal()) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return def;

#if 0
        auto t1 = int(CMathRound::RoundDown(frame.secs) % keyFrames.size());
        auto t2 = (t1 + 1) % keyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        if (keyFrames[t1].startValue.isSet() && keyFrames[t2].startValue.isSet()) {
          auto v1 = keyFrames[t1].startValue.vals[0];
          auto v2 = keyFrames[t2].startValue.vals[0];

          return CLottieUtil::mapValue(dt, v1, v2);
        }
        else
          return def;
#else
        return keyFrameValue(frame, def);
#endif
      }
      else
        return value(def);
    }

#if 0
    std::vector<T> tvalues(const TimeFrame &frame) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return values();

        return keyFrameValues(frame);
      }
      else
        return values();
    }
#endif

    OptVal keyFrameValue(const TimeFrame &frame, const OptVal &def) const {
      if (keyFrames.empty())
        return def;

      if (keyFrames.size() == 1) {
        auto v1 = keyFrames[0].startValue.vals[0];
        auto v2 = keyFrames[0].endValue  .vals[0];

        auto dt = CMathUtil::map(frame.frame, frame.frameStart, frame.frameStop, 0.0, 1.0);

        return CLottieUtil::mapValue(dt, v1, v2);
      }

      for (size_t i = 0; i < keyFrames.size(); ++i) {
        auto frameStart1 = keyFrames[i].timeFrame.value_or(0.0);
        auto frameStop1  = frame.frameStop;

        if (i < keyFrames.size() - 1)
          frameStop1 = keyFrames[i + 1].timeFrame.value_or(0.0);

        if (frame.frame >= frameStart1 && frame.frame < frameStop1) {
          if (keyFrames[i].startValue.vals.empty() || keyFrames[i].endValue.vals.empty()) {
            if (i > 0 && i == keyFrames.size() - 1)
              --i;
          }

          if (keyFrames[i].startValue.vals.empty() && keyFrames[i].endValue.vals.empty())
            return def;

          T v1 { }, v2 { };

          if (! keyFrames[i].startValue.vals.empty())
            v1 = keyFrames[i].startValue.vals[0];

          if (! keyFrames[i].endValue.vals.empty())
            v2 = keyFrames[i].endValue  .vals[0];

          auto dt = CMathUtil::map(frame.frame, frameStart1, frameStop1, 0.0, 1.0);

          return CLottieUtil::mapValue(dt, v1, v2);
        }
      }

      return def;
    }

    void setValue(const T &v) {
      if (values.empty())
        values.push_back(v);
      else
        values[0] = v;
    }
  };

  struct SplitPositionProperty : Property {
    using KeyFrame       = CLottieUtil::KeyFrameT<CPoint2D>;
    using ScalarProperty = PropertyT<double>;

    std::vector<CPoint2D> values;
    std::vector<KeyFrame> keyFrames;

    OptBool        split;
    ScalarProperty x;
    ScalarProperty y;

    bool isSet() const {
      return (! values.empty() || ! keyFrames.empty());
    }

    bool isTSet() const {
      return ! keyFrames.empty();
    }

    void print(const std::string &prefix="") const {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      auto optPrintValue = [&](const std::string &n, const auto &value) {
        if (value)
          std::cout << prefix << n << "=" << *value << "\n";
      };

      optPrintValue("animated", animated);

      optPrintValue("index", index);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! keyFrames.empty()) {
        printValue("KeyFrames", "");

        int i = 0;

        for (const auto &kf : keyFrames) {
          std::cout << prefix << " Frame [" << i << "]\n";

          kf.print(prefix + "  ");

          ++i;
        }
      }
    }

    const CPoint2D &value(const CPoint2D &def) const {
      if (values.empty())
        return def;

      return values[0];
    }

    CPoint2D tvalue(const TimeFrame &frame, const CPoint2D &def) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return def;

#if 0
        auto t1 = int(CMathRound::RoundDown(frame.secs) % keyFrames.size());
        auto t2 = (t1 + 1) % keyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        if (keyFrames[t1].startValue.isSet() && keyFrames[t2].startValue.isSet()) {
          auto v1 = keyFrames[t1].startValue.vals[0];
          auto v2 = keyFrames[t2].startValue.vals[0];

          return CLottieUtil::mapValue(dt, v1, v2);
        }
        else
          return def;
#else
        return keyFrameValue(frame, def);
#endif
      }
      else
        return value(def);
    }

    CPoint2D keyFrameValue(const TimeFrame &frame, const CPoint2D &def) const {
      if (keyFrames.empty())
        return def;

      if (keyFrames.size() == 1) {
        auto v1 = keyFrames[0].startValue.vals[0];
        auto v2 = keyFrames[0].endValue  .vals[0];

        auto dt = CMathUtil::map(frame.frame, frame.frameStart, frame.frameStop, 0.0, 1.0);

        return CLottieUtil::mapValue(dt, v1, v2);
      }

      for (size_t i = 0; i < keyFrames.size(); ++i) {
        auto frameStart1 = keyFrames[i].timeFrame.value_or(0.0);
        auto frameStop1  = frame.frameStop;

        if (i < keyFrames.size() - 1)
          frameStop1 = keyFrames[i + 1].timeFrame.value_or(0.0);

        if (frame.frame >= frameStart1 && frame.frame < frameStop1) {
          if (keyFrames[i].startValue.vals.empty() || keyFrames[i].endValue.vals.empty()) {
            if (i > 0 && i == keyFrames.size() - 1)
              --i;
          }

          if (! keyFrames[i].startValue.vals.empty() && ! keyFrames[i].endValue.vals.empty()) {
            auto v1 = keyFrames[i].startValue.vals[0];
            auto v2 = keyFrames[i].endValue  .vals[0];

            auto dt = CMathUtil::map(frame.frame, frameStart1, frameStop1, 0.0, 1.0);

            return CLottieUtil::mapValue(dt, v1, v2);
          }
          else
            return def;
        }
      }

      return def;
    }
  };

  struct VectorProperty : Property {
    using KeyFrame = CLottieUtil::KeyFrameT<CPoint2D>;

    std::vector<CPoint2D> values;
    std::vector<KeyFrame> keyFrames;

    bool isSet() const {
      return (! values.empty() || ! keyFrames.empty());
    }

    bool isTSet() const {
      return ! keyFrames.empty();
    }

    void print(const std::string &prefix="") const {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      auto optPrintValue = [&](const std::string &n, const auto &value) {
        if (value)
          std::cout << prefix << n << "=" << *value << "\n";
      };

      optPrintValue("animated", animated);

      optPrintValue("index", index);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! keyFrames.empty()) {
        printValue("KeyFrames", "");

        int i = 0;

        for (const auto &kf : keyFrames) {
          std::cout << prefix << " Frame [" << i << "]\n";

          kf.print(prefix + "  ");

          ++i;
        }
      }
    }

    const CPoint2D &value(const CPoint2D &def) const {
      if (values.empty())
        return def;

      return values[0];
    }

    CPoint2D tvalue(const TimeFrame &frame, const CPoint2D &def) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return def;

#if 0
        auto t1 = int(CMathRound::RoundDown(frame.secs) % keyFrames.size());
        auto t2 = (t1 + 1) % keyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        if (keyFrames[t1].startValue.isSet() && keyFrames[t2].startValue.isSet()) {
          auto v1 = keyFrames[t1].startValue.vals[0];
          auto v2 = keyFrames[t2].startValue.vals[0];

          return CLottieUtil::mapValue(dt, v1, v2);
        }
        else
          return def;
#else
        return keyFrameValue(frame, def);
#endif
      }
      else
        return value(def);
    }

    CPoint2D keyFrameValue(const TimeFrame &frame, const CPoint2D &def) const {
      if (keyFrames.empty())
        return def;

      if (keyFrames.size() == 1) {
        auto v1 = keyFrames[0].startValue.vals[0];
        auto v2 = keyFrames[0].endValue  .vals[0];

        auto dt = CMathUtil::map(frame.frame, frame.frameStart, frame.frameStop, 0.0, 1.0);

        return CLottieUtil::mapValue(dt, v1, v2);
      }

      for (size_t i = 0; i < keyFrames.size(); ++i) {
        auto frameStart1 = keyFrames[i].timeFrame.value_or(0.0);
        auto frameStop1  = frame.frameStop;

        if (i < keyFrames.size() - 1)
          frameStop1 = keyFrames[i + 1].timeFrame.value_or(0.0);

        if (frame.frame >= frameStart1 && frame.frame < frameStop1) {
          if (keyFrames[i].startValue.vals.empty() || keyFrames[i].endValue.vals.empty()) {
            if (i > 0 && i == keyFrames.size() - 1)
              --i;
          }

          if (! keyFrames[i].startValue.vals.empty() && ! keyFrames[i].endValue.vals.empty()) {
            auto v1 = keyFrames[i].startValue.vals[0];
            auto v2 = keyFrames[i].endValue  .vals[0];

            auto dt = CMathUtil::map(frame.frame, frameStart1, frameStop1, 0.0, 1.0);

            return CLottieUtil::mapValue(dt, v1, v2);
          }
          else
            return def;
        }
      }

      return def;
    }
  };

  struct PositionProperty : Property {
    using KeyFrame = CLottieUtil::KeyFrameT<XYVals>;

    std::vector<XYVals>   values;
    std::vector<KeyFrame> keyFrames;

    bool isSet() const {
      return (! values.empty() || ! keyFrames.empty());
    }

    bool isTSet() const {
      return ! keyFrames.empty();
    }

    void print(const std::string &prefix="") const {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      auto optPrintValue = [&](const std::string &n, const auto &value) {
        if (value)
          std::cout << prefix << n << "=" << *value << "\n";
      };

      optPrintValue("animated", animated);

      optPrintValue("index", index);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! keyFrames.empty()) {
        printValue("KeyFrames", "");

        int i = 0;

        for (const auto &kf : keyFrames) {
          std::cout << prefix << " Frame [" << i << "]\n";

          kf.print(prefix + "  ");

          ++i;
        }
      }
    }

    XYVals value(const XYVals &def) const {
      if (values.empty())
        return def;

      return values[0];
    }

    XYVals tvalue(const TimeFrame &frame, const XYVals &def) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return def;

#if 0
        auto t1 = int(CMathRound::RoundDown(frame.secs) % keyFrames.size());
        auto t2 = (t1 + 1) % keyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        if (keyFrames[t1].startValue.isSet() && keyFrames[t2].startValue.isSet()) {
          auto v1 = keyFrames[t1].startValue.vals[0];
          auto v2 = keyFrames[t2].startValue.vals[0];

          return CLottieUtil::mapValue(dt, v1, v2);
        }
        else
          return def;
#else
        return keyFrameValue(frame, def);
#endif
      }
      else
        return value(def);
    }

    XYVals keyFrameValue(const TimeFrame &frame, const XYVals &def) const {
      if (keyFrames.empty())
        return def;

      if (keyFrames.size() == 1) {
        auto v1 = keyFrames[0].startValue.vals[0];
        auto v2 = keyFrames[0].endValue  .vals[0];

        auto dt = CMathUtil::map(frame.frame, frame.frameStart, frame.frameStop, 0.0, 1.0);

        return CLottieUtil::mapValue(dt, v1, v2);
      }

      for (size_t i = 0; i < keyFrames.size(); ++i) {
        auto frameStart1 = keyFrames[i].timeFrame.value_or(0.0);
        auto frameStop1  = frame.frameStop;

        if (i < keyFrames.size() - 1)
          frameStop1 = keyFrames[i + 1].timeFrame.value_or(0.0);

        if (frame.frame >= frameStart1 && frame.frame < frameStop1) {
          if (keyFrames[i].startValue.vals.empty() || keyFrames[i].endValue.vals.empty()) {
            if (i > 0 && i == keyFrames.size() - 1)
              --i;
          }

          if (keyFrames[i].startValue.vals.empty() && keyFrames[i].endValue.vals.empty())
            return def;

          XYVals v1, v2;

          if (! keyFrames[i].startValue.vals.empty())
            v1 = keyFrames[i].startValue.vals[0];

          if (! keyFrames[i].endValue.vals.empty())
            v2 = keyFrames[i].endValue.vals[0];

          auto dt = CMathUtil::map(frame.frame, frameStart1, frameStop1, 0.0, 1.0);

          return CLottieUtil::mapValue(dt, v1, v2);
        }
      }

      return def;
    }
  };

  struct SizeProperty : Property {
    using KeyFrame = CLottieUtil::KeyFrameT<XYVals>;

    std::vector<XYVals>   values;
    std::vector<KeyFrame> keyFrames;

    bool isSet() const {
      return (! values.empty() || ! keyFrames.empty());
    }

    bool isTSet() const {
      return ! keyFrames.empty();
    }

    void print(const std::string &prefix="") const {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      auto optPrintValue = [&](const std::string &n, const auto &value) {
        if (value)
          std::cout << prefix << n << "=" << *value << "\n";
      };

      optPrintValue("animated", animated);

      optPrintValue("index", index);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! keyFrames.empty()) {
        printValue("KeyFrames", "");

        int i = 0;

        for (const auto &kf : keyFrames) {
          std::cout << prefix << " Frame [" << i << "]\n";

          kf.print(prefix + "  ");

          ++i;
        }
      }
    }

    XYVals value(const XYVals &def) const {
      if (values.empty())
        return def;

      return values[0];
    }

    XYVals tvalue(const TimeFrame &frame, const XYVals &def) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return def;

#if 0
        auto t1 = int(CMathRound::RoundDown(frame.secs) % keyFrames.size());
        auto t2 = (t1 + 1) % keyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        if (keyFrames[t1].startValue.isSet() && keyFrames[t2].startValue.isSet()) {
          auto v1 = keyFrames[t1].startValue.vals[0];
          auto v2 = keyFrames[t2].startValue.vals[0];

          return CLottieUtil::mapValue(dt, v1, v2);
        }
        else
          return def;
#else
        return keyFrameValue(frame, def);
#endif
      }
      else
        return value(def);
    }

    XYVals keyFrameValue(const TimeFrame &frame, const XYVals &def) const {
      if (keyFrames.empty())
        return def;

      if (keyFrames.size() == 1) {
        auto v1 = keyFrames[0].startValue.vals[0];
        auto v2 = keyFrames[0].endValue  .vals[0];

        auto dt = CMathUtil::map(frame.frame, frame.frameStart, frame.frameStop, 0.0, 1.0);

        return CLottieUtil::mapValue(dt, v1, v2);
      }

      for (size_t i = 0; i < keyFrames.size(); ++i) {
        auto frameStart1 = keyFrames[i].timeFrame.value_or(0.0);
        auto frameStop1  = frame.frameStop;

        if (i < keyFrames.size() - 1)
          frameStop1 = keyFrames[i + 1].timeFrame.value_or(0.0);

        if (frame.frame >= frameStart1 && frame.frame < frameStop1) {
          if (keyFrames[i].startValue.vals.empty() || keyFrames[i].endValue.vals.empty()) {
            if (i > 0 && i == keyFrames.size() - 1)
              --i;
          }

          if (keyFrames[i].startValue.vals.empty() && keyFrames[i].endValue.vals.empty())
            return def;

          XYVals v1, v2;

          if (! keyFrames[i].startValue.vals.empty())
            v1 = keyFrames[i].startValue.vals[0];

          if (! keyFrames[i].endValue.vals.empty())
            v2 = keyFrames[i].endValue.vals[0];

          auto dt = CMathUtil::map(frame.frame, frameStart1, frameStop1, 0.0, 1.0);

          return CLottieUtil::mapValue(dt, v1, v2);
        }
      }

      return def;
    }
  };

  using PointList = CLottieUtil::PointList;

  struct BezierProperty : Property {
    struct KeyFrame {
      std::vector<CPoint2D> ipoints;
      std::vector<CPoint2D> opoints;
      std::vector<CPoint2D> vpoints;
      bool                  closed { false };

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
    };

    bool                     closed { false };
    std::vector<PointList>   values;
    std::vector<PointList>   ivalues;
    std::vector<PointList>   ovalues;
    std::vector<PointList>   vvalues;
    std::vector<std::string> interpolation;
    OptReal                  timeFrame;
    std::vector<KeyFrame>    ikeyFrames;
    std::vector<KeyFrame>    ekeyFrames;

    void print(const std::string &prefix="") const {
      auto printValue = [&](const std::string &n, auto value) {
        std::cout << prefix << n << "=" << value << "\n";
      };

      auto optPrintValue = [&](const std::string &n, const auto &value) {
        if (value)
          std::cout << prefix << n << "=" << *value << "\n";
      };

      optPrintValue("animated", animated);

      optPrintValue("index", index);

      printValue("closed", closed);

      if (! values.empty()) {
        printValue("values", "");

        for (auto &v : values) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! ivalues.empty()) {
        printValue("ivalues", "");

        for (auto &v : ivalues) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! ovalues.empty()) {
        printValue("ovalues", "");

        for (auto &v : ovalues) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! vvalues.empty()) {
        printValue("vvalues", "");

        for (auto &v : vvalues) {
          std::cout << prefix << "  " << v << "\n";
        }
      }

      if (! ikeyFrames.empty()) {
        printValue("ikeyFrames", "");

        for (auto &k : ikeyFrames)
          k.print(prefix + "  ");
      }

      if (! ekeyFrames.empty()) {
        printValue("ikeyFrames", "");

        for (auto &k : ekeyFrames)
          k.print(prefix + "  ");
      }
    }

    bool isSet() const {
      return (! vvalues.empty() || ! ivalues.empty());
    }

    bool isTSet() const {
      return (! ikeyFrames.empty() && ! ekeyFrames.empty());
    }

    bool isISet() const { return ! ivalues.empty(); }
    bool isOSet() const { return ! ovalues.empty(); }
    bool isVSet() const { return ! vvalues.empty(); }

    const PointList &ivalue(const PointList &def) const {
      if (! isISet())
        return def;

      return ivalues[0];
    }

    const PointList &ovalue(const PointList &def) const {
      if (! isOSet())
        return def;

      return ovalues[0];
    }

    const PointList &vvalue(const PointList &def) const {
      if (! isVSet())
        return def;

      return vvalues[0];
    }

    PointList tivalue(const TimeFrame &frame, const PointList &def) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return ivalue(def);

        auto t1 = int(CMathRound::RoundDown(frame.secs) % ikeyFrames.size());
        auto t2 = (t1 + 1) % ekeyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        const auto &v1 = ikeyFrames[t1].ipoints;
        const auto &v2 = ekeyFrames[t2].ipoints;

        return CLottieUtil::mapValue(dt, v1, v2);
      }
      else
        return ivalue(def);
    }

    PointList tovalue(const TimeFrame &frame, const PointList &def) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return ovalue(def);

        auto t1 = int(CMathRound::RoundDown(frame.secs) % ikeyFrames.size());
        auto t2 = (t1 + 1) % ekeyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        const auto &v1 = ikeyFrames[t1].opoints;
        const auto &v2 = ekeyFrames[t2].opoints;

        return CLottieUtil::mapValue(dt, v1, v2);
      }
      else
        return ovalue(def);
    }

    PointList tvvalue(const TimeFrame &frame, const PointList &def) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return vvalue(def);

#if 1
        auto t1 = int(CMathRound::RoundDown(frame.secs) % ikeyFrames.size());
        auto t2 = (t1 + 1) % ekeyFrames.size();
        auto dt = frame.secs - CMathRound::RoundDown(frame.secs);

        const auto &v1 = ikeyFrames[t1].vpoints;
        const auto &v2 = ekeyFrames[t2].vpoints;

        return CLottieUtil::mapValue(dt, v1, v2);
#else
        return keyFrameValue(frame, def);
#endif
      }
      else
        return vvalue(def);
    }

    bool tclosed(const TimeFrame &frame) const {
      if (animated.value_or(false)) {
        if (! isTSet())
          return closed;

#if 1
        auto t1 = int(CMathRound::RoundDown(frame.secs) % ikeyFrames.size());

        auto v1 = ikeyFrames[t1].closed;

        return v1;
#else
        return keyFrameClosed(frame, def);
#endif
      }
      else
        return closed;
    }

#if 0
    PointList keyFrameValue(const TimeFrame &frame, const PointList &def) const {
      if (keyFrames.empty())
        return def;

      if (keyFrames.size() == 1) {
        auto v1 = keyFrames[0].startValue.vals[0];
        auto v2 = keyFrames[0].endValue  .vals[0];

        auto dt = CMathUtil::map(frame.frame, frame.frameStart, frame.frameStop, 0.0, 1.0);

        return CLottieUtil::mapValue(dt, v1, v2);
      }

      for (size_t i = 0; i < keyFrames.size(); ++i) {
        auto frameStart1 = keyFrames[i].timeFrame.value_or(0.0);
        auto frameStop1  = frame.frameStop;

        if (i < keyFrames.size() - 1)
          frameStop1 = keyFrames[i + 1].timeFrame.value_or(0.0);

        if (frame.frame >= frameStart1 && frame.frame < frameStop1) {
          if (keyFrames[i].startValue.vals.empty() || keyFrames[i].endValue.vals.empty()) {
            if (i > 0 && i == keyFrames.size() - 1)
              --i;
          }

          if (! keyFrames[i].startValue.vals.empty() && ! keyFrames[i].endValue.vals.empty()) {
            auto v1 = keyFrames[i].startValue.vals[0];
            auto v2 = keyFrames[i].endValue  .vals[0];

            auto dt = CMathUtil::map(frame.frame, frameStart1, frameStop1, 0.0, 1.0);

            return CLottieUtil::mapValue(dt, v1, v2);
          }
          else
            return def;
        }
      }

      return def;
    }
#endif
  };

  using ColorProperty  = PropertyT<CRGBA>;
  using ScalarProperty = PropertyT<double>;
  using ArrayProperty  = PropertyT<CLottieUtil::RValArray>;

  struct Transform {
    PositionProperty      anchorPoint;
    SplitPositionProperty position;
    ScalarProperty        rotation;
    VectorProperty        scale;
    ScalarProperty        opacity;
    ScalarProperty        skew;
    ScalarProperty        skewAxis;
    ScalarProperty        x_rotation;
    ScalarProperty        y_rotation;
    ScalarProperty        z_rotation;
    VectorProperty        orientation;

    void* repeater { nullptr };

    void print(const std::string &prefix="") const;
  };

 public:
  CLottie();
 ~CLottie();

  bool isDebug() const { return debug_; }
  void setDebug(bool b) { debug_ = b; }

  bool isPrint() const { return print_; }
  void setPrint(bool b) { print_ = b; }

  bool load(const std::string &file);

  CLottieRoot *root() const { return root_; }

  CLottieAsset *getAssetById(const std::string &id) const;

  CLottieLayer *getLayerById(int id) const;

  void deselectAll();

  CMatrix2D getTransformMatrix(const TimeFrame &timeFrame, CLottie::Transform *transform) const;

 private:
  bool readRoot(const std::string &msg, const CJson::ValueP &value);

  bool readLayer (const std::string &msg, const CJson::ValueP &ivalue, CLottieLayer *layer);
  bool readMarker(const std::string &msg, const CJson::ValueP &ivalue, CLottieMarker *marker);
  bool readShape (const std::string &msg, const CJson::ValueP &ivalue, CLottieShape *shape);
  bool readAsset (const std::string &msg, const CJson::ValueP &ivalue, CLottieAsset *asset);

  bool readLayerMaskProperties(const std::string &msg, const CJson::ValueP &iValue,
                               CLottieLayer *layer);
  bool readLayerShapes(const std::string &msg, const CJson::ValueP &iValue, CLottieLayer *layer);

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

  bool readEffect(const std::string &msg, const CJson::ValueP &iValue,
                  CLottieEffect *effect) const;
  bool readEffectValue(const std::string &msg, const CJson::ValueP &iValue,
                       CLottieEffectValue *effectValue) const;

  bool getKeyFrameValues(const std::string &xyMsg, const std::string &name, CJson::ValueP &xyValue,
                         std::vector<XYVals> &xyValues) const;

  bool readPointList(const std::string &msg, const CJson::ValueP &iValue,
                     std::vector<CPoint2D> &points) const;

  bool readVector(const std::string &msg, const CJson::ValueP &iValue, CPoint2D &p) const;
  bool readColor (const std::string &msg, const CJson::ValueP &iValue, CRGBA &c) const;

  bool readStrings(const std::string &msg, const CJson::ValueP &iValue,
                   std::vector<std::string> &strs) const;
  bool readNumbers(const std::string &msg, const CJson::ValueP &iValue,
                   std::vector<double> &numbers) const;

  void addLayer (CLottieLayer  *layer);
  void addMarker(CLottieMarker *marker);
  void addShape (CLottieShape  *shape);

  std::string valueToString(const CJson::ValueP &value, const std::string &def="") const;
  double      valueToReal  (const CJson::ValueP &value, double def=0.0) const;
  int         valueToInt   (const CJson::ValueP &value, int def=0) const;
  bool        valueToBool  (const CJson::ValueP &value, bool def=false) const;

 private:
  bool debug_ { false };
  bool print_ { false };

  CLottieRoot* root_ { nullptr };

  std::map<std::string, CLottieAsset *> assetIds_;
  std::map<int        , CLottieLayer *> layerIds_;
  std::map<int        , CLottieShape *> shapeIds_;

  std::vector<CLottieLayer *>  layers_;
  std::vector<CLottieMarker *> markers_;
  std::vector<CLottieShape *>  shapes_;
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
    EFFECT
  };

 public:
  static std::string typeName(const Type &type) {
    switch (type) {
      case Type::ROOT  : return "Root";
      case Type::LAYER : return "Layer";
      case Type::SHAPE : return "Shape";
      case Type::ASSET : return "Asset";
      case Type::MARKER: return "Marker";
      case Type::EFFECT: return "Effect";
      default:           return "None";
    }
  };

  CLottieObject(CLottie *l, const Type &t);

  virtual ~CLottieObject();

  //---

  const Type &objectType() const { return objectType_; }

  const OptStr &name() const { return name_; }
  void setName(const OptStr &s) { name_ = s; }

  const std::string &type() const { return type_; }
  void setType(const std::string &s) { type_ = s; }

  const OptInt &typeId() const { return typeId_; }
  void setTypeId(const OptInt &v) { typeId_ = v; }

  bool selected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  const OptBool &hidden() const { return hidden_; }
  void setHidden(const OptBool &v) { hidden_ = v; }

  const OptInt &ind() const { return ind_; }
  void setInd(const OptInt &v) { ind_ = v; }

  CLottieObject *parent() const { return parent_; }
  void setParent(CLottieObject *v) { parent_ = v; }

  //---

  const CBBox2D &bbox() const { return bbox_; }
  void setBBox(const CBBox2D &v) { bbox_ = v; }

  //---

  virtual CLottieRoot *getRoot() const = 0;

  virtual CMatrix2D calcTransform(const TimeFrame &) const = 0;

  virtual CMatrix2D calcHierTransform(const TimeFrame &timeFrame) const {
    return calcTransform(timeFrame);
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
  std::string    type_;
  OptInt         typeId_;
  bool           selected_ { false };
  OptBool        hidden_;
  OptInt         ind_;
  CLottieObject* parent_ { nullptr };

  CBBox2D bbox_;
};

//---

class CLottieRoot : public CLottieObject {
 public:
  CLottieRoot(CLottie *l);
 ~CLottieRoot() override;

  //---

  const std::string &version() const { return version_; }
  void setVersion(const std::string &s) { version_ = s; }

  const OptStr &matchName() const { return matchName_; }
  void setMatchName(const OptStr &v) { matchName_ = v; }

  //---

  double frameRate() const { return timeFrame_.frameRate; }
  void setFrameRate(double r) { timeFrame_.frameRate = r; }

  double frameStart() const { return timeFrame_.frameStart; }
  void setFrameStart(double r) { timeFrame_.frameStart = r; }

  double frameStop() const { return timeFrame_.frameStop; }
  void setFrameStop(double r) { timeFrame_.frameStop = r; }

  //---

  const OptReal &width() const { return width_; }
  void setWidth(const OptReal &v) { width_ = v; }

  const OptReal &height() const { return height_; }
  void setHeight(const OptReal &v) { height_ = v; }

  const OptBool &threeD() const { return threeD_; }
  void setThreeD(const OptBool &v) { threeD_ = v; }

  //---

  const std::vector<CLottieLayer *> &layers() const { return layers_; }
  const std::vector<CLottieAsset *> &assets() const { return assets_; }

  void addLayer(CLottieLayer *layer) { layers_.push_back(layer); }
  void addAsset(CLottieAsset *asset) { assets_.push_back(asset); }

  //---

  CLottieRoot *getRoot() const override { return const_cast<CLottieRoot *>(this); }

  CMatrix2D calcTransform(const TimeFrame &) const override { return CMatrix2D::identity(); }

  void printI(const std::string &prefix, bool hier) const override;

 private:
  std::string version_;

  OptStr matchName_;

  TimeFrame timeFrame_;

  OptReal width_;
  OptReal height_;

  OptBool threeD_;

  std::vector<CLottieAsset *> assets_;
  std::vector<CLottieLayer *> layers_;
};

//---

class CLottieAsset : public CLottieObject {
 public:
  CLottieAsset(CLottie *l);
 ~CLottieAsset() override;

  //---

  const std::string &id() const { return id_; }
  void setId(const std::string &s) { id_ = s; }

  const OptStr &css() const { return css_; }
  void setCss(const OptStr &v) { css_ = v; }

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

  const std::vector<CLottieLayer *> &layers() const { return layers_; }

  void addLayer(CLottieLayer *layer) { layers_.push_back(layer); }

  //---

  CLottieRoot *getRoot() const override;

  CMatrix2D calcTransform(const TimeFrame &) const override { return CMatrix2D::identity(); }

  void printI(const std::string &prefix, bool hier) const override;

 private:
  std::string id_;

  OptStr css_;

  OptReal width_;
  OptReal height_;

  OptStr  dir_;
  OptStr  path_;
  OptBool embedded_;

  std::vector<CLottieLayer *> layers_;
};

//---

class CLottieEffect : public CLottieObject {
 public:
  CLottieEffect(CLottie *lottie);
 ~CLottieEffect();

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

  CMatrix2D calcTransform(const TimeFrame &) const override { return CMatrix2D::identity(); }

  CLottieLayer *getLayer() const;

  void addValue(CLottieEffectValue *value) {
    children_.push_back(value);
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

  std::vector<CLottieEffectValue *> children_;
};

struct CLottieEffectValue {
  using OptInt = std::optional<int>;
  using OptStr = std::optional<std::string>;

  using ScalarProperty = CLottie::ScalarProperty;
  using ColorProperty  = CLottie::ColorProperty;
  using VectorProperty = CLottie::VectorProperty;

  OptInt type;
  OptStr name;
  OptStr match;
  OptInt index;

  OptInt         ivalue;
  ScalarProperty scalar;
  ColorProperty  color;
  VectorProperty point;

  CLottieEffect* parent { nullptr };

  void print(const std::string &prefix="") const;
};

class CLottieLayer : public CLottieObject {
 public:
  struct Mask {
    OptStr         mode;
    ScalarProperty opacity;
    BezierProperty path;
    ScalarProperty expand;
    OptBool        inverted;
    OptStr         name;

    void print(const std::string &prefix="") const;
  };

  using Effect = CLottieEffect;

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

  const OptStr &css() const { return css_; }
  void setCss(const OptStr &v) { css_ = v; }

  const OptInt &frameIn() const { return frameIn_; }
  void setFrameIn(const OptInt &v) { frameIn_ = v; }

  const OptInt &frameOut() const { return frameOut_; }
  void setFrameOut(const OptInt &v) { frameOut_ = v; }

  const OptReal &startTime() const { return startTime_; }
  void setStartTime(const OptReal &v) { startTime_ = v; }

  const OptReal &timeStretch() const { return timeStretch_; }
  void setTimeStretch(const OptReal &v) { timeStretch_ = v; }

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

  Effect *effect() const { return effect_; }

  Effect *getEffect() {
    if (! effect_) {
      effect_ = new Effect(lottie_);

      effect_->setParent(this);
    }

    return effect_;
  }

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

  const std::vector<CLottieShape *> &shapes() const { return shapes_; }

  void addShape(CLottieShape *shape) { shapes_.push_back(shape); }

  //---

  CLottieRoot *getRoot() const override;

  CMatrix2D calcTransform(const TimeFrame &) const override;

  CMatrix2D calcHierTransform(const TimeFrame &timeFrame) const override;

  void printI(const std::string &prefix, bool hier) const override;

  //---

 private:
  OptStr matchName_;
  OptStr css_;

  OptBool threeD_;
  OptBool autoOrient_;
  OptInt  blendMode_;

  OptInt  matteMode_;
  OptInt  matteParent_;
  OptInt  matteTarget_;
  OptBool hasMask_;

  OptInt parentInd_;

  OptReal width_;
  OptReal height_;

  OptInt  frameIn_;
  OptInt  frameOut_;
  OptReal startTime_;
  OptReal timeStretch_;

  OptStr refId_;

  // transform
  Transform *transform_ { nullptr };

  Mask*    mask_    { nullptr };
  Effect*  effect_  { nullptr };
  Solid*   solid_   { nullptr };
  Precomp* precomp_ { nullptr };

  std::vector<CLottieShape *> shapes_;
};

//---

class CLottieMarker : public CLottieObject {
 public:
  CLottieMarker(CLottie *l);
 ~CLottieMarker() override;

  //---

  CLottieRoot *getRoot() const override;

  CMatrix2D calcTransform(const TimeFrame &) const override { return CMatrix2D::identity(); }

  void printI(const std::string &prefix, bool hier) const override;
};

//---

class CLottieShape : public CLottieObject {
 public:
  // rectangle
  struct Rectangle {
    ScalarProperty roundness;

    void print(const std::string &prefix="") const;
  };

  // repeater
  struct Repeater {
    ScalarProperty copies;
    ScalarProperty offset;
    OptInt         composite;
    Transform*     transform { nullptr };

    ScalarProperty startOpacity;
    ScalarProperty endOpacity;

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
    OptStr         dashType;
    OptStr         dashName;
    ScalarProperty dashValue;
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

  struct GradientFill {
    ColorProperty  color;
    ScalarProperty opacity;
    OptInt         type;
    OptInt         stopCount;
    OptInt         index;
    VectorProperty startPoint;
    VectorProperty endPoint;
    ScalarProperty highlightLength;
    ScalarProperty highlightAngle;
    OptInt         fillRule;
    OptInt         blendMode;
    ArrayProperty  colors;

    void print(const std::string &prefix="") const;
  };

  struct GradientStroke {
    ScalarProperty opacity;
    OptInt         type;
    OptInt         stopCount;
    OptInt         index;
    VectorProperty startPoint;
    VectorProperty endPoint;
    ScalarProperty width;
    OptInt         lineCap;
    OptInt         lineJoin;
    OptReal        miterLimit;
    ArrayProperty  colors;

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
    ScalarProperty   innerRoundness;
    ScalarProperty   outerRadius;
    ScalarProperty   outerRoundness;
    ScalarProperty   rotation;
    ScalarProperty   points;

    void print(const std::string &prefix="") const;
  };

  struct Merge {
    OptInt mode;

    void print(const std::string &prefix="") const;
  };

  struct Rounded {
    ScalarProperty roundness;

    void print(const std::string &prefix="") const;
  };

  //---

  CLottieShape(CLottie *l);
 ~CLottieShape() override;

  //---

  CLottieRoot *getRoot() const override;

  CMatrix2D calcTransform(const TimeFrame &) const override;

  CMatrix2D calcHierTransform(const TimeFrame &timeFrame) const override;

  CLottieLayer *getParentLayer() const;
  CLottieShape *getParentShape() const;

  const std::vector<CLottieShape *> &shapes() const { return shapes_; }

  void addShape(CLottieShape *shape) { shapes_.push_back(shape); }

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

  Repeater *repeater() const { return repeater_; }

  Repeater *getRepeater() {
    if (! repeater_)
      repeater_ = new Repeater;
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

  void printI(const std::string &prefix, bool hier) const override;

 private:
  OptStr longName_;
  OptInt index_;
  OptInt direction_;

  // transform
  Transform *transform_ { nullptr };

 public:
  // style
  PositionProperty pos_;
  SizeProperty     size_;
  ColorProperty    color_;
  BezierProperty   path_;

 private:
  Stroke* stroke_ { nullptr };
  Fill*   fill_   { nullptr };

  Group*          group_          { nullptr };
  Rectangle*      rectangle_      { nullptr };
  Repeater*       repeater_       { nullptr };
  GradientFill*   gradientFill_   { nullptr };
  GradientStroke* gradientStroke_ { nullptr };
  Trim*           trim_           { nullptr };
  PolyStar*       polyStar_       { nullptr };
  Merge*          merge_          { nullptr };
  Rounded*        rounded_        { nullptr };

  //---

  std::vector<CLottieShape *> shapes_;
};

#endif
