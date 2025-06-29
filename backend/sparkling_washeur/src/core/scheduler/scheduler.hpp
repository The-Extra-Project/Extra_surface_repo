/*
 * This file is part of the watertight surface reconstruction code https://github.com/lcaraffa/spark-ddt
 * Copyright (c) 2024 Caraffa Laurent, Mathieu Brédif.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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
