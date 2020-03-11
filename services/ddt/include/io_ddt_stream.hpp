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
        ddt_data<typename DDTT::Traits> w_datas;
        bool do_serialize = false;
        bool do_clean_data = true;
        if(hpi.get_lab() == "t")
        {
            read_ddt_stream(ddt,w_datas, hpi.get_input_stream(), hpi.get_id(0),do_serialize,do_clean_data,log);
        }
    }
    Scheduler sch(1);
    ddt.finalize(sch);
    return 0; // FIXME ?
}



template<typename DDT>
int read_ddt_stream(DDT & ddt, ddt_data<typename DDT::Traits> & w_datas_in, std::istream & ifile, typename DDT::Id tid,bool do_serialize, bool do_clean_data, ddt::logging_stream & log)
{


    typename DDT::Traits ttraits;
    ddt.init(tid);
    auto  tile  = ddt.get_tile(tid);
    typename DDT::Traits::Delaunay_triangulation & ttri = tile->tri();

    
    if(do_serialize)
    {

      //	if(!do_clean_data){
      //	  w_datas_in.read_b64_generic_stream(ifile);
	  //	  ttraits.build_tri_from_data(ttri,w_datas_in,false,tid);
      //	}else{
      ttraits.deserialize_b64_cgal(ttri,ifile);
      deserialize_b64_vect(tile->tile_ids,ifile);
	  //	}
	
      std::string input;
      std::getline(ifile, input);
      std::stringstream ifile2(input);
      ddt::read_map_stream(tile->points_sent_,ifile2,tile->traits());
      ddt::read_json_stream<typename DDT::Tile_iterator, typename DDT::Id>(tile,ifile2);



	
    }
    else
    {
        log.step("[read]read_ply_stream");
        w_datas_in.read_ply_stream(ifile,PLY_CHAR);
        log.step("[read]build_tri_from_data");
        ttraits.build_tri_from_data(ttri,w_datas_in,do_clean_data,tid);
        log.step("[read]build_json");
	ddt::read_map_stream(tile->points_sent_,ifile,tile->traits());
	ddt::read_json_stream<typename DDT::Tile_iterator, typename DDT::Id>(tile,ifile);
    }


    tile->set_id(tid);
    tile->finalize();
    return 0; // FIXME ?
}



template<typename DDT>
std::ostream & write_ddt_stream(const DDT& ddt, std::ostream & ofile, int tid,bool do_serialize,ddt::logging_stream & log)
{
    //std::cout << "write tile [id:" << tid << "]" << std::endl;
    ddt_data<typename DDT::Traits> w_datas_out;
    //    bool do_clean_data = false;
    return write_ddt_stream(ddt, w_datas_out,ofile, tid,do_serialize,log);

}






template<typename DDT>
std::ostream & write_ddt_stream(const DDT& ddt, ddt_data<typename DDT::Traits> & w_datas_out, std::ostream & ofile, int tid,bool do_serialize, ddt::logging_stream & log)
{
    //std::cout << "write tile [id:" << tid << "]" << std::endl;
    typename DDT::Traits ttraits;
    auto tile  = ddt.get_tile(tid);
    tile->update_local_flag();
    auto ttri = tile->triangulation();



    if(do_serialize)
    {
        ttraits.serialize_b64_cgal(ttri,ofile);
	serialize_b64_vect(tile->tile_ids,ofile);
    }
    else
    {
        ttraits.export_tri_to_data(ttri,w_datas_out);
        w_datas_out.write_ply_stream(ofile,PLY_CHAR);
    }
    //tile->write_cgal(ofile);
    ddt::write_map_stream(tile->points_sent_,ofile,tile->current_dimension());
    ddt::write_json_stream(tile,ofile);



    return ofile; // FIXME ?
}

}
#endif
