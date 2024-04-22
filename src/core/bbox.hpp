#ifndef DDT_BBOX_HPP
#define DDT_BBOX_HPP

#include <iostream>

namespace ddt
{

template<int D>
struct Bbox
{
    Bbox(double range)
    {
        for(int i=0; i<D; ++i)
        {
            value[i  ] = -range;
            value[i+D] =  range;
        }
    }
    Bbox(double m, double M)
    {
        for(int i=0; i<D; ++i)
        {
            value[i  ] = m;
            value[i+D] = M;
        }
    }
    Bbox()
    {
        for(int i=0; i<D; ++i)
        {
            value[i  ] = +std::numeric_limits<double>::infinity();
            value[i+D] = -std::numeric_limits<double>::infinity();
        }
    }

    template<typename It>
    Bbox& insert(It begin, It end)
    {
        for(It it = begin; it != end; ++it)
            *this += *it;
        return *this;
    }

    template<typename P>
    Bbox& operator+=(const P& p)
    {
        for(int i=0; i<D; ++i)
        {
            if(value[i  ] > p[i]) value[i  ] = p[i];
            if(value[i+D] < p[i]) value[i+D] = p[i];
        }
        return *this;
    }

    Bbox& operator+=(const Bbox& bbox)
    {
        for(int i=0; i<D; ++i)
        {
            if(value[i  ] > bbox.value[i  ]) value[i  ] = bbox.value[i  ];
            if(value[i+D] < bbox.value[i+D]) value[i+D] = bbox.value[i+D];
        }
        return *this;
    }

    inline double min(int i) const
    {
        return value[i  ];
    }
    inline double max(int i) const
    {
        return value[i+D];
    }

    double value[2*D];
};

template<int D>
std::ostream& operator<<(std::ostream& out, const Bbox<D>& bbox)
{
    out << std::fixed;
    for(int i=0; i<D; ++i)
        out  << bbox.value[i] << "  " << bbox.value[i+D] << " ";
    return out;
}


template<int D>
std::istream& operator>>(std::istream& in, Bbox<D>& bbox)
{
    for(int i=0; i<D; ++i)
        in  >> bbox.value[i] >> bbox.value[i+D];
    return in;
}


}

#endif // DDT_BBOX_HPP

