#include "wasure_typedefs.hpp"
#include "write_geojson_wasure.hpp"

#include "io/stream_api.hpp"
#include "io/write_stream.hpp"
#include "io/write_vrt.hpp"
#include "io/read_stream.hpp"
#include "io/logging_stream.hpp"

//#include "main_stream.hpp"

//#include "write_surface.hpp"
#include "wasure_data.hpp"
#include "wasure_algo.hpp"
#include "tbmrf_reco.hpp"
#include "tbmrf_conflict.hpp"
#include "io_ddt_stream.hpp"
#include "graph_cut.hpp"
#include "ddt_spark_utils.hpp"


typedef std::map<Id,wasure_data<Traits> > D_MAP;
typedef std::map<Id,std::list<wasure_data<Traits>> > D_LMAP;
typedef std::tuple<Id,double,double,double>                                SharedData;
typedef std::tuple<Id,double,double,double,double,double,double>           SharedDataDst;
typedef Id                                EdgeData;


int write_id_double_serialized(const std::map<Id,SharedData>  & lp, std::ostream & ofile,bool do_print = false)
{
    do_print = false;
    std::vector<double> outputv;
    for(auto pp : lp)
    {
      outputv.emplace_back(pp.first);
      outputv.emplace_back(std::get<0>(pp.second));
      outputv.emplace_back(std::get<1>(pp.second));
      outputv.emplace_back(std::get<2>(pp.second));
      outputv.emplace_back(std::get<3>(pp.second));
      if(do_print){
	std::cerr << "WRITE10_TUPLE_" << pp.first << " ";
	std::cerr << std::get<0>(pp.second) << " ";
	std::cerr << std::get<1>(pp.second) << " ";
	std::cerr << std::get<2>(pp.second) << " ";
	std::cerr << std::get<3>(pp.second) << " ";
	std::cerr << std::endl;
      }
    }
    serialize_b64_vect(outputv,ofile);
    return 0;
}

int write_id_dst_serialized(const std::map<Id,SharedDataDst>  & lp, std::ostream & ofile,bool do_print = false)
{

    std::vector<double> outputv;
    for(auto pp : lp)
    {
      outputv.emplace_back(pp.first);
      outputv.emplace_back(std::get<0>(pp.second));
      outputv.emplace_back(std::get<1>(pp.second));
      outputv.emplace_back(std::get<2>(pp.second));
      outputv.emplace_back(std::get<3>(pp.second));
      outputv.emplace_back(std::get<4>(pp.second));
      outputv.emplace_back(std::get<5>(pp.second));
      outputv.emplace_back(std::get<6>(pp.second));
    }
    serialize_b64_vect(outputv,ofile);
    return 0;
}



std::istream & read_id_dst_serialized(std::map<Id,SharedDataDst> & lp, std::istream & ifile, bool do_print = false)
{
  do_print = false;
  std::vector<double> input_v;
  deserialize_b64_vect(input_v,ifile);
  int nbe = 8;
  for(int n = 0; n < input_v.size()/nbe;n++){
    Id id1 = input_v[n*nbe];
    // id - bary - dst
    lp[id1] = std::make_tuple(input_v[n*nbe+1],input_v[n*nbe+2],input_v[n*nbe+3],input_v[n*nbe+4],input_v[n*nbe+5],input_v[n*nbe+6],input_v[n*nbe+7]);
  }					     
  return ifile;
}



std::istream & read_id_double_serialized(std::map<Id,SharedData> & lp, std::istream & ifile, bool do_print = false)
{
  do_print = false;
  std::vector<double> input_v;
  deserialize_b64_vect(input_v,ifile);
  int nbe = 5;
  for(int n = 0; n< input_v.size()/nbe;n++){
    Id id1 = input_v[n*nbe];
    lp[id1] = std::make_tuple(input_v[n*nbe+1],input_v[n*nbe+2],input_v[n*nbe+3],input_v[n*nbe+4]);
    if(do_print){
      std::cerr << "READ10_TUPLE_" << id1 << " ";
      std::cerr << std::get<0>(lp[id1]) << " ";
      std::cerr << std::get<1>(lp[id1]) << " ";
      std::cerr << std::get<2>(lp[id1]) << " ";
      std::cerr << std::get<3>(lp[id1]) << " ";
      std::cerr << std::endl;
    }
    
  }					     
  return ifile;
}


int write_edges_serialized(const std::map<Id,EdgeData>  & lp, std::ostream & ofile)
{

    std::vector<double> outputv;
    for(auto pp : lp)
    {
      outputv.emplace_back(pp.first);
      outputv.emplace_back(pp.second);
    }
    serialize_b64_vect(outputv,ofile);
    return 0;
}



std::istream & read_edges_serialized(std::map<Id,EdgeData> & lp, std::istream & ifile)
{
  std::vector<double> input_v;
  deserialize_b64_vect(input_v,ifile);
  int nbe = 2;
  for(int n = 0; n< input_v.size()/nbe;n++){
    Id id1 = input_v[n*nbe];
    lp[id1] = input_v[n*nbe+1];
  }					     
  return ifile;
}




//typedef typename Traits::Full_cell_const_iterator                 Cell_const_iterator;


// template<typename DDT>
// void init_global_id(const DDT& ddt, D_MAP & w_datas_tri, int tid)
// {
//     typedef typename DDT::Traits Traits;
//     typedef typename Traits::Flag_C                    Flag_C;
//     typedef typename Traits::Data_C                    Data_C;
//     int nextid = 0;
//     int D = Traits::D;
//     int acc = 0;
//     std::vector<int>  & format_gids = w_datas_tri[tid].format_gids;
//     int nbs = w_datas_tri[tid].nb_simplex_shpt_vect();
//     format_gids.resize(nbs);
//     for(int i = 0; i < nbs; i++)
//         format_gids[i] = -1;
//     for(auto iit = ddt.cells_begin(); iit != ddt.cells_end(); ++iit)
//     {
//         int local = 0;
//         const Data_C & cd = iit->cell_data();
//         Data_C & cd_quickndirty = const_cast<Data_C &>(cd);
//         if(cd_quickndirty.flag > 0)
//             format_gids[cd_quickndirty.id] = w_datas_tri[tid].tile_ids[2] + (acc++);
//         //cd_quickndirty.id = acc++;
//     }
//     w_datas_tri[tid].fill_gids(w_datas_tri[tid].format_gids);
// }


int simplify(Id tid,wasure_params & params,int nb_dat)
{
    std::cout.setstate(std::ios_base::failbit);

    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        if(hpi.get_lab() == "p")
        {
            ddt_data<Traits> w_datas;
            w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
            hpi.finalize();

            std::cout.clear();
            Id id = hpi.get_id(0);
            ddt::stream_data_header oqh("p","s",id);
            std::string filename(params.output_dir + "/" + params.slabel +"_id_"+ std::to_string(tid) + "_" + std::to_string(id));
            if(params.dump_ply)
                oqh.write_into_file(filename,".ply");
            oqh.write_header(std::cout);
            w_datas.write_ply_stream(oqh.get_output_stream(),PLY_CHAR);
            oqh.finalize();
            std::cout << std::endl;
        }
    }
    return 0;
}


int preprocess(Id tid,wasure_params & params, int nb_dat)
{
    std::cout.setstate(std::ios_base::failbit);
    Traits  traits;
    wasure_algo w_algo;
    int D = Traits::D;
    int max_ppt = 300000;
    std::map<Id,wasure_data<Traits> > datas_map;
    std::map<Id,std::string > fname_map;
    std::map<Id,std::string > fname_map2;

    ddt::Bbox<Traits::D> full_bbox;
    int full_nbp = 0;
    
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
	//        wasure_data<Traits> w_datas;
        hpi.parse_header(std::cin);
	Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "p")
        {
            datas_map[hid].read_ply_stream(hpi.get_input_stream());
        }
        if(hpi.get_lab() == "g")
        {
            datas_map[hid].read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
        }

        hpi.finalize();

        std::cerr << "hpi finalized" << std::endl;
        int count = datas_map[hid].nb_pts_shpt_vect();
        std::vector<bool> do_keep(count,false);
	fname_map[hid] = get_bname(hpi.get_file_name());



	std::vector<double> doubleVect;
	if(datas_map[hid].dmap[datas_map[hid].xyz_name].type == tinyply::Type::FLOAT64){
	  datas_map[hid].dmap[datas_map[hid].xyz_name].extract_full_shpt_vect(doubleVect,false);
	}else{
	  std::vector<float> v_fxyz;
	  datas_map[hid].dmap[datas_map[hid].xyz_name].extract_full_shpt_vect(v_fxyz,false);
	  doubleVect.insert(doubleVect.end(),v_fxyz.begin(),v_fxyz.end());
	  datas_map[hid].dmap[datas_map[hid].xyz_name].type = tinyply::Type::FLOAT64;

	}


	for(int n = 0; n < doubleVect.size(); n+=D){
	  auto pp = traits.make_point(doubleVect.begin() + n);
	  full_bbox += pp;
	  full_nbp++;
	}

	std::cerr << "bbox_preprocess:" << full_bbox << std::endl;
	datas_map[hid].dmap[datas_map[hid].xyz_name].fill_full_uint8_vect(doubleVect);

	// if(true){
	//   count = datas_map[hid].nb_pts_uint8_vect();
	//   datas_map[hid].dmap[datas_map[hid].glob_scale_name] = ddt_data<Traits>::Data_ply(datas_map[hid].glob_scale_name,"vertex",1,1,DATA_FLOAT_TYPE);
	//   std::vector<double> glob_scale(count,50);
	//   datas_map[hid].dmap[datas_map[hid].glob_scale_name].fill_full_uint8_vect(glob_scale);
	// }

	if(!datas_map[hid].dmap[datas_map[hid].center_name].do_exist){
	  std::cerr << "NO CENTER : " << fname_map[hid] << std::endl;
	  datas_map[hid].dmap[datas_map[hid].center_name] = ddt_data<Traits>::Data_ply(datas_map[hid].center_name,"vertex",D,D,DATA_FLOAT_TYPE);
	  datas_map[hid].dmap[datas_map[hid].flags_name] = ddt_data<Traits>::Data_ply(datas_map[hid].flags_name,"vertex",1,1,tinyply::Type::INT32);

	  std::vector<double> v_xyz;
	  std::vector<double> v_center;
	  std::vector<int> v_flags;

	  datas_map[hid].dmap[datas_map[hid].xyz_name].extract_raw_uint8_vect(v_xyz,false);
	  std::cerr << "extract done " << std::endl;

	  std::vector<float> v_angle,s_flag,num_r;
	  
	  datas_map[hid].dmap[std::vector<std::string>({"ScanAngleRank"})].extract_full_shpt_vect(v_angle,false);
	  datas_map[hid].dmap[std::vector<std::string>({"ScanDirectionFlag"})].extract_full_shpt_vect(s_flag,false);
	  datas_map[hid].dmap[std::vector<std::string>({"NumberOfReturns"})].extract_full_shpt_vect(num_r,false);

	  // 45 y , -45x
	  // -19 == 26 (-33n
	  double dr = 0.3925;
	  for(int jj = 0; jj < v_xyz.size()/D; jj++)
	    {
	      //	      double aa = (s_flag[jj] == 0) ? dr : dr + 3.14/2.0;
	      double aa =  dr;
	      double angle = v_angle[jj]*3.14/180.0;
	      double vx = v_xyz[jj*D];
	      double vy = v_xyz[jj*D+1];
	      double vz = v_xyz[jj*D+2];

	      double lx = sin(-angle);
	      double ly = 0;
	      double lz = cos(-angle);
	      double dist = 50;
	      v_center.push_back(vx + dist*lx*cos(aa));
	      v_center.push_back(vy + dist*lx*sin(aa));
	      v_center.push_back(vz + dist*lz);
	      int flag = 1;
	      if(num_r[jj] > 1)
		flag = -1;
	      v_flags.push_back(flag);
	      
	      //	      glob_scale.push_back(10);
	      //v_center.push_back(alt*0.1 + 0.52*);

	    }
	  std::cerr << "loop done" << std::endl;
	  datas_map[hid].dmap[datas_map[hid].center_name].fill_full_uint8_vect(v_center);
	  datas_map[hid].dmap[datas_map[hid].flags_name].fill_full_uint8_vect(v_flags);

	  std::cerr << "fill done" << std::endl;
	  
	}else{
	  int count = datas_map[hid].nb_pts_uint8_vect();
	  std::vector<int> v_flags(count,0);
	  datas_map[hid].dmap[datas_map[hid].flags_name] = ddt_data<Traits>::Data_ply(datas_map[hid].flags_name,"vertex",1,1,tinyply::Type::INT32);
	  datas_map[hid].dmap[datas_map[hid].flags_name].fill_full_uint8_vect(v_flags);

	  if(datas_map[hid].dmap[datas_map[hid].center_name].type == tinyply::Type::FLOAT64){
	    datas_map[hid].dmap[datas_map[hid].center_name].shpt_vect2uint8_vect();
	  }else{
	    std::vector<float> v_fcenters;
	    datas_map[hid].dmap[datas_map[hid].center_name].extract_full_shpt_vect(v_fcenters,false);
	    std::vector<double> doubleVec(v_fcenters.begin(),v_fcenters.end());

	    datas_map[hid].dmap[datas_map[hid].center_name].type = tinyply::Type::FLOAT64;
	    datas_map[hid].dmap[datas_map[hid].center_name].fill_full_uint8_vect(doubleVec);
	  }

	}

	std::cerr << "yo" << std::endl;
	for (  auto &ee : datas_map[hid].dmap ) {
	  if(ee.second.do_exist){
	    std::cerr << fname_map[hid] << std::endl;
	    ee.second.print_elems(std::cerr);
	    if((! datas_map[hid].dmap[ee.first].has_label("x")) &&
	       (! datas_map[hid].dmap[ee.first].has_label("flags")) &&
	       (! datas_map[hid].dmap[ee.first].has_label("x_origin"))){
	      datas_map[hid].dmap[ee.first].do_exist = false;
	    }
	  }
	}
	// std::cerr << "yo2" << std::endl;	
        // w_algo.simplify(datas_map[hid],do_keep,0.02);
        // int curr_tid = 0;
        // int nb_keep = 0;
        // for(int ii = 0; ii < count ; ii++)
        // {
        //     if(do_keep[ii])
        //     {
        //         Id id = Id(nb_keep/max_ppt);
        //         auto it = datas_map.find(id);
        //         if(it==datas_map.end())
        //         {
        //             datas_map[id] = ddt_data<Traits>(w_datas.dmap);
        //         }
        //         datas_map[id].copy_attribute(w_datas,ii,std::string("x"));
        //         datas_map[id].copy_attribute(w_datas,ii,std::string("x_origin"));
        //         nb_keep++;
        //     }

        // }

    }


    std::cout.clear();
    std::cerr << "count finalized" << std::endl;

    //    if(false){
    std::map<Id,wasure_data<Traits> > datas_map_splitted;
    Id new_id = 0;
    for ( const auto &myPair : datas_map )
    {      
        Id id = myPair.first;
        int nb_out = datas_map[id].nb_pts_uint8_vect();
	std::cerr << "start looping on data :"  << nb_out << std::endl;
	for(int n = 0; n < nb_out; n++)
	  {
	    
	    if(n % max_ppt == 0){

	      new_id++;
	      fname_map2[new_id] = fname_map[id];
	      std::cerr << "new id : " << new_id << std::endl;
	      auto it = datas_map_splitted.find(new_id);
	      if(it==datas_map_splitted.end())
	      {
		std::cerr << "add" << std::endl;
		datas_map_splitted[new_id] = wasure_data<Traits>(datas_map[id].dmap);

	      }
	    }

	    datas_map_splitted[new_id].copy_attribute(datas_map[id],n,std::string("x"));
	    datas_map_splitted[new_id].copy_attribute(datas_map[id],n,std::string("x_origin"));
	    datas_map_splitted[new_id].copy_attribute(datas_map[id],n,std::string("flags"));
	  }
	
    }
    //}
    std::cerr << "Dump result" << std::endl;
    for ( const auto &myPair : datas_map_splitted )
    {
        Id id = myPair.first;
        int nb_out = datas_map_splitted[id].nb_pts_uint8_vect();


	std::string filename(params.output_dir + "/" + fname_map2[id] + "_" + std::to_string(id));
	if(true){
	  ddt::stream_data_header oqh("p","f",id);
	  oqh.write_into_file(filename,".stream");
	  oqh.write_header(std::cout);
	  datas_map_splitted[id].write_serialized_stream(oqh.get_output_stream());
	  oqh.finalize();
	  std::cout << std::endl;
	}
	if(true){
	  ddt::stream_data_header oqh("p","f",id);
	  oqh.write_into_file(filename,"_visu.ply");
	  oqh.write_header(std::cout);
	  datas_map_splitted[id].write_ply_stream(oqh.get_output_stream(),'\n',true);
	  oqh.finalize();
	  std::cout << std::endl;
	}
    }
    // Dump stats
    ddt::stream_data_header sth("s","z",0);
    sth.write_header(std::cout); 
    std::cout <<  full_bbox << " " << full_nbp;
    sth.finalize();
    std::cout << std::endl;


    return 0;
}


