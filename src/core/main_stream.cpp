typedef int Id;
typedef int Flag;
typedef int FlagC;




#include <CGAL/Random.h>

#include <stdio.h>      /* printf */
#include <math.h>
#include <main_stream.hpp>
#include <io/stream_api.hpp>
#include "io/write_stream.hpp"
#include "io/write_vrt.hpp"
#include "io/write_geojson.hpp"
#include "io/read_stream.hpp"

#include "tile.hpp"
#include "traits/traits.hpp"
#include "scheduler/scheduler.hpp"
#include "partitioner/grid_partitioner.hpp"
#include "partitioner/const_partitioner.hpp"
#include "DDT.hpp"
#include "io/logging.hpp"

typedef ddt::Traits<Id,Flag,FlagC> Traits;
typedef ddt::Tile<Traits> Tile;
typedef ddt::Scheduler<Tile> Scheduler;
typedef ddt::DDT<Traits> DDT;
typedef ddt::grid_partitioner<Traits> Grid_partitioner;


typedef Traits::Random_points_in_box Random_points;
typedef typename DDT::Tile_const_iterator  Tile_const_iterator ;
typedef typename DDT::Tile_iterator  Tile_iterator ;

typedef typename Traits::Point       Point;
typedef typename Traits::Id          Id;
typedef typename Traits::Point_id    Point_id;
typedef typename Traits::Point_id_id Point_id_id;

typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
typedef typename Tile::Vertex_const_handle Vertex_const_handle;


typedef typename Tile::Point_and_id Point_and_id;
typedef std::tuple<Point,Id,Id>                  Point_id_source;






int generate_points(Id tid,algo_params & params)
{
    ddt::logging log("Generate point ", params.log_level);
    std::cout.setstate(std::ios_base::failbit);
    int dim = params.dim;
    int ND = params.nbt_side;
    int NP = params.nbp;
    int NT = pow(ND,dim);
    double range = 1.;
    ddt::Bbox<Traits::D> bbox(-range,range);
    CGAL::Random rnd(params.seed);
    Random_points pit(dim, range,rnd);
    Grid_partitioner part(bbox, ND);
    //std::srand(0);
    int count = NP;
    std::vector<Point> vp;

    log.step("Generate point");
    for(; count; --count, ++pit)
    {
        Point p(*pit);
        std::vector<double> coords(Traits::D);
        Id id = Id(part(p) % NT);
        if(tid == id)
        {
            vp.push_back(p);
        }
    }

    log.step("Write point");
    std::cout.clear();
    ddt::stream_data_header oqh("p","s",tid),och("c","s",tid);;
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    oqh.init_file_name(filename,".pts");
    log.step("Write header");
    oqh.write_header(std::cout);
    log.step("Write point");
    ddt::write_points_stream(vp,oqh.get_output_stream(),dim);
    log.step("Finalize");
    oqh.finalize();
    std::cout << std::endl;

    och.write_header(std::cout);
    och.get_output_stream() << vp.size();
    och.finalize();
    std::cout << std::endl;

    return 0;
}


int insert_in_triangulation(Id tid,algo_params & params, int nb_dat)
{

    ddt::logging log("Insert in triangulation ", params.log_level);
    std::cout.setstate(std::ios_base::failbit);
    // int dim = params.dim;
    std::vector<Point_id_id> vpis;
    std::vector<Point> vp;
    ddt::const_partitioner<Traits> part(tid);

    Scheduler sch(1);
    DDT tri;
    Traits traits;

    bool do_simplify = true;
    for(int i = 0; i < nb_dat; i++)
    {
        ddt::stream_data_header hpi;
        log.step("Parse reader");
        hpi.parse_header(std::cin);
        log.step("Read stream");
        if(hpi.get_lab() == "t")
        {
            read_tile_stream(tri, hpi.get_input_stream(),hpi.get_id(0));
        }
        if(hpi.get_lab() == "p")
        {
            ddt::read_points_stream(vp,hpi.get_input_stream(),traits);
        }
        if(hpi.get_lab() == "q" || hpi.get_lab() == "e")
        {

            ddt::read_points_id_source_stream(vpis, hpi.get_input_stream(), traits);
            if(hpi.get_lab() == "q")
                do_simplify=true;
        }
        log.step("Finalize");
        hpi.finalize();
    }

    log.step("Process");
    int nbi = 0;
    std::cerr << "vpis size:" << vpis.size() << std::endl;
    std::cerr << "vp   size:" << vp.size() << std::endl;
    if(vpis.size() > 0)
    {

        std::vector<Point_id_id> svh_new;
        std::vector<Point_id> v_pai;
        for(auto & pp : vpis)
        {
            Id idp =  std::get<1>(pp);
            Id id_source =  std::get<2>(pp);
            bool is_inserted = tri.get_tile(tid)->points_sent_[id_source].insert(std::get<0>(pp)).second;
            if( id_source==tid  || !is_inserted)
            {
                continue;
            }
            else
            {
                std::cout << "[" <<  idp << "," << id_source << "," << is_inserted << "]";
                svh_new.push_back(pp);
                v_pai.emplace_back(std::get<0>(pp), idp);
            }
        }
        if(svh_new.size()> 0)
            nbi +=tri.get_tile(tid)->insert(v_pai,do_simplify);
    }
    std::cout << std::endl;
    if(vp.size() > 0)
    {
        tri.send_points(sch,vp.begin(),vp.size(),part);
        nbi += tri.insert(sch);
    }



    std::cout.clear();
    log.step("Write header");
    ddt::stream_data_header oth("t","s",tid),och("c","s",tid);
    std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid));
    oth.init_file_name(filename,".tri");
    oth.write_header(std::cout);
    log.step("Write File");
    ddt::write_tile_stream(tri, oth.get_output_stream(),tid);
    log.step("Finalize");
    oth.finalize();
    std::cout << std::endl;


    och.write_header(std::cout);
    och.get_output_stream() << nbi;
    och.finalize();
    std::cout << std::endl;
    return 0;
}

