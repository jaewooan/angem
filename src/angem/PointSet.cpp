#include "angem/PointSet.hpp"

#ifndef WIN32  // windows platform

namespace angem
{

template class PointSet<3,double>;

template<>
PointSet<3,double>::PointSet(const double tol)
    :
    tol(tol)
{
  const pset_hash_type max_value = std::numeric_limits<pset_hash_type>::max();
  const double nx = std::cbrt( static_cast<double>(max_value) );

  upper = {tol*nx / 2, tol*nx / 2, tol*nx / 2};
  lower = -upper;
}

}  // end namesace

#endif