// int dim_with_crown(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
// {
//   std::cout.setstate(std::ios_base::failbit);
//   std::cerr << "into fun simp" << std::endl;
//   wasure_algo w_algo;
//   int D = Traits::D;
//   Traits  traits;
//   D_LMAP w_datas_map;
//   D_MAP datas_map_crown;

//   bool do_splitted =  false;
//   wasure_data<Traits>  w_datas_full;
//   std::vector<Point> p_simp_full;

//   /// Quick and fucking dirty for the crown
//   Id ND = params.nbt_side;
//   Id NP = params.nbp;
//   Id NT = pow(ND,D);

//   ddt::Bbox<Traits::D> bbox;
//   std::stringstream ss;
//   ss << params.bbox_string;
//   ss >> bbox;
//   Grid_partitioner part(bbox, ND);
//   double eps_side[Traits::D];
//   for(int d = 0; d > D; d++)
//     eps_side[d] = ((bbox.max(d) - bbox.min(d))/params.nbt_side)*0.1;
//   // End quick and diry crown stuff

  
//   for(int i = 0; i < nb_dat; i++)
//     {
//       ddt::stream_data_header hpi;
//       hpi.parse_header(std::cin);
//       Id hid = hpi.get_id(0);
//       log.step("read");
//       if(hpi.get_lab() == "z" )
// 	{
// 	  w_datas_map[hid].push_back(wasure_data<Traits>());
// 	  if(hpi.is_serialized()){
// 	    std::cerr << "start read ply" << std::endl;
// 	    w_datas_map[hid].back().read_serialized_stream(hpi.get_input_stream());
// 	    //	      w_datas.read_serialized_stream(hpi.get_input_stream());
// 	    //w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
// 	    std::cerr << "end read ply" << std::endl;
// 	  }else{
// 	    w_datas_map[hid].back().read_ply_stream(hpi.get_input_stream());
// 	    w_datas_map[hid].back().shpt2uint8();
// 	  }
// 	  std::cerr << "start read ply" << std::endl;
// 	  //w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
// 	  std::cerr << "end read ply" << std::endl;
// 	}
//       std::cerr << "reading end" << std::endl;
//       wasure_data<Traits> & w_datas = w_datas_map[hid].back();
//       //}
//       hpi.finalize();
//       std::cerr << "finalize" << std::endl;

//       //      log.step("compute");
//       // w_datas.extract_ptsvect(w_datas.xyz_name,w_datas.format_points,false);
//       // w_datas.extract_ptsvect(w_datas.center_name,w_datas.format_centers,false);
//       w_datas.dmap[w_datas.xyz_name].extract_full_uint8_vect(w_datas.format_points,false);
//       std::cerr << "xyz ok" << std::endl;
//       w_datas.dmap[w_datas.center_name].extract_full_uint8_vect(w_datas.format_centers,false);
//       std::cerr << "centers ok " << std::endl;
//       w_datas.extract_flags(w_datas.format_flags,false);
//       std::cerr << "flags ok " << std::endl;
//       int acc = 0;
//       std::cerr << "center ok" << std::endl;
//       std::vector<Point> p_simp;    
//       // if(params.pscale >= 0)
//       // {
//       //     w_algo.compute_dim_with_simp(w_datas.format_points,
//       //                                  w_datas.format_egv,
//       //                                  w_datas.format_sigs,
//       //                                  p_simp,
//       //                                  params.pscale);
//       // }
//       // else
//       // {

//       if(w_datas.format_centers.size() == 0)
// 	{
// 	  double coords[Traits::D];
// 	  for(auto pp : w_datas.format_points)
// 	    {
// 	      for(int d = 0; d < D; d++)
// 		{
// 		  if(d < D-1)
// 		    coords[d] = pp[d];
// 		  else
// 		    coords[d] = pp[d] + 30;
// 		}
// 	      w_datas.format_centers.push_back(traits.make_point(coords));
// 	    }
// 	}

//       if(do_splitted){
// 	w_algo.compute_dim(w_datas.format_points,
// 			   w_datas.format_egv,
// 			   w_datas.format_sigs);
	
// 	w_algo.flip_dim_ori(w_datas.format_points,
// 			    w_datas.format_egv,
// 			    w_datas.format_centers);

// 	std::cerr << i << "~~ ==" << w_datas.format_points.size() << " " << w_datas.format_points.size() << std::endl;
// 	std::cerr << i << "~~ ==" << w_datas.format_egv.size() << " " << w_datas.format_sigs.size() << std::endl;
// 	std::cerr << i << "~~ ==" << w_datas.format_centers.size() << " " << w_datas.format_centers.size() << std::endl;
// 	w_datas_full.format_points.insert(w_datas_full.format_points.end(),w_datas.format_points.begin(),w_datas.format_points.end());
// 	w_datas_full.format_egv.insert(w_datas_full.format_egv.end(),w_datas.format_egv.begin(),w_datas.format_egv.end());
// 	w_datas_full.format_sigs.insert(w_datas_full.format_sigs.end(),w_datas.format_sigs.begin(),w_datas.format_sigs.end());
// 	p_simp_full.insert(p_simp_full.end(),p_simp.begin(),p_simp.end());
// 	std::cerr << "inserted" << std::endl;
//       }else{
// 	if(i == 0){
// 	  w_datas_full = w_datas;
// 	}else{
// 	  w_datas_full.insert(w_datas);
// 	}
//       }
//     }

//   if(!do_splitted){
//     w_datas_full.dmap[w_datas_full.xyz_name].extract_full_uint8_vect(w_datas_full.format_points,false);
//     w_datas_full.dmap[w_datas_full.center_name].extract_full_uint8_vect(w_datas_full.format_centers,false);
//     w_algo.compute_dim(w_datas_full.format_points,
// 		       w_datas_full.format_egv,
// 		       w_datas_full.format_sigs);
    
//     if(w_datas_full.format_centers.size() == 0)
//       {
// 	double coords[Traits::D];
// 	for(auto pp : w_datas_full.format_points)
// 	  {
// 	    for(int d = 0; d < D; d++)
// 	      {
// 		if(d < D-1)
// 		  coords[d] = pp[d];
// 		else
// 		  coords[d] = pp[d] + 30;
// 	      }
// 	    w_datas_full.format_centers.push_back(traits.make_point(coords));
// 	  }
//       }

	
//     w_algo.flip_dim_ori(w_datas_full.format_points,
// 			w_datas_full.format_egv,
// 			w_datas_full.format_centers);
//   }
    
//   std::cerr << "start tessel" << std::endl;
//   if(params.pscale < 1){
//     w_algo.tessel_adapt(w_datas_full.format_points,
// 		      p_simp_full,
// 		      w_datas_full.format_egv,
// 		      w_datas_full.format_sigs,
// 		      20,params.pscale,D,tid
// 		      );
//   }else{
//     p_simp_full.insert(p_simp_full.end(),w_datas_full.format_points.begin(),w_datas_full.format_points.end());
    
//   }



//   // Creating the crown
//   // Adding crown
//   int count = w_datas_full.nb_pts();
//   std::cerr << "start extract crown with " << count <<  " pts" << std::endl;
//   // for(; count != 0; --count)
//   //   {
//   //     Point  p = w_datas_full.format_points[count];
//   //     std::vector<double> coords1(Traits::D);
//   //     std::vector<double> coords2(Traits::D);
//   //     for(int d1 = 0; d1 < D; d1++){
//   // 	for(int d2 = 0 ; d2 < D; d2++)
//   // 	  coords1[d2] = coords2[d2] = 0;
//   // 	for(int d2 = 0 ; d2 < D; d2++){
//   // 	  if(d1 != d2)
//   // 	    coords1[d2] = coords2[d2] = p[d2];
//   // 	  else{
//   // 	    coords1[d2] = p[d2] + eps_side[d2];
//   // 	    coords2[d2] = p[d2] - eps_side[d2];
//   // 	  }
//   // 	}

//   // 	auto np1 = traits.make_point(coords1.begin());
//   // 	auto np2 = traits.make_point(coords2.begin());

//   // 	Id pp1 = part(np1);
//   // 	Id id1 = Id(pp1 % NT);
//   // 	Id pp2 = part(np2);
//   // 	Id id2 = Id(pp2 % NT);


//   // 	if(id1 != tid){
//   // 	  auto it1 = datas_map_crown.find(id1);
//   // 	  if(it1==datas_map_crown.end())
//   // 	    datas_map_crown[id1] = wasure_data<Traits>(w_datas_full.dmap);
//   // 	  datas_map_crown[id1].copy_point(w_datas_full,count);
//   // 	}
//   // 	if(id2 != tid){
//   // 	  auto it2 = datas_map_crown.find(id2);
//   // 	  if(it2==datas_map_crown.end())
//   // 	    datas_map_crown[id2] = wasure_data<Traits>(w_datas_full.dmap);
//   // 	  datas_map_crown[id2].copy_point(w_datas_full,count);
//   // 	}
//   //     }
//   //   }


//     // for(; count != 0; --count)
//     // {
//     //   Point  p = w_datas_full.format_points[count];
//     //   std::vector<double> coords1(Traits::D);

//     //   for(int d1 = 0; d1 < D; d1++){
//     // 	for(int d2 = 0 ; d2 < D; d2++)
//     // 	  coords1[d2] = coords2[d2] = 0;
//     // 	for(int d2 = 0 ; d2 < D; d2++){
//     // 	  if(d1 != d2)
//     // 	    coords1[d2] = coords2[d2] = p[d2];
//     // 	  else{
//     // 	    coords1[d2] = p[d2] + eps_side[d2];
//     // 	    coords2[d2] = p[d2] - eps_side[d2];
//     // 	  }
//     // 	}

//     // 	auto np1 = traits.make_point(coords1.begin());
//     // 	auto np2 = traits.make_point(coords2.begin());

//     // 	Id pp1 = part(np1);
//     // 	Id id1 = Id(pp1 % NT);
//     // 	Id pp2 = part(np2);
//     // 	Id id2 = Id(pp2 % NT);


//     // 	if(id1 != tid){
//     // 	  auto it1 = datas_map_crown.find(id1);
//     // 	  if(it1==datas_map_crown.end())
//     // 	    datas_map_crown[id1] = wasure_data<Traits>(w_datas_full.dmap);
//     // 	  datas_map_crown[id1].copy_point(w_datas_full,count);
//     // 	}
//     // 	if(id2 != tid){
//     // 	  auto it2 = datas_map_crown.find(id2);
//     // 	  if(it2==datas_map_crown.end())
//     // 	    datas_map_crown[id2] = wasure_data<Traits>(w_datas_full.dmap);
//     // 	  datas_map_crown[id2].copy_point(w_datas_full,count);
//     // 	}
//     //   }
//     // }

  

//   if(do_splitted){
//     for ( auto it = w_datas_map.begin(); it != w_datas_map.end(); it++ )
//       {
// 	int acc = 0;
// 	for(auto & w_datas : w_datas_map[it->first])
// 	  {
      

// 	    //w_datas.dmap[w_datas.egv_name].fill_full_uint8_vect(w_datas.format_egv);
// 	    w_datas.fill_egv(w_datas.format_egv);
// 	    //w_datas.dmap[w_datas.sig_name].fill_full_uint8_vect(w_datas.format_sigs);
// 	    w_datas.fill_sigs(w_datas.format_sigs);


// 	    std::cerr << "dim done tile : "<< tid << std::endl;
// 	    std::string ply_name(params.output_dir +  "/" + params.slabel + "_id_" + std::to_string(it->first) + "_" + std::to_string(acc++) +  "_dim");
// 	    std::cout.clear();


// 	    log.step("write");
// 	    ddt::stream_data_header oth("z","s",tid);
// 	    if(params.dump_ply)
// 	      oth.write_into_file(ply_name,".ply");
// 	    oth.write_header(std::cout);

// 	    if(params.dump_ply)
// 	      w_datas.write_ply_stream(oth.get_output_stream(),'\n',true);
// 	    else
// 	      w_datas.write_serialized_stream(oth.get_output_stream());

// 	    oth.finalize();
// 	    std::cout << std::endl;
// 	  }
//       }
//   }else{

//     //w_datas_full.dmap[w_datas_full.egv_name].fill_full_uint8_vect(w_datas_full.format_egv);
//     w_datas_full.fill_egv(w_datas_full.format_egv);
//     //w_datas_full.dmap[w_datas_full.sig_name].fill_full_uint8_vect(w_datas_full.format_sigs);
//     w_datas_full.fill_sigs(w_datas_full.format_sigs);
//     w_datas_full.dmap[w_datas_full.xyz_name].fill_full_uint8_vect(w_datas_full.format_points);
//     std::cerr << "dim done tile : "<< tid << std::endl;
//     std::string ply_name(params.output_dir +  "/" + params.slabel + "_id_" + std::to_string(tid) +  "_dim");
//     std::cout.clear();


//     log.step("write");
//     ddt::stream_data_header oth("z","s",tid);
//     if(params.dump_ply)
//       oth.write_into_file(ply_name,".ply");
//     oth.write_header(std::cout);

//     if(params.dump_ply)
//       w_datas_full.write_ply_stream(oth.get_output_stream(),'\n',true);
//     else
//       w_datas_full.write_serialized_stream(oth.get_output_stream());

//     oth.finalize();
//     std::cout << std::endl;

//   }

//   bool do_debug  false;
//   // Extract the crown of the data
//   int acc = 0;
//   for ( auto it = datas_map_crown.begin(); it != datas_map_crown.end(); it++ )
//     {
//       Id ntid = it->first;
//       auto & w_datas = it->second;

// 	  //w_datas.dmap[w_datas.egv_name].fill_full_uint8_vect(w_datas.format_egv);
// 	  w_datas.fill_egv(w_datas.format_egv);
// 	  //w_datas.dmap[w_datas.sig_name].fill_full_uint8_vect(w_datas.format_sigs);
// 	  w_datas.fill_sigs(w_datas.format_sigs);


// 	  std::cerr << "dim done tile : "<< tid << std::endl;
// 	  std::string ply_name(params.output_dir +  "/" + params.slabel + "_tid_" + std::to_string(tid) + "_cid_" + std::to_string(it->first) + "_" + std::to_string(acc++) +  "_crown");
// 	  std::cout.clear();


// 	  log.step("write");
// 	  ddt::stream_data_header oth("y","s",tid);
// 	  if(params.dump_ply || do_debug)
// 	    oth.write_into_file(ply_name,".ply");
// 	  oth.write_header(std::cout);

// 	  if(params.dump_ply)
// 	    w_datas.write_ply_stream(oth.get_output_stream(),'\n',true);
// 	  else
// 	    w_datas.write_serialized_stream(oth.get_output_stream());

// 	  oth.finalize();
// 	  std::cout << std::endl;

//     }
  
  

//   if(p_simp_full.size() > 0)
//     {
//       ddt::stream_data_header oxh("x","z",tid);
//       ddt_data<Traits> datas_out;
//       datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
//       datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(p_simp_full);

//       std::string ply_name(params.output_dir +  "/simp_id_" + std::to_string(tid) + "_simp");
//       std::cerr << "dump ply:" << ply_name << std::endl;
//       if(do_debug){
// 	if(params.dump_ply)
// 	  oxh.write_into_file(ply_name,".ply");
// 	oxh.write_header(std::cout);
// 	if(params.dump_ply)
// 	  datas_out.write_ply_stream(oxh.get_output_stream(),'\n',true);
// 	else
// 	  datas_out.write_serialized_stream(oxh.get_output_stream());
//       }else{
// 	datas_out.write_ply_stream(oxh.get_output_stream(),PLY_CHAR);
//       }
//       oxh.finalize();
//       std::cout << std::endl;
//     }

  
  
//   return 0;
// }