int get_bbox_points(Id tid,algo_params & params, int nb_dat)
{
    std::cout.setstate(std::ios_base::failbit);
    int dim = params.dim;

    ddt::logging log("get bbox point ", params.log_level);
    std::vector<Vertex_const_handle> vvhc;


    ddt::const_partitioner<Traits> part(tid);
    DDT tri;
    Scheduler sch(1);

    for(int i = 0; i < nb_dat; i++)
    {
        log.step("Parse header");
        ddt::stream_data_header hpi;
        hpi.parse_header(std::cin);
        if(hpi.get_lab() == "t")
        {
            log.step("read stream");
            std::string filename = hpi.get_file_name();
            read_tile_stream(tri, hpi.get_input_stream(),hpi.get_id(0));
        }
        log.step("Finalize");
        hpi.finalize();
    }

    ddt::get_bbox_points()(*(tri.get_tile(tid)), std::back_inserter(vvhc));

    std::cout.clear();
    for(auto vv : vvhc)
    {
        int tmp_id = ((int)tid);
        Point_id_id pis = std::make_tuple(vv->point(),tmp_id,tmp_id);
        ddt::stream_data_header oqh("b","s",tid);
        ddt::write_point_id_source<Point_id_id,Point>(pis,oqh.get_output_stream(),dim);
        std::cout << std::endl;
    }


    return 0;
}



int extract_tile_vrt(Id tid,algo_params & params, int nb_dat)
{

    DDT tri;
    // Scheduler sch(1);

    ddt::read_full_stream(tri,std::cin,nb_dat);
    // auto  tci = tri.get_tile(tid);


    // if(params.show_ghost)
    // {
    //   std::string vrt_name(params.output_dir +  "/" + params.slabel + "_" + "tri_cell_full" +  std::to_string(tid));
    //   write_tile_vrt_cells(*tci,vrt_name,false);

    //   ddt::stream_data_header oth("j","s",tid);
    //   oth.init_file_name(vrt_name,".vrt");
    //   oth.write_header(std::cout);
    //   oth.finalize();
    //   std::cout << std::endl;

    // }


    std::cout.clear();
    // for(auto tile = tri.tiles_begin(); tile != tri.tiles_end(); ++tile)
    //   {
    //     ddt::stream_data_header oth("j","f",tid);
    //     std::string json_name(params.output_dir +  "/" + params.slabel + "_" + "tri_cell_full" +  std::to_string(tid));
    //     oth.init_file_name(json_name,".vrt");
    //     oth.write_header(std::cout);
    //     oth.finalize();
    //     std::cout << std::endl;
    //     ddt::write_geojson_tri(*tile,oth.get_output_stream());
    //   }



    //  write_tile_vrt_cells(*tci,params.output_dir +  "/" + params.slabel + "_" + "tri_cell_main" +  std::to_string(tid)+ ".vrt",true);
    return 0;
}

int extract_tri_vrt(Id tid,algo_params & params, int nb_dat)
{

    DDT tri;
    Scheduler sch(1);

    ddt::read_full_stream(tri,std::cin,nb_dat);
    std::string json_name(params.output_dir +  "/" + params.slabel + "_" + std::to_string(tid) + "_tri");

    std::cout.clear();
    ddt::stream_data_header oth("j","h",tid);
    oth.init_file_name(json_name,".json");
    oth.write_header(std::cout);
    ddt::write_geojson_tri(tri,oth.get_output_stream());
    oth.finalize();
    std::cout << std::endl;
    return 0;
}



