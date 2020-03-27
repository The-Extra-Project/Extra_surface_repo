#include "ddt_typedefs.hpp"
#include "ddt_stream_params.hpp"
#include "algorithm.hpp"
#include "double-conversion.h"
#include "io/write_ply.hpp"
// CGAL and co
typedef std::map<Id,ddt_data<Traits> > D_MAP;
typedef std::map<Id,std::vector<ddt_data<Traits> > > D_VMAP;


// The test test test test

// "broadcast" the neighbors of the given tiles 
int send_neighbors(Id tid,algo_params & params, std::map<Id, std::vector<Point_id_id>> & outbox,bool do_send_empty)
{
  int D = Traits::D;
  std::cout.clear();
  for(auto&& mit : outbox)
    {
      Id nb_tid = mit.first;
      std::vector<Point_id_id> svh = mit.second;
      if(!svh.empty() || (svh.empty() && do_send_empty))
        {
	  ddt::stream_data_header hto("e","s",std::vector<int> {tid,nb_tid});
	  std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid) + "_nid" + std::to_string(nb_tid));
	  //hto.init_file_name(filename,".pts");
	  hto.write_header(std::cout);
	  ddt::write_points_id_source_serialized<Point_id_id,Point>(svh,hto.get_output_stream(),D);
	  hto.finalize();
	  std::cout << std::endl;
        }
    }

  return 0;
}


// Extract the edge of the global graph
void get_edges(Tile_iterator & tci, std::map<Id, std::vector<Point_id_id>> & outbox)
{
  std::vector<Vertex_const_handle_and_id> out;
  ddt::get_neighbors()(*tci, std::back_inserter(out));
  for(auto&& pair : out)
    {
      if (outbox.find(pair.second) == outbox.end() )
	outbox[pair.second] = std::vector<Point_id_id>();

    }
}


void get_all_neighbors(Tile_iterator & tci, std::map<Id, std::vector<Point_id_id>> & outbox,bool skip_inserted = false)
{
  std::vector<Vertex_const_handle_and_id> out;
  ddt::get_neighbors()(*tci, std::back_inserter(out));
  for(auto&& pair : out)
    {
      auto pp = pair.first->point();
      Id idp =  tci->id(pair.first);
      Id id_source =  tci->id();
      outbox[pair.second].emplace_back(std::make_tuple(pp,idp,id_source));
    }
}

// Ok here we 
void get_neighbors(Tile_iterator & tci, std::map<Id, std::vector<Point_id_id>> & outbox, bool skip_inserted = false)
{
  std::vector<Vertex_const_handle_and_id> out;
  ddt::get_neighbors()(*tci, std::back_inserter(out));
  for(auto&& pair : out)
    {
      auto pp = pair.first->point();
      Id idp =  tci->id(pair.first);
      Id id_source =  tci->id();
      bool is_inserted = tci->points_sent_[pair.second].insert(pp).second;
      if(is_inserted)
        {
	  outbox[pair.second].emplace_back(std::make_tuple(pp,idp,id_source));
        }
      else if ( outbox.find(pair.second) == outbox.end() )
        {
	  outbox[pair.second] = std::vector<Point_id_id>();
        }
    }
}





// Extract the crown of each triangulation
// This is the first step of the iterative scheme
int extract_tri_crown(DDT & tri1, std::vector<Point> & vp_crown,int tid,int D,ddt::logging_stream & log)
{
  //    log.step("[process]split2tri");
  Scheduler sch(1);
  ddt::const_partitioner<Traits> part(tid);
  int nbi = 0;

  typedef typename DDT::Tile_vertex_handle   Vertex_handle;
  typedef typename DDT::DT DT;

  Traits::v_hmap_bool out_v;
  log.step("[process]extract_tri_crown_gettile");
  Tile_iterator  tile1  = tri1.get_tile(tid);
  log.step("[process]extract_tri_crown_getttri");
  const DT & ttri1 = tile1->triangulation();

  log.step("[process]extract_tri_crown_labelize");
  auto bbox = tile1->bbox();
  for(auto iit = tri1.cells_begin(); iit != tri1.cells_end(); ++iit)
    {
      bool do_keep = false;
      if(iit->is_infinite())
        {
	  do_keep = true;
        }
      else if(!iit->is_inside())
        {
	  do_keep = true;
        }

      if(do_keep)
        {
	  for(int d = 0 ; d <= D; d++)
            {
	      if(!iit->vertex(d)->is_infinite())
                {
		  out_v[iit->vertex(d)->vertex()] = true;
                }
            }
        }

    }

  log.step("[process]extract_tri_crown_tri");
  for(auto iit = tri1.vertices_begin(); iit != tri1.vertices_end(); ++iit)
    {
      if(out_v[iit->vertex()])
        {
	  tile1->flag(iit->vertex(),1,true);
	  vp_crown.emplace_back(iit->point());  // string (key)
        }
    }
  return vp_crown.size();
}





