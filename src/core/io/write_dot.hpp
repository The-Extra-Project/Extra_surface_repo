#ifndef DDT_WRITE_DOT_HPP
#define DDT_WRITE_DOT_HPP

#include <fstream>
#include <unordered_map>
#include <string>

namespace ddt
{

template<typename DDT>
void write_adjacency_graph_dot(const DDT& tri, const std::string& dot, bool oriented=false)
{
    typedef typename DDT::Id Id;
    std::unordered_multimap<Id,Id> edges;
    tri.get_adjacency_graph(edges);
    std::ofstream out(dot);
    out << (oriented?"digraph":"graph") << " tile_adjacency {\n";
    for(auto& p : edges)
        if(oriented || p.first < p.second)
            out << "\t"<<int(p.first) << (oriented?" -> ":" -- ") << int(p.second) << ";\n";
    out << "}";
}

}

#endif // DDT_WRITE_DOT_HPP
