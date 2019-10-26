// #ifndef DDT_WRITE_GEOJSON_HPP
// #define DDT_WRITE_GEOJSON_HPP

// #include <iostream>
// #include <string>
// #include <map>
// #include <fstream>
// #include <stdio.h>
// #include <stdlib.h>
// #include <set>
// #define BOOST_NO_CXX11_SCOPED_ENUMS
// #include <boost/filesystem.hpp>
// #undef BOOST_NO_CXX11_SCOPED_ENUMS
// #include "conf_header/conf.hpp"


// // https://en.wikipedia.org/wiki/GeoJSON
// namespace ddt
// {


// template<typename Iterator>
// void write_geojson_facet_range(Iterator begin, Iterator end, std::ostream & ofs, bool is_first = true)
// {

//     typedef typename Iterator::value_type Facet_const_iterator;
//     typedef typename Facet_const_iterator::Traits Traits;
//     int D = Traits::D;
//     for(auto fit = begin; fit != end; ++fit)
//     {

//         //if(fit->is_infinite()) continue;
//         if(!is_first)
//             ofs << "," << std::endl;

//         auto cit = fit->full_cell();
//         int idx = fit->index_of_covertex();
//         ofs << "{" << std::endl;
//         ofs << "\"type\": \"Feature\"," << std::endl;
//         ofs << "\"geometry\": {" << std::endl;
//         ofs << "\"type\": \"LineString\"," << std::endl;
//         ofs << "\"coordinates\": [[";
//         int local = 0;
//         int j = 0;
//         for(int i=0; i<=D; ++i)
//         {
//             if(i == idx) continue;
//             auto v = cit->vertex(i);
//             local += v->is_local();
//             for(int d=0; d<D; ++d)
//             {
//                 ofs << v->point()[d];
//                 if(d < 1)
//                     ofs << ",";
//             }
//             if (++j < D) ofs << "],[";
//         }
//         ofs << "]]";
//         ofs << "}," << std::endl;
//         ofs << "\"properties\": {" << std::endl;
//         ofs << "\"is_local\": " << int(fit->is_local()) << "," << std::endl;
//         ofs << "\"local_score\": " << int(fit->local_score()) << "," << std::endl;
//         ofs << "\"is_infinite\": " << int(fit->is_infinite()) << "," << std::endl;
//         ofs << "\"is_main\": " << int(fit->is_main()) << "," << std::endl;
//         ofs << "\"main_id\": " <<  int(fit->main_id())  << "," << std::endl;
//         ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
//         ofs << "}" << std::endl;
//         ofs << "}" << std::endl;


//     }
// }

// template<typename Iterator>
// void write_geojson_vert_range(Iterator begin, Iterator end, std::ostream & ofs, bool is_first = true)
// {
//     typedef typename Iterator::value_type Vertex_const_iterator;
//     typedef typename Vertex_const_iterator::Traits Traits;
//     int D = Traits::D;
//     for(auto vit = begin; vit != end; ++vit)
//     {
//         if(vit->is_infinite()) continue;
//         if(!is_first)
//             ofs << "," << std::endl;
//         is_first=false;
//         ofs << "{" << std::endl;
//         ofs << "\"type\": \"Feature\"," << std::endl;
//         ofs << "\"geometry\": {" << std::endl;
//         ofs << "\"type\": \"Point\"," << std::endl;
//         ofs << "\"coordinates\": [";
//         for(int d=0; d<D-1; ++d)
//             ofs << vit->point()[d] << ",";
//         ofs << vit->point()[D-1] << "]" << std::endl;;
//         ofs << "}," << std::endl;
//         ofs << "\"properties\": {" << std::endl;
//         ofs << "\"fill\":" << (vit->is_local() ? "\"red\"" : "\"blue\"") <<  "," << std::endl;
//         ofs << "\"tid\": " << int(vit->tile()->id()) <<  "," << std::endl;
//         ofs << "\"main_id\": " <<  int(vit->main_id())  << std::endl;
//         ofs << "}" << std::endl;
//         ofs << "}" << std::endl;
//     }
// }





// template<typename Iterator>
// void write_geojson_cell_range(Iterator begin, Iterator end, std::ostream & ofs,bool is_first = true)
// {

