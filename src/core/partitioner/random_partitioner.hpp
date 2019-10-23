#ifndef DDT_RANDOM_PARTITIONER_HPP
#define DDT_RANDOM_PARTITIONER_HPP

#include <chrono>
#include <random>

namespace ddt
{

template<typename Traits, typename Generator = std::default_random_engine>
class random_partitioner
{
public:
    typedef typename Traits::Point Point;
    typedef typename Traits::Id    Id;

    random_partitioner(Id a, Id b, unsigned int seed = 0) : distribution(a,b), generator(seed)
    {
        if(seed == 0)
        {
            seed = std::chrono::system_clock::now().time_since_epoch().count();
            generator.seed(seed);
        }
        std::cout << "random_partitioner seed: " << seed << std::endl;
    }

    random_partitioner(Id a, Id b, const Generator& g) : distribution(a,b), generator(g)
    {
    }

    inline Id operator()(const Point& p)
    {
        return distribution(generator);
    }

private:
    std::uniform_int_distribution<Id> distribution;
    Generator generator;
};

}

#endif // DDT_RANDOM_PARTITIONER_HPP
