#include "logging.hpp"
#include <iostream>

namespace ddt
{

logging::logging(const std::string& s, int l) : level(l), time(), overall(s)
{
    time = last = start = std::chrono::system_clock::now();
}

logging::~logging()
{
    step(overall);
    time = std::chrono::system_clock::now();
    operator()(0, std::chrono::duration<float>(time-start).count(), "\n");
    last = time;
    std::cerr << std::endl;
}

void logging::step(const std::string& s) const
{
    time = std::chrono::system_clock::now();
    if(last!=start)
    {
        operator()(2, "\n");
        operator()(0, std::chrono::duration<float>(time-last).count(), "\n");
    }
    last = time;
    operator()(0, s, "\t");
}

void logging::do_log() const
{
    std::cerr << std::flush;
}

}
