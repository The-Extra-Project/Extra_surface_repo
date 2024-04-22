#ifndef DATA_CELL_BASE_HPP
#define DATA_CELL_BASE_HPP

#include "../bbox.hpp"

namespace ddt
{

template<typename I>
struct Data_cell_base
{
    typedef I Id;

    Data_cell_base(Id i=0) : id(i) {}

    void write(std::ostream& os,bool is_ascii) const
    {
        if(is_ascii)
        {
            os << id << " ";
        }
        else
        {
            os.write((char*)(&(id)), sizeof(id));
        }
    }
    void read(std::istream& is, bool is_ascii)
    {
        if(is_ascii)
        {
            is >> id;
        }
        else
        {
            is.read((char*)(&(id)), sizeof(id));
        }
    }


    Id id;
};

// template<typename T1, typename T2>
// struct Pair2nd : public std::unary_function<const T1&, std::pair<T1,T2>>
// {
//     T2 _2;
//     Pair2nd(const T2& x) : _2(x) {}
//     inline std::pair<T1,T2> operator()(const T1& _1) const
//     {
//         return std::make_pair(_1, _2);
//     }
// };

}

#endif // DATA_CELL_BASE_HPP
