#ifndef IO_DDT_STREAM_HPP
#define IO_DDT_STREAM_HPP

#include "ddt_data.hpp"
#include "io/write_stream.hpp"
#include "io/read_stream.hpp"
#include "io/logging_stream.hpp"


namespace ddt
{


  
template<typename DDTT,typename Scheduler >
int read_ddt_full_stream(DDTT & ddt, std::istream & ifile, int nb_dat,ddt::logging_stream & log)
{

    for(int i = 0; i < nb_dat; i++)
    {
        stream_data_header hpi;
        hpi.parse_header(ifile);
        bool do_serialize = false;
        bool do_clean_data = true;
        if(hpi.get_lab() == "t")
        {
            read_ddt_stream(ddt,hpi.get_input_stream(), hpi.get_id(0),do_serialize,do_clean_data,log);
        }
    }
    Scheduler sch(1);
    ddt.finalize(sch);
    return 0; // FIXME ?
}



  template<typename DDT,typename WD>
  int read_ddt_stream(DDT & ddt, WD & wd,  std::istream & ifile, typename DDT::Id tid,bool do_serialize, bool do_clean_data, ddt::logging_stream & log)
{
    read_ddt_stream(ddt,ifile, tid,do_serialize,  do_clean_data, log);
    wd.read_serialized_stream(ifile);
}


template<typename DDT>
int read_ddt_stream(DDT & ddt,  std::istream & ifile, typename DDT::Id tid,bool do_serialize, bool do_clean_data, ddt::logging_stream & log)
{


    typename DDT::Traits ttraits;
    ddt.init(tid);
    auto  tile  = ddt.get_tile(tid);
    typename DDT::Traits::Delaunay_triangulation & ttri = tile->tri();

    ttraits.deserialize_b64_cgal(ttri,ifile);
    deserialize_b64_vect(tile->tile_ids,ifile);
	
      std::string input;
      std::getline(ifile, input);
      std::stringstream ifile2(input);
      ddt::read_map_stream(tile->points_sent_,ifile2,tile->traits());
      ddt::read_json_stream<typename DDT::Tile_iterator, typename DDT::Id>(tile,ifile2);



    tile->set_id(tid);
    tile->finalize();
    return 0; // FIXME ?
}

  template<typename DDT,typename WD>
std::ostream & write_ddt_stream(const DDT& ddt, const WD& wd , std::ostream & ofile, int tid,bool do_serialize,ddt::logging_stream & log)
{
  write_ddt_stream(ddt, ofile, tid,do_serialize, log);
  wd.write_serialized_stream(ofile);
  return ofile;
}


template<typename DDT>
std::ostream & write_ddt_stream(const DDT& ddt, std::ostream & ofile, int tid,bool do_serialize,ddt::logging_stream & log)
{

    //std::cout << "write tile [id:" << tid << "]" << std::endl;
    typename DDT::Traits ttraits;
    auto tile  = ddt.get_tile(tid);
    tile->update_local_flag();
    auto ttri = tile->triangulation();


        ttraits.serialize_b64_cgal(ttri,ofile);
	serialize_b64_vect(tile->tile_ids,ofile);


    //tile->write_cgal(ofile);
    ddt::write_map_stream(tile->points_sent_,ofile,tile->current_dimension());
    ddt::write_json_stream(tile,ofile);



    return ofile; // FIXME ?
}

}
#endif