//     typedef typename Iterator::value_type Cell_const_iterator;
//     typedef typename Cell_const_iterator::Traits Traits;
//     std::map<Cell_const_iterator, int> cmap;
//     int nextid = 0;
//     int D = Traits::D;
//     for(auto iit = begin; iit != end; ++iit)
//     {
//         //if(iit->is_infinite()) continue;
//         if(!is_first)
//             ofs << "," << std::endl;
//         is_first=false;
//         ofs << "{" << std::endl;
//         ofs << "\"type\": \"Feature\"," << std::endl;
//         ofs << "\"geometry\": {" << std::endl;
//         ofs << "\"type\": \"Polygon\"," << std::endl;
//         ofs << "\"coordinates\": [" << std::endl;
//         int local = 0;
//         ofs << "[[";
//         for(int i=0; i<=D+1; ++i) // repeat first to close the polygon
//         {
//             auto v = iit->vertex(i % (D+1));
//             if(i>0)
//             {
//                 ofs << "],[";
//                 local += v->is_local();
//             }
//             auto p = v->point();
//             for(int d=0; d<D-1; ++d) ofs << p[d] << ",";
//             ofs << p[D-1];
//         }
//         ofs << "]]";
//         ofs << "]";
//         ofs << "}," << std::endl;
//         ofs << "\"properties\": {" << std::endl;
//         ofs << "\"local_score\": " << int(iit->local_score()) << "," << std::endl;
//         ofs << "\"is_local\": " << int(iit->is_local()) << "," << std::endl;
//         ofs << "\"is_infinite\": " << int(iit->is_infinite()) << "," << std::endl;
//         ofs << "\"main_id\": " <<  int(iit->main_id())  << "," << std::endl;
//         ofs << "\"tid\": " << int(iit->tile()->id())  << "," << std::endl;
//         ofs << "\"data_id\": " << int(iit->cell_data().id)  << "," << std::endl;

//         if(true)
//         {

//             if(!cmap.count(*iit)) cmap[*iit] = nextid++;
//             ofs << "\"opt_id\": " << cmap[*iit] << "," << std::endl;
//             for(int i = 0 ; i < D+1; i++)
//             {
//                 int iid = -1;
//                 // Maybe neighbors does not exists if not loaded completly
//                 try
//                 {
//                     auto nb0 = iit->neighbor(i);
//                     auto n0 = nb0->main();
//                     if(!cmap.count(n0)) cmap[n0] = nextid++;
//                     iid = cmap[n0];
//                 }
//                 catch (...)
//                 {

//                 }
//                 ofs << "\"neigbhor " << i << "\": " << iid << "," << std::endl;
//             }
//         }



//         //	iit->data().dump_geojson(ofs);
//         ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
//         ofs << "}" << std::endl;
//         ofs << "}" << std::endl;
//     }
// }



// template<typename DDT>
// void write_geojson_tri(const DDT& ddt, std::ostream & ofs)
// {

//     ofs << "{" << std::endl;
//     ofs << "\"type\": \"FeatureCollection\"," << std::endl;
//     ofs << "\"features\": [" << std::endl;
//     write_geojson_vert_range(ddt.vertices_begin(), ddt.vertices_end(), ofs,true);
//     write_geojson_cell_range(ddt.cells_begin(), ddt.cells_end(), ofs,false);
//     write_geojson_facet_range(ddt.facets_begin(), ddt.facets_end(), ofs,false);
//     ofs << "]" << std::endl;
//     ofs << "}" << std::endl;
// }


// template<typename tile>
// void write_geojson_tile(const tile& tt, std::ostream & ofs)
// {

//     ofs << "{" << std::endl;
//     ofs << "\"type\": \"FeatureCollection\"," << std::endl;
//     ofs << "\"features\": [" << std::endl;
//     write_geojson_vert_range(tt.vertices_begin(), tt.vertices_end(), ofs,true);
//     write_geojson_cell_range(tt.cells_begin(), tt.cells_end(), ofs,false);
//     ofs << "]" << std::endl;
//     ofs << "}" << std::endl;
// }





// }

// #endif // DDT_WRITE_GEOJSON_HPP
