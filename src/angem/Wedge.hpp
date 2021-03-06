#pragma once

#include "angem/Polyhedron.hpp"
#include "angem/Plane.hpp"

namespace angem
{

/*
  Node numbering:
  Prism:                      Prism15:               Prism18:

            w
            ^
            |
            3                       3                      3
          ,/|`\                   ,/|`\                  ,/|`\
        ,/  |  `\               12  |  13              12  |  13
      ,/    |    `\           ,/    |    `\          ,/    |    `\
      4------+------5         4------14-----5        4------14-----5
      |      |      |         |      8      |        |      8      |
      |    ,/|`\    |         |      |      |        |    ,/|`\    |
      |  ,/  |  `\  |         |      |      |        |  15  |  16  |
      |,/    |    `\|         |      |      |        |,/    |    `\|
    ,|      |      |\        10     |      11       10-----17-----11
  ,/ |      0      | `\      |      0      |        |      0      |
  u   |    ,/ `\    |    v    |    ,/ `\    |        |    ,/ `\    |
      |  ,/     `\  |         |  ,6     `7  |        |  ,6     `7  |
      |,/         `\|         |,/         `\|        |,/         `\|
      1-------------2         1------9------2        1------9------2

*/

template<typename Scalar>
class Wedge: public Polyhedron<Scalar>
{
 public:
  // CONSTRICTORS
  // input given with vtk numbering
  Wedge(const std::vector<Point<3,Scalar>> & vertices,
        const std::vector<std::size_t>     & indices);

  // SETTERS
  void set_data(const std::vector<Point<3,Scalar>> & vertices);
  void set_data(const std::vector<Point<3,Scalar>> & vertices,
                const std::vector<std::size_t>     & indices);

  // GETTERS
  // don't create polyhedron, just give me vector of faces with indices in
  // the global vector
  static std::vector<std::vector<std::size_t>>
  get_faces(const std::vector<std::size_t>     & indices);
  virtual Scalar volume() const override;

};


template<typename Scalar>
Wedge<Scalar>::Wedge(const std::vector<Point<3,Scalar>> & vertices,
                     const std::vector<std::size_t>     & indices)
    :
    Polyhedron<Scalar>(VTK_ID::WedgeID)
{
  set_data(vertices, indices);
}


template<typename Scalar>
void
Wedge<Scalar>::set_data(const std::vector<Point<3,Scalar>> & vertices)
{
  if (vertices.size() == 15 or vertices.size() == 18)
    throw NotImplemented("Only first order Wedges implemented");
  assert(vertices.size() == 6);

  this->points = vertices;
  this->m_faces.resize(5);
  this->m_faces[0] = {0, 1, 2};
  this->m_faces[1] = {3, 4, 5};
  this->m_faces[2] = {0, 3, 4, 1};
  this->m_faces[3] = {1, 2, 5, 4};
  this->m_faces[4] = {0, 3, 5, 2};
}


template<typename Scalar>
void
Wedge<Scalar>::set_data(const std::vector<Point<3,Scalar>> & vertices,
                        const std::vector<std::size_t>     & indices)
{
  std::vector<Point<3,Scalar>> points(indices.size());
  int counter = 0;
  for (const auto & ivert : indices)
  {
    points[counter] = vertices[ivert];
    counter++;
  }

  set_data(points);
}


template<typename Scalar>
std::vector<std::vector<std::size_t>>
Wedge<Scalar>::get_faces(const std::vector<std::size_t>     & indices)
{
  std::vector<std::vector<std::size_t>> global_faces(5);
  global_faces[0] = {indices[0], indices[1], indices[2]};
  global_faces[1] = {indices[3], indices[4], indices[5]};
  global_faces[2] = {indices[0], indices[3], indices[4], indices[1]};
  global_faces[3] = {indices[1], indices[2], indices[5], indices[4]};
  global_faces[4] = {indices[0], indices[3], indices[5], indices[2]};
  return global_faces;
}


template<typename Scalar>
inline
Scalar Wedge<Scalar>::volume() const
{
  // see on wiki
  const auto & pts = this->points;
  const auto a = pts[5].distance(pts[2]);
  const auto b = pts[1].distance(pts[2]);
  const auto c = pts[0].distance(pts[3]);
  const Plane<Scalar> pln(pts[1], pts[2], pts[4]);
  const auto h = fabs(pln.signed_distance(pts[0]));
  return b*h/3. * (a + c/2);
}

}
