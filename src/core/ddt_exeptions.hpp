#ifndef DDT_EXT_HPP
#define DDT_EXT_HPP

#include <iostream>
#include <exception>

namespace ddt
{
struct DDT_exeption : public std::exception
{
    std::string s;
    DDT_exeption(std::string ss) : s(ss) {}
    ~DDT_exeption() throw () {} // Updated
    const char* what() const throw() { return s.c_str(); }
};
}

#endif
