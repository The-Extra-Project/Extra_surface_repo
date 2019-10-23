#ifndef DDT_READ_CGAL_HPP
#define DDT_READ_CGAL_HPP

#include <string>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace ddt
{

template<typename Tile>
std::istream& read_json(Tile & tile,std::istream&  ifile)
{

    typedef typename Tile::Id Id;
    boost::property_tree::ptree root_node;
    boost::property_tree::read_json(ifile, root_node);
    auto & bbox = tile.bbox();
    // int id =  root_node.get<Id>("id");
    // tile.set_id(id);
    for (auto its : root_node.get_child("bbox"))
    {
        int iid = std::stoi(its.first);
        Id id = iid;
        std::stringstream ss (its.second.data());
        ss >> bbox[id];
        //std::cout <<  "bbox reading :" << its.first << " " << its.second.data() << std::endl;
    }
    return ifile;
}

template<typename Tile>
int read_cgal_tile(Tile& tile, const std::string& dirname)
{
    std::string filename = dirname + "/" + std::to_string(tile.id() ) + ".bin";
    std::string json_name = dirname + "/" + std::to_string(tile.id() ) + ".json";
    std::ifstream ifile_tri(filename, std::ios::binary | std::ios::out);
    std::ifstream ifile_json(json_name, std::ifstream::in);
    read_json(tile,ifile_json);
    ifile_json.close();
    tile.read_cgal(ifile_tri);
    return 0;
}

template<typename DDT>
int read_cgal(DDT& tri, const std::string& dirname)
{
    int i = 0;
    for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
    {
        i += read_cgal_tile(*tile, dirname);

    }
    return i;
}

}

#endif // DDT_READ_CGAL_HPP