int dim_splitted(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{
  std::cout.setstate(std::ios_base::failbit);
  std::cerr << "into fun simp" << std::endl;
  wasure_algo w_algo;
  int D = Traits::D;
  Traits  traits;
  D_LMAP w_datas_map;

  bool do_splitted =  false;
  wasure_data<Traits>  w_datas_full;
  std::vector<Point> p_simp_full;    
  for(int i = 0; i < nb_dat; i++)
    {

      ddt::stream_data_header hpi;
      hpi.parse_header(std::cin);
	
      Id hid = hpi.get_id(0);
      log.step("read");
      if(hpi.get_lab() == "z" )
	{
	  // if(hpi.is_serialized()){
	  //   std::vector<Point> rvp;
	  //   ddt::read_point_set_serialized(rvp, hpi.get_input_stream(),traits);
	  //   for(auto pp : rvp)
	  //     {
          //       vp.emplace_back(std::make_pair(pp,tid));
	  //     }
	  w_datas_map[hid].push_back(wasure_data<Traits>());
	  if(hpi.is_serialized()){
	    std::cerr << "start read ply" << std::endl;
	    w_datas_map[hid].back().read_serialized_stream(hpi.get_input_stream());
	    //	      w_datas.read_serialized_stream(hpi.get_input_stream());
	    //w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
	    std::cerr << "end read ply" << std::endl;
	  }else{
	    w_datas_map[hid].back().read_ply_stream(hpi.get_input_stream());
	    w_datas_map[hid].back().shpt2uint8();
	  }
	    

	  std::cerr << "start read ply" << std::endl;

	  //w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
	  std::cerr << "end read ply" << std::endl;
	}

      std::cerr << "reading end" << std::endl;
      wasure_data<Traits> & w_datas = w_datas_map[hid].back();
      //}
      hpi.finalize();
      std::cerr << "finalize" << std::endl;

      //      log.step("compute");
      // w_datas.extract_ptsvect(w_datas.xyz_name,w_datas.format_points,false);
      // w_datas.extract_ptsvect(w_datas.center_name,w_datas.format_centers,false);
      w_datas.dmap[w_datas.xyz_name].extract_full_uint8_vect(w_datas.format_points,false);
      std::cerr << "xyz ok" << std::endl;
      w_datas.dmap[w_datas.center_name].extract_full_uint8_vect(w_datas.format_centers,false);
      std::cerr << "centers ok " << std::endl;
      w_datas.extract_flags(w_datas.format_flags,false);
      std::cerr << "flags ok " << std::endl;
      int acc = 0;


	
      std::cerr << "center ok" << std::endl;
      std::vector<Point> p_simp;    
      // if(params.pscale >= 0)
      // {
      //     w_algo.compute_dim_with_simp(w_datas.format_points,
      //                                  w_datas.format_egv,
      //                                  w_datas.format_sigs,
      //                                  p_simp,
      //                                  params.pscale);
      // }
      // else
      // {

      if(w_datas.format_centers.size() == 0)
	{
	  double coords[Traits::D];
	  for(auto pp : w_datas.format_points)
	    {
	      for(int d = 0; d < D; d++)
		{
		  if(d < D-1)
		    coords[d] = pp[d];
		  else
		    coords[d] = pp[d] + 30;
		}
	      w_datas.format_centers.push_back(traits.make_point(coords));
	    }
	}

      if(do_splitted){
	w_algo.compute_dim(w_datas.format_points,
			   w_datas.format_egv,
			   w_datas.format_sigs);



	
	w_algo.flip_dim_ori(w_datas.format_points,
			    w_datas.format_egv,
			    w_datas.format_centers);

	std::cerr << i << "~~ ==" << w_datas.format_points.size() << " " << w_datas.format_points.size() << std::endl;
	std::cerr << i << "~~ ==" << w_datas.format_egv.size() << " " << w_datas.format_sigs.size() << std::endl;
	std::cerr << i << "~~ ==" << w_datas.format_centers.size() << " " << w_datas.format_centers.size() << std::endl;
	w_datas_full.format_points.insert(w_datas_full.format_points.end(),w_datas.format_points.begin(),w_datas.format_points.end());
	w_datas_full.format_egv.insert(w_datas_full.format_egv.end(),w_datas.format_egv.begin(),w_datas.format_egv.end());
	w_datas_full.format_sigs.insert(w_datas_full.format_sigs.end(),w_datas.format_sigs.begin(),w_datas.format_sigs.end());
	p_simp_full.insert(p_simp_full.end(),p_simp.begin(),p_simp.end());
	std::cerr << "inserted" << std::endl;
      }else{
	if(i == 0){
	  w_datas_full = w_datas;
	}else{
	  w_datas_full.insert(w_datas);
	}
      }


    }

  if(!do_splitted){
    w_datas_full.dmap[w_datas_full.xyz_name].extract_full_uint8_vect(w_datas_full.format_points,false);

    w_datas_full.dmap[w_datas_full.center_name].extract_full_uint8_vect(w_datas_full.format_centers,false);
    
    w_algo.compute_dim(w_datas_full.format_points,
		       w_datas_full.format_egv,
		       w_datas_full.format_sigs);
    
    if(w_datas_full.format_centers.size() == 0)
      {
	double coords[Traits::D];
	for(auto pp : w_datas_full.format_points)
	  {
	    for(int d = 0; d < D; d++)
	      {
		if(d < D-1)
		  coords[d] = pp[d];
		else
		  coords[d] = pp[d] + 30;
	      }
	    w_datas_full.format_centers.push_back(traits.make_point(coords));
	  }
      }

	
    w_algo.flip_dim_ori(w_datas_full.format_points,
			w_datas_full.format_egv,
			w_datas_full.format_centers);
  }
    
  std::cerr << "start tessel" << std::endl;
  if(params.pscale < 1){
    w_algo.tessel_adapt(w_datas_full.format_points,
		      p_simp_full,
		      w_datas_full.format_egv,
		      w_datas_full.format_sigs,
		      20,params.pscale,D,tid
		      );
  }else{
    p_simp_full.insert(p_simp_full.end(),w_datas_full.format_points.begin(),w_datas_full.format_points.end());
    
  }

  // w_algo.tessel(w_datas_full.format_points,
  // 		  p_simp_full,
  // 		  w_datas_full.format_egv,
  // 		  w_datas_full.format_sigs,tid);

  if(do_splitted){
    for ( auto it = w_datas_map.begin(); it != w_datas_map.end(); it++ )
      {
	int acc = 0;
	for(auto & w_datas : w_datas_map[it->first])
	  {
      

	    //w_datas.dmap[w_datas.egv_name].fill_full_uint8_vect(w_datas.format_egv);
	    w_datas.fill_egv(w_datas.format_egv);
	    //w_datas.dmap[w_datas.sig_name].fill_full_uint8_vect(w_datas.format_sigs);
	    w_datas.fill_sigs(w_datas.format_sigs);


	    std::cerr << "dim done tile : "<< tid << std::endl;
	    std::string ply_name(params.output_dir +  "/" + params.slabel + "_id_" + std::to_string(it->first) + "_" + std::to_string(acc++) +  "_dim");
	    std::cout.clear();


	    log.step("write");
	    ddt::stream_data_header oth("z","s",tid);
	    if(params.dump_ply)
	      oth.write_into_file(ply_name,".ply");
	    oth.write_header(std::cout);

	    if(params.dump_ply)
	      w_datas.write_ply_stream(oth.get_output_stream(),'\n',true);
	    else
	      w_datas.write_serialized_stream(oth.get_output_stream());

	    oth.finalize();
	    std::cout << std::endl;
	  }
      }
  }else{

    //w_datas_full.dmap[w_datas_full.egv_name].fill_full_uint8_vect(w_datas_full.format_egv);
    w_datas_full.fill_egv(w_datas_full.format_egv);
    //w_datas_full.dmap[w_datas_full.sig_name].fill_full_uint8_vect(w_datas_full.format_sigs);
    w_datas_full.fill_sigs(w_datas_full.format_sigs);
    w_datas_full.dmap[w_datas_full.xyz_name].fill_full_uint8_vect(w_datas_full.format_points);
    std::cerr << "dim done tile : "<< tid << std::endl;
    std::string ply_name(params.output_dir +  "/" + params.slabel + "_id_" + std::to_string(tid) +  "_dim");
    std::cout.clear();


    log.step("write");
    ddt::stream_data_header oth("z","s",tid);
    if(params.dump_ply)
      oth.write_into_file(ply_name,".ply");
    oth.write_header(std::cout);

    if(params.dump_ply)
      w_datas_full.write_ply_stream(oth.get_output_stream(),'\n',true);
    else
      w_datas_full.write_serialized_stream(oth.get_output_stream());

    oth.finalize();
    std::cout << std::endl;

  }
  bool do_debug = false;
  if(p_simp_full.size() > 0)
    {
      ddt::stream_data_header oxh("x","z",tid);
      ddt_data<Traits> datas_out;
      datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
      datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(p_simp_full);

      std::string ply_name(params.output_dir +  "/simp_id_" + std::to_string(tid) + "_simp");
      if(!do_debug){
	if(params.dump_ply)
	  oxh.write_into_file(ply_name,".ply");
	oxh.write_header(std::cout);
	if(params.dump_ply)
	  datas_out.write_ply_stream(oxh.get_output_stream());
	else
	  datas_out.write_serialized_stream(oxh.get_output_stream());
      }else{
	datas_out.write_ply_stream(oxh.get_output_stream(),PLY_CHAR);
      }
      oxh.finalize();
      std::cout << std::endl;
    }

  return 0;
}


		  








int dst_new(const Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{
    std::cout.setstate(std::ios_base::failbit);

    DTW tri;
    Scheduler sch(1);
    int D = Traits::D;
    std::cerr << "dst_step1" << std::endl;
    wasure_algo w_algo;

    D_LMAP w_datas_pts;
    D_MAP w_datas_tri;
    std::map<Id,ddt_data<Traits> > d_datas_tri;


    auto w_data_full = wasure_data<Traits>();

    std::cerr << "dst_step2" << std::endl;

    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {

            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri, hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);

        }
        if(hpi.get_lab() == "s")
        {
            std::vector<int> vv(3);
            for(int d = 0; d < 3; d++)
            {
                hpi.get_input_stream() >> vv[d];
            }
            w_datas_tri[hid].tile_ids = vv;
        }
        if(hpi.get_lab() == "z")
        {

            w_datas_pts[hid].push_back(wasure_data<Traits>());
            //w_datas_pts[hid].back().read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
	    w_datas_pts[hid].back().read_serialized_stream(hpi.get_input_stream());

        }
        hpi.finalize();
    }
    log.step("preprocess");
    
    std::cerr << "dst exrract pts dat" << std::endl;
    for ( auto it = w_datas_pts.begin(); it != w_datas_pts.end(); it++ )
    {

        //   w_datas_pts[it->first].extract_ptsvect(w_datas_pts[it->first].xyz_name,w_datas_pts[tid].format_points,false);

        //   w_datas_pts[it->first].extract_egv(w_datas_pts[it->first].format_egv,false);
        //   w_datas_pts[it->first].extract_sigs(w_datas_pts[it->first].format_sigs,false);

      for(auto & wpt : w_datas_pts[it->first])
        {
	  //wpt.extract_ptsvect(w_data_full.xyz_name,w_data_full.format_points,false);
	  std::cerr << "extract xyz" << std::endl;
	  wpt.dmap[wpt.xyz_name].extract_full_uint8_vect(wpt.format_points,false);
	  w_data_full.format_points.insert(w_data_full.format_points.end(),wpt.format_points.begin(),wpt.format_points.end());
	  std::cerr << "extract centers" << std::endl;
	  wpt.dmap[wpt.center_name].extract_full_uint8_vect(wpt.format_centers,false);
	  w_data_full.format_centers.insert(w_data_full.format_centers.end(),wpt.format_centers.begin(),wpt.format_centers.end());
	  std::cerr << "extract flags" << std::endl;
	  // wpt.extract_flags(wpt.format_flags,false);
	  // w_data_full.format_flags.insert(w_data_full.format_flags.end(),wpt.format_flags.begin(),wpt.format_flags.end());
	  std::cerr << "extract sig" << std::endl;
	  wpt.extract_sigs(w_data_full.format_sigs,false);
	  std::cerr << "extract egv" << std::endl;
	  wpt.extract_egv(w_data_full.format_egv,false);
	  // std::cerr << "extract flagss" << std::endl;
	  //wpt.extract_ptsvect(w_data_full.center_name,w_data_full.format_centers,false);
	  //wpt.extract_ptsvect(w_data_full.xyz_name,w_data_full.format_points,false);

        }
    }

    Tile_iterator  tile1  = tri.get_tile(tid);
    int nbs = tile1->number_of_cells();//w_datas_tri[tid].nb_simplex_uint8_vect();
    int accll = 0;


    if(w_data_full.format_flags.size() == 0){
      for(int ss = 0; ss < nbs ; ss++)
        {
            w_data_full.format_flags.push_back(0);
        }

    }
      
    
    
    std::cerr << "dst_step3" << std::endl;
    std::vector<std::vector<double>>  & format_dst = w_datas_tri[tid].format_dst; ;
    if(format_dst.size() == 0)
    {

        for(int ss = 0; ss < nbs ; ss++)
        {
            format_dst.push_back(std::vector<double>({0.0,0.0,1.0}));
        }
    }

    log.step("compute");
    DT & tri_tile  = tri.get_tile(tid)->triangulation();
    // if(params.rat_ray_sample == 0 ){
    //   w_algo.compute_dst_tri(tri_tile,w_datas_tri[tid],w_datas_pts[tid],params);
    // }else{w


    if(false)  // On all pts
    {
        for ( auto it = w_datas_pts.begin(); it != w_datas_pts.end(); it++ )
        {
            //            for(auto & wpt : w_datas_pts[it->first]){
            for(auto wpt = w_datas_pts[it->first].begin() ; wpt != w_datas_pts[it->first].end() ; wpt++)
            {
	      w_algo.compute_dst_with_center(tri,w_datas_tri[tid],*wpt,params,tid);
            }
        }
    }
    else   // Only On local pts
    {
      w_algo.compute_dst_with_center(tri,w_datas_tri[tid],w_data_full,params,tid);
        // for(auto wpt = w_datas_pts[tid].begin() ; wpt != w_datas_pts[tid].end() ; wpt++){
        // 	w_algo.compute_dst_with_center(tri,w_datas_tri[tid],*wpt,params);
        // }
        //w_algo.compute_dst_with_center(tri,w_datas_tri[tid],*w_datas_pts[tid].begin(),params);
    }
    //}0


    //    init_global_id(tri,w_datas_tri,tid);


    // if(Traits::D == 2){
    //   std::string outt = get_img_2d_name(params.output_dir, tid);
    //   //dump_dst_in_img(datas, params, outt );
    // }

    log.step("finalize");
    w_datas_tri[tid].fill_dst(w_datas_tri[tid].format_dst);
    log.step("write");
    std::cerr << "dst_step6" << std::endl;
    std::cout.clear();
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    if(params.dump_ply)
        oth.write_into_file(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    std::cerr << "stream dumped" << std::endl;
    oth.finalize();
    std::cout << std::endl;


    return 0;
}


int dst_conflict(const Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{
    std::cerr << "dst_mode:conflict" << std::endl;
    std::cout.setstate(std::ios_base::failbit);
    std::cerr << "dst_step0" << std::endl;

    DTW tri;
    Scheduler sch(1);
    int D = Traits::D;
    std::cerr << "dst_step1" << std::endl;
    wasure_algo w_algo;

    D_LMAP w_datas_pts;
    D_MAP w_datas_tri;
    std::map<Id,ddt_data<Traits> > d_datas_tri;


    auto w_data_full = wasure_data<Traits>();

    std::cerr << "dst_step2" << std::endl;

    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
        }
        if(hpi.get_lab() == "s")
        {
            std::vector<int> vv(3);
            for(int d = 0; d < 3; d++)
            {
                hpi.get_input_stream() >> vv[d];
            }
            w_datas_tri[hid].tile_ids = vv;
        }
        if(hpi.get_lab() == "z")
        {
            //      auto wdt = wasure_data<Traits>();
            // if(w_datas_pts.find(hid) == w_datas_pts.end())
            // 	w_datas_pts[hid] = new std::list<wasure_data<Traits>>();
            w_datas_pts[hid].push_back(wasure_data<Traits>());
            //w_datas_pts[hid].back().read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
	    w_datas_pts[hid].back().read_serialized_stream(hpi.get_input_stream());

        }
        hpi.finalize();
    }
    log.step("preprocess");


    for ( auto it = w_datas_pts.begin(); it != w_datas_pts.end(); it++ )
    {
        for(auto & wpt : w_datas_pts[it->first])
        {
	    wpt.dmap[w_data_full.xyz_name].extract_full_uint8_vect(w_data_full.format_points,false);
          //  wpt.extract_ptsvect(wpt.xyz_name,wpt.format_points,false);
            wpt.extract_egv(wpt.format_egv,false);
            wpt.extract_sigs(wpt.format_sigs,false);
            //wpt.extract_ptsvect(wpt.center_name,wpt.format_centers,false);
	    wpt.dmap[w_data_full.center_name].extract_full_uint8_vect(w_data_full.format_centers,false);
        }
    }

    std::cerr << "dst_step3" << std::endl;
    std::vector<std::vector<double>>  & format_dst = w_datas_tri[tid].format_dst; ;
    int nbs = w_datas_tri[tid].nb_simplex_uint8_vect();
    if(format_dst.size() == 0)
    {
        for(int ss = 0; ss < nbs ; ss++)
        {
            format_dst.push_back(std::vector<double>({0.0,0.0,1.0}));
        }
    }

    log.step("compute");
    DT & tri_tile  = tri.get_tile(tid)->triangulation();
    // if(params.rat_ray_sample == 0 ){
    //   w_algo.compute_dst_tri(tri_tile,w_datas_tri[tid],w_datas_pts[tid],params);
    // }else{



    std::list<wasure_data<Traits>> l_tri;
    if(true)  // On all pts
    {
        for ( auto it = w_datas_pts.begin(); it != w_datas_pts.end(); it++ )
        {
            //            for(auto & wpt : w_datas_pts[it->first]){


            for(auto & wpt : w_datas_pts[it->first])
            {
                l_tri.push_back(wasure_data<Traits>());
                auto & wd_tri = l_tri.back();
                std::vector<std::vector<double>>  & format_dst_pts = wd_tri.format_dst; ;
                if(format_dst_pts.size() == 0)
                {
                    for(int ss = 0; ss < nbs ; ss++)
                    {
                        format_dst_pts.push_back(std::vector<double>({0.0,0.0,1.0}));
                    }
                }
                w_algo.compute_dst_with_center(tri,wd_tri,wpt,params,tid);
                for(int ss = 0; ss < nbs ; ss++)
                {
                    double & vpe = format_dst_pts[ss][0];
                    double & vpo = format_dst_pts[ss][1];
                    double & vpu = format_dst_pts[ss][2];
                    if(vpe > vpo)
                    {
                        vpe = vpe-vpo;
                        vpo = 0;
                        if(vpe > 0.95)
                            vpe = 0.95;
                        vpu =1-vpe;
                    }
                    else
                    {
                        vpo = vpo-vpe;
                        vpe = 0;
                        if(vpo > 0.95)
                            vpo = 0.95;
                        vpu = 1-vpo;
                    }
                }
            }
        }
    }
    int acc = 0;
    for(auto & wd_tri : l_tri)
    {
        std::cout << "dst_acc:" << acc << std::endl;
        // if(acc++ == 1)
        //   continue;
        std::vector<std::vector<double>>  & format_dst_pts = wd_tri.format_dst;
        for(int ss = 0; ss < nbs ; ss++)
        {
            double  vpe = format_dst[ss][0];
            double  vpo = format_dst[ss][1];
            double  vpu = format_dst[ss][2];
            w_algo.ds_score(vpe,vpo,vpu,
                            format_dst_pts[ss][0],format_dst_pts[ss][1],format_dst_pts[ss][2],
                            format_dst[ss][0],format_dst[ss][1],format_dst[ss][2]);
            // std::cerr << "==== acc:" << acc << "========" << std::endl;
            // std::cerr << "  " << vpe << "  " << vpo << "  " << vpu << std::endl;
            // std::cerr << "  " << format_dst_pts[ss][0] << "  " << format_dst_pts[ss][1] << "  " << format_dst_pts[ss][2] << std::endl;
            // std::cerr <<  "  " << format_dst[ss][0] << "  " << format_dst[ss][1] << "  " << format_dst[ss][2] << std::endl;
            regularize(format_dst[ss][0],format_dst[ss][1],format_dst[ss][2]);
            acc++;
        }
    }
    //init_global_id(tri,w_datas_tri,tid);


    // if(Traits::D == 2){
    //   std::string outt = get_img_2d_name(params.output_dir, tid);
    //   //dump_dst_in_img(datas, params, outt );
    // }

    log.step("finalize");
    w_datas_tri[tid].fill_dst(w_datas_tri[tid].format_dst);
    log.step("write");
    std::cerr << "dst_step6" << std::endl;
    std::cout.clear();
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    if(params.dump_ply)
        oth.write_into_file(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    oth.finalize();
    std::cout << std::endl;
    return 0;
}



int dst_good(const Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{

    std::cout.setstate(std::ios_base::failbit);
    std::cerr << "dst_step0" << std::endl;

    DTW tri;
    Scheduler sch(1);
    int D = Traits::D;
    std::cerr << "dst_step1" << std::endl;
    wasure_algo w_algo;

    D_LMAP w_datas_pts;
    D_MAP w_datas_tri;
    std::map<Id,ddt_data<Traits> > d_datas_tri;




    std::cerr << "dst_step2" << std::endl;

    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {

            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);

        }
        if(hpi.get_lab() == "s")
        {
            std::vector<int> vv(3);
            for(int d = 0; d < 3; d++)
            {
                hpi.get_input_stream() >> vv[d];
            }
            w_datas_tri[hid].tile_ids = vv;
        }
        if(hpi.get_lab() == "z")
        {
            //      auto wdt = wasure_data<Traits>();
            // if(w_datas_pts.find(hid) == w_datas_pts.end())
            // 	w_datas_pts[hid] = new std::list<wasure_data<Traits>>();
            w_datas_pts[hid].push_back(wasure_data<Traits>());
	    w_datas_pts[hid].back().read_serialized_stream(hpi.get_input_stream());
            //w_datas_pts[hid].back().read_ply_stream(hpi.get_input_stream(),PLY_CHAR);

        }
        hpi.finalize();
    }
    log.step("preprocess");

    std::cerr << "dst_step3" << std::endl;

    for ( auto it = w_datas_pts.begin(); it != w_datas_pts.end(); it++ )
    {

        //   w_datas_pts[it->first].extract_ptsvect(w_datas_pts[it->first].xyz_name,w_datas_pts[tid].format_points,false);

        //   w_datas_pts[it->first].extract_egv(w_datas_pts[it->first].format_egv,false);
        //   w_datas_pts[it->first].extract_sigs(w_datas_pts[it->first].format_sigs,false);

        for(auto & wpt : w_datas_pts[it->first])
        {
	    wpt.dmap[wpt.xyz_name].extract_full_uint8_vect(wpt.format_points,false);
            //wpt.extract_ptsvect(wpt.xyz_name,wpt.format_points,false);
            wpt.extract_egv(wpt.format_egv,false);
            wpt.extract_sigs(wpt.format_sigs,false);
	    wpt.dmap[wpt.center_name].extract_full_uint8_vect(wpt.format_centers,false);
            //wpt.extract_ptsvect(wpt.center_name,wpt.format_centers,false);
        }

    }

    std::cerr << "dst_step3" << std::endl;
    std::vector<std::vector<double>>  & format_dst = w_datas_tri[tid].format_dst; ;
    if(format_dst.size() == 0)
    {
        int nbs = w_datas_tri[tid].nb_simplex_uint8_vect();
        for(int ss = 0; ss < nbs ; ss++)
        {
            format_dst.push_back(std::vector<double>({0.0,0.0,1.0}));
        }
    }

    log.step("compute");
    DT & tri_tile  = tri.get_tile(tid)->triangulation();
    // if(params.rat_ray_sample == 0 ){
    //   w_algo.compute_dst_tri(tri_tile,w_datas_tri[tid],w_datas_pts[tid],params);
    // }else{


    if(false)  // On all pts
    {
        for ( auto it = w_datas_pts.begin(); it != w_datas_pts.end(); it++ )
        {
            //            for(auto & wpt : w_datas_pts[it->first]){
            for(auto wpt = w_datas_pts[it->first].begin() ; wpt != w_datas_pts[it->first].end() ; wpt++)
            {
	      w_algo.compute_dst_with_center(tri,w_datas_tri[tid],*wpt,params,tid);
            }
        }
    }
    else   // Only On local pts
    {
        for(auto wpt = w_datas_pts[tid].begin() ; wpt != w_datas_pts[tid].end() ; wpt++)
        {
	  w_algo.compute_dst_with_center(tri,w_datas_tri[tid],*wpt,params,tid);
        }
        //w_algo.compute_dst_with_center(tri_tile,w_datas_tri[tid],*w_datas_pts[tid].begin(),params);
    }
    //}0


    //    init_global_id(tri,w_datas_tri,tid);


    // if(Traits::D == 2){
    //   std::string outt = get_img_2d_name(params.output_dir, tid);
    //   //dump_dst_in_img(datas, params, outt );
    // }

    log.step("finalize");
    w_datas_tri[tid].fill_dst(w_datas_tri[tid].format_dst);
    log.step("write");
    std::cerr << "dst_step6" << std::endl;
    std::cout.clear();
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    if(params.dump_ply)
        oth.write_into_file(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    oth.finalize();
    std::cout << std::endl;
    return 0;
}




int regularize_slave_focal(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{
    std::cout.setstate(std::ios_base::failbit);
    DTW tri;
    Scheduler sch(1);
    wasure_algo w_algo;
    int D = Traits::D;
    D_MAP w_datas_tri;

    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
        }
        tri.finalize(sch);
        hpi.finalize();
    }

    Tile_iterator  tile_k  = tri.get_tile(tid);
    for(auto cit = tile_k->cells_begin();
	cit != tile_k->cells_end();
	cit++){
      if(!tile_k->cell_is_mixed(cit) || tile_k->cell_is_main(cit))
	continue;
      Id main_tid = tile_k->cell_main_id(cit);
      Tile_iterator main_tile = tri.get_tile(main_tid);
      Id lid1 = tile_k->lid(cit);
      
      auto main_cell = main_tile->locate_cell(*tile_k,cit);
      Id lid2 = main_tile->lid(main_cell);
      w_datas_tri[tid].replace_attribute(w_datas_tri[main_tid],lid1,lid2);
    }

    Id tid_k = tid;
    Tile_const_iterator  tilec_k  = tri.get_const_tile(tid);
    std::map<Id,std::map<Id,SharedData> > shared_data_map;
    // Loop over each shared cell to extracts id relation
    // lid_l , lid_l <-> lid_k
    for( auto cit_k = tilec_k->cells_begin();
    	 cit_k != tilec_k->cells_end(); ++cit_k )
      {
	Cell_const_iterator fch = Cell_const_iterator(tilec_k,tilec_k, tilec_k, cit_k);
	if(!tile_k->cell_is_mixed(cit_k) || tile_k->cell_is_infinite(cit_k))
	  continue;
	Id lid_k = tile_k->lid(cit_k);
	int D = tile_k->current_dimension();
	// Loop on shared tiles
	std::unordered_set<Id> idSet ;	  
	for(int i=0; i<=D; ++i)
    	    {
    	      // Select only id != tid_k and new ids
    	      Id tid_l = tile_k->id(tile_k->vertex(cit_k,i));
	      if ((idSet.find(tid_l) != idSet.end()) || tid_l == tid_k ){
	      	continue;
	      }
	      idSet.insert(tid_l);
    	      if(tid_l == tid_k)
    		continue;
    	      Tile_iterator tile_l = tri.get_tile(tid_l);
	      // If 
	      auto cit_l = tile_l->locate_cell(*tile_k,cit_k);
	      Id lid_l = tile_l->lid(cit_l);	      
 	      if(shared_data_map.find(tid_l) == shared_data_map.end())
		shared_data_map[tid_l] = std::map<Id,SharedData>();

	      // The current data structure => local_id of the shared tet, lag and tau
	      shared_data_map[tid_l][lid_k] = std::make_tuple(lid_l,0,1,0);
	    }
      }



    std::cerr << "regularize" << std::endl;
    std::cout.clear();
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    if(params.dump_ply)
        oth.write_into_file(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    std::cerr << "stream dumped" << std::endl;
    oth.finalize();
    std::cout << std::endl;


    // Dum edges
    for(auto ee : shared_data_map){
      Id tid2 = ee.first;
      if(tid2 == tid)
	continue;
      ddt::stream_data_header hto("e","z",std::vector<int> {tid,tid2});
      std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid) + "_nid" + std::to_string(tid2));
      //hto.write_into_file(filename,".pts");
      if(params.dump_ply)
    	hto.write_into_file(filename,".ply");
      hto.write_header(std::cout);
      write_id_double_serialized(ee.second,hto.get_output_stream());
      hto.finalize();
      std::cout << std::endl;
    }

    
    return 0;



 
}



