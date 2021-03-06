#pragma once

#include "Point.hpp"

namespace angem
{

/* This class implements a orthonormal basis
 */
template <int dim, typename Scalar>
class Basis
{
 public:
  // default constructor. Creates e.g. in 3D three vectors
  // {1,0,0}, {0,1,0}, {0,0,1}
  Basis();
  // create basis from vector of points
  // checks that vecs.size() == dim
  Basis(const std::vector<Point<dim,Scalar>> & vecs);

  // setters
  // assign basis components from the vector of points
  void set_data(const std::vector<Point<dim,Scalar>> & vecs);

  // non-constant getter
  Point<dim,Scalar> & operator[] (int i);
  // constant getter
  const Point<dim,Scalar> & operator() (int i) const;

  // get coordinates of a point in this basis
  Point<dim,Scalar> transform(const Point<dim,Scalar> & p) const;
  // probably do not need this any more since i changed the default constructor
  bool is_empty() const;

  // printout
  template <int d, typename S>
  friend std::ostream &operator<<(std::ostream     & os,
                                  const Basis<d,S> & p);

 private:
  std::vector<Point<dim,Scalar>> vectors;
};


template <int dim, typename Scalar>
Basis<dim,Scalar>::Basis()
{
  vectors.resize(dim);
  for (int i=0; i<dim; ++i)
    vectors[i][i] = static_cast<Scalar>(1);
}


template <int dim, typename Scalar>
Basis<dim,Scalar>::Basis(const std::vector<Point<dim,Scalar>> & vecs)
    :
    vectors(vecs)
{
  assert(vecs.size() == dim);
}


template <int dim, typename Scalar>
void
Basis<dim,Scalar>::set_data(const std::vector<Point<dim,Scalar>> & vecs)
{
  assert(vecs.size() == dim);
  vectors = vecs;
}


template <int dim, typename Scalar>
Point<dim,Scalar> &
Basis<dim,Scalar>::operator[](int i)
{
  assert(i < dim);
  return vectors[i];
}


template <int dim, typename Scalar>
const Point<dim,Scalar> &
Basis<dim,Scalar>::operator()(int i) const
{
  assert(i < dim);
  return vectors[i];
}


template <int dim, typename Scalar>
Point<dim,Scalar>
Basis<dim,Scalar>::transform(const Point<dim,Scalar> & p) const
{
  assert(!is_empty());
  Point<dim,Scalar> result;
  for (int i=0; i<dim; ++i)
    result[i] = p.dot(vectors[i]);
  return result;
}


template <int dim, typename Scalar>
bool
Basis<dim,Scalar>::is_empty() const
{
  Scalar sum = static_cast<Scalar>(0);
  for (int i=0; i<dim; ++i)
    sum += vectors[i].norm();
  if (fabs(sum - static_cast<Scalar>(3)) < 1e-8)
    return false;
  else
    return true;
}

template <int d, typename S>
std::ostream &operator<<(std::ostream & os, const Basis<d,S> & basis)
{
  os << "(" << basis(0) << ",\n " << basis(1) << ",\n " << basis(2) << ")";
  return os;
}

}  // end namespace
