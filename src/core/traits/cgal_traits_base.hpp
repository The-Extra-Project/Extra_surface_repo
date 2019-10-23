#ifndef DDT_CGAL_TRAITS_BASE_HPP
#define DDT_CGAL_TRAITS_BASE_HPP

#include "../bbox.hpp"

namespace ddt
{

template<typename I, typename F>
struct Data
{
    typedef I Id;
    typedef F Flag;

    Data(Id i=0, Flag f=0) : id(i), flag(f) {}

    void write(std::ostream& os,bool is_ascii) const
    {
        if(is_ascii)
        {
            os << id << " "  << flag << " ";
        }
        else
        {
            os.write((char*)(&(id)), sizeof(id));
            os.write((char*)(&(flag)), sizeof(flag));
        }
    }
    void read(std::istream& is, bool is_ascii)
    {
        if(is_ascii)
        {
            is >> id >> flag;
        }
        else
        {
            is.read((char*)(&(id)), sizeof(id));
            is.read((char*)(&(flag)), sizeof(flag));
        }
    }


    Id id;
    mutable Flag flag;
};



template<typename I, typename F>
struct DataF
{
    typedef I Id;
    typedef F Flag;

    DataF() :  flag(0) {}
    DataF(Flag f) :  flag(f) {}
    mutable Flag flag;
};


template<typename T1, typename T2>
struct Pair2nd : public std::unary_function<const T1&, std::pair<T1,T2>>
{
    T2 _2;
    Pair2nd(const T2& x) : _2(x) {}
    inline std::pair<T1,T2> operator()(const T1& _1) const
    {
        return std::make_pair(_1, _2);
    }
};

}

#endif // DDT_CGAL_TRAITS_BASE_HPP
