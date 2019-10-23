#ifndef DDT_WRITE_PLY_HPP
#define DDT_WRITE_PLY_HPP

#include <string>
#include <fstream>
#include <map>

namespace ddt
{

void write_ply_header_begin(std::ostream& out);
void write_ply_header_end(std::ostream& out);

template<typename DDT>
void write_ply_element_cell(const DDT& tri, std::ostream& out)
{
    int nc = tri.number_of_cells();
    out << "element face " << nc << std::endl;
    out << "property list uint8 int vertex_indices" << std::endl;
    out << "property uint8 tile" << std::endl;
    out << "property uint8 local" << std::endl;
}

template<typename DDT>
void write_ply_element_vert(const DDT& tri, std::ostream& out)
{
    int D = tri.maximal_dimension();
    int nv = tri.number_of_vertices();
    out << "element vertex " << nv << std::endl;
    out << "property float32 x" << std::endl;
    out << "property float32 y" << std::endl;
    if(D>2) out << "property float32 z" << std::endl;
    out << "property uint8 tile" << std::endl;
    out << "property uint8 id" << std::endl;
}

template<typename Tile>
void write_ply_property_cell(const Tile& tile, std::ostream& out)
{
    unsigned char N = (unsigned char)(tile.maximal_dimension()+1);
    unsigned char tid = tile.id();
    std::map<typename Tile::Vertex_const_handle, int> dict;

    int id = 0;
    for(auto it = tile.vertices_begin(); it != tile.vertices_end(); ++it)
        if(!tile.vertex_is_infinite(it)) dict[it] = id++;

    for(auto cit = tile.cells_begin(); cit != tile.cells_end(); ++cit)
    {
        if(tile.cell_is_infinite(cit)) continue;
        unsigned char local = 0;
        out.write(reinterpret_cast<char *>(&N), sizeof(N));
        for(int i=0; i<N; ++i)
        {
            typename Tile::Vertex_const_handle v = tile.vertex(cit, i);
            int id = dict[v];
            out.write(reinterpret_cast<char *>(&id), sizeof(id));
            local += tile.id(v) == tid;
        }
        out.write(reinterpret_cast<char *>(&tid), sizeof(tid));
        out.write(reinterpret_cast<char *>(&local), sizeof(local));

    }
}

template<typename Tile>
void write_ply_property_vert(const Tile& tile, std::ostream& out)
{
    int D = tile.maximal_dimension();
    unsigned char tid = tile.id();
    for(auto it = tile.vertices_begin(); it != tile.vertices_end(); ++it)
    {
        if(tile.vertex_is_infinite(it)) continue;
        unsigned char id = tile.id(it);
        for(int d=0; d<D; ++d)
        {
            float coord = float(tile.coord(tile.point(it),d));
            out.write(reinterpret_cast<char *>(&coord), sizeof(coord));
        }
        out.write(reinterpret_cast<char *>(&tid), sizeof(tid));
        out.write(reinterpret_cast<char *>(&id), sizeof(id));
    }
}

template<typename DDT>
void write_ply_cell(const DDT& tri, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    write_ply_header_begin(out);
    write_ply_element_cell(tri, out);
    write_ply_header_end(out);
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        write_ply_property_cell(*tile, out);
    out.close();
}

template<typename DDT>
void write_ply_vert(const DDT& tri, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    write_ply_header_begin(out);
    write_ply_element_vert(tri, out);
    write_ply_header_end(out);
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        write_ply_property_vert(*tile, out);
    out.close();
}

template<typename DDT>
void write_ply(const DDT& tri, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);
    write_ply_header_begin(out);
    write_ply_element_vert(tri, out);
    write_ply_element_cell(tri, out);
    write_ply_header_end(out);
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        write_ply_property_vert(*tile, out);
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
        write_ply_property_cell(*tile, out);
    out.close();
}

}

#endif // DDT_WRITE_PLY_HPP
