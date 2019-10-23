#include "write_ply.hpp"

namespace ddt
{

void write_ply_header_begin(std::ostream& out)
{
    out << "ply" << std::endl;
    out << "format binary_little_endian 1.0" << std::endl;
    out << "comment creator: Mathieu Bredif" << std::endl;
}

void write_ply_header_end(std::ostream& out)
{
    out << "end_header" << std::endl;
}

}

