#pragma once

#include "Shape.hpp"
#include "Plane.hpp"
#include "PointSet.hpp"

#include "utils.hpp"
#include <iostream>

namespace angem
{

using Edge = std::pair<std::size_t, std::size_t>;

/* This class implements 2D polygons in 3D space.
 */
template<typename Scalar>
class Polygon: public Shape<Scalar>
{
 public:
  // Default empty constructor. Use only if assigning data later
	Polygon();
  // Create a polygon from a vector of points.
  // Vertices are ordered in a clock-wise manner upon creation.
  Polygon(const std::vector<Point<3,Scalar>> & points_list);
  // Helper constructor. Construct a polygon (face) from some mesh vertices.
  // Vertices are ordered in a clock-wise manner upon creation.
  Polygon(const std::vector<Point<3,Scalar>> & all_mesh_vertices,
          const std::vector<std::size_t>     & indices);
  // Helper constructor. Construct a polygon (face) from some mesh vertices.
  // Vertices are ordered in a clock-wise manner upon creation.
  Polygon(const PointSet<3,Scalar>           & all_mesh_vertices,
          const std::vector<std::size_t>     & indices);

  // helper function: get a plane that contains an edge wich the normal in the
  // plane of the polygon
  Plane<Scalar> get_side(const Edge & edge) const;

  // returns true if point is inside a 3D shape formed by
  // lines going through the poly vertices in the direction of
  // poly plane normal
  bool point_inside(const Point<3,Scalar> & p,
                    const Scalar            tol = 1e-10) const;

  // compute the area of the polygon
  Scalar area() const;
  // compute the center of mass of the polygons
  virtual Point<3,Scalar> center() const override;
  // helper function: get a vector of edges represented by pairs of vertex indices
  std::vector<Edge> get_edges() const;

  // setter
  virtual void set_data(const std::vector<Point<3,Scalar>> & point_list) override;
  // shift all points in direction p
  virtual void move(const Point<3,Scalar> & p) override;
  // order vertices in a clockwise fashion
  static  void reorder(std::vector<Point<3,Scalar>> & points);
  // order indices vector so that the corresponding points are in a clockwise fashiok
  static  void reorder_indices(const std::vector<Point<3, Scalar>> &verts,
                               std::vector<std::size_t>            &indices);
  angem::Point<3,double> normal() const { return plane().normal(); }
  inline const Plane<Scalar> & plane() const { return m_plane; }
  inline Plane<Scalar> & plane() { return m_plane; }

