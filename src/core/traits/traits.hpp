#ifndef DDT_TRAITS_HPP
#define DDT_TRAITS_HPP

#if defined(DDT_CGAL_TRAITS_2)

#include "cgal_traits_2.hpp"
namespace ddt
{
template <typename DataV,typename DataC> using Traits = ddt::Cgal_traits_2<DataV,DataC>;
}

#elif defined(DDT_CGAL_TRAITS_3)

#include "cgal_traits_3.hpp"
namespace ddt
{
template <typename DataV,typename DataC> using Traits = ddt::Cgal_traits_3<DataV,DataC>;
 using Traits_raw = ddt::Cgal_traits_3_Raw;
}

#elif defined(DDT_CGAL_TRAITS_D)

#include "cgal_traits_d.hpp"
namespace ddt
{
template <typename DataV,typename DataC> using Traits = ddt::Cgal_traits<DDT_CGAL_TRAITS_D,DataV,DataC>;
using Traits_raw = ddt::Cgal_traits_raw<DDT_CGAL_TRAITS_D>;
}

#endif

#endif // DDT_TRAITS_HPP