int regularize_slave_extract(Id tid_1,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{
    std::cout.setstate(std::ios_base::failbit);
    DTW tri;
    Scheduler sch(1);
    wasure_algo w_algo;
    int D = Traits::D;
    D_MAP w_datas_tri;

    std::map<Id,std::map<Id,SharedDataDst> > edges_dst_map;    
    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);

	    w_datas_tri[hid].extract_dst(w_datas_tri[hid].format_dst,false);
        }
        tri.finalize(sch);
        hpi.finalize();
    }


    Tile_const_iterator  tilec_1  = tri.get_const_tile(tid_1);
    std::vector<std::vector<double>> & v_dst = w_datas_tri[tid_1].format_dst;
    // Loop over each shared cell to extracts id relation
    // lid_2 , lid_2 <-> lid_1
    for( auto cit_1 = tilec_1->cells_begin();
    	 cit_1 != tilec_1->cells_end(); ++cit_1 )
      {
	Cell_const_iterator fch = Cell_const_iterator(tilec_1,tilec_1, tilec_1, cit_1);
	std::vector<double> bary = tilec_1->get_cell_barycenter(cit_1);
	if(!tilec_1->cell_is_mixed(cit_1) || tilec_1->cell_is_infinite(cit_1))
	  continue;
	Id lid_1 = tilec_1->lid(cit_1);
	int D = tilec_1->current_dimension();
	// Loop on shared tiles
	std::unordered_set<Id> idSet ;	  
	for(int i=0; i<=D; ++i)
    	    {
    	      // Select only id != tid_1 and new ids
    	      Id tid_2 = tilec_1->id(tilec_1->vertex(cit_1,i));
	      if ((idSet.find(tid_2) != idSet.end()) || tid_2 == tid_1 ){
	      	continue;
	      }
	      idSet.insert(tid_2);

 	      if(edges_dst_map.find(tid_2) == edges_dst_map.end())
		edges_dst_map[tid_2] = std::map<Id,SharedDataDst>();

	      // The current data structure => local_id of the shared tet, lag and tau
	      edges_dst_map[tid_2][lid_1] = std::make_tuple(lid_1,bary[0],bary[1],bary[2],v_dst[lid_1][0],v_dst[lid_1][1],v_dst[lid_1][2]);
	    }
      }





    std::cerr << "regularize" << std::endl;
    std::cout.clear();
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid_1);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid_1));
    if(params.dump_ply)
        oth.write_into_file(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid_1], oth.get_output_stream(),tid_1,false,log);
    std::cerr << "stream dumped" << std::endl;
    oth.finalize();
    std::cout << std::endl;


    for(auto ee : edges_dst_map){
      Id tid_2 = ee.first;
      ddt::stream_data_header hto("f","z",std::vector<int> {tid_1,tid_2});
      std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid_1) + "_nid" + std::to_string(tid_2));
      //hto.write_into_file(filename,".pts");
      if(params.dump_ply)
    	hto.write_into_file(filename,".ply");
      hto.write_header(std::cout);
      write_id_dst_serialized(ee.second,hto.get_output_stream(),tid_1 ==3);
      hto.finalize();
      std::cout << std::endl;
    }
    
    return 0;

}






int regularize_slave_insert(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{
    std::cout.setstate(std::ios_base::failbit);
    DTW tri;
    Scheduler sch(1);
    wasure_algo w_algo;
    int D = Traits::D;
    D_MAP w_datas_tri;
    Traits  traits;
    
    std::map<Id,std::map<Id,SharedDataDst> > edges_dst_map;    
    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
	    w_datas_tri[hid].extract_dst(w_datas_tri[hid].format_dst,true);
        }
	if(hpi.get_lab() == "f")
        {
	  Id eid1 = hpi.get_id(0);
	  Id eid2 = hpi.get_id(1);
	  edges_dst_map[eid1] = std::map<Id,SharedDataDst>();	  
	  std::map<Id,SharedDataDst>  & lp = edges_dst_map[eid1];
	  read_id_dst_serialized(lp, hpi.get_input_stream(),tid == 3);
        }

        tri.finalize(sch);
        hpi.finalize();
    }

    
    Id tid_k = tid;
    Tile_const_iterator  tile_k  = tri.get_const_tile(tid);
    std::map<Id,std::map<Id,SharedData> > shared_data_map;
    std::vector<std::vector<double>>  & format_dst = w_datas_tri[tid].format_dst; ;
    // Loop over each shared cell to extracts id relation
    // lid_l , lid_l <-> lid_k
    std::cerr << "RECIEVE BARY" << std::endl; 
    for(auto ee : edges_dst_map){
      Id tid_l = ee.first;
      for(auto ee_map : ee.second){
	auto edm_l = ee_map.second;
	Id lid_l = std::get<0>(edm_l);
	std::vector<double> bary{std::get<1>(edm_l),std::get<2>(edm_l),std::get<3>(edm_l)};
	std::vector<double> dstv{std::get<4>(edm_l),std::get<5>(edm_l),std::get<6>(edm_l)};

	auto pp_bary = traits.make_point(bary.begin());
	auto main_cell = tile_k->locate_cell_point(*tile_k,pp_bary);
	
	Id cmid = tile_k->cell_main_id(main_cell);
	Id lid_k = tile_k->lid(main_cell);
	
	if(shared_data_map.find(tid_l) == shared_data_map.end())
	  shared_data_map[tid_l] = std::map<Id,SharedData>();

	// The current data structure => local_id of the shared tet, lag and tau

	shared_data_map[tid_l][lid_k] = std::make_tuple(lid_l,0,1,0);

	// std::cerr << "baryy:";
	// for(int i = 0; i < 3; i++){
	//   std::cerr << bary[i] << " ";
	// }
	// std::cerr << " - ";
	// for(int i = 0; i < 3; i++){
	//   std::cerr << dstv[i] << " ";
	// }
	// std::cerr << " - " << cmid  << " - " << tid_k << " - " << tid_l << std::endl;
	
	// if main
	if(cmid == tid_l){
	  for(int i = 0 ; i < 3; i++)
	    format_dst[lid_k][i] = dstv[i];
	}	
      }
    }
    
    // for( auto cit_k = tile_k->cells_begin();
    // 	 cit_k != tile_k->cells_end(); ++cit_k )
    //   {
    // 	Cell_const_iterator fch = Cell_const_iterator(tile_k,tile_k, tile_k, cit_k);
    // 	if(!tile_k->cell_is_mixed(cit_k) || tile_k->cell_is_infinite(cit_k))
    // 	  continue;
    // 	Id lid_k = tile_k->lid(cit_k);
    // 	int D = tile_k->current_dimension();
    // 	// Loop on shared tiles
    // 	std::unordered_set<Id> idSet ;
    // 	Id cmid = tile_k->cell_main_id(cit_k);
    // 	for(int i=0; i<=D; ++i)
    // 	    {
    // 	      // Select only id != tid_k and new ids
    // 	      Id tid_l = tile_k->id(tile_k->vertex(cit_k,i));
    // 	      if ((idSet.find(tid_l) != idSet.end()) || tid_l == tid_k ){
    // 	      	continue;
    // 	      }
    // 	      idSet.insert(tid_l);
    // 	      if(tid_l == tid_k)
    // 		continue;

    // 	      auto edm_l = edges_dst_map[tid_l][lid_k];
    // 	      Id lid_l = std::get<1>(edm_l);

    // 	      std::vector<double> bary{std::get<1>(edm_l),std::get<2>(edm_l),std::get<3>(edm_l)};
    // 	      std::vector<double> dstv{std::get<4>(edm_l),std::get<5>(edm_l),std::get<6>(edm_l)};
    // 	      auto pp_bary = traits.make_point(bary.begin());
    // 	      //	      pvect.push_back(traits.make_point(bary.begin()));
    // 	      auto main_cell = tile_k->locate_cell_point(*tile_k,pp_bary);
	      
    // 	      if(shared_data_map.find(tid_l) == shared_data_map.end())
    // 		shared_data_map[tid_l] = std::map<Id,SharedData>();

    // 	      // The current data structure => local_id of the shared tet, lag and tau

    // 	      shared_data_map[tid_l][lid_k] = std::make_tuple(lid_l,0,1,0);
    // 	      if(cmid == tid_l){
    // 		for(int i = 0 ; i < 3; i++)
    // 		  format_dst[lid_k][i] = dstv[i];

    // 	      }
    // 	    }
    //   }
    w_datas_tri[tid].fill_dst(w_datas_tri[tid].format_dst);


    std::cerr << "regularize" << std::endl;
    std::cout.clear();
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    if(params.dump_ply)
        oth.write_into_file(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    std::cerr << "stream dumped" << std::endl;
    oth.finalize();
    std::cout << std::endl;


    // Dum edges
    for(auto ee : shared_data_map){
      Id tid2 = ee.first;
      if(tid2 == tid)
	continue;
      ddt::stream_data_header hto("e","z",std::vector<int> {tid,tid2});
      std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid) + "_nid" + std::to_string(tid2));
      //hto.write_into_file(filename,".pts");
      if(params.dump_ply)
    	hto.write_into_file(filename,".ply");
      hto.write_header(std::cout);
      write_id_double_serialized(ee.second,hto.get_output_stream());
      hto.finalize();
      std::cout << std::endl;
    }

    
    return 0;


}



