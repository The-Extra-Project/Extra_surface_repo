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
                oqh.init_file_name(filename,".ply");
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
    int max_ppt = 200000;
    std::map<Id,wasure_data<Traits> > datas_map;
    std::map<Id,std::string > fname_map;

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

	
	if(datas_map[hid].dmap[datas_map[hid].xyz_name].type == tinyply::Type::FLOAT64){
	  datas_map[hid].dmap[datas_map[hid].xyz_name].shpt_vect2uint8_vect();
	}else{
	  std::vector<float> v_fxyz;
	  datas_map[hid].dmap[datas_map[hid].xyz_name].extract_full_shpt_vect(v_fxyz,false);
	  std::vector<double> doubleVec(v_fxyz.begin(),v_fxyz.end());

	  datas_map[hid].dmap[datas_map[hid].xyz_name].type = tinyply::Type::FLOAT64;
	  datas_map[hid].dmap[datas_map[hid].xyz_name].fill_full_uint8_vect(doubleVec);
	}


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
	for ( const auto &ee : datas_map[hid].dmap ) {
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
	std::cerr << "yo2" << std::endl;	
        // w_algo.simplify(w_datas.format_points,do_keep,0.02);
        // std::cerr << "start tiling" << std::endl;
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


    for ( const auto &myPair : datas_map )
    {
        Id id = myPair.first;
        int nb_out = datas_map[id].nb_pts_uint8_vect();



	if(true){
	  ddt::stream_data_header oqh("p","f",id);
	  std::string filename(params.output_dir + "/" + fname_map[id]);
	  oqh.init_file_name(filename,".stream");
	  oqh.write_header(std::cout);
	  datas_map[id].write_serialized_stream(oqh.get_output_stream());
	  oqh.finalize();
	  std::cout << std::endl;
	}
	if(true){
	  ddt::stream_data_header oqh("p","f",id);
	  std::string filename(params.output_dir + "/" + fname_map[id]);
	  oqh.init_file_name(filename,".ply");
	  oqh.write_header(std::cout);
	  datas_map[id].write_ply_stream(oqh.get_output_stream(),'\n',true);
	  oqh.finalize();
	  std::cout << std::endl;
	}

    }
    return 0;
}


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

      log.step("compute");
      // w_datas.extract_ptsvect(w_datas.xyz_name,w_datas.format_points,false);
      // w_datas.extract_ptsvect(w_datas.center_name,w_datas.format_centers,false);
      w_datas.dmap[w_datas.xyz_name].extract_full_uint8_vect(w_datas.format_points,false);
      std::cerr << "xyz ok" << std::endl;
      w_datas.dmap[w_datas.center_name].extract_full_uint8_vect(w_datas.format_centers,false);

      // std::cerr << "flags" << std::endl;
      // w_datas.dmap[w_datas.flags_name].extract_full_uint8_vect(w_datas.format_flags,false);

      // for(auto ff : w_datas.format_flags)
      //   std::cerr << "flags:" << ff << std::endl;
      // exit(10);
	
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
  w_algo.tessel_adapt(w_datas_full.format_points,
		      p_simp_full,
		      w_datas_full.format_egv,
		      w_datas_full.format_sigs,
		      20,params.pscale,D,tid
		      );

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
	      oth.init_file_name(ply_name,".ply");
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
      oth.init_file_name(ply_name,".ply");
    oth.write_header(std::cout);

    if(params.dump_ply)
      w_datas_full.write_ply_stream(oth.get_output_stream(),'\n',true);
    else
      w_datas_full.write_serialized_stream(oth.get_output_stream());

    oth.finalize();
    std::cout << std::endl;

  }
  if(p_simp_full.size() > 0)
    {
      ddt::stream_data_header oxh("x","z",tid);
      ddt_data<Traits> datas_out;
      datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
      datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(p_simp_full);

      std::string ply_name(params.output_dir +  "/simp_id_" + std::to_string(tid) + "_simp");
      if(params.dump_ply)
	oxh.init_file_name(ply_name,".ply");
      oxh.write_header(std::cout);
      if(params.dump_ply)
	datas_out.write_ply_stream(oxh.get_output_stream());
      else
	datas_out.write_serialized_stream(oxh.get_output_stream());
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
	  std::cerr << "extract flagss" << std::endl;
	  wpt.dmap[wpt.flags_name].extract_full_uint8_vect(wpt.format_flags,false);
	  w_data_full.format_flags.insert(w_data_full.format_flags.end(),wpt.format_flags.begin(),wpt.format_flags.end());
	  std::cerr << "extract sig" << std::endl;
	  //wpt.dmap[w_data_full.sig_name].extract_full_uint8_vect(w_data_full.format_sigs,false);
	  wpt.extract_sigs(w_data_full.format_sigs,false);
	  std::cerr << "extract egv" << std::endl;
	  wpt.extract_egv(w_data_full.format_egv,false);
	  //wpt.dmap[w_data_full.egv_name].extract_full_uint8_vect(w_data_full.format_egv,false);



	  //wpt.extract_ptsvect(w_data_full.center_name,w_data_full.format_centers,false);
	  //wpt.extract_ptsvect(w_data_full.xyz_name,w_data_full.format_points,false);

        }
    }

    Tile_iterator  tile1  = tri.get_tile(tid);
    int nbs = tile1->number_of_cells();//w_datas_tri[tid].nb_simplex_uint8_vect();
    int accll = 0;
    for(auto pp : w_data_full.format_flags){
      std::cerr << "formatflag :"<< pp <<  std::endl;
      if(accll++ > 100)
	break;
      // 	exit(1);
    }

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
        oth.init_file_name(filename,".ply");
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
        oth.init_file_name(filename,".ply");
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
        oth.init_file_name(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    oth.finalize();
    std::cout << std::endl;


    return 0;
}



