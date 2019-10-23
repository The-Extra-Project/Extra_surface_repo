#ifndef DDT_MAP_ITERATOR_HPP
#define DDT_MAP_ITERATOR_HPP

namespace ddt
{

template<typename Map_const_iterator>
class Key_const_iterator : public Map_const_iterator
{
public:
    typedef typename Map_const_iterator::value_type::first_type value_type;
    typedef const value_type& reference;
    typedef const value_type* pointer;

    Key_const_iterator ( ) : Map_const_iterator ( ) { }
    Key_const_iterator ( Map_const_iterator it_ ) : Map_const_iterator ( it_ ) { }

    pointer operator -> ( ) const { return &(Map_const_iterator::operator->()->first); }
    reference operator * ( ) const { return Map_const_iterator::operator*().first; }
    Key_const_iterator operator++() { return Map_const_iterator::operator++(); }
    Key_const_iterator operator++(int) { return Map_const_iterator::operator++(0); }
};

template<typename Map_const_iterator>
class Mapped_const_iterator : public Map_const_iterator
{
public:
    typedef typename Map_const_iterator::value_type::second_type value_type;
    typedef const value_type& reference;
    typedef const value_type* pointer;

    Mapped_const_iterator ( ) : Map_const_iterator ( ) { }
    Mapped_const_iterator ( Map_const_iterator it_ ) : Map_const_iterator ( it_ ) { }

    pointer operator -> ( ) const { return &(Map_const_iterator::operator->()->second); }
    reference operator * ( ) const { return Map_const_iterator::operator*().second; }
    Mapped_const_iterator operator++() { return Map_const_iterator::operator++(); }
    Mapped_const_iterator operator++(int) { return Map_const_iterator::operator++(0); }
};

template<typename Map_iterator>
class Mapped_iterator : public Map_iterator
{
public:
    typedef typename Map_iterator::value_type::second_type value_type;
    typedef value_type& reference;
    typedef value_type* pointer;

    Mapped_iterator ( ) : Map_iterator ( ) { }
    Mapped_iterator ( Map_iterator it_ ) : Map_iterator ( it_ ) { }

    pointer operator -> ( ) { return &(Map_iterator::operator->()->second); }
    reference operator * ( ) { return Map_iterator::operator*().second; }
    Mapped_iterator operator++() { return Map_iterator::operator++(); }
    Mapped_iterator operator++(int) { return Map_iterator::operator++(0); }
};

}

#endif // DDT_MAP_ITERATOR_HPP