int extract_surface(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{

    std::vector<Facet_const_iterator> lft;
    std::vector<bool> lbool;
    std::cout.setstate(std::ios_base::failbit);
    DTW tri;
    Scheduler sch(1);
    wasure_algo w_algo;
    int D = Traits::D;

    D_MAP w_datas_tri;
    ddt_data<Traits> datas_out;
    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
            w_datas_tri[hid].extract_labs(w_datas_tri[hid].format_labs,false);
        }
        tri.finalize(sch);
        hpi.finalize();
    }

    //    w_datas_tri[tid].extract_ptsvect(w_datas.xyz_name,w_datas.format_points,false);
    // Cell_const_iterator fch;


    log.step("compute");
    int mode = -1;

    //mode = 1;
    // if(params.mode.find(std::string("out")) != std::string::npos)
    mode = 0;


       
    for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
        try
        {
            if(fit->main_id() != tid || fit->is_infinite())
                continue;

	  // if(fit->is_infinite())
	  //   continue;


            Cell_const_iterator tmp_fch = fit.full_cell();
            int tmp_idx = fit.index_of_covertex();
            Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

            if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	       !tri.tile_is_loaded(tmp_fchn->main_id())){
	      std::cerr << "ERROR tile not loaded" << std::endl;
                continue;
	    }

	    bool is_on_convex = false;
	    if(tmp_fch->is_infinite() ||  tmp_fchn->is_infinite() )
	      is_on_convex = true;


            Cell_const_iterator fch = tmp_fch->main();
            int id_cov = fit.index_of_covertex();
            Cell_const_iterator fchn = tmp_fchn->main();
	    //            Vertex_h_iterator vht;

            int cccid = fch->lid();
            int cccidn = fchn->lid();

            int ch1lab = w_datas_tri[fch->tile()->id()].format_labs[cccid];
            int chnlab = w_datas_tri[fchn->tile()->id()].format_labs[cccidn];
            if(
	       (ch1lab != chnlab)  || (mode == 0 && (is_on_convex && (ch1lab == 0 || chnlab == 0)))
	       ){
                lft.push_back(*fit);
		
		const Point& a = fch->vertex((id_cov+1)&3)->point();
		const Point& b = fch->vertex((id_cov+2)&3)->point();
		const Point& c = fch->vertex((id_cov+3)&3)->point();
		const Point& d = fch->vertex((id_cov)&3)->point();

		datas_out.bbox += a;
		datas_out.bbox += b;
		datas_out.bbox += c;
		bool bl =
		  (CGAL::orientation(a,b,c,d) == 1 && chnlab == 0) ||
		  (CGAL::orientation(a,b,c,d) == -1 && chnlab == 1);
		lbool.push_back(!bl);
	    }

        }
        catch (ddt::DDT_exeption& e)
        {
            std::cerr << "!! WARNING !!!" << std::endl;
            std::cerr << "Exception catched : " << e.what() << std::endl;
            continue;
        }
    }


    log.step("write");

    std::cerr << "dim done tile : "<< tid << std::endl;
    std::string ply_name(params.output_dir +  "/" + params.slabel + "_id_" + std::to_string(tid) + "_surface");
    std::cout.clear();

    ddt::stream_data_header oth("p","z",tid);
    if(D == 2){
      oth.write_into_file(ply_name,".geojson");
      oth.write_header(std::cout);
    }
    // else
    //     oth.write_into_file(ply_name,".ply");




    switch (D)
    {
    case 2 :
    {
        wasure::dump_2d_surface_geojson<DTW>(lft,oth.get_output_stream());
        break;
    }
    case 3 :
    {

        std::vector<Point>  format_points;
        std::vector<int> v_simplex;
        std::map<Vertex_const_iterator, uint> vertex_map;


        int acc = 0;
        for(auto fit = lft.begin(); fit != lft.end(); ++fit)
        {
            Cell_const_iterator fch = fit->full_cell();
            int id_cov = fit->index_of_covertex();
            for(int i = 0; i < D+1; ++i)
            {
                if(i != id_cov)
                {
                    Vertex_const_iterator v = fch->vertex(i);
                    if(vertex_map.find(v) == vertex_map.end())
                    {
                        vertex_map[v] = acc++;
                        format_points.push_back(v->point());
                    }
                }
            }
        }

	acc=0;
        for(auto fit = lft.begin(); fit != lft.end(); ++fit)
        {
            Cell_const_iterator fch = fit->full_cell();
            int id_cov = fit->index_of_covertex();

	    
	    Id ida = (id_cov+1)&3;
	    Id idb = (id_cov+2)&3;
	    Id idc = (id_cov+3)&3;

	    v_simplex.push_back(vertex_map[fch->vertex(ida)]);
	    if(!lbool[acc]){
	      v_simplex.push_back(vertex_map[fch->vertex(idb)]);
	      v_simplex.push_back(vertex_map[fch->vertex(idc)]);
	    }else{
	      v_simplex.push_back(vertex_map[fch->vertex(idc)]);
	      v_simplex.push_back(vertex_map[fch->vertex(idb)]);
	    }
	    
            // for(int i = 0; i < D+1; ++i)
            // {
            //     if(i != id_cov)
            //     {
            //         Vertex_const_iterator v = fch->vertex(i);
            //         v_simplex.push_back(vertex_map[fch->vertex(0)]);
            //     }
            // }
	    acc++;
        }



        datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
        datas_out.dmap[datas_out.simplex_name] = ddt_data<Traits>::Data_ply(datas_out.simplex_name,"face",D,D,tinyply::Type::INT32);
        datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(format_points);
        datas_out.dmap[datas_out.simplex_name].fill_full_uint8_vect(v_simplex);
	datas_out.write_ply_stream(oth.get_output_stream(),PLY_CHAR,false,false,true);
	
	
        break;
    }
    default :             // Note the colon, not a semicolon
    {
        return 1;
        break;
    }
    }


    oth.finalize();
    std::cout << std::endl;

    return 0;
}





int extract_surface_area(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{

    std::vector<Facet_const_iterator> lft;
    std::vector<bool> lbool;
    std::cout.setstate(std::ios_base::failbit);
    DTW tri;
    Scheduler sch(1);
    wasure_algo w_algo;
    int D = Traits::D;

    D_MAP w_datas_tri;
    
    log.step("read");
    std::vector<Id> lid;
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
	lid.push_back(hid);
        if(hpi.get_lab() == "t")
        {
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
            w_datas_tri[hid].extract_labs(w_datas_tri[hid].format_labs,false);
        }
        tri.finalize(sch);
        hpi.finalize();
    }

    //    w_datas_tri[tid].extract_ptsvect(w_datas.xyz_name,w_datas.format_points,false);
    // Cell_const_iterator fch;

    log.step("compute");
    int mode = -1;
    //mode = 1;
	// if(params.mode.find(std::string("out")) != std::string::npos)
    mode = 0;


    for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
        try
        {
            if(fit->is_infinite())
                continue;

            Cell_const_iterator tmp_fch = fit.full_cell();
            int tmp_idx = fit.index_of_covertex();
            Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);




	    if(params.area_processed == 2  ){
	      bool is_edge =  false;
	      if((tmp_fch->has_id(lid[0]) && tmp_fchn->has_id(lid[1])) ||
		 (tmp_fch->has_id(lid[1]) && tmp_fchn->has_id(lid[0]))
		 ){
		is_edge = true;
	      // for(auto ii : lid){
	      // 	if(!tmp_fch->had_id(ii) || !tmp_fchn->had_id(ii) )
	      // 	is_edge = false;
	      }
	      if(!is_edge )
		continue;
	    }

	    if(params.area_processed == 1){
	      if(tmp_fch->is_mixed() || tmp_fchn->is_mixed())
	    	continue;
	     }

	    bool do_keep_local = false;
            if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	       !tri.tile_is_loaded(tmp_fchn->main_id())){
              //  continue;
	    }


	    
	    // if(
	    //    (params.area_processed == 1 && (!fit->is_local()))  ||
	    // 	(params.area_processed == 2 && !fit->is_mixed()))
            // continue;

            // if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	    //    !tri.tile_is_loaded(tmp_fchn->main_id())){

            //     continue;
	    // }



            Cell_const_iterator fch = tmp_fch->main();
            int id_cov = fit.index_of_covertex();
            Cell_const_iterator fchn = tmp_fchn->main();

	    if(!tri.tile_is_loaded(tmp_fch->main_id()))
	      fch = tmp_fch;
	    if(!tri.tile_is_loaded(tmp_fchn->main_id()))
	      fchn = tmp_fchn;


		
	    //            Vertex_h_iterator vht;

            int cccid = fch->lid();
            int cccidn = fchn->lid();

            int ch1lab = w_datas_tri[fch->tile()->id()].format_labs[cccid];
            int chnlab = w_datas_tri[fchn->tile()->id()].format_labs[cccidn];
            if(
                (ch1lab != chnlab)
	       ){
                lft.push_back(*fit);
		
		const Point& a = fch->vertex((id_cov+1)&3)->point();
		const Point& b = fch->vertex((id_cov+2)&3)->point();
		const Point& c = fch->vertex((id_cov+3)&3)->point();
		const Point& d = fch->vertex((id_cov)&3)->point();

		bool bl =
		  (CGAL::orientation(a,b,c,d) == 1 && chnlab == 0) ||
		  (CGAL::orientation(a,b,c,d) == -1 && chnlab == 1);
		lbool.push_back(!bl);
	    }

        }
        catch (ddt::DDT_exeption& e)
        {
            std::cerr << "!! WARNING !!!" << std::endl;
            std::cerr << "Exception catched : " << e.what() << std::endl;
            continue;
        }
    }




    log.step("write");

    std::cerr << "dim done tile : "<< tid << std::endl;
    std::string ply_name(params.output_dir +  "/" + params.slabel + "_id_" + std::to_string(tid) + "_surface");
    std::cout.clear();

    ddt::stream_data_header oth("p","z",tid);
    if(D == 2){
      oth.write_into_file(ply_name,".geojson");
      oth.write_header(std::cout);
    }
    // else
    //     oth.write_into_file(ply_name,".ply");


    if(lft.size() == 0)
      return 0;


    switch (D)
    {
    case 2 :
    {
        wasure::dump_2d_surface_geojson<DTW>(lft,oth.get_output_stream());
        break;
    }
    case 3 :
    {

        std::vector<Point>  format_points;
        std::vector<int> v_simplex;
        std::map<Vertex_const_iterator, uint> vertex_map;
        ddt_data<Traits> datas_out;

        int acc = 0;
        for(auto fit = lft.begin(); fit != lft.end(); ++fit)
        {
            Cell_const_iterator fch = fit->full_cell();
            int id_cov = fit->index_of_covertex();
            for(int i = 0; i < D+1; ++i)
            {
                if(i != id_cov)
                {
                    Vertex_const_iterator v = fch->vertex(i);
                    if(vertex_map.find(v) == vertex_map.end())
                    {
                        vertex_map[v] = acc++;
                        format_points.push_back(v->point());
                    }
                }
            }
        }

	acc=0;
        for(auto fit = lft.begin(); fit != lft.end(); ++fit)
        {
            Cell_const_iterator fch = fit->full_cell();
            int id_cov = fit->index_of_covertex();

	    
	    Id ida = (id_cov+1)&3;
	    Id idb = (id_cov+2)&3;
	    Id idc = (id_cov+3)&3;

	    v_simplex.push_back(vertex_map[fch->vertex(ida)]);
	    if(!lbool[acc]){
	      v_simplex.push_back(vertex_map[fch->vertex(idb)]);
	      v_simplex.push_back(vertex_map[fch->vertex(idc)]);
	    }else{
	      v_simplex.push_back(vertex_map[fch->vertex(idc)]);
	      v_simplex.push_back(vertex_map[fch->vertex(idb)]);
	    }
	    
            // for(int i = 0; i < D+1; ++i)
            // {
            //     if(i != id_cov)
            //     {
            //         Vertex_const_iterator v = fch->vertex(i);
            //         v_simplex.push_back(vertex_map[fch->vertex(0)]);
            //     }
            // }
	    acc++;
        }



        datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
        datas_out.dmap[datas_out.simplex_name] = ddt_data<Traits>::Data_ply(datas_out.simplex_name,"face",D,D,tinyply::Type::INT32);
        datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(format_points);
        datas_out.dmap[datas_out.simplex_name].fill_full_uint8_vect(v_simplex);
	datas_out.write_ply_stream(oth.get_output_stream(),PLY_CHAR);
	
	
        break;
    }
    default :             // Note the colon, not a semicolon
    {
        return 1;
        break;
    }
    }


    oth.finalize();
    std::cout << std::endl;

    return 0;
}


