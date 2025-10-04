#include "commands.hpp"
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include "streamGuard.hpp"

namespace
{
  using namespace shchadilov;

  struct IsEven
  {
    bool operator()(const Polygon& poly) const
    {
      return poly.points.size() % 2 == 0;
    }
  };

  struct IsOdd
  {
    bool operator()(const Polygon& poly) const
    {
      return poly.points.size() % 2 != 0;
    }
  };

  struct AreaAccumulator
  {
    double operator()(double sum, const Polygon& poly) const
    {
      return sum + getArea(poly);
    }
  };

  struct ConditionalAreaAccumulator
  {
    std::function< bool(const Polygon&) > predicate_;
    ConditionalAreaAccumulator(std::function< bool(const Polygon&) > pred) :
      predicate_(pred)
    {
    }

    double operator()(double sum, const Polygon& poly) const
    {
      return predicate_(poly) ? sum + getArea(poly) : sum;
    }
  };

  struct ConditionalCounter
  {
    std::function< bool(const Polygon&) > predicate_;

    ConditionalCounter(std::function< bool(const Polygon&) > pred) :
      predicate_(pred)
    {
    }

    size_t operator()(size_t count, const Polygon& poly) const
    {
      return predicate_(poly) ? count + 1 : count;
    }
  };

  struct VertexCountChecker
  {
    size_t count_;

    VertexCountChecker(size_t cnt) :
      count_(cnt)
    {
    }

    bool operator()(const Polygon& poly) const
    {
      return poly.points.size() == count_;
    }
  };

  struct RightAngleChecker
  {
    bool operator()(const Polygon& poly) const
    {
      const auto& points = poly.points;
      size_t n = points.size();

      std::vector< size_t > indices(n);
      std::iota(indices.begin(), indices.end(), 0);

      struct AngleCheck
      {
        const std::vector< Point >& points_;
        size_t n_;

        AngleCheck(const std::vector< Point >& pts, size_t size) :
          points_(pts),
          n_(size)
        {
        }

        bool operator()(size_t i) const
        {
          const Point& a = points_[i];
          const Point& b = points_[(i + 1) % n_];
          const Point& c = points_[(i + 2) % n_];

          int dx1 = b.x - a.x;
          int dy1 = b.y - a.y;
          int dx2 = c.x - b.x;
          int dy2 = c.y - b.y;

          return dx1 * dx2 + dy1 * dy2 == 0;
        }
      };

      AngleCheck angleCheck(points, n);
      return std::any_of(indices.begin(), indices.end(), angleCheck);
    }
  };

  double accumulateArea(const std::vector< Polygon >& polygons, std::function< bool(const Polygon&) > pred)
  {
    ConditionalAreaAccumulator accumulator(pred);
    return std::accumulate(polygons.begin(), polygons.end(), 0.0, accumulator);
  }

  size_t countIf(const std::vector< Polygon >& polygons, std::function< bool(const Polygon&) > pred)
  {
    ConditionalCounter counter(pred);
    return std::accumulate(polygons.begin(), polygons.end(), 0, counter);
  }
}

void shchadilov::printArea(std::istream& in, std::ostream& out, const std::vector<Polygon>& polygons)
{
  std::string param;
  in >> param;
  StreamGuard guard(out);
  out << std::fixed << std::setprecision(1);

  if (param == "EVEN")
  {
    out << accumulateArea(polygons, IsEven());
  }
  else if (param == "ODD")
  {
    out << accumulateArea(polygons, IsOdd());
  }
  else if (param == "MEAN")
  {
    if (polygons.empty())
    {
      throw std::invalid_argument("No polygons for MEAN");
    }
    AreaAccumulator accumulator;
    out << std::accumulate(polygons.begin(), polygons.end(), 0.0, accumulator) / polygons.size();
  }
  else
  {
    size_t num = std::stoul(param);
    if (num < 3)
    {
      throw std::invalid_argument("Invalid vertex count");
    }
    out << accumulateArea(polygons, VertexCountChecker(num));
  }
}