// bool is_local(Cell_const_iterator cci){
//     int local = 0;
//     for(int i=0; i<=D+1; ++i)
//       {
// 	auto v = cci->vertex(i % (D+1));
// 	if(i>0)
// 	  {
// 	    local += v->is_local();
// 	  }
//       }
//     return local > 0;
// }




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
    if(true)
        mode = 1;
    // if(params.mode.find(std::string("out")) != std::string::npos)
    //   mode = 0;


    for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
        try
        {
            if(fit->main_id() != tid || fit->is_infinite())
                continue;


            Cell_const_iterator tmp_fch = fit.full_cell();
            int tmp_idx = fit.index_of_covertex();
            Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);



            if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	       !tri.tile_is_loaded(tmp_fchn->main_id())){
	      std::cerr << "ERROR tile not loaded" << std::endl;
                continue;
	    }



            Cell_const_iterator fch = tmp_fch->main();
            int id_cov = fit.index_of_covertex();
            Cell_const_iterator fchn = tmp_fchn->main();
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
      oth.init_file_name(ply_name,".geojson");
      oth.write_header(std::cout);
    }
    // else
    //     oth.init_file_name(ply_name,".ply");




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
    if(true)
        mode = 1;
    // if(params.mode.find(std::string("out")) != std::string::npos)
    //   mode = 0;


    for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
        try
        {
            if(fit->is_infinite())
                continue;

            Cell_const_iterator tmp_fch = fit.full_cell();
            int tmp_idx = fit.index_of_covertex();
            Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);


	    if(
            (params.area_processed == 1 && ((tmp_fch->main_id() != tmp_fchn->main_id()) )) ||
            (params.area_processed == 2 && ((tmp_fch->main_id() == tmp_fchn->main_id()) )))
            continue;

            if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	       !tri.tile_is_loaded(tmp_fchn->main_id())){

                continue;
	    }

	    
	    // if(
	    //    (params.area_processed == 1 && (!fit->is_local()))  ||
	    // 	(params.area_processed == 2 && fit->is_local()))
            // continue;

            // if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	    //    !tri.tile_is_loaded(tmp_fchn->main_id())){

            //     continue;
	    // }



            Cell_const_iterator fch = tmp_fch->main();
            int id_cov = fit.index_of_covertex();
            Cell_const_iterator fchn = tmp_fchn->main();
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
      oth.init_file_name(ply_name,".geojson");
      oth.write_header(std::cout);
    }
    // else
    //     oth.init_file_name(ply_name,".ply");


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
        oth.init_file_name(ply_name,".geojson");
    else
        oth.init_file_name(ply_name,".ply");

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

        std::cerr << format_points.size() << std::endl;

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
    mrf.set_mode(0);


    
    
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
        oth.init_file_name(filename,".ply");
    oth.write_header(std::cout);
    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    oth.finalize();
    std::cerr << "done" << tid << std::endl;
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
    mrf.set_mode(0);
    mrf.alpha_exp(tri,w_datas_tri);

    log.step("finalize");
    w_datas_tri[tid].fill_labs(w_datas_tri[tid].format_labs);

    log.step("write");
    std::cout.clear();
    //  log.step("Write header");
    ddt::stream_data_header oth("t","z",tid);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    if(params.dump_ply)
        oth.init_file_name(filename,".ply");
    oth.write_header(std::cout);


    ddt::write_ddt_stream(tri, w_datas_tri[tid], oth.get_output_stream(),tid,false,log);
    oth.finalize();
    std::cout << std::endl;


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
    oth.init_file_name(json_name,".geojson");
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
        oqh_1.init_file_name(filename,"_pts.geojson");
        oqh_1.write_header(std::cout);
        oqh_2.init_file_name(filename,"_spx.geojson");
        oqh_2.write_header(std::cout);
        oqh_3.init_file_name(filename,"_nrm.geojson");
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
        oth.init_file_name("hello",".json");
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

        std::cerr << " ======================================================= " << std::endl;
        std::cerr << "     [MAIN_DDT_STREAM_LOG] stream  : " << params.slabel << std::endl;
        std::cerr << "     [MAIN_DDT_STREAM_LOG] tile_id : " << tile_id <<  std::endl;
        std::cerr << "     [MAIN_DDT_STREAM_LOG] step    : " << params.algo_step << std::endl;
        std::cerr << "     [MAIN_DDT_STREAM_LOG] acc     : " << acc++ << std::endl;
	std::cerr << "     [MAIN_DDT_STREAM_LOG] nb dat  : " << nb_dat << std::endl;
        std::cerr << " ======================================================= " << std::endl;
        try
        {
            if(params.algo_step == std::string("hello"))
            {
                rv = hello(tile_id,params,nb_dat);
            }
            else if(params.algo_step == std::string("dim"))
            {
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
            else if(params.algo_step == std::string("extract_surface"))
            {
                if(params.area_processed == 0)
                    rv = extract_surface(tile_id,params,nb_dat,log);
                else
                    rv = extract_surface_area(tile_id,params,nb_dat,log);
		do_dump_log = false;
            }
            else if(params.algo_step == std::string("extract_graph"))
            {
                rv = extract_graph(tile_id,params,nb_dat,log);
                do_dump_log = false;
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
                do_dump_log = false;
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
                ddt::stream_data_header olh("l","s",tile_id);
                olh.write_header(std::cout);
                log.dump_log(olh.get_output_stream());
                olh.finalize();
                std::cout << std::endl;
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