int extract_surface_area_old(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{

    std::vector<Facet_const_iterator> lft;

    std::cout.setstate(std::ios_base::failbit);
    DTW tri;
    Scheduler sch(1);
    wasure_algo w_algo;
    int D = Traits::D;

    D_MAP w_datas_tri;

    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
            w_datas_tri[hid].extract_labs(w_datas_tri[hid].format_labs,false);
        }
        hpi.finalize();
    }
    tri.finalize(sch);
    //    w_datas_tri[tid].extract_ptsvect(w_datas.xyz_name,w_datas.format_points,false);
    // Cell_const_iterator fch;

    log.step("compute");

    int id2 = tid;
    for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
        if(fit->is_infinite())
            continue;


        Cell_const_iterator tmp_fch = fit.full_cell();
        int tmp_idx = fit.index_of_covertex();
        Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

        if(
            (params.area_processed == 1 && ((tmp_fch->main_id() != tmp_fchn->main_id()) )) ||
            (params.area_processed == 2 && !((tmp_fch->main_id() != tmp_fchn->main_id()) )))
            continue;

        // if(
        //    (params.area_processed == 1 && !fit->is_local() ) ||
        //    (params.area_processed == 2 && fit->is_local()))
        //   continue;


        if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
                !tri.tile_is_loaded(tmp_fchn->main_id()))
            continue;

        if(tmp_fch->main_id() != tid)
            id2 = tmp_fch->main_id();
        if(tmp_fchn->main_id() != tid)
            id2 = tmp_fchn->main_id();

        Cell_const_iterator fch = tmp_fch->main();
        int id_cov = fit.index_of_covertex();
        Cell_const_iterator fchn = tmp_fchn->main();
	//        Vertex_h_iterator vht;

        int cccid = fch->cell_data().id;
        int cccidn = fchn->cell_data().id;

        int ch1lab = w_datas_tri[fch->tile()->id()].format_labs[cccid];
        int chnlab = w_datas_tri[fchn->tile()->id()].format_labs[cccidn];
        if(ch1lab != chnlab)
            lft.push_back(*fit);
    }
    if(lft.size() == 0)
        return 0;


    log.step("write");

    std::cerr << "dim done tile : "<< tid << std::endl;
    std::string ply_name(params.output_dir +  "/" + params.slabel + "_id1_" + std::to_string(tid) + "_id1_" + std::to_string(id2) + "_surface");
    std::cout.clear();

    ddt::stream_data_header oth("p","f",tid);

    if(D == 2)
        oth.write_into_file(ply_name,".geojson");
    else
        oth.write_into_file(ply_name,".ply");

    oth.write_header(std::cout);


    switch (D)
    {
    case 2 :
    {
        wasure::dump_2d_surface_geojson<DTW>(lft,oth.get_output_stream());
        break;
    }
    case 3 :
    {

        std::vector<Point>  format_points;
        std::vector<int> v_simplex;
        std::map<Vertex_const_iterator, uint> vertex_map;
        ddt_data<Traits> datas_out;

        int acc = 0;
        for(auto fit = lft.begin(); fit != lft.end(); ++fit)
        {
            Cell_const_iterator fch = fit->full_cell();
            int id_cov = fit->index_of_covertex();
            for(int i = 0; i < D+1; ++i)
            {
                if(i != id_cov)
                {
                    Vertex_const_iterator v = fch->vertex(i);
                    if(vertex_map.find(v) == vertex_map.end())
                    {
                        vertex_map[v] = acc++;
                        format_points.push_back(v->point());
                    }
                }
            }
        }

        for(auto fit = lft.begin(); fit != lft.end(); ++fit)
        {
            Cell_const_iterator fch = fit->full_cell();
            int id_cov = fit->index_of_covertex();
            for(int i = 0; i < D+1; ++i)
            {
                if(i != id_cov)
                {
                    Vertex_const_iterator v = fch->vertex(i);
                    v_simplex.push_back(vertex_map[v]);
                }
            }
        }



        datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
        datas_out.dmap[datas_out.simplex_name] = ddt_data<Traits>::Data_ply(datas_out.simplex_name,"face",D,D,tinyply::Type::INT32);
        datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(format_points);
        datas_out.dmap[datas_out.simplex_name].fill_full_uint8_vect(v_simplex);

        datas_out.write_ply_stream(oth.get_output_stream(),'\n',true);
        break;
    }
    default :
    {
        return 1;
        break;
    }
    }


    oth.finalize();
    std::cout << std::endl;

    return 0;
}





int gc_on_stream(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{
    std::cout.clear();
    gc_on_stream(std::cin,std::cout);
    //  hpi.finalize();
    //  oth.finalize();
    std::cerr << "gc done!" << std::endl;
    return 1;
}



int extract_graph(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{

    std::cout.setstate(std::ios_base::failbit);
    std::cerr << "seg_step0" << std::endl;

    DTW tri;
    Scheduler sch(1);

    std::cerr << "seg_step1" << std::endl;
    wasure_algo w_algo;
    D_MAP w_datas_tri;


    log.step("read");
    int D = Traits::D;
    std::map<int,std::vector<int>> tile_ids;;

    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);

        if(hpi.get_lab() == "t")
        {
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    std::cerr << "read ddt stream" << std::endl;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
            w_datas_tri[hid].extract_dst(w_datas_tri[hid].format_dst,false);
	    std::cerr << "extract dst stream done" << std::endl;
	    //            w_datas_tri[hid].extract_gids(w_datas_tri[hid].format_gids,false);
            std::vector<int>  & format_labs = w_datas_tri[hid].format_labs ;
	    std::cerr << "format_lab it" << std::endl;
            if(format_labs.size() == 0)
            {
	      //	        int nbs = w_datas_tri[hid].nb_simplex_uint8_vect();
		int nbs = w_datas_tri[hid].format_dst.size();
		//int nbs = tri.number_of_cells();
		std::cerr << "nbs:" << nbs << std::endl;
                for(int ss = 0; ss < nbs ; ss++)
                {
                    format_labs.push_back(0);
                }
            }
	    std::cerr << "format_lab it done" << std::endl;
        }
        if(hpi.get_lab() == "s")
        {
            std::vector<int> vv(3);
            for(int d = 0; d < 3; d++)
            {
                hpi.get_input_stream() >> vv[d];
            }
            tile_ids[hid] = vv;
        }
        tri.finalize(sch);
        hpi.finalize();
    }

    log.step("compute");
    std::cerr << "seg_step5" << std::endl;

    tbmrf_reco<DTW,D_MAP> mrf(params.nb_labs,&tri,&w_datas_tri);
    mrf.lambda = params.lambda;
    mrf.set_mode(-1);


    
    
    log.step("write");
    std::cout.clear();
    ddt::stream_data_header oth("t","z",tid),osh("s","s",tid);;
    //oth.write_header(std::cout);
    std::cerr << "seg_step6" << std::endl;
    //  int nbc = mrf.extract_factor_graph(1,tri,w_datas_tri,tile_ids,oth.get_output_stream(),tid);
    std::cerr << "area_processed:" << params.area_processed << std::endl;
    int nbc = 0;
    nbc = mrf.extract_stream_graph_v2(1,tri,w_datas_tri,tile_ids,oth.get_output_stream(),tid,params.graph_type,params.area_processed,params.coef_mult);

    std::cerr << "seg_step7" << std::endl;
    oth.finalize();
    std::cout << std::endl;

    if(params.graph_type == 0)
    {
        osh.write_header(std::cout);
        osh.get_output_stream() << 0 << " " ;
        osh.get_output_stream() << 0 << " " ;
        osh.get_output_stream() << nbc << " " ;
        osh.finalize();
        std::cout << std::endl;
    }
    return 0;
}