 protected:
  // Attributes
  Plane<Scalar> m_plane;
};


template<typename Scalar>
Polygon<Scalar>::Polygon()
    :
    Shape<Scalar>::Shape()
{}


template<typename Scalar>
Polygon<Scalar>::Polygon(const std::vector<Point<3,Scalar>> & point_list)
{
  assert(point_list.size() > 2);
  set_data(point_list);
}


template<typename Scalar>
Polygon<Scalar>::Polygon(const std::vector<Point<3,Scalar>> & all_mesh_vertices,
                         const std::vector<std::size_t>     & indices)
{
  assert(indices.size() > 2);
  std::vector<Point<3,Scalar>> point_list;
  for (const std::size_t & i : indices)
    point_list.push_back(all_mesh_vertices[i]);
  set_data(point_list);
}


template<typename Scalar>
Polygon<Scalar>::Polygon(const PointSet<3,Scalar>           & all_mesh_vertices,
                         const std::vector<std::size_t>     & indices)
{
  assert(indices.size() > 2);
  std::vector<Point<3,Scalar>> point_list;
  for (const std::size_t i : indices)
    point_list.push_back(all_mesh_vertices[i]);
  set_data(point_list);
}


template<typename Scalar>
void
Polygon<Scalar>::set_data(const std::vector<Point<3,Scalar>> & point_list)
{
  assert(point_list.size() >= 3);
  this->points = point_list;
  reorder(this->points);
  const Point<3, Scalar> cm = compute_center_mass(point_list);
  m_plane = Plane<Scalar>(point_list);
  m_plane.set_point( cm );
}


template<typename Scalar>
void
Polygon<Scalar>::move(const Point<3,Scalar> & p)
{
  Shape<Scalar>::move(p);
  plane().move(p);
}


template<typename Scalar>
void
Polygon<Scalar>::reorder(std::vector<Point<3, Scalar> > & points)
{
  const std::size_t n_points = points.size();
  assert(n_points > 2);
  if (n_points == 3) return;

  const Point<3,Scalar> cm = compute_center_mass(points);
  Plane<Scalar> plane = Plane<Scalar>(points);
  plane.set_point(cm);
  Point<3,Scalar> normal = plane.normal();

  std::vector<Point<3, Scalar> > v_points;
  std::vector<Point<3,Scalar>> copy(points.begin()+1, points.end());
  v_points.push_back(points.front());

  std::size_t safety_counter = 0, counter_max = 2 * copy.size();
  while (!copy.empty())
  {
    if (safety_counter >= counter_max)
      throw std::runtime_error("polygon is not convex");

    if (copy.size() == 1)
    {
      v_points.push_back(copy[0]);
      break;
    }
    // find such vertex that all other vertices are on one side of the edge
    for (std::size_t i=0; i<copy.size(); ++i)
    {
      // make plane object that we use to check on which side of the plane any point is
      const Scalar len = (copy[i] - v_points.back()).norm();
      assert ( len > 0 );
      const Point<3,Scalar> p_perp = v_points.back() + normal * len;
      const Plane<Scalar> pln(v_points.back(), p_perp, copy[i]);

      bool all_above = true;
      bool orientation;
      bool orientation_set = false;  // set after first assignment
      for (std::size_t j=0; j<points.size(); ++j)
      {
        if (points[j] == copy[i] || points[j] == v_points.back())
          continue;
        const bool above = pln.signed_distance(points[j]) > -1e-8;
        if (!orientation_set)
        {
          orientation = above;
          orientation_set = true;
        }
        if (above != orientation)
        {
          all_above = false;
          break;
        }
      }
      if (all_above)
      {
        v_points.push_back(copy[i]);
        copy.erase(copy.begin() + i);
        break;
      }

    }
    safety_counter++;
  }

  points = v_points;
}


template<typename Scalar>
void
Polygon<Scalar>::reorder_indices(const std::vector<Point<3, Scalar>> &verts,
                                 std::vector<std::size_t>            &indices)
{
  std::vector<Point<3, Scalar>> points(indices.size());
  for (std::size_t i=0; i<indices.size(); ++i)
    points[i] = verts[indices[i]];

  reorder(points);

  for (std::size_t i=0; i<points.size(); ++i)
  {
    std::size_t ind = find(points[i], verts, 1e-6);
    indices[i] = ind;
  }
}

template<typename Scalar>
Scalar Polygon<Scalar>::area() const
{
  const Point<3, Scalar> cm = compute_center_mass(this->points);
  Scalar total_area = 0;
  for (std::size_t i=0; i<this->points.size(); ++i)
  {
    if (i < this->points.size() - 1)
    {
      const Point<3,Scalar> v1 = this->points[i];
      const Point<3,Scalar> v2 = this->points[i + 1];
      total_area += triangle_area(v1, v2, cm);
    }
    else
    {
      const Point<3,Scalar> v1 = this->points[i];
      const Point<3,Scalar> v2 = this->points[0];
      total_area += triangle_area(v1, v2, cm);
    }
  }

  return total_area;
}


template<typename Scalar>
std::vector<Edge> Polygon<Scalar>::get_edges() const
{
  std::vector<Edge> edges;
  for (std::size_t i=0; i<this->points.size(); ++i)
  {
    std::size_t i1, i2;
    if (i < this->points.size() - 1)
    {
      i1 = i;
      i2 = i + 1;
    }
    else
    {
      i1 = i;
      i2 = 0;
    }

    edges.push_back({i1, i2});
  }

  return edges;
}


template<typename Scalar>
bool Polygon<Scalar>::point_inside(const Point<3, Scalar> & p ,
                                   const Scalar             tol) const
{
  if ( std::fabs(this->plane().signed_distance(p)) > tol )
    return false;

  const auto & points = this->points;
  const Point<3,Scalar> cm = this->center();
  const Point<3,Scalar> n = this->plane().normal();
  for (const auto & edge : get_edges())
  {
    const Plane<Scalar> side = get_side(edge);
    if (side.above(p) != side.above(cm) and std::fabs( side.signed_distance(p) ) > tol)
      return false;
  }

  return true;
}


template<typename Scalar>
Plane<Scalar> Polygon<Scalar>::get_side(const Edge & edge) const
{
  if (edge.first >= this->points.size() or edge.second >= this->points.size())
    throw std::out_of_range("Edge does not exist");

  Point<3,Scalar> point3 =
      this->points[edge.first] + m_plane.normal() * (this->points[edge.first] -
                                                   this->points[edge.second]).norm();
  Plane<Scalar> side(this->points[edge.first],
                     this->points[edge.second],
                     point3);
  return side;
}


template<typename Scalar>
Point<3,Scalar> Polygon<Scalar>::center() const
{
  Point<3,Scalar> u, v, n, c;
  Scalar poly_area = 0;

  /* This essentially breaks the poly into triangles with
   * vertices in points[0], points[j], points[j+1] */
  for (std::size_t j=1; j<this->points.size()-1; j++)
  {
    /* compute normal and offset w from first 3 vertices */
    // u and v are tangent vectors
    u = this->points[j] - this->points[0];
    v = this->points[j+1] - this->points[0];
    // normal vector components (not normalized)
    n = cross_product(u, v);
    const Scalar areatmp = 0.5 * n.norm();
    c += areatmp/3 * (this->points[0] + this->points[j] + this->points[j+1]);
    poly_area += areatmp;
  }

  c /= poly_area;
  return c;
}

}
