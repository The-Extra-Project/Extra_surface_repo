#ifndef DDT_LOGGING_STREAM_HPP
#define DDT_LOGGING_STREAM_HPP

#include <chrono>
#include <iostream>
#include <sstream>

namespace ddt
{

class logging_stream
{
public:
    logging_stream(const std::string& s, int l);

    template<typename...Args>
    void operator()(int l, Args&&... args)
    {
        if(level>=l) do_log(args...);
    }
    void step(const std::string& s) ;
    void dump_log(std::ostream & oos) ;
    int level;
private:

    void do_log() ;
    template<typename Arg, typename...Args>
    void do_log(Arg&& arg, Args&&... args)
    {
        sstr << arg;
        do_log(args...);
    }

    mutable std::chrono::time_point<std::chrono::system_clock> time, last, start;
    std::stringstream sstr;
    std::string overall;
};

}

#endif // DDT_LOGGING_STREAM_HPP
