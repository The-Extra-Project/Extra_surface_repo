#ifndef SAFE_HP
#define SAFE_HPP

#include <mutex>

// Thread safe implementation of a Queue using a std::queue
template <typename Container>
class safe
{
private:
    Container m_container;
    std::mutex m_mutex;
public:
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;
    safe() {}

    safe(const safe& other) = delete;

    ~safe() {}

    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_container.empty();
    }

    size_type size()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_container.size();
    }

    void enqueue(value_type& t)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_container.push(t);
    }

    bool dequeue(value_type& t)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_container.empty()) return false;
        t = std::move(m_container.front());
        m_container.pop();
        return true;
    }

    template< class... Args >
    void emplace_back( Args&&... args )
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_container.emplace_back(std::forward<Args>(args)...);
    }

    void swap( Container& container )
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_container.swap(container);
    }

    void append( const Container& container )
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_container.insert(m_container.end(), container.begin(), container.end());
    }
};

#endif // SAFE_HPP
