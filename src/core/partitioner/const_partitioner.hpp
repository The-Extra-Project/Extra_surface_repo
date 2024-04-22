#ifndef DDT_CONST_PARTITIONER_HPP
#define DDT_CONST_PARTITIONER_HPP

namespace ddt
{

template<typename Traits>
class const_partitioner
{
public:
    typedef typename Traits::Point Point;
    typedef typename Traits::Id    Id;

    const_partitioner(Id id) : id_(id) {}

    inline Id operator()(const Point& p) const
    {
        return id_;
    }

private:
    Id id_;
};

}

#endif // DDT_CONST_PARTITIONER_HPP
