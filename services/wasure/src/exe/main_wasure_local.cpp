#include "wasure_typedefs.hpp"
#include "write_geojson_wasure.hpp"

#include "io/stream_api.hpp"
#include "io/write_stream.hpp"
#include "io/write_vrt.hpp"
//#include "io/write_geojson.hpp"
#include "io/read_stream.hpp"
#include "io/logging_stream.hpp"


#include "wasure_data.hpp"
#include "wasure_algo.hpp"
#include "tbmrf_reco.hpp"
#include "tbmrf_conflict.hpp"
#include "io_ddt_stream.hpp"
#include "graph_cut.hpp"


// Storing the data
typedef std::map<Id,wasure_data<Traits> > D_MAP;
typedef std::map<Id,std::list<wasure_data<Traits>> > D_LMAP;

int main(int argc, char **argv)
{
  std::cerr << "START" << std::endl;
  wasure_params params;
  params.parse(argc,argv);

  Scheduler sch(1);
  Traits traits;
  int D = Traits::D;
  Id tid = 0;


  // ==== Parsing the data
  wasure_data<Traits> w_datas_pts;
  std::ifstream ifile;
  ifile.open(params.filename);
  w_datas_pts.read_ply_stream(ifile);
  ifile.close();
  w_datas_pts.shpt2uint8();
  int count = w_datas_pts.nb_pts_uint8_vect();
  std::cerr << "nbp inputs:" << count << std::endl;

  
  // ===== Data extraction =====
  // Extract the unformated data into formated vector format_points and format_centers
  w_datas_pts.dmap[w_datas_pts.xyz_name].extract_full_uint8_vect(w_datas_pts.format_points);
  w_datas_pts.dmap[w_datas_pts.center_name].extract_full_uint8_vect(w_datas_pts.format_centers);
  if(w_datas_pts.dmap[w_datas_pts.flags_name].do_exist)
    w_datas_pts.dmap[w_datas_pts.flags_name].extract_full_uint8_vect(w_datas_pts.format_flags);
  else
    w_datas_pts.format_flags.resize(count,0);

 

  std::string filename("/home/laurent/shared_spark/tmp/centrs.xyz");
  std::ofstream ofile;
  ofile.open(filename);
  for(auto cc : w_datas_pts.format_centers){
    ofile << cc << std::endl;
  }
  ofile.close();

  
  // ===== Dimensionality =====
  // Compute the dimensionality of each points and simplify the input point cloud
  // The result is stored into
  // format_egv -> the egein vector
  // format_egv -> the egein values
  // p_simp => a sub sampling of the input point cloud
  wasure_algo w_algo;


  std::cout << "Start dim" << std::endl;
  std::vector<Point> p_simp;
  w_algo.compute_dim_with_simp(w_datas_pts.format_points,
			       w_datas_pts.format_egv,
			       w_datas_pts.format_sigs,
			       p_simp,
			       params.pscale);

  // Flip the normal according to the optical center
  w_algo.flip_dim_ori(w_datas_pts.format_points,
		      w_datas_pts.format_egv,
		      w_datas_pts.format_centers);


  std::cout << "Start tessel" << std::endl;
  w_algo.tessel_adapt(w_datas_pts.format_points,
		      p_simp,
		      w_datas_pts.format_egv,
		      w_datas_pts.format_sigs,
		      20,1,D,tid
		      );
  

  // ====== Delaunay triangulation
  std::vector<Point_id>  vp;
  DTW tri1;
  tri1.init(tid);
  Tile_iterator tci = tri1.get_tile(tid);

  //  for(auto pp : w_datas_pts.format_points)
  for(auto pp : p_simp)
    {
      vp.emplace_back(std::make_pair(pp,tid));
    }
  int nbi1 = tci->insert(vp,false);
  std::cerr << "number of points insteted" << nbi1 << std::endl;
  int acc = 0;
  tri1.finalize(sch);

  // ===== Init the id of each cell
  for(auto iit = tri1.cells_begin(); iit != tri1.cells_end(); ++iit)
    {
      const Data_C & cd = iit->cell_data();
      Data_C & cd_quickndirty = const_cast<Data_C &>(cd);
      cd_quickndirty.id = acc;
      cd_quickndirty.gid = acc++;
    }
    
    
  // ==== DST ====
  // Do the dempster shafer theory for each simplex
  D_MAP w_datas_tri;
  w_datas_tri[tid] = wasure_data<Traits>();
  DT & tri_tile  = tri1.get_tile(tid)->triangulation();
  auto tile = tri1.get_tile(tid);
  std::vector<std::vector<double>>  & format_dst = w_datas_tri[tid].format_dst; ;
  int nbs = tile->number_of_cells();

  // Init each simplex at "unknown"
  // 0 0 1 => 0% in, 0% out, 100% unknown
  if(format_dst.size() == 0)
    {
      for(int ss = 0; ss < nbs ; ss++)
	{
	  format_dst.push_back(std::vector<double>({0.0,0.0,1.0}));
	}
    }
  // Compute the dst 
  w_algo.compute_dst_with_center(tri1,w_datas_tri[tid],w_datas_pts,params,tid);



  
  // ===== Segmentation =====
  std::vector<int>  & format_labs = w_datas_tri[tid].format_labs ;
  format_labs.resize(nbs);
  tbmrf_reco<DTW,D_MAP> mrf(params.nb_labs,&tri1,&w_datas_tri);


  std::vector<double> lambda_list({0,0.001,0.01,0.05,0.1,0.5,1,2,10,20});
  //std::vector<double> lambda_list({0});
  // Mode 0 => outdoor scene
  // Mode 1 => indoor scene
  mrf.set_mode(0);


  for(auto ll : lambda_list){
    mrf.lambda = ll;
    // Optimizing with alpha expansion
    for(int ii = 0; ii < nbs;ii++)
      format_labs[ii] = 0;
    mrf.alpha_exp(tri1,w_datas_tri);


    w_datas_tri[tid].fill_labs(w_datas_tri[tid].format_labs);


  
    // ===== Surface extraction =====
    // Extract the surface from the simplex segmentation
    if(D == 2){
      traits.export_tri_to_data(tri_tile,w_datas_tri[tid]);    
      ddt::stream_data_header oqh_1("p","s",tid),oqh_2("p","s",tid);
      std::string filename(params.output_dir +  "/" + params.slabel + "_id_" + std::to_string(tid) + "_seg");
      oqh_1.init_file_name(filename,"_pts.geojson");
      oqh_1.write_header(std::cout);
      oqh_2.init_file_name(filename,"_spx.geojson");
      oqh_2.write_header(std::cout);
      w_datas_tri[tid].write_geojson_tri(oqh_1.get_output_stream(),oqh_2.get_output_stream());
      oqh_1.finalize();
      oqh_2.finalize();
      ddt::add_qgis_style(oqh_2.get_file_name(),"tri_seg.qml");
      std::cout << std::endl;
    }
  
    std::vector<Facet_const_iterator> lft;
    std::vector<bool> lbool;
    mrf.extract_surface(tid,lft,w_datas_tri);



    std::string ply_name(params.output_dir +  "/" + params.slabel + "_ll_" + std::to_string(ll) + "_surface");
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
	  std::vector<int> vid;
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

	      int cccid = fch->lid();
	      int ch1lab = w_datas_tri[fch->tile()->id()].format_labs[cccid];
	    
	      const Point& a = fch->vertex((id_cov+1)&3)->point();
	      const Point& b = fch->vertex((id_cov+2)&3)->point();
	      const Point& c = fch->vertex((id_cov+3)&3)->point();
	      const Point& d = fch->vertex((id_cov)&3)->point();


	    
	      bool bl =
		(CGAL::orientation(a,b,c,d) == 1 && ch1lab == 0) ||
		(CGAL::orientation(a,b,c,d) == -1 && ch1lab == 1);

	      Id ida = (id_cov+1)&3;
	      Id idb = (id_cov+2)&3;
	      Id idc = (id_cov+3)&3;


	      vid.push_back(fch->lid());
	      v_simplex.push_back(vertex_map[fch->vertex(ida)]);
	      if(!bl){
		v_simplex.push_back(vertex_map[fch->vertex(idb)]);
		v_simplex.push_back(vertex_map[fch->vertex(idc)]);
	      }else{
		v_simplex.push_back(vertex_map[fch->vertex(idc)]);
		v_simplex.push_back(vertex_map[fch->vertex(idb)]);
	      }

	    
	      // for(int i = 0; i < D+1; ++i)
	      //   {
	      //     if(i != id_cov)
	      // 	  {
	      //         Vertex_const_iterator v = fch->vertex(i);
	      //         v_simplex.push_back(vertex_map[v]);
	      // 	  }
	      //   }
	    }


	  datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
	  datas_out.dmap[datas_out.simplex_name] = ddt_data<Traits>::Data_ply(datas_out.simplex_name,"face",D,D,tinyply::Type::INT32);
	  datas_out.dmap[std::vector<std::string>{"id"}] = ddt_data<Traits>::Data_ply(std::vector<std::string>{"id"},"face",1,1,tinyply::Type::INT32);
	  datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(format_points);
	  datas_out.dmap[datas_out.simplex_name].fill_full_uint8_vect(v_simplex);
	  datas_out.dmap[std::vector<std::string>{"id"}].fill_full_uint8_vect(vid);
	  datas_out.write_ply_stream(oth.get_output_stream(),'\n',true);
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
  }
  return 0;

}
