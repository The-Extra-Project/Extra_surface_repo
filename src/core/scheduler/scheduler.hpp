#ifndef DDT_SCHEDULER_HPP
#define DDT_SCHEDULER_HPP

#if DDT_USE_THREADS

#include "scheduler/multithread_scheduler.hpp"
namespace ddt
{
template <typename T> using Scheduler = ddt::multithread_scheduler<T>;
}

#else

#include "scheduler/sequential_scheduler.hpp"
namespace ddt
{
template <typename T> using Scheduler = ddt::sequential_scheduler<T>;
}

#endif

#endif // DDT_SCHEDULER_HPP