int fill_graph(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{

    std::cout.setstate(std::ios_base::failbit);
    std::cerr << "fill_step0" << std::endl;

    DTW tri;
    Scheduler sch(1);

    std::cerr << "fill_step1" << std::endl;
    wasure_algo w_algo;


    D_MAP w_datas_tri;
    int D = Traits::D;
    std::map<int,std::vector<int>> tile_ids;
    std::map<int,int> labs_map;
    bool tri_parsed = false;
    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
	std::cerr << "=== loop dat : " << hpi.get_lab() << std::endl;
	
        if(hpi.get_lab() == "t")
        {
            std::cerr << "read tri " << tid <<  std::endl;
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
            w_datas_tri[hid].extract_dst(w_datas_tri[hid].format_dst,false);
            w_datas_tri[hid].extract_labs(w_datas_tri[hid].format_labs,false);
            // std::vector<int>  & format_labs = w_datas_tri[hid].format_labs ;
            // if(format_labs.size() == 0){
            // 	int nbs = w_datas_tri[hid].nb_simplex_shpt_vect();
            // 	for(int ss = 0; ss < nbs ; ss++){
            // 	  format_labs.push_back(0);
            // 	}
            // }
            tri_parsed = true;

        }
        if(hpi.get_lab() == "s")
        {
            std::vector<int> vv(3);
            for(int d = 0; d < 3; d++)
            {
                hpi.get_input_stream() >> vv[d];
                std::cerr << "[STATS: " << d << " " << vv[d] << "]"<< std::endl;
            }
            tile_ids[hid] = vv;
        }
        if(hpi.get_lab() == "l")
        {
            int id,lab_val,nb_elems;
            hpi.get_input_stream() >> nb_elems;
            //      std::sort(id_vect.begin(), id_vect.end());
            std::cerr << "start fill nb_elems" << nb_elems<< std::endl;
            for(int ss = 0; ss < nb_elems ; ss++)
            {
                hpi.get_input_stream() >> id;
                hpi.get_input_stream() >> lab_val;
                //format_labs[g2lmap[id]] = lab_val;
                labs_map[id] = lab_val;
            }
            std::cerr << "filed map" << std::endl;
        }
        hpi.finalize();
    }
    tri.finalize(sch);
    std::cerr << "start processing" << std::endl;
    //      std::vector<int> id_vect;
    std::map<int,int> g2lmap;
    for( auto cit = tri.cells_begin();
            cit != tri.cells_end(); ++cit )
    {
        //	id_vect.push_back(cit->cell_data().id);
      int lid = cit->lid();
	//        int gid = w_datas_tri[tid].format_gids[lid] ;
      int gid = cit->gid();
      g2lmap[gid] = lid;
    }

    std::cerr << "start processing2" << std::endl;
    int nbs = tri.number_of_cells();
    //int nbs = w_datas_tri[tid].nb_simplex_uint8_vect();
    std::cerr << "start processing2b" << std::endl;
    std::vector<int>  & format_labs = w_datas_tri[tid].format_labs ;
    format_labs.resize(nbs);
    std::cerr << "resize => " <<  nbs << std::endl;
    for ( auto it = labs_map.begin(); it != labs_map.end(); it++ )
    {

        // if(g2lmap.find(it->first) == g2lmap.end()){
        // 	std::cerr << "ERROR NOT INT MAP:" << it->first << " " << it->second << std::endl;
        // }else{
        // 	if(g2lmap[it->first] > format_labs.size())
        // 	  std::cerr << "err !val:" << g2lmap[it->first] << std::endl;
        // 	//else

        format_labs[g2lmap[it->first]] = it->second;
        //}
    }


    log.step("compute");
    std::cerr << "fill_step5" << tid << std::endl;
    w_datas_tri[tid].fill_labs(w_datas_tri[tid].format_labs);
    std::cerr << "fill_step5b" << tid << std::endl;
    log.step("write");
    std::cout.clear();
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    if(params.dump_ply)
        oth.write_into_file(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    oth.finalize();
    std::cerr << "done" << tid << std::endl;
    std::cout << std::endl;

    return 0;
}


// int seg_lagrange_old(Id tid1,wasure_params & params,int nb_dat,ddt::logging_stream & log)
// {

//     std::cout.setstate(std::ios_base::failbit);
//     std::cerr << "START LAGRANGE" << std::endl;

//     DTW tri;
//     Scheduler sch(1);
//     wasure_algo w_algo;
//     D_MAP w_datas_tri;

//     double tau = params.tau;
//     bool is_first = true;
//     log.step("read");

//     std::map<Id,std::map<Id,SharedData> > edges_map;
    
//     for(int i = 0; i < nb_dat; i++)
//     {
//         ddt::stream_data_header hpi;
//         hpi.parse_header(std::cin);
//         Id hid = hpi.get_id(0);
// 	std::cerr << "PARSE : " << hpi.get_lab() << std::endl;
//         if(hpi.get_lab() == "t")
//         {

//             w_datas_tri[hid] = wasure_data<Traits>();
//             bool do_clean_data = false;
//             bool do_serialize = false;
// 	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
//             w_datas_tri[hid].extract_dst(w_datas_tri[hid].format_dst,false);

// 	    std::vector<int>  & format_labs = w_datas_tri[hid].format_labs ;
// 	    if(w_datas_tri[hid].dmap[w_datas_tri[hid].labseg_name].do_exist){
// 	      is_first = false;
// 	      w_datas_tri[hid].extract_labs(w_datas_tri[hid].format_labs,false);
// 	    }else
// 	      {
// 		int nbs = w_datas_tri[hid].format_dst.size();
//                 for(int ss = 0; ss < nbs ; ss++)
// 		  {
//                     format_labs.push_back(0);
//                 }
//             }
//         }
// 	if(hpi.get_lab() == "e")
//         {
// 	  Id eid1 = hpi.get_id(0);
// 	  Id eid2 = hpi.get_id(1);
// 	  edges_map[eid2] = std::map<Id,SharedData>();	  
// 	  std::map<Id,SharedData>  & lp = edges_map[eid2];
// 	  read_id_double_serialized(lp, hpi.get_input_stream());
//         }

	
//         tri.finalize(sch);
//         hpi.finalize();
//     }

//     //     ==== Compute the new lambda =====
//     // If a graph cut has been done before, computer the new lagrangian, otherwite do a first graphcut
//     Id tid_k = tid1;
//     Tile_const_iterator tile_k = tri.get_const_tile(tid_k);
//     Tile_iterator main_tile = tri.get_tile(tid_k);
//     std::cerr << "start update lagrange : " << std::endl;

//     std::cerr << "seg_step5_lagrange" << std::endl;
//     tbmrf_reco<DTW,D_MAP> mrf(params.nb_labs,&tri,&w_datas_tri);
    
//     for( auto cit_k = tile_k->cells_begin();
//     	 cit_k != tile_k->cells_end(); ++cit_k )
//       {
// 	Cell_const_iterator fch = Cell_const_iterator(tile_k,tile_k, tile_k, cit_k);
//     	  // lagrangian only on mixed cell
//       	  if(!tile_k->cell_is_mixed(cit_k) || tile_k->cell_is_infinite(cit_k))
//       	    continue;
//       	  Id lid_k = tile_k->lid(cit_k);
//     	  int lab_k = 0;
//     	  if(!is_first){
// 	    if(tid_k == 3)	    
// 	      std::cerr << "get labk" << std::endl;
// 	    lab_k = w_datas_tri[tid_k].format_labs[lid_k];
// 	    if(tid_k == 3)	    
// 	      std::cerr << "get labk done" << std::endl;
// 	  }

// 	  int D = tile_k->current_dimension();
//     	  // Loop on shared tiles

// 	  std::unordered_set<Id> idSet ;
// 	  // std::unordered_set<Id> idSet2 ;
//     	  // for(int i=0; i<=D; ++i)
//     	  //   {
//     	  //     Id tid_l = tile_k->id(tile_k->vertex(cit_k,i));
// 	  //     idSet2.insert(tid_l);
// 	  //   }
	  
//     	  for(int i=0; i<=D; ++i)
//     	    {
//     	      // Select only id != tid_k
//     	      Id tid_l = tile_k->id(tile_k->vertex(cit_k,i));
// 	      if (idSet.find(tid_l) != idSet.end()){
// 	      	continue;
// 	      }
// 	      idSet.insert(tid_l);
// 	      if(!tri.tile_is_loaded(tid_l)){
// 		std::cerr << "ERROR : TILE NOT LOADED!!" << std::endl;
// 		continue;
// 	      }
//     	      if(tid_l == tid_k)
//     		continue;
//     	      Tile_iterator tile_l = tri.get_tile(tid_l);
//     	      //auto cit_l = main_tile->locate_cell(*tile_l,cit_k);
// 	      auto cit_l = tile_l->locate_cell(*tile_k,cit_k);

	      
//     	      // Get curent lambda for the cell lid in the edge tid_l
//     	      double cur_lagrange = 0;
// 	      double cur_tau = mrf.get_volume_reco(fch);

	      
//     	      if(!is_first){
//     		cur_lagrange = std::get<1>(edges_map[tid_l][lid_k]);
// 		cur_tau = std::get<2>(edges_map[tid_l][lid_k]);
//     		Id lid_l = tile_l->lid(cit_l);	      
//     		int lab_l = w_datas_tri[tid_l].format_labs[lid_l];
// 		//    		TODO : Compute the new lambda
// 		if(tid_k == 3)
// 		  std::cerr << "KWXKK --- tid_k:" << tid_k << "\t tid_l:" << tid_l << "\t lid_k:" <<  lid_k << "\t lid_l:" << lid_l << "\t lab_k:" <<  lab_k << "\t lab_l:" << lab_l << "\t cur_lag:"<< cur_lagrange << "\t new_l:";
// 		if(tid_k < tid_l)
// 		  cur_lagrange  = cur_lagrange + cur_tau*(lab_k-lab_l);
// 		 else
// 		  cur_lagrange  = cur_lagrange + cur_tau*(lab_l-lab_k);
// 		if(tid_k == 3)
// 		  std::cerr  << cur_lagrange  << std::endl;
		
// 		//cur_tau = cur_tau/2;
//     	      }else{
// 		if(edges_map.find(tid_l) == edges_map.end())
// 		  edges_map[tid_l] = std::map<Id,SharedData>();
// 	      }
//     	      edges_map[tid_l][lid_k] = std::make_tuple(0,cur_lagrange,cur_tau);
//     	    }
//       }
//     std::cerr << "end lagrange" << std::endl;

//     // === Do a graphcut ============
//     log.step("compute");

//     mrf.lambda = params.lambda;
//     mrf.set_mode(-1);

//     mrf.opt_gc_lagrange(1,tri,w_datas_tri,edges_map,tid1);

//     log.step("finalize");
//     w_datas_tri[tid1].fill_labs(w_datas_tri[tid1].format_labs);

//     log.step("write");
//     std::cout.clear();


//     // ============== Dump results =================
    
//     //  log.step("Write header");
//     ddt::stream_data_header oth("t","z",tid1);
//     std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid1));
//     if(params.dump_ply)
//         oth.write_into_file(filename,".ply");
//     oth.write_header(std::cout);
//     ddt::write_ddt_stream(tri, w_datas_tri[tid1], oth.get_output_stream(),tid1,false,log);
//     oth.finalize();
//     std::cout << std::endl;


//     for(auto ee : edges_map){
//       Id tid2 = ee.first;
//       ddt::stream_data_header hto("e","z",std::vector<int> {tid1,tid2});
//       std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid1) + "_nid" + std::to_string(tid2));
//       //hto.write_into_file(filename,".pts");
//       if(params.dump_ply)
//     	hto.write_into_file(filename,".ply");
//       hto.write_header(std::cout);
//       write_id_double_serialized(ee.second,hto.get_output_stream());
//       hto.finalize();
//       std::cout << std::endl;
//     }
    
//     return 0;
// }




int seg_lagrange(Id tid_1,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{

    std::cout.setstate(std::ios_base::failbit);
    std::cerr << "START LAGRANGE" << std::endl;

    DTW tri;
    Scheduler sch(1);
    wasure_algo w_algo;
    D_MAP w_datas_tri;

    double tau = params.tau;
    bool is_first = true;
    log.step("read");

    std::map<Id,std::map<Id,SharedData> > shared_data_map;
    std::map<Id,std::map<Id,SharedData> > edges_data_map;    
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
	std::cerr << "PARSE : " << hpi.get_lab() << std::endl;
        if(hpi.get_lab() == "t")
        {

            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
            w_datas_tri[hid].extract_dst(w_datas_tri[hid].format_dst,false);

	    std::vector<int>  & format_labs = w_datas_tri[hid].format_labs ;
	    if(w_datas_tri[hid].dmap[w_datas_tri[hid].labseg_name].do_exist){
	      is_first = false;
	      w_datas_tri[hid].extract_labs(w_datas_tri[hid].format_labs,false);
	    }else
	      {
		int nbs = w_datas_tri[hid].format_dst.size();
                for(int ss = 0; ss < nbs ; ss++)
		  {
                    format_labs.push_back(0);
                }
            }
        }
	if(hpi.get_lab() == "e")
        {
	  Id eid1 = hpi.get_id(0);
	  Id eid2 = hpi.get_id(1);
	  shared_data_map[eid2] = std::map<Id,SharedData>();	  
	  std::map<Id,SharedData>  & lp = shared_data_map[eid2];

	  if(tid_1==3)
	    std::cerr << "READ_SHARED" << eid1 << "<->" << eid2 << std::endl;
	  read_id_double_serialized(lp, hpi.get_input_stream(),tid_1 == 3);
        }

	if(hpi.get_lab() == "f")
        {
	  Id eid1 = hpi.get_id(0);
	  Id eid2 = hpi.get_id(1);
	  edges_data_map[eid1] = std::map<Id,SharedData>();	  
	  std::map<Id,SharedData>  & lp = edges_data_map[eid1];
	  // if(tid_1==3)
	  //   std::cerr << "READ_EDGES" << eid2 << "<->" << eid1 << std::endl;
	  read_id_double_serialized(lp, hpi.get_input_stream(),tid_1 == 3);
        }

	tri.finalize(sch);
        hpi.finalize();
    }

    //     ==== Compute the new lambda =====
    // If a graph cut has been done before, computer the new lagrangian, otherwite do a first graphcut

    Tile_const_iterator tile_1 = tri.get_const_tile(tid_1);
    Tile_iterator main_tile = tri.get_tile(tid_1);
    std::cerr << "start update lagrange : " << std::endl;

    std::cerr << "seg_step5_lagrange" << std::endl;
    tbmrf_reco<DTW,D_MAP> mrf(params.nb_labs,&tri,&w_datas_tri);
    mrf.lambda = params.lambda;
    mrf.set_mode(-1);


    int acc_tot = 0;
    int acc_diff = 0;


    std::cerr<< std::fixed << std::setprecision(15);
    for( auto cit_1 = tile_1->cells_begin();
    	 cit_1 != tile_1->cells_end(); ++cit_1 )
      {

	Cell_const_iterator fch = Cell_const_iterator(tile_1,tile_1, tile_1, cit_1);
    	  // lagrangian only on mixed cell
      	  if(!tile_1->cell_is_mixed(cit_1) || tile_1->cell_is_infinite(cit_1))
      	    continue;
      	  Id lid_1 = tile_1->lid(cit_1);
    	  int lab_1 = 0;
    	  if(!is_first){
	    lab_1 = w_datas_tri[tid_1].format_labs[lid_1];
	  }

	  int D = tile_1->current_dimension();
    	  // Loop on shared tiles

	  std::unordered_set<Id> idSet ;
	  // std::unordered_set<Id> idSet2 ;
    	  // for(int i=0; i<=D; ++i)
    	  //   {
    	  //     Id tid_l = tile_1->id(tile_1->vertex(cit_1,i));
	  //     idSet2.insert(tid_l);
	  //   }
	  
    	  for(int i=0; i<=D; ++i)
    	    {
    	      // Select only id != tid_1
    	      Id tid_2 = tile_1->id(tile_1->vertex(cit_1,i));
	      if (idSet.find(tid_2) != idSet.end()){
	      	continue;
	      }
	      idSet.insert(tid_2);
	      // if(!tri.tile_is_loaded(tid_2)){
	      // 	std::cerr << "ERROR : TILE NOT LOADED!!" << std::endl;
	      // 	continue;
	      // }
    	      if(tid_2 == tid_1)
    		continue;
	      //    	      Tile_iterator tile_2 = tri.get_tile(tid_2);
    	      //auto cit_2 = main_tile->locate_cell(*tile_2,cit_1);
	      //	      auto cit_2 = tile_2->locate_cell(*tile_1,cit_1);

	      
    	      // Get curent lambda for the cell lid in the edge tid_2
    	      double new_lagrange = 0;
	      double cur_tau = mrf.get_volume_reco(fch) * params.lambda * params.coef_mult;
	      Id lid_2 = std::get<0>(shared_data_map[tid_2][lid_1]);
	      double v_diff = 0;
		
    	      if(!is_first){
    		new_lagrange = std::get<1>(shared_data_map[tid_2][lid_1]);
		cur_tau = std::get<2>(shared_data_map[tid_2][lid_1]);
		double old_lagrange = new_lagrange;
		double old_diff = std::get<3>(shared_data_map[tid_2][lid_1]);
		int lab_2 = std::get<1>(edges_data_map[tid_2][lid_2]); 
		//    		TODO : Compute the new lambda


		if(lab_2 != lab_1)
		  acc_diff++;
		acc_tot++;


		if(tid_1 < tid_2)
		  v_diff = lab_1-lab_2;
		else
		  v_diff = lab_2-lab_1;

		new_lagrange  = new_lagrange + cur_tau*(v_diff);
		if(old_diff != v_diff)
		  cur_tau /=2;
		// else if(old_diff == v_diff && lab_2 != lab_1)
		//   cur_tau *=2;

    	      }else{
		if(shared_data_map.find(tid_2) == shared_data_map.end())
		  shared_data_map[tid_2] = std::map<Id,SharedData>();
	      }

    	      shared_data_map[tid_2][lid_1] = std::make_tuple(lid_2,new_lagrange,cur_tau,v_diff);
    	    }
      }



    // ============== Debug results ============
    if(params.dump_debug && false){
      Traits  traits;
      std::vector<Point> pvect;
      std::vector<int> v_labs;
      ddt_data<Traits> datas_out;

      std::vector<std::string> label_name = {"lab"};
	
      datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",3,3,DATA_FLOAT_TYPE);
      datas_out.dmap[label_name] = ddt_data<Traits>::Data_ply(label_name,"vertex",1,1,tinyply::Type::INT32);
      std::string filename_debug(params.output_dir + "/" + params.slabel + "_" + std::to_string(tid_1) + "_debug.ply");
      for( auto cit_k = tile_1->cells_begin();
	   cit_k != tile_1->cells_end(); ++cit_k )
	{
	  Cell_const_iterator fch = Cell_const_iterator(tile_1,tile_1, tile_1, cit_k);
    	  // lagrangian only on mixed cell
      	  if(!tile_1->cell_is_mixed(cit_k) || tile_1->cell_is_infinite(cit_k))
      	    continue;

	  Id lid_1 = tile_1->lid(cit_k);
	  int lab_1 = w_datas_tri[tid_1].format_labs[lid_1];
	  
	  std::vector<double> bary = tile_1->get_cell_barycenter(cit_k);
	  pvect.push_back(traits.make_point(bary.begin()));
	  v_labs.push_back(lab_1);
	}
      datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(pvect);
      datas_out.dmap[label_name].fill_full_uint8_vect(v_labs);

      std::ofstream ofile;
      // ofile.open(filename_debug);
      // datas_out.write_ply_stream(ofile,'\n',true);
      // ofile.close();


      std::ostringstream ss;
      ss  << std::setfill('0') << std::setw(3) << tid_1;
      std::string filename_debug2(params.output_dir + "/" + params.slabel + "_" + ss.str() + "_debug.txt");
      ofile.open(filename_debug2);

      for( auto cit_1 = tile_1->cells_begin();
	   cit_1 != tile_1->cells_end(); ++cit_1 )
      {
	Cell_const_iterator fch = Cell_const_iterator(tile_1,tile_1, tile_1, cit_1);
    	  // lagrangian only on mixed cell
      	  if(!tile_1->cell_is_mixed(cit_1) || tile_1->cell_is_infinite(cit_1))
      	    continue;
      	  Id lid_1 = tile_1->lid(cit_1);
	  int lab_1 = w_datas_tri[tid_1].format_labs[lid_1];
	  int D = tile_1->current_dimension();
	  std::unordered_set<Id> idSet ;
	  ofile << params.slabel << "_" << tid_1 << " lid_1:" << lid_1 << " lab_1:" << lab_1 << " -- ";
	  std::vector<double> bary = tile_1->get_cell_barycenter(cit_1);
    	  for(int i=0; i<D; ++i)
	    ofile << bary[i] << " ";
	  ofile << " -- ";
	  int conv = 1;
	  int card = 1;
    	  for(int i=0; i<=D; ++i){
	    Id tid_2 = tile_1->id(tile_1->vertex(cit_1,i));
	    idSet.insert(tid_2);
	  }
	  card = idSet.size();
	  idSet.clear();
    	  for(int i=0; i<=D; ++i)
    	    {
    	      Id tid_2 = tile_1->id(tile_1->vertex(cit_1,i));
	      if (idSet.find(tid_2) != idSet.end()){
	      	continue;
	      }
	      idSet.insert(tid_2);
    	      if(tid_2 == tid_1)
    		continue;
	      Id lid_2 = std::get<0>(shared_data_map[tid_2][lid_1]);
	      int lab_2 = std::get<1>(edges_data_map[tid_2][lid_2]);
	      double new_lagrange = std::get<1>(shared_data_map[tid_2][lid_1]);
	      double cur_tau = std::get<2>(shared_data_map[tid_2][lid_1]);
	      double c_i = mrf.get_score_linear(fch,0,w_datas_tri);
	      double c_j = mrf.get_score_linear(fch,1,w_datas_tri);
	      
	      ofile << " tid_2:" << tid_2 << " lid_2:" << lid_2 << " lab_2:" << lab_2
		    << " cur_lag:" << new_lagrange << " cur_tau:" << cur_tau 
		    << " ci:" << c_i << " cj:" << c_j <<  " card:" << card
		    << " -- ";
	      if(lab_2 != lab_1)
		conv = 0;
	    }
	  
	  ofile << "conv:" << conv << std::endl;
      }
      ofile.close();
    }



    
    std::cerr << "seg_step6_lagrange" << std::endl;

    // === Do a graphcut ============
    log.step("compute");
    mrf.opt_gc_lagrange(1,tri,w_datas_tri,shared_data_map,tid_1,params.use_weight);


    log.step("write");
    std::cout.clear();


    std::cerr << "seg_step7_lagrange" << std::endl;
    edges_data_map.clear();
    // ==== Build new edges =====

    for( auto cit_1 = tile_1->cells_begin();
    	 cit_1 != tile_1->cells_end(); ++cit_1 )
      {
	Cell_const_iterator fch = Cell_const_iterator(tile_1,tile_1, tile_1, cit_1);
	// lagrangian only on mixed cell
	if(!tile_1->cell_is_mixed(cit_1) || tile_1->cell_is_infinite(cit_1))
	  continue;
	Id lid_1 = tile_1->lid(cit_1);
	int lab_1 = w_datas_tri[tid_1].format_labs[lid_1];
	int D = tile_1->current_dimension();
	std::unordered_set<Id> idSet ;	  
	for(int i=0; i<=D; ++i)
	  {
	    // Select only id != tid_1
	    Id tid_2 = tile_1->id(tile_1->vertex(cit_1,i));
	    Id lid_2 = std::get<0>(shared_data_map[tid_2][lid_1]);
	    if (idSet.find(tid_2) != idSet.end()){
	      continue;
	    }
	    idSet.insert(tid_2);
	    if(tid_2 == tid_1)
	      continue;


	    if(edges_data_map.find(tid_2) == edges_data_map.end())
	      edges_data_map[tid_2] = std::map<Id,SharedData>();
	    //  send to the neighbor tile the value of the tets at LID_1 
	    edges_data_map[tid_2][lid_1] = std::make_tuple(lid_1,((double)lab_1),0,0);

	  }
      }    
    log.step("finalize");
    w_datas_tri[tid_1].fill_labs(w_datas_tri[tid_1].format_labs);

    
    std::cerr << "seg_step8_lagrange" << std::endl;
    // ============== Dump results =================
    
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid_1);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid_1));
    if(params.dump_ply)
        oth.write_into_file(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid_1], oth.get_output_stream(),tid_1,false,log);
    oth.finalize();
    std::cout << std::endl;


    for(auto ee : shared_data_map){
      Id tid2 = ee.first;
      if(tid2 == tid_1)
	continue;
      ddt::stream_data_header hto("e","z",std::vector<int> {tid_1,tid2});
      std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid_1) + "_nid" + std::to_string(tid2));
      //hto.write_into_file(filename,".pts");
      if(params.dump_ply)
    	hto.write_into_file(filename,".ply");
      hto.write_header(std::cout);
      write_id_double_serialized(ee.second,hto.get_output_stream(),tid_1 ==3);
      hto.finalize();
      std::cout << std::endl;
    }


    for(auto ee : edges_data_map){
      Id tid2 = ee.first;
      ddt::stream_data_header hto("f","z",std::vector<int> {tid_1,tid2});
      std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid_1) + "_nid" + std::to_string(tid2));
      //hto.write_into_file(filename,".pts");
      if(params.dump_ply)
    	hto.write_into_file(filename,".ply");
      hto.write_header(std::cout);
      write_id_double_serialized(ee.second,hto.get_output_stream(),tid_1 ==3);
      hto.finalize();
      std::cout << std::endl;
    }



    // Dump stats
    ddt::stream_data_header sth("s","z",tid_1);
    sth.write_header(std::cout);
    std::cout << "[diff:tot] " << acc_diff << " " << acc_tot ;
    sth.finalize();
    std::cout << std::endl;


    
    return 0;
}



int seg(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{

    std::cout.setstate(std::ios_base::failbit);
    std::cerr << "seg_step0" << std::endl;

    DTW tri;
    Scheduler sch(1);

    std::cerr << "seg_step1" << std::endl;
    wasure_algo w_algo;


    D_MAP w_datas_tri;


    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {
            //std::cerr << "read filename: " << hpi.get_file_name() << " [" << hpi.get_lab() << "]" << std::endl;
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
            w_datas_tri[hid].extract_dst(w_datas_tri[hid].format_dst,false);

            std::vector<int>  & format_labs = w_datas_tri[hid].format_labs ;
            if(format_labs.size() == 0)
            {
	      //int nbs = w_datas_tri[hid].nb_simplex_uint8_vect();
		int nbs = w_datas_tri[hid].format_dst.size();
                for(int ss = 0; ss < nbs ; ss++)
                {
                    format_labs.push_back(0);
                }
            }
            //      std::cerr << "done filename: " << hpi.get_file_name() << " [" << hpi.get_lab() << "]" << std::endl;
        }

        tri.finalize(sch);
        hpi.finalize();
    }

    // ===== Init the id of each cell


    log.step("compute");
    std::cerr << "seg_step5" << std::endl;
    tbmrf_reco<DTW,D_MAP> mrf(params.nb_labs,&tri,&w_datas_tri);
    mrf.lambda = params.lambda;
    mrf.set_mode(-1);
    mrf.opt_gc(1,tri,w_datas_tri);

    log.step("finalize");
    w_datas_tri[tid].fill_labs(w_datas_tri[tid].format_labs);

    log.step("write");
    std::cout.clear();
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    if(params.dump_ply)
        oth.write_into_file(filename,".ply");
    oth.write_header(std::cout);


    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    oth.finalize();
    std::cout << std::endl;


    return 0;
}