// Lighway first step insertion function in order to minimize the memory footprint when all the points are processed
// This step is also handled by the generic insertion function, but in a less efficient way.
int insert_raw(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{

  // We use a traits with a raw triangulation
  int D = Traits::D;
  Traits_raw traits_raw;
  Traits traits;
  DT_raw  tri_raw = traits_raw.triangulation(D) ;

  typedef Traits_raw::Vertex_handle_raw                            Vertex_handle_raw;
  std::vector<Point> vp;

  // ==================================
  log.step("[read]parse_data");
  std::cerr << "parse data" << std::endl;

  for(int i = 0; i < nb_dat; i++)
    {
      ddt::stream_data_header hpi;
      hpi.parse_header(std::cin);
      if(hpi.get_lab() == "z" )
        {
	  
	  ddt_data<Traits> w_datas;
	  w_datas.read_serialized_stream(hpi.get_input_stream());
	  //	  w_datas.extract_ptsvect(w_datas.xyz_name,vp,false);
	  w_datas.dmap[w_datas.xyz_name].extract_full_uint8_vect(vp,true);
	  //	  ddt::read_point_set_serialized(vp, hpi.get_input_stream(),traits);
        }
      if(hpi.get_lab() == "p" || hpi.get_lab() == "x")
        {
	  ddt_data<Traits> w_datas;
	  w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
	  for(int i = 0; i < w_datas.nb_pts_shpt_vect() ; i++)
            {
	      vp.emplace_back(w_datas.get_pts(i));
            }
        }
      hpi.finalize();
    }

  std::cerr << "Do the triangulation : " << vp.size() <<  std::endl;
  log.step("[process]triangulation");
  tri_raw.insert(vp.begin(),vp.end());
  ddt::Bbox<Traits::D> tri_bbox;

  for(auto& vv : vp)
    {
      tri_bbox += vv;
    }

  std::cerr << "get convex hull pts" << std::endl;
  log.step("[process]get_convex_hull_pts");
  // ==================================
  std::vector<Vertex_handle_raw> vvhc;
  //  ddt::get_bbox_points()(*(tri.get_tile(tid)), std::back_inserter(vvhc));
  //  tri_raw.adjacent_vertices(tri_raw.infinite_vertex(), std::back_inserter(vvhc)); // get convex hull points
  get_bbox_points_raw(tri_raw,std::back_inserter(vvhc),traits_raw);


  std::cerr << "get crow" << std::endl;
  // ==================================
  std::vector<Point>  vp_crown;
  std::set<Vertex_handle_raw>  vh_crown,vh_finalized;
  log.step("[process]get_crown_pts");
  int nbc_finalized = 0;
  for(auto cit = traits_raw.cells_begin(tri_raw); cit != traits_raw.cells_end(tri_raw); ++cit)
    {
      bool do_keep = !traits_raw.is_inside(tri_raw,tri_bbox,cit);
      if(do_keep)
        {
	  for(int d = 0; d < D+1; d++)
            {
	      if(!tri_raw.is_infinite(cit->vertex(d)))
                {
		  vh_crown.insert(cit->vertex(d));
                }
            }
        }
      else
        {
	  if(params.dump_mode == "NONE")
            {
	      for(int d = 0; d < D+1; d++)
                {
		  vh_finalized.insert(cit->vertex(d));
                }
            }
	  nbc_finalized++;
        }
    }

  for(auto vv : vh_crown)
    vp_crown.emplace_back(vv->point());

  std::cerr << vvhc.size() << std::endl;
  std::cerr << vp_crown.size() << std::endl;
  std::cerr << "get dump" << std::endl;


  // ==== Stat dumping section ======
  std::cout.clear();

  if(params.dump_mode == "NONE")
    {
      std::vector<Point_id_id> vvp_finalized;
      for(auto vv : vh_finalized)
        {
	  Point_id_id pis = std::make_tuple(vv->point(),tid,tid);
	  vvp_finalized.emplace_back(pis);
        }
      ddt::stream_data_header orh("r","s",tid);
      orh.write_header(std::cout);
      log.step("[write]write_finalized_pts");
      std::cerr << "finalized_pts_size:" << vvp_finalized.size() << std::endl;
      ddt::write_points_id_source_stream<Point_id_id,Point>(vvp_finalized,orh.get_output_stream(),D);
      orh.finalize();
      std::cout << std::endl;
    }

  ddt::stream_data_header oqh("q","s",tid);
  oqh.write_header(std::cout);
  log.step("[write]write_convex_hull");

  std::vector<Point_id_id> vvpc;
  for(auto vv : vvhc)
    {
      Point_id_id pis = std::make_tuple(vv->point(),tid,tid);
      vvpc.emplace_back(pis);
      //      ddt::write_point_id_source<Point_id_id,Point>(pis,oqh.get_output_stream(),D);
    }
  ddt::write_points_id_source_stream<Point_id_id,Point>(vvpc,oqh.get_output_stream(),D);
  oqh.finalize();
  std::cout << std::endl;

  log.step("[write]write_crown");
  ddt::stream_data_header ozh("z","z",tid);
  ozh.write_header(std::cout);
  //ddt::write_point_set_serialized(vp_crown,ozh.get_output_stream(),D);
  ddt_data<Traits> w_datas;
  w_datas.dmap[w_datas.xyz_name].fill_full_uint8_vect(vp_crown);
  w_datas.write_serialized_stream(ozh.get_output_stream());
  ozh.finalize();
  std::cout << std::endl;
  vp_crown.clear();
  vh_crown.clear();
  log.step("[write]write_ply");

  // if dump_mode > 0 we dump ply
  // The if it's note the case, we'll extract a soup of simplex
  if(params.dump_mode != "NONE" ){
    //      ddt::stream_data_header oph("p","s",tid);
    //      oph.write_header(std::cout);
    ddt::filter_cell<Traits_raw> filt(tri_bbox);
    ddt::cgal2ply_split<Traits_raw>(std::cout,tri_raw,filt,nbc_finalized,params.dump_mode,tid);

    std::cout << std::endl;
    //    cgal2plysplit(std::cout,tri_raw,tri_bbox,params,log);
    //oph.finalize();
  }else{
    // 


  }
  std::cerr << "insert tri done" << std::endl;

  return 0;
}


// Generic parsing function
// Loop on the input stream and fill all the structure.
// Can read distributed triangulation, Point id and point id id source.
int parse_datas(DDT & tri1, std::vector<Point_id> & vp,std::vector<Point_id_id> & vpis,int nb_dat,int tid,ddt::logging_stream & log,algo_params & params)
{
  // loop over all input tiles and insert it
  log.step("[read]init_trait");
  Traits traits;
  for(int i = 0; i < nb_dat; i++)
    {
      ddt::stream_data_header hpi;
      //      hpi.set_logger(&log);

      hpi.parse_header(std::cin);
      //      std::cerr << "filename: " << hpi.get_file_name() << " [" << hpi.get_lab() << "]" << std::endl;
      if(hpi.get_lab() == "t" )
        {
	  //  log.step("[read]read_triangulation");
	  std::cerr << " " << std::endl;
	  std::cerr << "=== Parse tri ===" << std::endl;
	  bool do_clean_data = true;
	  std::cerr << "parse stream tri" << std::endl;
	  read_ddt_stream(tri1, hpi.get_input_stream(),hpi.get_id(0),hpi.is_serialized(),do_clean_data,log);
	  std::cerr << "tri finalized" << std::endl;
        }
      if(hpi.get_lab() == "d")
        {
	  //  log.step("[read]read_triangulation");
	  std::cerr << " " << std::endl;
	  std::cerr << "=== Parse tri ===" << std::endl;
	  ddt_data<Traits> w_datas;
	  w_datas.dmap[w_datas.xyz_name].extract_full_shpt_vect(vp,false);

        }

      // Parse point only
      if(hpi.get_lab() == "p"  || hpi.get_lab() == "z")
	{
	  std::cerr << " " << std::endl;
	  std::cerr << "=== Parse pts ===" << std::endl;
	  if(hpi.is_serialized()){
	    std::vector<Point> rvp;
	    ddt::read_point_set_serialized(rvp, hpi.get_input_stream(),traits);
	    // ddt_data<Traits> w_datas;
	    // w_datas.read_serialized_stream(hpi.get_input_stream());
	    // w_datas.dmap[w_datas.xyz_name].extract_full_uint8_vect(rvp,true);
	    //w_datas.extract_ptsvect(w_datas.xyz_name,rvp,false);
	    for(auto pp : rvp)
	      {
                vp.emplace_back(std::make_pair(pp,tid));
	      }
	  }else{
	    ddt_data<Traits> w_datas;
	    std::string ext = hpi.get_ext();
	    if(ext == "pts")
	      {
		// Not handled anymore
		//ddt::read_points_stream(vp,hpi.get_input_stream(),traits);
		return 10;
	      }
	    else if(ext == "ply" || hpi.is_stream())
	      {
		w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
		for(int i = 0; i < w_datas.nb_pts_shpt_vect() ; i++)
		  {
		    vp.emplace_back(std::make_pair(w_datas.get_pts(i),tid));
		  }
	      }
	    else
	      {
		std::cerr << "format " << ext << " not supported" << std::endl;
		return 1;
	      }
	  }
	}

      // Parse point and Id
      if(hpi.get_lab() == "q" || hpi.get_lab() == "r" || hpi.get_lab() == "e")
        {
	  std::cerr << " " << std::endl;
	  std::cerr << "=== Parse q ===" << std::endl;
	  ddt::read_points_id_source_serialized(vpis, hpi.get_input_stream(), traits);
        }
      std::cerr << "parsing done" << std::endl;
      hpi.finalize();
    }
  return 0;
}


// Upodate the global id
// Takes input triangulation and triangulation global id
int update_global_id(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{

  std::cout.setstate(std::ios_base::failbit);
  std::cerr << "seg_step0" << std::endl;

  DDT tri; 
  Scheduler sch(1);

  std::cerr << "seg_step1" << std::endl;
  //  D_MAP w_datas_tri;


  log.step("read");
  int D = Traits::D;
  std::map<int,std::vector<int>> tile_ids;;

  for(int i = 0; i < nb_dat; i++)
    {
      ddt::stream_data_header hpi;
      hpi.parse_header(std::cin);
      Id hid = hpi.get_id(0);
      auto tile  = tri.get_tile(tid);
      if(hpi.get_lab() == "t")
        {
	  bool do_clean_data = true;
	  bool do_serialize = false;
	  read_ddt_stream(tri, hpi.get_input_stream(),hpi.get_id(0),hpi.is_serialized(),do_clean_data,log);
									      
        }
      if(hpi.get_lab() == "s")
        {
	  std::cerr << "tile ids loal:" << hid << std::endl;
	  std::vector<int> vv(3);
	  for(int d = 0; d < 3; d++)
            {
	      hpi.get_input_stream() >> vv[d];
	      tile->tile_ids[d] = vv[d];
            }
	  tile_ids[hid] = vv;
	  
        }
      hpi.finalize();

    }

  tri.init_local_id();

  std::cout.clear();
  ddt::stream_data_header oth("t","s",tid);
  oth.serialize(true);
  std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
  oth.set_logger(&log);
  oth.write_header(std::cout);
  log.step("[write]write_tri");
  Tile_iterator tci = tri.get_tile(tid);

  ddt::write_ddt_stream(tri, oth.get_output_stream(),tid,oth.is_serialized(),log);
  return 0;

}


// Insert in triangulation
// Main function of the algorithm
int insert_in_triangulation(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{

  // Shut down cout during processing in case of
  std::cout.setstate(std::ios_base::failbit);

  // Global en var
  int D = Traits::D;
  Scheduler sch(1);
  DDT tri1;
  Traits traits;


  // Input datas
  std::vector<Point_id_id> vpis;
  std::vector<Point_id> vp;
  std::vector<Point> vp_crown;
  std::map<Id, std::vector<Point_id_id>>  outbox_nbrs;





  // ======== Parse input datas section  ===========
  std::cerr << "PARSE: " << tid <<  std::endl;
  log.step("[read]Start_parse_data");
  parse_datas(tri1,vp,vpis,nb_dat,tid,log,params);

  std::cerr << "PROCESSING:" << tid <<  std::endl;
  std::cerr << "vpis size:" << vpis.size() << std::endl;
  std::cerr << "vp   size:" << vp.size() << std::endl;

  // ======== Processing Section ===========
  // inserting stats stats
  int nbi1 = 0,nbi2 = 0;
  bool do_data = true;

  // Insertion section
  if(vp.size() > 0) // If points without label recieved, classic insertion
    {
      log.step("[process]insert");
      tri1.init(tid);
      Tile_iterator tci = tri1.get_tile(tid);
      std::cerr << "start_insert_pts" << std::endl;
      nbi1 = tci->insert(vp,false);
      vp.clear();
    }
  if(vpis.size() > 0) // If point with label recieved, insertion by taking into account the id
    {
      log.step("[process]insert");
      bool do_insert_local = params.finalize_tri;
      Tile_iterator tci = tri1.get_tile(tid);
      nbi1 += tci->insert_points_id_id(vpis,tid,do_insert_local);
      vpis.clear();
      std::cerr << "insert_id_id done" << std::endl;
    }


  // Extraction section
  //We call "tri crown" the triangulation with non finalized simplex.
  if(params.extract_tri_crown) // If dump 2 tri
    {

      log.step("[process]extract_tri_crown");
      do_data=false;
      nbi2 = extract_tri_crown(tri1,vp_crown,tid,D,log);
    }


  if(params.extract_edg_nbrs) // If extract edges with nbr inside
    {
      log.step("[process]getnbrs");
      Tile_iterator tci = tri1.get_tile(tid);
      if(outbox_nbrs.size() == 0)
        {
	  get_neighbors(tci,outbox_nbrs);
        }
    }


  // If finalized, computer the total number of simplex for using gobal iterators next
  // If not useless for distributed delaunay triangulation algorithm
  bool is_finalized  = (nbi1 == 0) || params.finalize_tri;
  if(is_finalized){
    tri1.finalize(sch);

  }


  // ===================== Dumping ================
  // Activate cout stream
  std::cout.clear();

  // Mode > 0 && triangulation finalized : Dumping simplex soup
  log.step("[write]Start");
  if(params.finalize_tri && params.dump_mode != "NONE")
    {
      // if(params.dump_mode == 3 )
      //   {
      ddt::Bbox<Traits::D> tri_bbox_local;
      Tile_iterator tci = tri1.get_tile(tid);
      for(auto vit = tci->vertices_begin(); vit != tci->vertices_end(); ++vit)
	{
	  if(!tci->vertex_is_infinite(vit))
	    {
	      if(tci->vertex_is_local(vit))
		{
		  tri_bbox_local += vit->point();
		}
	    }
	}
      int nb_keep = 0;
      ddt::filter_cell_ddt<Traits> filt(tri_bbox_local,tid);

      for(auto cit = tci->cells_begin(); cit != tci->cells_end(); ++cit)
	{
	  if(filt.do_keep(tci->triangulation(),cit,traits))
	    nb_keep++;
	}


      ddt::cgal2ply_split<Traits>(std::cout,tci->triangulation(), filt, nb_keep,params.dump_mode,tid);

      std::cout << std::endl;
      //}
    }
  else
    {
      ddt::stream_data_header oth("t","s",tid);
      if(params.extract_tri_crown)
	oth.set_lab("v");
      oth.serialize(true);
      std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
      oth.set_logger(&log);
      oth.write_header(std::cout);
      log.step("[write]write_tri");
      Tile_iterator tci = tri1.get_tile(tid);
      ddt::write_ddt_stream(tri1, oth.get_output_stream(),tid,oth.is_serialized(),log);
      oth.finalize();
      std::cout << std::endl;
    }

  // If finalized, dump the number of simplex for global graph algorithms
  if(is_finalized)
    {
      log.step("[process]finalize");
      ddt::stream_data_header osh("s","s",tid);
      osh.write_header(std::cout);
      osh.get_output_stream() << tri1.number_of_vertices() << " " ;
      osh.get_output_stream() << tri1.number_of_facets() << " " ;
      osh.get_output_stream() << tri1.number_of_cells() << " " ;
      osh.finalize();
      std::cout << std::endl;
    }

  // If edge extraction, extract the edges (can be empty for the graph structure
  if(params.extract_edg_nbrs)
    {
      log.step("[process]sendnbrs");
      std::cerr << "send nbrs" << std::endl;
      send_neighbors(tid,params,outbox_nbrs,params.do_send_empty_edges);
    }

  // Extract the extrma point to initialize the first messsage broadcasting
  if(params.extract_tri_crown)
    {
      log.step("[process]send_bbox");
      std::vector<Vertex_const_handle> vvhc;

      // Many strategy possible : bbox point or convex hull points.
      // Convex hull point converge faster but can be too heavy if many tiles
      ddt::get_bbox_points()(*(tri1.get_tile(tid)), std::back_inserter(vvhc));
      //ddt::get_local_convex_hull()(*(tri1.get_tile(tid)), std::back_inserter(vvhc));


      ddt::stream_data_header oqh("q","s",tid);
      oqh.write_header(std::cout);
      int tmp_id = ((int)tid);
      oqh.get_output_stream()  << D <<  " " << vvhc.size() << " ";
      for(auto vv : vvhc)
        {
	  Point_id_id pis = std::make_tuple(vv->point(),tmp_id,tmp_id);
	  ddt::write_point_id_source<Point_id_id,Point>(pis,oqh.get_output_stream(),D);
        }
      oqh.finalize();
      std::cout << std::endl;
    }

  // For the first step, you may want to extract crown of the triangulation (useless for the efficient algorithm because handle by the insert raw function)
  if(params.extract_tri_crown)
    {
      ddt::stream_data_header oqh("z","s",tid);
      oqh.write_header(std::cout);
      ddt::write_point_set_serialized(vp_crown,oqh.get_output_stream(),D);
      oqh.finalize();
      std::cout << std::endl;
    }
  return 0;
}





int get_bbox_points(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{
  std::cout.setstate(std::ios_base::failbit);
  int D = Traits::D;

  std::vector<Vertex_const_handle> vvhc;


  ddt::const_partitioner<Traits> part(tid);
  DDT tri;
  Scheduler sch(1);
  log.step("read");
  for(int i = 0; i < nb_dat; i++)
    {

      ddt::stream_data_header hpi;
      hpi.parse_header(std::cin);
      if(hpi.get_lab() == "t" || hpi.get_lab() == "u")
        {
	  std::string filename = hpi.get_file_name();
	  std::cerr << "read : " << filename << std::endl;
	  bool do_clean_data = true;
	  ddt::read_ddt_stream(tri, hpi.get_input_stream(),hpi.get_id(0),hpi.is_serialized(),do_clean_data,log);
	  std::cerr << "read stream done" << std::endl;
        }

      hpi.finalize();
    }
  log.step("write");
  std::cerr << "get bbox points" << std::endl;
  ddt::get_bbox_points()(*(tri.get_tile(tid)), std::back_inserter(vvhc));
  //ddt::get_local_convex_hull()(*(tri.get_tile(tid)), std::back_inserter(vvhc));
  std::cerr << "get bbox points done" << std::endl;
  std::cout.clear();
  std::cerr << tid << " size:" << vvhc.size() << std::endl;
  for(auto vv : vvhc)
    {
      int tmp_id = ((int)tid);
      Point_id_id pis = std::make_tuple(vv->point(),tmp_id,tmp_id);
      ddt::stream_data_header oqh("b","s",tid);
      ddt::write_point_id_source<Point_id_id,Point>(pis,oqh.get_output_stream(),D);
      std::cout << std::endl;
    }
  std::cerr << tid << "done!" <<  std::endl;

  return 0;
}




int get_neighbors(Id tid,algo_params & params, std::map<Id, std::vector<Point_id_id>> & outbox,ddt::logging_stream & log)
{
  DDT tri;
  Scheduler sch(1);

  ddt::read_ddt_full_stream<DDT,Scheduler>(tri,std::cin,1,log);
  Tile_iterator tci = tri.get_tile(tid);
  std::cerr << "get neighbors pids" << std::endl;
  get_all_neighbors(tci,outbox);

  return 0;
}


// CPP steps functions
// OK
int ply2dataset(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{


  for(int i = 0; i < nb_dat; i++)
    {
      ddt::stream_data_header hpi;
      hpi.parse_header(std::cin);
      ddt_data<Traits> w_datas;
      w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
      hpi.finalize();


      std::cout.clear();
      Id id = hpi.get_id(0);
      ddt::stream_data_header oth("h","s",tid);
      oth.write_header(std::cout);
      w_datas.write_dataset_stream(std::cout,PLY_CHAR,id);
      std::cout << std::endl;
    }
  return 0;
}

// OK
int ply2geojson(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{

  for(int i = 0; i < nb_dat; i++)
    {
      ddt::stream_data_header hpi;
      hpi.parse_header(std::cin);
      std::cerr << "Start reading : " << hpi.get_file_name() << std::endl;
      ddt_data<Traits> w_datas;
      w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
      hpi.finalize();

      std::cout.clear();
      Id id = hpi.get_id(0);
      ddt::stream_data_header oqh_1("p","s",id),oqh_2("p","s",id);
      std::string filename(params.output_dir + "/" + params.slabel +
			   "_id_" + std::to_string(id) + "_" + std::to_string(tid) + "_" + std::to_string(i)) ;
      oqh_1.init_file_name(filename,"_pts.geojson");
      oqh_1.write_header(std::cout);
      oqh_2.init_file_name(filename,"_spx.geojson");
      oqh_2.write_header(std::cout);


      w_datas.write_geojson_tri(oqh_1.get_output_stream(),oqh_2.get_output_stream());


      oqh_1.finalize();
      oqh_2.finalize();

      ddt::add_qgis_style(oqh_2.get_file_name(),params.style);

      std::cout << std::endl;
    }
  return 0;
}

int serialized2geojson(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{

  int D = Traits::D;
  D_VMAP datas_map;
  std::cerr << "nbdats:" << nb_dat << std::endl;   
  for(int i = 0; i < nb_dat; i++){
    std::cerr << "convert data serialized " << i << std::endl;

    ddt::stream_data_header hpi;
    hpi.parse_header(std::cin);

    ddt_data<Traits> w_datas;
    DDT tri1;
    Traits traits;
        Id hid = hpi.get_id(0);
    if(hpi.get_lab() == "t" || hpi.get_lab() == "u" || hpi.get_lab() == "v")
      {
	std::cerr << "READ:" << hpi.get_lab() << std::endl;
	bool do_clean_data = true;
	read_ddt_stream(tri1,hpi.get_input_stream(),hpi.get_id(0),hpi.is_serialized(),do_clean_data,log);
	auto  tile  = tri1.get_tile(tid);
	tile->update_local_flag();
	typename DDT::Traits::Delaunay_triangulation & ttri = tile->tri();
	traits.export_tri_to_data(ttri,w_datas);
      } else if(hpi.get_lab() == "p"  || hpi.get_lab() == "z")
      {
	std::cerr << " " << std::endl;
	std::cerr << "=== Parse pts ===" << std::endl;
	std::vector<Point> vp;
	if(hpi.is_serialized()){
	  std::cerr << "is ser!" << std::endl;
	  std::vector<Point> rvp;
	  //ddt::read_point_set_serialized(rvp, hpi.get_input_stream(),traits);
	  ddt_data<Traits> w_datas;
	  w_datas.read_serialized_stream(hpi.get_input_stream());
	  w_datas.dmap[w_datas.xyz_name].extract_full_uint8_vect(rvp,true);
	  //w_datas.extract_ptsvect(w_datas.xyz_name,rvp,false);
	  for(auto pp : rvp)
	    {
	      vp.emplace_back(pp);
	    }
	}
	w_datas.dmap[w_datas.xyz_name] = ddt_data<Traits>::Data_ply(w_datas.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
	w_datas.dmap[w_datas.xyz_name].fill_full_uint8_vect(vp);

	//	datas_map[hid].dmap[datas_map[hid].xyz_name].last().fill_full_uint8_vect(vp);
      }
    datas_map[hid].push_back(w_datas);
    std::cerr << "ser1" << std::endl;
    hpi.finalize();


    
  }
  std::cout.clear();


  for (auto  it = datas_map.begin(); it != datas_map.end(); it++ )
    {
      
      Id id =  it->first;
      int acc = 0;
      for(auto ddtm : it->second){
	std::cerr << "ser2" << std::endl;
	ddt::stream_data_header oqh_1("p","s",id),oqh_2("p","s",id);
	std::string filename(params.output_dir + "/" + params.slabel +
			     "_id_" + std::to_string(id) + "_" + std::to_string(tid) + "_" + std::to_string(acc++)) ;
	oqh_1.init_file_name(filename,"_pts.geojson");
	oqh_1.write_header(std::cout);
	oqh_2.init_file_name(filename,"_spx.geojson");
	oqh_2.write_header(std::cout);
        
	std::cerr << "ser3" << std::endl;
	ddtm.write_geojson_tri(oqh_1.get_output_stream(),oqh_2.get_output_stream());

  
	oqh_1.finalize();
	oqh_2.finalize();
	std::cerr << "ser4" << std::endl;
	ddt::add_qgis_style(oqh_2.get_file_name(),params.style);
	std::cerr << "DONE!" << std::endl;
	std::cout << std::endl;
      }
    }
  
  return 0;
}



// OK
int extract_struct(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{

  int D = Traits::D;
  std::cout.setstate(std::ios_base::failbit);
  // int D = Traits::D;
  std::vector<Point_id_id> vpis;
  std::vector<Point> vp;
  ddt::const_partitioner<Traits> part(tid);

  Scheduler sch(1);
  DDT tri;
  Traits traits;

  std::cerr << "load triangulation" << std::endl;
  for(int i = 0; i < nb_dat; i++)
    {
      ddt::stream_data_header hpi;
      hpi.set_logger(&log);
      hpi.parse_header(std::cin);
      //      std::cerr << "filename: " << hpi.get_file_name() << " [" << hpi.get_lab() << "]" << std::endl;
	
      if(hpi.get_lab() == "t" || hpi.get_lab() == "v" )
        {
	  log.step("[read]read_triangulation");
	  bool do_clean_data = true;
	  read_ddt_stream(tri,hpi.get_input_stream(),hpi.get_id(0),hpi.is_serialized(),do_clean_data,log);
        }
      hpi.finalize();
    }


  std::map<Id, std::vector<Point_id_id>>  outbox_nbrs;
  Tile_iterator tci = tri.get_tile(tid);
  get_edges(tci,outbox_nbrs);

  // Dump
  send_neighbors(tid,params,outbox_nbrs,true);
  std::cout << std::endl;

  typedef typename ddt_data<Traits>::Data_ply Data_ply;
  typedef typename Traits::Data_V Data_V;


  auto tile = tri.get_tile(tid);
  auto ttri = tile->triangulation();
  auto bbox = tile->bbox(tid);

  std::cout.clear();
  ddt::stream_data_header oth("b","s",tid);
  oth.write_header(std::cout);
  oth.get_output_stream() << bbox << std::endl;
  oth.finalize();
  std::cout << std::endl;

  return 0;
}



// Extract t he voronoi of the delaunay triangulation
// For each simplex, extract the centroi
// After looping on the edges in order to extract the neighboorhood
int extract_tri_voronoi(DDT & tri, std::map<int,std::vector<int>> & tile_ids,std::ostream & ofile, int main_tile_id, int area_processed)
{
  ofile << std::fixed << std::setprecision(15);

  typedef typename DDT::Cell_const_iterator                 Cell_const_iterator;
  typedef typename DDT::Vertex_const_iterator                 Vertex_const_iterator;
  typedef typename DDT::DT::Full_cell::Vertex_handle_iterator Vertex_h_iterator;
    
  int chunk_size = 10;
  int sourceId = 0;
  int targetId = 1;
  int  N = tri.number_of_cells();
  int NF = 0;
  int dim = Traits::D;
  chunk_size = 1;

  std::cerr << "init graph" << std::endl;
  for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
      NF++;
    }

  double e0,e1;

  int acc = 0;
  //    std::map<int,int> id_map;
  std::map<Vertex_const_iterator,int> vid_map;
  std::vector<int> id2gid_vec;
  std::cerr << "create ids" << std::endl;
  std::cerr << "area_processed:" << area_processed  << std::endl;
  if(area_processed == 1){
    for( auto vit = tri.vertices_begin();
	 vit != tri.vertices_end(); ++vit )
      {
	vid_map[vit] = acc++;
	if(vit->is_main()){
	  std::cerr << "ismain" << std::endl;
	}else{
	  std::cerr << "isnotmain" << std::endl;
	}
	if(vit->is_main()){
	  ofile << "v ";
	  ofile << vit->gid() << " ";
	  for(int i = 0 ; i < dim;i++)
	    ofile << vit->point()[i] << " ";
	  ofile << std::endl;
	}
      }


    std::cerr << "score simplex" << std::endl;
    for( auto cit = tri.cells_begin();
	 cit != tri.cells_end(); ++cit )
      {
	Cell_const_iterator fch = *cit;
	if(!cit->is_main())
	  continue;
   
	// if(tri->is_infinite(fch))
	//    continue;
	int tid = cit->tile()->id();
	int lid = cit->cell_data().id;
	int gid = cit->gid();
	int lcurr = 0;

	std::vector<double> cent(dim,0);
	for(int i = 0 ; i < dim+1;i++)
	  for(int j = 0 ; j < dim;j++)
	    cent[j] += cit->vertex(i)->point()[j];
 
      
	ofile << "s " <<   gid  << " ";
	// for(int i = 0 ; i < dim;i++)
	// 	ofile << cent[i]/(dim+1) << " ";
	std::cerr << "get_circumcenter" << std::endl;
	auto circumcenter = cit->tile()->circumcenter(cit->full_cell());
	std::cerr << "get_circumcenter" << std::endl;
	std::cerr << circumcenter << std::endl; 
	for(int i = 0 ; i < dim;i++)
	  ofile << circumcenter[i] << " ";
	// for(int i = 0 ; i < dim+1;i++)
	// 	ofile << cit->vertex(i)->vertex_data().gid << " ";

	// if(++acc % chunk_size == 0)
	ofile << std::endl;

      }
  }

  std::cerr << "score facet " << std::endl;
  for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
      if(fit->is_infinite())
	continue;
      try
	{

	  Cell_const_iterator tmp_fch = fit.full_cell();
	  int tmp_idx = fit.index_of_covertex();
	  Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

	  if(
	     (area_processed == 1 && tmp_fch->main_id() != tmp_fchn->main_id()) ||
 	     (area_processed == 2 && tmp_fch->main_id() == tmp_fchn->main_id()))
	    {
	      continue;
	    }

	  if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	     !tri.tile_is_loaded(tmp_fchn->main_id()))
	    {
	      //std::cerr << "ERROR : CELL NOT LOADED" << std::endl;
	      //	   return 0;
	      continue;
	    }


	  if(tmp_fch->is_infinite()  || tmp_fchn->is_infinite())
	    continue;

	  Cell_const_iterator fch = tmp_fch->main();
	  int idx = tmp_idx;
	  Cell_const_iterator fchn = tmp_fchn->main();


	  Vertex_h_iterator vht;

	  int lidc = fch->cell_data().id;
	  int lidn = fchn->cell_data().id;

	  int tidc = fch->tile()->id();
	  int tidn = fchn->tile()->id();

	  int gidc = fch->gid();
	  int gidn = fchn->gid();



	  // Belief spark
	  ofile << "e " << gidc << " " << gidn  << " ";
	  // if(++acc % chunk_size == 0)
	  ofile << std::endl;

	}
      catch (ddt::DDT_exeption& e)
	{
	  std::cerr << "!! WARNING !!!" << std::endl;
	  std::cerr << "Exception catched : " << e.what() << std::endl;
	  continue;
	}
    }
  std::cerr << "acc = " << acc << std::endl;
  return acc;
}

int extract_voronoi(Id tid,algo_params & params,int nb_dat,ddt::logging_stream & log)
{
  std::cout.setstate(std::ios_base::failbit);
  std::cerr << "seg_step0" << std::endl;

  DDT tri; 
  Scheduler sch(1);

  std::cerr << "seg_step1" << std::endl;

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
	  bool do_clean_data = true;
	  bool do_serialize = false;
	  read_ddt_stream(tri,hpi.get_input_stream(),hpi.get_id(0),hpi.is_serialized(),do_clean_data,log);

									      
        }
      if(hpi.get_lab() == "s")
        {
	  std::cerr << "tile ids loal:" << hid << std::endl;
	  std::vector<int> vv(3);
	  for(int d = 0; d < 3; d++)
            {
	      hpi.get_input_stream() >> vv[d];
            }
	  tile_ids[hid] = vv;
        }
      hpi.finalize();

    }






  log.step("compute");
  std::cerr << "seg_step5" << std::endl;


  log.step("write");
  std::cout.clear();
  ddt::stream_data_header oth("t","s",tid),osh("s","s",tid);;
  //oth.write_header(std::cout);
  std::cerr << "seg_step6" << std::endl;
  int nbc = 0;

  nbc = extract_tri_voronoi(tri,tile_ids,oth.get_output_stream(),tid,params.area_processed);

  std::cerr << "seg_step7" << std::endl;

  oth.finalize();
  std::cout << std::endl;


  osh.write_header(std::cout);
  osh.get_output_stream() << 0 << " " ;
  osh.get_output_stream() << 0 << " " ;
  osh.get_output_stream() << nbc << " " ;
  osh.finalize();
  std::cout << std::endl;

  return 0;
}


template <typename FTC>
int extract_simplex_soup(DDT & tri,FTC &filter,std::ostream & ofile, int main_tile_id, int area_processed)
{
  // ======================================
  tri.init_local_id();

  // ======================================


  ofile << std::fixed << std::setprecision(15);
  typedef typename DDT::Cell_const_iterator                 Cell_const_iterator;
  typedef typename DDT::Vertex_const_iterator                 Vertex_const_iterator;
  typedef typename DDT::DT::Full_cell::Vertex_handle_iterator Vertex_h_iterator;
    

  int  N = tri.number_of_cells();
  int NF = 0;
  int dim = Traits::D;
  
  std::cerr << "init graph" << std::endl;
  for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
      NF++;
    }

  double e0,e1;

  int acc = 0;
  //    std::map<int,int> id_map;
  std::map<Vertex_const_iterator,int> vid_map;
  std::vector<int> id2gid_vec;
  std::cerr << "create ids" << std::endl;
  std::cerr << "area_processed:" << area_processed  << std::endl;

  int lo_id=0;
  if(area_processed == 1){
    for( auto vit = tri.vertices_begin();
	 vit != tri.vertices_end(); ++vit )
      {
	vid_map[vit] = acc++;
	if(vit->is_main()){
	  //	  vit->vertex_data().id = (lo_id++);
	  ofile << "v ";
	  ofile << vit->vertex_data().gid << " ";
	  for(int i = 0 ; i < dim;i++)
	    ofile << vit->point()[i] << " ";
	  ofile << std::endl;
	}
      }


    std::cerr << "score simplex" << std::endl;
    for( auto cit = tri.cells_begin();
	 cit != tri.cells_end(); ++cit )
      {
	Cell_const_iterator fch = *cit;
	if(!cit->is_main())
	  continue;
   
	// if(tri->is_infinite(fch))
	//    continue;
	int tid = cit->tile()->id();
	int lid = cit->cell_data().id;
	int gid = cit->cell_data().gid;
	int lcurr = 0; 

	std::vector<double> cent(dim,0);
	for(int i = 0 ; i < dim+1;i++)
	  for(int j = 0 ; j < dim;j++)
	    cent[j] += cit->vertex(i)->point()[j];
 
      	ofile << "s " <<   gid  << " ";
	// for(int i = 0 ; i < dim;i++)
	// 	ofile << cent[i]/(dim+1) << " ";
	std::cerr << "get_circumcenter" << std::endl;
	auto circumcenter = cit->tile()->circumcenter(cit->full_cell());
	std::cerr << "get_circumcenter" << std::endl;
	std::cerr << circumcenter << std::endl; 
	for(int i = 0 ; i < dim;i++)
	  ofile << circumcenter[i] << " ";
	// for(int i = 0 ; i < dim+1;i++)
	// 	ofile << cit->vertex(i)->vertex_data().gid << " ";

	// if(++acc % chunk_size == 0)
	ofile << std::endl;

      }
  }

  std::cerr << "score facet " << std::endl;
  for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
    {
      if(fit->is_infinite())
	continue;
      try
	{

	  Cell_const_iterator tmp_fch = fit.full_cell();
	  int tmp_idx = fit.index_of_covertex();
	  Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

	  if(
	     (area_processed == 1 && tmp_fch->main_id() != tmp_fchn->main_id()) ||
 	     (area_processed == 2 && tmp_fch->main_id() == tmp_fchn->main_id()))
	    {
	      continue;
	    }

	  if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	     !tri.tile_is_loaded(tmp_fchn->main_id()))
	    {
	      //std::cerr << "ERROR : CELL NOT LOADED" << std::endl;
	      //	   return 0;
	      continue;
	    }


	  if(tmp_fch->is_infinite()  || tmp_fchn->is_infinite())
	    continue;

	  Cell_const_iterator fch = tmp_fch->main();
	  int idx = tmp_idx;
	  Cell_const_iterator fchn = tmp_fchn->main();


	  Vertex_h_iterator vht;

	  int lidc = fch->cell_data().id;
	  int lidn = fchn->cell_data().id;

	  int tidc = fch->tile()->id();
	  int tidn = fchn->tile()->id();

	  int gidc = fch->cell_data().gid;
	  int gidn = fchn->cell_data().gid;



	  // Belief spark
	  ofile << "e " << gidc << " " << gidn  << " ";
	  // if(++acc % chunk_size == 0)
	  ofile << std::endl;

	}
      catch (ddt::DDT_exeption& e)
	{
	  std::cerr << "!! WARNING !!!" << std::endl;
	  std::cerr << "Exception catched : " << e.what() << std::endl;
	  continue;
	}
    }
  // ================================


  std::cerr << "seg_step7" << std::endl;




}

int extract_simplex_soup_main(Id tid,algo_params & params,int nb_dat,ddt::logging_stream & log)
{
  std::cout.setstate(std::ios_base::failbit);
  std::cerr << "seg_step0" << std::endl;

  DDT tri; 
  Scheduler sch(1);

  std::cerr << "seg_step1" << std::endl;



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
	  bool do_clean_data = true;
	  bool do_serialize = false;
	  read_ddt_stream(tri,hpi.get_input_stream(),hpi.get_id(0),hpi.is_serialized(),do_clean_data,log);
        }
      if(hpi.get_lab() == "s")
        {
	  std::cerr << "tile ids loal:" << hid << std::endl;
	  std::vector<int> vv(3);
	  for(int d = 0; d < 3; d++)
            {
	      hpi.get_input_stream() >> vv[d];
            }
	  tile_ids[hid] = vv;
        }
      hpi.finalize();

    }
  log.step("compute");
  std::cout.clear();
  ddt::stream_data_header oth("t","s",tid);
  std::cerr << "seg_step6" << std::endl;
  int nbc = 0;


  //
  
  //  extract_simplex_soup(tri,w_datas_tri,oth.get_output_stream(),tid,params.area_processed);
  oth.finalize();
  std::cout << std::endl;
  return 0;
}


