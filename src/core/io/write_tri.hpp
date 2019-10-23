#ifndef DDT_WRITE_TRI_HPP
#define DDT_WRITE_TRI_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace ddt
{

template<typename Tile>
std::ostream & write_json(Tile & tile,std::string filename,std::ostream & ofile)
{

    const int D = tile.dimension();

    //ss <<
    boost::property_tree::ptree root_node;
    boost::property_tree::ptree bbox_node;
    root_node.put("filename", filename);
    root_node.put("id", tile.id());
    auto & bbox = tile.bbox();
    for(auto iter = bbox.begin(); iter != bbox.end(); ++iter)
    {
        std::stringstream ss;
        ss << iter->second;
        bbox_node.put(std::to_string(iter->first),ss.str());
    }

    root_node.add_child("bbox", bbox_node);
    boost::property_tree::write_json(ofile, root_node);

    return ofile;
}

template<typename Tile>
std::istream& read_json(Tile & tile,std::istream&  ifile)
{

    const int D = tile.dimension();
    boost::property_tree::ptree root_node;
    boost::property_tree::read_json(ifile, root_node);

    for (auto its : root_node.get_child("bbox"))
    {
        std::cout <<  "bbox reading :" << its.first << " " << its.second.data() << std::endl;
    }

    return ifile;
}


template<typename Tile>
std::function<int(Tile&, bool)>
write_tri(std::string dirname)
{

    return [dirname](Tile& tile, bool /*unused*/)
    {
        std::string filename = dirname + "/" + std::to_string(tile.id() ) + ".bin";
        std::string json_name = dirname + "/" + std::to_string(tile.id() ) + ".json";
        std::ofstream ofile_tri(filename, std::ios::binary | std::ios::out);
        std::ofstream ofile_json(json_name, std::ios::out);
        if(!ofile_tri.is_open())
        {
            std::cerr << "dump_tri_binary : File could not be opened" << std::endl;
            std::cerr << filename << std::endl;
            return 1;
        }

        write(tile,ofile_tri);
        ofile_tri.close();
        write_json(tile,filename,ofile_json);
        ofile_json.close();

        return 0;
    };
}

template<typename Tile>
std::function<int(Tile&, bool)>
read_tri(std::string dirname)
{

    return [dirname](Tile& tile, bool /*unused*/)
    {
        std::string filename = dirname + "/" + std::to_string(tile.id() ) + ".bin";
        std::string json_name = dirname + "/" + std::to_string(tile.id() ) + ".json";
        std::ifstream ifile_tri(filename, std::ios::binary | std::ios::out);
        std::ifstream ifile_json(json_name, std::ifstream::in);
        read_json(tile,ifile_json);
        ifile_json.close();

        read(tile,ifile_tri);
        return 0;
    };
}

}

#endif // DDT_WRITE_TRI_HPP