int seg_global_extract(Id tid,wasure_params & params,int nb_dat,ddt::logging_stream & log)
{

    std::cout.setstate(std::ios_base::failbit);
    std::cerr << "seg_step0" << std::endl;

    DTW tri;
    Scheduler sch(1);

    std::cerr << "seg_step1" << std::endl;
    wasure_algo w_algo;

    D_MAP w_datas_tri_ori;
    D_MAP w_datas_tri;

    std::map<Id,std::vector<int>> v_map;

    log.step("read");
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {
            //std::cerr << "read filename: " << hpi.get_file_name() << " [" << hpi.get_lab() << "]" << std::endl;
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
            w_datas_tri[hid].extract_dst(w_datas_tri[hid].format_dst,false);

	    std::vector<int>  & format_labs = w_datas_tri[hid].format_labs ;
	    if(w_datas_tri[hid].dmap[w_datas_tri[hid].labseg_name].do_exist){
	      w_datas_tri[hid].extract_labs(w_datas_tri[hid].format_labs,false);
	    }
            else
	      {
		//int nbs = w_datas_tri[hid].nb_simplex_uint8_vect();
		int nbs = w_datas_tri[hid].format_dst.size();
                for(int ss = 0; ss < nbs ; ss++)
		  {
                    format_labs.push_back(0);
		  }
	      }
            //      std::cerr << "done filename: " << hpi.get_file_name() << " [" << hpi.get_lab() << "]" << std::endl;
        }

        tri.finalize(sch);
        hpi.finalize();
    }

    tbmrf_reco<DTW,D_MAP> mrf_ori(params.nb_labs,&tri,&w_datas_tri);
    mrf_ori.lambda = params.lambda;
    mrf_ori.set_mode(-1);
    double energy_largrange = mrf_ori.get_energy(tri,w_datas_tri);
    
    // Clean new struct
    for (auto it = w_datas_tri.begin(); it != w_datas_tri.end(); it++)
      {
	int hid = it->first;    // string (key)
	std::vector<int>  & format_labs = w_datas_tri[hid].format_labs ;
	std::vector<int> format_labs_new(format_labs);
	v_map[hid] = format_labs_new;
	std::fill(format_labs.begin(), format_labs.end(), 0);
      }


    
    // ===== Init the id of each cell
    log.step("compute");
    std::cerr << "seg_step5" << std::endl;
    tbmrf_reco<DTW,D_MAP> mrf(params.nb_labs,&tri,&w_datas_tri);
    mrf.lambda = params.lambda;
    mrf.set_mode(-1);
    mrf.opt_gc(1,tri,w_datas_tri);
    double energy_full = mrf.get_energy(tri,w_datas_tri);
    std::cerr << "seg_setp6 stats" << std::endl;
    // Stats and finalizing
    int acc_tot = 0;
    int acc_diff = 0;
    double wacc_tot = 0;
    double wacc_diff = 0;

    for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            if(cit->is_foreign())
            {
                continue;
            }

	    int cccid = cit->lid();
            Cell_const_iterator fch = *cit;
	    double cur_tau =  mrf.get_volume_reco(fch);
	    // En théorie lcurr = 0, mais ici on a une fonction générique.
            int lcurr = w_datas_tri[fch->tile()->id()].format_labs[cccid];
	    int lnew = v_map[fch->tile()->id()][cccid];
	    if(lcurr != lnew){
	      acc_diff++;
	      wacc_diff+=cur_tau;
	    }
	    acc_tot++;
	    wacc_tot+=cur_tau;
	}
    

    
    // =============

    // SURFACE EXTRACTION 
    std::vector<Facet_const_iterator> lft;
    std::vector<bool> lbool;
    std::cout.setstate(std::ios_base::failbit);
      int D = Traits::D;

    int mode = -1;
    //mode = 1;
    // if(params.mode.find(std::string("out")) != std::string::npos)
    mode = 0;

    std::cerr << "seg_setp7 surface extraction" << std::endl;

    for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
        try
        {
            if(fit->is_infinite())
                continue;

	  // if(fit->is_infinite())
	  //   continue;


            Cell_const_iterator tmp_fch = fit.full_cell();
            int tmp_idx = fit.index_of_covertex();
            Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

            if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	       !tri.tile_is_loaded(tmp_fchn->main_id())){
	      std::cerr << "ERROR tile not loaded" << std::endl;
                continue;
	    }

	    bool is_on_convex = false;
	    if(tmp_fch->is_infinite() ||  tmp_fchn->is_infinite() )
	      is_on_convex = true;


            Cell_const_iterator fch = tmp_fch->main();
            int id_cov = fit.index_of_covertex();
            Cell_const_iterator fchn = tmp_fchn->main();
	    //            Vertex_h_iterator vht;

            int cccid = fch->lid();
            int cccidn = fchn->lid();

            int ch1lab = w_datas_tri[fch->tile()->id()].format_labs[cccid];
            int chnlab = w_datas_tri[fchn->tile()->id()].format_labs[cccidn];
            if(
	       (ch1lab != chnlab)  || (mode == 0 && (is_on_convex && (ch1lab == 0 || chnlab == 0)))
	       ){
                lft.push_back(*fit);
		
		const Point& a = fch->vertex((id_cov+1)&3)->point();
		const Point& b = fch->vertex((id_cov+2)&3)->point();
		const Point& c = fch->vertex((id_cov+3)&3)->point();
		const Point& d = fch->vertex((id_cov)&3)->point();

		bool bl =
		  (CGAL::orientation(a,b,c,d) == 1 && chnlab == 0) ||
		  (CGAL::orientation(a,b,c,d) == -1 && chnlab == 1);
		lbool.push_back(!bl);
	    }

        }
        catch (ddt::DDT_exeption& e)
        {
            std::cerr << "!! WARNING !!!" << std::endl;
            std::cerr << "Exception catched : " << e.what() << std::endl;
            continue;
        }
    }





    std::string ply_name(params.output_dir +  "/" + params.slabel + "_id_surface");
    std::cout.clear();

    // Dump stats
    ddt::stream_data_header sth("s","z",tid);
    sth.write_header(std::cout);
    //std::cout << "[diff:tot] " << acc_diff << " " << acc_tot << " " << wacc_diff << " " << wacc_tot;
    std::cout << "[diff:tot:e_ori:e_new] " << wacc_diff << " " << wacc_tot << " " << energy_largrange << " " << energy_full;
    sth.finalize();
    std::cout << std::endl;

    
    ddt::stream_data_header oth("p","z",0);
    if(D == 2){
      oth.write_into_file(ply_name,".geojson");
      oth.write_header(std::cout);
    }

    // else
    //     oth.write_into_file(ply_name,".ply");




    switch (D)
    {
    case 2 :
    {
        wasure::dump_2d_surface_geojson<DTW>(lft,oth.get_output_stream());
        break;
    }
    case 3 :
    {

        std::vector<Point>  format_points;
        std::vector<int> v_simplex;
        std::map<Vertex_const_iterator, uint> vertex_map;
        ddt_data<Traits> datas_out;

        int acc = 0;
        for(auto fit = lft.begin(); fit != lft.end(); ++fit)
        {
            Cell_const_iterator fch = fit->full_cell();
            int id_cov = fit->index_of_covertex();
            for(int i = 0; i < D+1; ++i)
            {
                if(i != id_cov)
                {
                    Vertex_const_iterator v = fch->vertex(i);
                    if(vertex_map.find(v) == vertex_map.end())
                    {
                        vertex_map[v] = acc++;
                        format_points.push_back(v->point());
                    }
                }
            }
        }

	acc=0;
        for(auto fit = lft.begin(); fit != lft.end(); ++fit)
        {
            Cell_const_iterator fch = fit->full_cell();
            int id_cov = fit->index_of_covertex();

	    
	    Id ida = (id_cov+1)&3;
	    Id idb = (id_cov+2)&3;
	    Id idc = (id_cov+3)&3;

	    v_simplex.push_back(vertex_map[fch->vertex(ida)]);
	    if(!lbool[acc]){
	      v_simplex.push_back(vertex_map[fch->vertex(idb)]);
	      v_simplex.push_back(vertex_map[fch->vertex(idc)]);
	    }else{
	      v_simplex.push_back(vertex_map[fch->vertex(idc)]);
	      v_simplex.push_back(vertex_map[fch->vertex(idb)]);
	    }
	    
            // for(int i = 0; i < D+1; ++i)
            // {
            //     if(i != id_cov)
            //     {
            //         Vertex_const_iterator v = fch->vertex(i);
            //         v_simplex.push_back(vertex_map[fch->vertex(0)]);
            //     }
            // }
	    acc++;
        }

        std::cerr << format_points.size() << std::endl;

        datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
        datas_out.dmap[datas_out.simplex_name] = ddt_data<Traits>::Data_ply(datas_out.simplex_name,"face",D,D,tinyply::Type::INT32);
        datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(format_points);
        datas_out.dmap[datas_out.simplex_name].fill_full_uint8_vect(v_simplex);
	datas_out.write_ply_stream(oth.get_output_stream(),PLY_CHAR);
	
	
        break;
    }
    default :             // Note the colon, not a semicolon
    {
        return 1;
        break;
    }
    }


    oth.finalize();
    std::cout << std::endl;

    return 0;





    

    return 0;
}




int tri2geojson(Id tid,wasure_params & params, int nb_dat,ddt::logging_stream & log)
{

    std::cerr << " ===== reading ====== " << std::endl;
    std::cout.setstate(std::ios_base::failbit);

    DTW tri;
    Scheduler sch(1);
    wasure_algo w_algo;
    int D = Traits::D;

    D_MAP w_datas_tri;


    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        Id hid = hpi.get_id(0);
        if(hpi.get_lab() == "t")
        {
            w_datas_tri[hid] = wasure_data<Traits>();
            bool do_clean_data = false;
            bool do_serialize = false;
	    ddt::read_ddt_stream(tri,w_datas_tri[hid], hpi.get_input_stream(),hid,do_serialize,do_clean_data,log);
            w_datas_tri[hid].extract_labs(w_datas_tri[hid].format_labs,false);
        }
        tri.finalize(sch);
        hpi.finalize();
    }




    std::string json_name(params.output_dir +  "/" + params.slabel + "_" + std::to_string(tid) + "_tri");
    std::cout.clear();
    ddt::stream_data_header oth("j","h",tid);
    oth.write_into_file(json_name,".geojson");
    oth.write_header(std::cout);
    wasure::write_geojson_tri_wasure(tri,w_datas_tri,oth.get_output_stream());
    oth.finalize();
    std::cout << std::endl;
    return 0;
}





int ply2geojson(Id tile_id,wasure_params & params,int nb_dat)
{
    std::cout.setstate(std::ios_base::failbit);
    std::cerr << "into fun" << std::endl;

    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        wasure_data<Traits> w_datas;
        hpi.parse_header(std::cin);

        if(hpi.get_lab() == "z")
        {
            std::cerr << "start read ply" << std::endl;
            //w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
	    w_datas.read_serialized_stream(hpi.get_input_stream());
            std::cerr << "end read ply" << std::endl;
        }
        hpi.finalize();
        std::cerr << "finalize" << std::endl;

        std::string json_name(params.output_dir +  "/" + params.slabel + "_" + std::to_string(tile_id) + "_geo");

        std::cout.clear();
        Id id = hpi.get_id(0);
        ddt::stream_data_header oqh_1("p","s",id),oqh_2("p","s",id),oqh_3("p","s",id);
        std::string filename(params.output_dir + "/" + params.slabel +"_id_"+ std::to_string(tile_id) + "_" + std::to_string(id));
        oqh_1.write_into_file(filename,"_pts.geojson");
        oqh_1.write_header(std::cout);
        oqh_2.write_into_file(filename,"_spx.geojson");
        oqh_2.write_header(std::cout);
        oqh_3.write_into_file(filename,"_nrm.geojson");
        oqh_3.write_header(std::cout);
        //w_datas.write_geojson_tri(oqh_1.get_output_stream(),oqh_2.get_output_stream());
        //w_datas.write_geojson_tri(oqh_1.get_output_stream(),oqh_2.get_output_stream());
        w_datas.write_geojson_norms(oqh_3.get_output_stream());

        oqh_1.finalize();
        oqh_2.finalize();
        oqh_3.finalize();

        ddt::add_qgis_style(oqh_2.get_file_name(), std::string("cell_style_flag.qml"));

        std::cout << std::endl;
    }

    return 0;
}


int hello(Id tile_id,wasure_params & params,int nb_dat)
{
    std::cout.setstate(std::ios_base::failbit);
    std::cerr << "into hello" << std::endl;



    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        wasure_data<Traits> w_datas;
        hpi.parse_header(std::cin);
        hpi.finalize();
        std::cerr << "finalize" << std::endl;
        std::cout.clear();
        ddt::stream_data_header oth("j","f",tile_id);
        oth.write_into_file("hello",".json");
        oth.write_header(std::cout);
        oth.finalize();
        std::cout << std::endl;
    }

    return 0;
}




int main(int argc, char **argv)
{

    //  std::cerr.setstate(std::ios_base::failbit);
    std::cout.setstate(std::ios_base::failbit);
    wasure_params params;
    params.parse(argc,argv);
    int rv = 0;
    int acc = 0;
    bool do_dump_log = true;
    while(true)
    {
        ddt::stream_app_header sah;
        Id tile_id = -1;
        int nb_dat = -1;
        if(!params.skip_app_header)
        {
            sah.parse_header(std::cin);
            if(sah.is_void())
                return 0;
            tile_id = ((Id)sah.tile_id);
            nb_dat = sah.get_nb_dat();

        }
        srand(params.seed*tile_id);
        ddt::logging_stream log(std::to_string(tile_id) + "_" + params.algo_step, params.log_level);

	if(acc == 0){
        std::cerr << " ======================================================= " << std::endl;
        std::cerr << "     [MAIN_DDT_STREAM_LOG] stream  : " << params.slabel << std::endl;
        std::cerr << "     [MAIN_DDT_STREAM_LOG] tile_id : " << tile_id <<  std::endl;
        std::cerr << "     [MAIN_DDT_STREAM_LOG] step    : " << params.algo_step << std::endl;
        std::cerr << "     [MAIN_DDT_STREAM_LOG] acc     : " << acc++ << std::endl;
	std::cerr << "     [MAIN_DDT_STREAM_LOG] nb dat  : " << nb_dat << std::endl;
        std::cerr << " ======================================================= " << std::endl;
	}
        try
        {
            if(params.algo_step == std::string("hello"))
            {
                rv = hello(tile_id,params,nb_dat);
            }
            else if(params.algo_step == std::string("dim"))
            {
	      //rv = dim_with_crown(tile_id,params,nb_dat,log);
		rv = dim_splitted(tile_id,params,nb_dat,log);
            }
            else if(params.algo_step == std::string("simplify"))
            {
                rv = simplify(tile_id,params,nb_dat);
            }
            else if(params.algo_step == std::string("preprocess"))
            {
                rv = preprocess(tile_id,params,nb_dat);
            }
	    else if(params.algo_step == std::string("regularize_slave_focal"))
            {
	      rv = regularize_slave_focal(tile_id,params,nb_dat,log);
            }
	    else if(params.algo_step == std::string("regularize_slave_extract"))
            {
	      rv = regularize_slave_extract(tile_id,params,nb_dat,log);
            }
	    else if(params.algo_step == std::string("regularize_slave_insert"))
            {
	      rv = regularize_slave_insert(tile_id,params,nb_dat,log);
            }
            else if(params.algo_step == std::string("dst"))
            {
                if(params.mode == std::string("surface"))
                    rv = dst_new(tile_id,params,nb_dat,log);
                else if(params.mode == std::string("conflict"))
                    rv = dst_conflict(tile_id,params,nb_dat,log);
            }
            else if(params.algo_step == std::string("seg"))
            {
                rv = seg(tile_id,params,nb_dat,log);
            }
	    else if(params.algo_step == std::string("seg_global_extract"))
            {
                rv = seg_global_extract(tile_id,params,nb_dat,log);
            }
	    else if(params.algo_step == std::string("seg_lagrange_raw"))
            {
	      params.use_weight = false;
	      rv = seg_lagrange(tile_id,params,nb_dat,log);
            }
	    else if(params.algo_step == std::string("seg_lagrange_weight"))
            {
	      params.use_weight = true;
	      rv = seg_lagrange(tile_id,params,nb_dat,log);
            }
            else if(params.algo_step == std::string("extract_surface"))
            {
                if(params.area_processed == 0)
                    rv = extract_surface(tile_id,params,nb_dat,log);
                else
                    rv = extract_surface_area(tile_id,params,nb_dat,log);
		//		do_dump_log = false;
            }
            else if(params.algo_step == std::string("extract_graph"))
            {
                rv = extract_graph(tile_id,params,nb_dat,log);
		//                do_dump_log = false;
            }
            else if(params.algo_step == std::string("fill_graph"))
            {
                rv = fill_graph(tile_id,params,nb_dat,log);
            }
            else if(params.algo_step == std::string("gc_on_stream"))
            {
                rv = gc_on_stream(tile_id,params,nb_dat,log);
                std::cerr << "finish" << std::endl;
                return 0;
		//                do_dump_log = false;
            }
            else if(params.algo_step == std::string("tri2geojson"))
            {
                rv = tri2geojson(tile_id,params,nb_dat,log);
            }
            else if(params.algo_step == std::string("ply2geojson"))
            {
                if(Traits::D == 2)
                {
                    rv = ply2geojson(tile_id,params,nb_dat);
                }
                else
                {
                    rv = 0;
                }
            }
            else
            {
                std::cerr << "==== exe : no params ===" << std::endl;
                return 1;
            }
            if(rv != 0) return rv;
            if(do_dump_log)
            {
                // ddt::stream_data_header olh("l","s",tile_id);
                // olh.write_header(std::cout);
                //log.dump_log(olh.get_output_stream());
                // olh.finalize();
                // std::cout << std::endl;
	      log.dump_log(std::cerr);
            }
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception catched : " << e.what() << std::endl;
            std::cerr << "tile_id               : " << tile_id << std::endl;
        }


    }

    std::cerr << "[MAIN_DDT_STREAM_LOG] end exe " << std::endl;
    return rv;
}