int serialized2datastruct(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{

  int D = Traits::D;

  D_MAP datas_map;
    
  for(int i = 0; i < nb_dat; i++){
    std::cerr << "convert data " << i << std::endl;

    ddt::stream_data_header hpi;
    hpi.parse_header(std::cin);

    DDT tri1;
    Traits traits;

    Id hid = hpi.get_id(0);
    if(hpi.get_lab() == "t" || hpi.get_lab() == "u" || hpi.get_lab() == "v")
      {
	std::cerr << "READ:" << hpi.get_lab() << std::endl;
	bool do_clean_data = true;
	read_ddt_stream(tri1,hpi.get_input_stream(),hpi.get_id(0),hpi.is_serialized(),do_clean_data,log);
	auto  tile  = tri1.get_tile(tid);
	tile->update_local_flag();
	typename DDT::Traits::Delaunay_triangulation & ttri = tile->tri();
	traits.export_tri_to_data(ttri,datas_map[hid]);
      } else if(hpi.get_lab() == "p"  || hpi.get_lab() == "z")
      {
	std::cerr << " " << std::endl;
	std::cerr << "=== Parse pts ===" << std::endl;
	std::vector<Point> vp;
	if(hpi.is_serialized()){
	  std::cerr << "is ser!" << std::endl;
	  std::vector<Point> rvp;
	  //ddt::read_point_set_serialized(rvp, hpi.get_input_stream(),traits);
	  ddt_data<Traits> w_datas;
	  w_datas.read_serialized_stream(hpi.get_input_stream());
	  w_datas.extract_ptsvect(w_datas.xyz_name,rvp,false);
	  for(auto pp : rvp)
	    {
	      vp.emplace_back(pp);
	    }
	}
	datas_map[hid].dmap[datas_map[hid].xyz_name] = ddt_data<Traits>::Data_ply(datas_map[hid].xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
	datas_map[hid].dmap[datas_map[hid].xyz_name].fill_full_uint8_vect(vp);
      }

    datas_map[hid].stream_lab = hpi.get_lab();
    std::cerr << "stream lab: " << datas_map[hid].stream_lab << std::endl;
    hpi.finalize();
  }
  std::cout.clear();

  for (auto  it = datas_map.begin(); it != datas_map.end(); it++ )
      {
	Id hid =  it->first;
	std::cerr << "hid" << hid << std::endl;
	std::string stream_lab = datas_map[hid].stream_lab;
	ddt::stream_data_header ozh(stream_lab,"z",hid);
	ozh.write_header(std::cout);
	datas_map[hid].write_serialized_stream(ozh.get_output_stream());
	ozh.finalize();  
	std::cout << std::endl;
      }

  return 0;
      
}

int datastruct_identity(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log){
  int D = Traits::D;
  D_MAP datas_map;
    
  for(int i = 0; i < nb_dat; i++){
    std::cerr << "convert data " << i << std::endl;

    ddt::stream_data_header hpi;
    hpi.parse_header(std::cin);


    DDT tri1;
    Traits traits;

    Id hid = hpi.get_id(0);

    ddt_data<Traits>  w_datas;
    w_datas.stream_lab = hpi.get_lab();
    w_datas.read_serialized_stream(hpi.get_input_stream());
    hpi.finalize();

    datas_map[hid] = w_datas;

  }
  std::cout.clear();

  for (auto  it = datas_map.begin(); it != datas_map.end(); it++ )
    {
      Id hid =  it->first;
      std::string stream_lab = datas_map[hid].stream_lab;
      std::cerr << "==== WRITE ==== " << hid << std::endl;
      ddt::stream_data_header ozh(stream_lab,"z",hid);
      ozh.write_header(std::cout);
      datas_map[hid].write_serialized_stream(ozh.get_output_stream());
      ozh.finalize();  
      std::cout << std::endl;
    }
  

  
  return 0;

}



  


double doubleRand() {
  return double(std::rand()) / (double(RAND_MAX) + 1.0);
}


// =================== Data Processing =====================
// Generate point from normal distribution
int generate_points_normal(Id tid,algo_params & params,ddt::logging_stream & log)
{

  int dim = Traits::D;
  Traits traits;

  ddt::Bbox<Traits::D> bbox;
  std::stringstream ss;
  ss << params.bbox_string;
  ss >> bbox;
  double bb_l = bbox.max(0)-bbox.min(0);
  
  std::vector<Point> vp;
  int nbs = 1;//doubleRand()*1.05; // One tile over 3 produce points to have empty area
  const int tot_count= params.nbp;//doubleRand()*5000;  // number of experiments
  int acc = 0;

  //  int rvvd = std::pow(10,NB_DIGIT_OUT-1);
  while(acc < tot_count-1){


    std::default_random_engine generator(params.seed);
    double sig = doubleRand()*(bb_l/50) + bb_l/10000;
    std::normal_distribution<double> distribution(0,sig);

    std::vector<double> ori(Traits::D);
    for(int d = 0 ; d < dim; d++){
      //ori[d] = doubleRand()*(((bbox.max(d)-bbox.min(d)) + bbox.min(d)) - sig*6) + sig*3;
      double len = bbox.max(d)-bbox.min(d);
      ori[d] = bbox.min(d) + doubleRand()*(len-sig*6) + sig*3;
    }
    int count = 0.5*(params.nbp)*doubleRand();

    for(int n = 0; n < count; n++)
      {
	std::vector<double> coords(Traits::D);
	for(int d = 0 ; d < dim; d++){
	  coords[d] = ori[d]+distribution(generator); ;
	}

	bool do_skip = false;
	for(int d = 0 ; d < dim; d++){
	  do_skip = ((coords[d] < bbox.min(d) || coords[d] > bbox.max(d)) || do_skip);
	}
	if(do_skip)
	  continue;
	
	vp.emplace_back(traits.make_point(coords.begin()));
	acc++;
	if(acc >= tot_count-1)
	  break;
      }
  }

  std::cout.clear();


 
  ddt::stream_data_header ozh("z","z",tid);
  ozh.write_header(std::cout);
  // ddt_data<Traits> w_datas;
  // w_datas.dmap[w_datas.xyz_name].fill_full_uint8_vect(vp);
  // w_datas.write_serialized_stream(ozh.get_output_stream());
  ddt::write_point_set_serialized(vp,ozh.get_output_stream(),dim);
  ozh.finalize();
  std::cout << std::endl;

  
  // ddt_data<Traits> datas_out;  
  // datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",dim,dim,DATA_FLOAT_TYPE);
  // datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(vp);
  // log.step("write");

  // ddt::stream_data_header oqh("g","s",tid);
  // std::string filename(params.output_dir + "/tile_" + params.slabel +"_id_"+ std::to_string(tid) + "_" + std::to_string(tid));
  // if(!params.do_stream)
  //   oqh.init_file_name(filename,".ply");
  // oqh.write_header(std::cout);
  // datas_out.write_ply_stream(oqh.get_output_stream(),PLY_CHAR);
  // oqh.finalize();
  // std::cout << std::endl;

  ddt::stream_data_header och("c","s",tid);
  och.write_header(std::cout);
  och.get_output_stream() << vp.size();
  och.finalize();
  std::cout << std::endl;
  
 
  return 0;
}
// Generate point from uniform distribution
int generate_points_uniform(Id tid,algo_params & params,ddt::logging_stream & log)
{

  std::cout.setstate(std::ios_base::failbit);
  Traits traits;
  int D = Traits::D;
  int ND = params.nbt_side;
  int NP = params.nbp;
  int NT = pow(ND,D);

  ddt::Bbox<Traits::D> bbox;
  std::stringstream ss;
  ss << params.bbox_string;
  ss >> bbox;

  //  double range = 100.0;
  std::vector<double> base(D);
  int d = tid;
  for(int n = 0; n < D; n++)
    {
      double range = bbox.max(n) - bbox.min(n);
      base[n] = bbox.min(n) + (d % ND )/(ND/(range*2)) - range;
      d = d/ND;
    }
  int py =tid % ND;
  int px = ((int)tid)/ND;


  //std::srand(0);
  int count = NP/NT;
  std::vector<Point> vp;

  std::cerr << "Count : " << count << std::endl;
  log.step("process");
  for(int n = 0; n < count; n++)
    {
      std::vector<double> coords(Traits::D);
      for(int d = 0 ; d < D; d++)
	{
	  double range = bbox.max(d) - bbox.min(d);
	  coords[d] = base[d] + (((double) rand() / (RAND_MAX))/(((double)ND)/2.0))*(range*0.95) + range*0.01;
	}
      vp.push_back(traits.make_point(coords.begin()));
    }

  ddt_data<Traits> datas_out;
  datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
  datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(vp);

  log.step("write");
  std::cout.clear();
  ddt::stream_data_header oqh("p","s",tid),och("c","s",tid);
  std::string filename(params.output_dir + "/tile_" + params.slabel +"_id_"+ std::to_string(tid) + "_" + std::to_string(tid));

  oqh.write_header(std::cout);
  datas_out.write_ply_stream(oqh.get_output_stream(),PLY_CHAR);
  oqh.finalize();
  std::cout << std::endl;
  och.write_header(std::cout);
  och.get_output_stream() << vp.size();
  och.finalize();
  std::cout << std::endl;

  return 0;
}





// ========================= Data tiling section ============================
// tile ply
int tile_ply(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{

  std::cout.setstate(std::ios_base::failbit);
  int D = Traits::D;
  int ND = params.nbt_side;
  int NP = params.nbp;
  int NT = pow(ND,D);
  double range = 1000.;
  ddt::Bbox<Traits::D> bbox;
  std::stringstream ss;
  ss << params.bbox_string;
  ss >> bbox;
  Grid_partitioner part(bbox, ND);
  std::cerr << "read : " << std::endl;
  log.step("read");
  std::map<Id,ddt_data<Traits> > tile_map;
  std::map<Id,std::vector<Point> > vp_map;
  Traits traits;
    
  for(int i = 0; i < nb_dat; i++)
    {
      std::vector<Point> vp_in;    
      ddt::stream_data_header hpi;
      ddt_data<Traits> w_datas;
      int count;
      hpi.parse_header(std::cin);
      if(hpi.get_lab() == "p")
	{
	  w_datas.read_ply_stream(hpi.get_input_stream());
	  count = w_datas.nb_pts_shpt_vect();
	}
      if(hpi.get_lab() == "g")
	{
	  w_datas.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
	  count = w_datas.nb_pts_shpt_vect();
	}
      if(hpi.get_lab() == "z" )
	{
	  //	  ddt::read_point_set_serialized(vp_in, hpi.get_input_stream(),traits);
	  ddt_data<Traits> w_datas;
	  std::cerr << " ======= read serialized ====== " << std::endl;
	  w_datas.read_serialized_stream(hpi.get_input_stream());
	  std::cerr << "extract" << std::endl;
	  //	  w_datas.extract_ptsvect(w_datas.xyz_name,vp_in,false);
	  w_datas.dmap[w_datas.xyz_name].extract_full_uint8_vect(vp_in,true);
	  std::cerr << "done" << std::endl;
	  count = vp_in.size();
	  std::cerr << "==== count:" << count << std::endl;

	}
      

      hpi.finalize();
      std::cerr << "hpi finalized" << std::endl;



      std::cerr << "start tiling" << std::endl;
      for(; count != 0; --count)
	{
	  Point  p;
	  if(vp_in.size() > 0)
	    p = vp_in[count];
	  else
	    p = w_datas.get_pts(count);
	  std::cerr << "ppp:" << p << std::endl;
	  
	  bool is_out = false;
	  for(int d = 0; d < D; d++)
	    {
	      if(p[d] < bbox.min(d) || p[d] > bbox.max(d))
		is_out = true;
	    }
	  if(is_out)
	    continue;

	  int pp = part(p);
	  Id id = Id(pp % NT);

	  if(true){
	    std::vector<double> coords(Traits::D);

	    for(int d = 0 ; d < D; d++){
	      double range = bbox.max(d) - bbox.min(d);
	      coords[d] = p[d];
	    }

	    vp_map[id].emplace_back(traits.make_point(coords.begin()));

	  }else{
	    auto it = tile_map.find(id);
	    if(it==tile_map.end())
	      {
		tile_map[id] = ddt_data<Traits>(w_datas.dmap);
	      }
	    tile_map[id].copy_point(w_datas,count);
	  }
	}
    }
  std::cout.clear();
  std::cerr << "count finalized" << std::endl;

  log.step("write");
  if(true){

    for ( const auto &myPair : vp_map ) {
      Id id = myPair.first;
      std::vector<Point> & vp = vp_map[id];
      int nb_out = vp.size();
      std::cerr << "vp : NBOUT:" << nb_out << std::endl;
      if(nb_out < params.min_ppt){
	std::cerr <<  " === !!!!! WARNING !!!! === "  << std::endl;
	std::cerr <<  "skiping tile : " << id << std::endl;
	continue;
      }

      ddt::stream_data_header oqh("z","z",id),och("c","s",id);
      oqh.write_header(std::cout);
      ddt_data<Traits> w_datas;
      w_datas.dmap[w_datas.xyz_name].fill_full_uint8_vect(vp);
      w_datas.write_serialized_stream(oqh.get_output_stream());
      //ddt::write_point_set_serialized(vp,oqh.get_output_stream(),D);
      oqh.finalize();
      std::cout << std::endl;
      och.write_header(std::cout);
      och.get_output_stream() << nb_out;
      och.finalize();
      std::cout << std::endl;
    }
    
  }else{
    for ( const auto &myPair : tile_map )
      {
	Id id = myPair.first;
	int nb_out = tile_map[id].nb_pts_uint8_vect ();
	if(nb_out < params.min_ppt)
	  {
	    continue;
	  }

	ddt::stream_data_header oqh("z","s",id),och("c","s",id);
	std::string filename(params.output_dir + "/tile_" + params.slabel +"_id_"+ std::to_string(tid) + "_" + std::to_string(id));

	oqh.write_header(std::cout);
	tile_map[id].write_serialized_stream(oqh.get_output_stream());
	//	tile_map[id].write_ply_stream(oqh.get_output_stream(),PLY_CHAR);
	oqh.finalize();
	std::cout << std::endl;
	och.write_header(std::cout);
	och.get_output_stream() << nb_out;
	och.finalize();
	std::cout << std::endl;
      }
  }
  return 0;
}

// OK
int dump_ply_binary(Id tid,algo_params & params, int nb_dat,ddt::logging_stream & log)
{

  std::cout.setstate(std::ios_base::failbit);
  int D = Traits::D;
  Traits traits;
  for(int i = 0; i < nb_dat; i++)
    {
      ddt::stream_data_header hpi;
      hpi.parse_header(std::cin);

      ddt_data<Traits> w_datas_in;	
      if(hpi.get_lab() == "p"  || hpi.get_lab() == "g"  || hpi.get_lab() == "t" || hpi.get_lab() == "u" || hpi.get_lab() == "v" || hpi.get_lab() == "z")
	{

	  if(hpi.get_lab() == "p"  || hpi.get_lab() == "z")
	    {
	      std::cerr << " " << std::endl;
	      std::cerr << "=== Parse pts ===" << std::endl;
	      std::vector<Point> vp;
	      if(hpi.is_serialized()){
		std::cerr << "is ser!" << std::endl;
		std::vector<Point> rvp;
		//		ddt::read_point_set_serialized(rvp, hpi.get_input_stream(),traits);
		ddt_data<Traits> w_datas;
		w_datas.read_serialized_stream(hpi.get_input_stream());
		w_datas.dmap[w_datas.xyz_name].extract_full_uint8_vect(rvp,true);
		//w_datas.extract_ptsvect(w_datas.xyz_name,rvp,false);
		for(auto pp : rvp)
		  {
		    vp.emplace_back(pp);
		  }
	      }
	      std::cerr << "read ser done" << std::endl;
	      w_datas_in.dmap[w_datas_in.xyz_name] = ddt_data<Traits>::Data_ply(w_datas_in.xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
	      w_datas_in.dmap[w_datas_in.xyz_name].fill_full_uint8_vect(vp);
	    }else{
	    w_datas_in.read_ply_stream(hpi.get_input_stream(),PLY_CHAR);
	  }
	  hpi.finalize();

	  std::vector<double> v_xyz;

	  std::cout.clear();
	  Id id = hpi.get_id(0);
	  ddt::stream_data_header oqh("p","s",id);
	  std::string filename(params.output_dir + "/" + params.slabel + "_id_"+ std::to_string(tid) + "_" + std::to_string(id) + "_" + std::to_string(i) );
	  oqh.init_file_name(filename,".ply");
	  oqh.write_header(std::cout);
	  //w_datas_in.write_ply_stream(oqh.get_output_stream(),'\n',false);
	  w_datas_in.write_ply_stream(oqh.get_output_stream(),'\n',false);
	  oqh.finalize();
	  std::cout << std::endl;
	}
    }
  return 0;
}


//  ================== Main function  ====================
int main(int argc, char **argv)
{


  std::cout.setstate(std::ios_base::failbit);

  // Main algo parser
  // Read input
  algo_params params;
  params.parse(argc,argv);
  int rv = 0;
  int loop_acc=0;
  // Loop over input if several inputs by partitions
  while(true)
    {
      // Header of the executable generate_points_uniform
      ddt::stream_app_header sah;
      sah.parse_header(std::cin);
      // If std::cin empty, exit
      if(sah.is_void())
	return 0;

      if(loop_acc == 0)
	{
	  std::cerr << "================= CPP PIPE LOG =======================" << std::endl;
	  std::cerr << "     cpp_label_" << params.slabel << std::endl;
	  std::cerr << "     cpp_step_" << params.algo_step << std::endl;
	}
      loop_acc++;

      Id tile_id = ((Id)sah.tile_id);

      // To have a different seed for each tiles, if not, each tiles in random case has the same point cloud
      srand(params.seed*tile_id);
      //srand(time(NULL));
      int nb_dat = sah.get_nb_dat();
      ddt::logging_stream log(params.algo_step, params.log_level);
      bool do_dump_log = false;
      std::cerr << " ------------  [LOOP DATA LOG] ===> " << tile_id << "_" << params.algo_step << std::endl;
      try
	{
	  if(params.algo_step == std::string("generate_points_random_uniform"))
	    {
	      rv = generate_points_uniform(tile_id,params,log);
	    }
	  else if(params.algo_step == std::string("generate_points_random_normal"))
	    {
	      rv = generate_points_normal(tile_id,params,log);
	    }
	  else if(params.algo_step == std::string("insert_in_triangulation"))
	    {
	      if(params.extract_tri_crown)
		rv = insert_raw(tile_id,params,nb_dat,log);
	      else
		rv = insert_in_triangulation(tile_id,params,nb_dat,log);
	    }
	  else if(params.algo_step == std::string("get_bbox_points"))
	    {
	      rv = get_bbox_points(tile_id,params,nb_dat,log);
	      do_dump_log  = false;
	    }
	  else if(params.algo_step == std::string("tile_ply"))
	    {
	      if(params.bbox_string.empty())
		{
		  std::cerr << "ERROR, no bbox " << std::endl;
		  return 1;
		}
	      rv = tile_ply(tile_id,params,nb_dat,log);
	      do_dump_log = false;
	    }
	  else if(params.algo_step == std::string("ply2geojson"))
	    {
	      do_dump_log = false;
	      rv = ply2geojson(tile_id,params,nb_dat,log);
	    }
	  else if(params.algo_step == std::string("tri2geojson"))
	    {
	      do_dump_log = false;
	      rv = serialized2geojson(tile_id,params,nb_dat,log);
	    }
	  else if(params.algo_step == std::string("datastruct_identity"))
	    {
	      do_dump_log = false;
	      rv = datastruct_identity(tile_id,params,nb_dat,log);
	    }
	  else if(params.algo_step == std::string("serialized2datastruct"))
	    {
	      do_dump_log = false;
	      rv = serialized2datastruct(tile_id,params,nb_dat,log);
	    }
	  else if(params.algo_step == std::string("ply2dataset"))
	    {
	      do_dump_log = false;
	      rv = ply2dataset(tile_id,params,nb_dat,log);
	    }
	  else if(params.algo_step == std::string("dump_ply_binary"))
	    {
	      rv = dump_ply_binary(tile_id,params,nb_dat,log);
	      do_dump_log  = false;
	    }
	  else if(params.algo_step == std::string("extract_struct"))
	    {
	      rv = extract_struct(tile_id,params,nb_dat,log);
	    }
	  else if(params.algo_step == std::string("extract_voronoi"))
	    {
	      rv = extract_voronoi(tile_id,params,nb_dat,log);
	      do_dump_log = false;
	    }
	  else if(params.algo_step == std::string("extract_simplex_soup_main"))
	    {
	      rv = extract_simplex_soup_main(tile_id,params,nb_dat,log);
	      do_dump_log = false;
	    }
	  else if(params.algo_step == std::string("update_global_id"))
	    {
	      rv = update_global_id(tile_id,params,nb_dat,log);
	      do_dump_log = false;
	    }
	  else if(params.algo_step == std::string("get_neighbors"))
	    {
	      std::map<Id, std::vector<Point_id_id>>  outbox;
	      log.step("read");
	      rv = get_neighbors(tile_id,params,outbox,log);
	      std::cout.clear();
	      log.step("write");
	      rv = send_neighbors(tile_id,params,outbox,true);
	      do_dump_log = false;
	    }
	  else if(params.algo_step == std::string("validity_check"))
	    {
	      //validity_check(tile_id,params,outbox);
	    }
	  else
	    {
	      std::cerr << "no params, step unknown =[" << params.algo_step << "]" << std::endl;
	      return 10;
	    }
	  if(rv != 0)
	    {
	      std::cerr << "ERROR RV : main_ddt_stream.cpp main function, RV != 0" << std::endl;
	      return rv;
	    }
	  //std::cerr << "     [ERR LOG] <=== " << tile_id << "_" << params.algo_step << std::endl;

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
	  std::cerr << "tid               : " << tile_id << std::endl;
	}
    }

  std::cerr << "[ERR LOG] end exe " << std::endl;
  return rv;
}
