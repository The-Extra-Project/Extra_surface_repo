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


}

#endif // DATA_CELL_BASE_HPP
