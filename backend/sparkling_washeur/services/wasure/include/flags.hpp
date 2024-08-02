#ifndef FLAGS_HPP
#define FLAGS_HPP

#include <iostream>
#include <climits>



template <typename T>
class bag_of_flags
{
public:

    bag_of_flags() : flags {static_cast<T>(0)} { }

    bag_of_flags(T fl) : flags {fl} { }

    bool flag(int f) const
    {
#ifdef DEBUG
        if(f > CHAR_BIT * sizeof(T) - 1)
        {
            std::cerr << "FLAG POSITION ERROR" << std::endl;
        }
#endif
        return flags & static_cast<T>(1 << f);
    }

    void flag(int f, bool val)
    {
#ifdef DEBUG
        if(f > CHAR_BIT * sizeof(T) - 1)
        {
            std::cerr << "FLAG POSITION ERROR" << std::endl;
        }
#endif
        val ? (flags |= static_cast<T>(1 << f)) : (flags &= ~static_cast<T>(1 << f));
    }

    void clear()
    {
        flags =  static_cast<T>(0);
    }

    void set_flags(T f)
    {
        flags = f;
    }

    T get_flags() const
    {
        return flags;
    }



    int get_max_flag_pos() const
    {
        int num_bits = sizeof(T) * CHAR_BIT;
        return (num_bits - 1);
    }

    void apply_AND_mask(T mask) { flags &= mask; }

    void apply_OR_mask(T mask)  { flags |= mask; }

protected:

    T flags;
};

#endif // FLAGS_HPP