struct AreaComparator
{
  bool operator()(const shchadilov::Polygon& a, const shchadilov::Polygon& b) const
  {
    return getArea(a) < getArea(b);
  }
};

struct VertexCountComparator
{
  bool operator()(const shchadilov::Polygon& a, const shchadilov::Polygon& b) const
  {
    return a.points.size() < b.points.size();
  }
};

void shchadilov::printMax(std::istream& in, std::ostream& out, const std::vector<Polygon>& polygons)
{
  std::string param;
  in >> param;
  if (polygons.empty())
  {
    throw std::invalid_argument("No polygons for MAX");
  }

  if (param == "AREA")
  {
    StreamGuard guard(out);
    out << std::fixed << std::setprecision(1);
    AreaComparator comparator;
    auto it = std::max_element(polygons.begin(), polygons.end(), comparator);
    out << getArea(*it);
  }
  else if (param == "VERTEXES")
  {
    VertexCountComparator comparator;
    auto it = std::max_element(polygons.begin(), polygons.end(), comparator);
    out << it->points.size();
  }
  else
  {
    throw std::invalid_argument("Invalid MAX parameter");
  }
}

void shchadilov::printMin(std::istream& in, std::ostream& out, const std::vector<Polygon>& polygons)
{
  std::string param;
  in >> param;
  if (polygons.empty())
  {
    throw std::invalid_argument("No polygons for MIN");
  }

  if (param == "AREA")
  {
    StreamGuard guard(out);
    out << std::fixed << std::setprecision(1);
    AreaComparator comparator;
    auto it = std::min_element(polygons.begin(), polygons.end(), comparator);
    out << getArea(*it);
  }
  else if (param == "VERTEXES")
  {
    VertexCountComparator comparator;
    auto it = std::min_element(polygons.begin(), polygons.end(), comparator);
    out << it->points.size();
  }
  else
  {
    throw std::invalid_argument("Invalid MIN parameter");
  }
}

void shchadilov::printCount(std::istream& in, std::ostream& out, const std::vector<Polygon>& polygons)
{
  std::string param;
  in >> param;

  if (param == "EVEN")
  {
    out << countIf(polygons, IsEven());
  }
  else if (param == "ODD")
  {
    out << countIf(polygons, IsOdd());
  }
  else
  {
    size_t num = std::stoul(param);
    if (num < 3)
    {
      throw std::invalid_argument("Invalid vertex count");
    }
    out << countIf(polygons, VertexCountChecker(num));
  }
}

struct RightAngleTester
{
  RightAngleChecker checker;

  bool operator()(const Polygon& poly) const
  {
    return checker(poly);
  }
};

void shchadilov::printRights(std::istream& in, std::ostream& out, const std::vector<Polygon>& polygons)
{
  RightAngleTester tester;
  size_t count = countIf(polygons, tester);
  out << count;
}

struct PolygonEqual
{
  bool operator()(const shchadilov::Polygon& a, const shchadilov::Polygon& b) const
  {
    return a == b;
  }
};

struct TargetDuplicate
{
  Polygon target_;
  PolygonEqual eq_;
  bool operator()(const Polygon& a, const Polygon& b) const
  {
    return eq_(a, target_) && eq_(b, target_);
  }
};

void shchadilov::printRmEcho(std::istream& in, std::ostream& out, std::vector<Polygon>& polygons)
{
  Polygon target;
  if (!(in >> target))
  {
    throw std::invalid_argument("Invalid polygon for RMECHO");
  }

  TargetDuplicate pred{ target, PolygonEqual{} };
  auto newEnd = std::unique(polygons.begin(), polygons.end(), pred);

  size_t removed = std::distance(newEnd, polygons.end());
  polygons.erase(newEnd, polygons.end());

  out << removed;
}
