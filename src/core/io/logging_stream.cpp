#include "logging_stream.hpp"
#include <iostream>

namespace ddt
{

logging_stream::logging_stream( const std::string& s, int l) : level(l), time(), overall(s)
{
    time = last = start = std::chrono::system_clock::now();
}

void logging_stream::dump_log(std::ostream & oos)
{
    step("[total]");
    time = std::chrono::system_clock::now();
    operator()(0, std::chrono::duration<float>(time-start).count(), "");
    last = time;
    oos << sstr.str();
}

void logging_stream::step(const std::string& s)
{
    time = std::chrono::system_clock::now();
    if(last!=start)
    {
        operator()(2, "");
        operator()(0, std::chrono::duration<float>(time-last).count(), ";");
    }
    last = time;
    operator()(0, overall + "_"+ s, ":");
}

void logging_stream::do_log()
{

    return;
}

}
