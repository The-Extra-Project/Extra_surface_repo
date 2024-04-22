#ifndef DDT_LOGGING_HPP
#define DDT_LOGGING_HPP

#include <chrono>
#include <iostream>

namespace ddt
{

class logging
{
public:
    logging(const std::string& s, int l);
    ~logging() ;
    template<typename...Args>
    void operator()(int l, Args&&... args) const
    {
        if(level>=l) do_log(args...);
    }
    void step(const std::string& s) const;
    int level;
private:

    void do_log() const;
    template<typename Arg, typename...Args>
    void do_log(Arg&& arg, Args&&... args) const
    {
        std::cerr << arg;
        do_log(args...);
    }

    mutable std::chrono::time_point<std::chrono::system_clock> time, last, start;
    std::string overall;
};

}

#endif // DDT_LOGGING_HPP