int extract_tri_json(Id tid,algo_params & params, int nb_dat)
{

    DDT tri;
    Scheduler sch(1);

    ddt::read_full_stream(tri,std::cin,nb_dat);
    std::string json_name(params.output_dir +  "/" + params.slabel + "_" + std::to_string(tid) + "_tri");

    std::cout.clear();
    ddt::stream_data_header oth("j","f",tid);
    oth.init_file_name(json_name,".json");
    oth.write_header(std::cout);
    ddt::write_geojson_tri(tri,oth.get_output_stream());
    oth.finalize();
    std::cout << std::endl;
    return 0;
}



int send_neighbors(Id tid,algo_params & params, std::map<Id, std::vector<Point_id_id>> & outbox)
{
    int dim = params.dim;
    std::cout.clear();
    for(auto&& mit : outbox)
    {
        Id nb_tid = mit.first;
        std::vector<Point_id_id> svh = mit.second;
        if(!svh.empty())
        {
            ddt::stream_data_header hto("e","s",std::vector<int> {tid,nb_tid});
            std::string filename(params.output_dir + "/" + params.slabel + "_id" + std::to_string(tid) + "_nid" + std::to_string(nb_tid));
            //hto.init_file_name(filename,".pts");
            hto.write_header(std::cout);
            ddt::write_points_id_source_stream<Point_id_id,Point>(svh,hto.get_output_stream(),dim);
            hto.finalize();
            std::cout << std::endl;
        }
    }

    return 0;
}


void get_neighbors_pids(Tile_iterator & tci, std::map<Id, std::vector<Point_id_id>> & outbox)
{
    std::vector<Vertex_const_handle_and_id> out;
    ddt::get_neighbors()(*tci, std::back_inserter(out));
    for(auto&& pair : out)
        outbox[pair.second].emplace_back(std::make_tuple(pair.first->point(),tci->id(pair.first),tci->id()));
}


int get_neighbors(Id tid,algo_params & params, std::map<Id, std::vector<Point_id_id>> & outbox)
{
    DDT tri;
    Scheduler sch(1);

    ddt::read_full_stream(tri,std::cin,1);
    Tile_iterator tci = tri.get_tile(tid);
    get_neighbors_pids(tci,outbox);

    return 0;
}



int main(int argc, char **argv)
{

    srand(time(NULL));
    std::cout.setstate(std::ios_base::failbit);



    // Read input
    algo_params params;
    params.parse(argc,argv);
    int rv = 0;
    // Loop over input if several inputs by partitions
    while(true)
    {

        // Header of the executable
        ddt::stream_app_header sah;
        sah.parse_header(std::cin);
        // If std::cin empty, exit
        if(sah.is_void())
            return 0;

        Id tile_id = ((Id)sah.tile_id);
        int nb_dat = sah.get_nb_dat();
        std::cerr << "     [ERR LOG] stream:" << params.slabel << std::endl;
        std::cerr << "     [ERR LOG] ===> " << tile_id << "_" << params.algo_step << std::endl;
        try
        {
            if(params.algo_step == std::string("generate_points"))
            {
                rv = generate_points(tile_id,params);
            }
            else if(params.algo_step == std::string("insert_in_triangulation"))
            {
                rv = insert_in_triangulation(tile_id,params,nb_dat);
            }
            else if(params.algo_step == std::string("get_bbox_points"))
            {
                rv = get_bbox_points(tile_id,params,nb_dat);
            }
            else if(params.algo_step == std::string("extract_tile_vrt"))
            {
                rv = extract_tile_vrt(tile_id,params,nb_dat);
            }
            else if(params.algo_step == std::string("extract_tri_vrt"))
            {
                rv = extract_tri_vrt(tile_id,params,nb_dat);
            }
            else if(params.algo_step == std::string("extract_tri_json"))
            {
                rv = extract_tri_json(tile_id,params,nb_dat);
            }
            else if(params.algo_step == std::string("get_neighbors"))
            {
                ddt::logging log("get neighbors ", params.log_level);
                std::map<Id, std::vector<Point_id_id>>  outbox;
                log.step("get it");
                rv = get_neighbors(tile_id,params,outbox);
                std::cout.clear();
                log.step("send it");
                rv = send_neighbors(tile_id,params,outbox);
            }
            else if(params.algo_step == std::string("validity_check"))
            {
                //validity_check(tile_id,params,outbox);
            }
            else
            {
                std::cerr << "no params" << std::endl;
                return 1;
            }
            if(rv != 0) return rv;
            //std::cerr << "     [ERR LOG] <=== " << tile_id << "_" << params.algo_step << std::endl;
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
