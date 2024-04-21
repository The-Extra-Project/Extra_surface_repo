typedef unsigned char Id;
typedef unsigned char Flag;
typedef unsigned char FlagC;

#include "traits/traits.hpp"
typedef ddt::Traits<Id,Flag,FlagC> Traits;

typedef Traits::Random_points_in_box Random_points;

#include "partitioner/grid_partitioner.hpp"
#include "DDT.hpp"
#include "scheduler/scheduler.hpp"
typedef ddt::Tile<Traits> Tile;
typedef ddt::Scheduler<Tile> Scheduler;
typedef ddt::DDT<Traits> DDT;
#include "io/write_ply.hpp"
#include "io/write_vrt.hpp"
#include "io/logging.hpp"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int argc, char **argv)
{
    enum { D = Traits::D };
    int NP, threads, loglevel, NK, mode;
    std::vector<int> NT;
    std::string out, iter;
    double range;
    bool simplify;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("check", "check validity")
    ("mode,m", po::value<int>(&mode)->default_value(3), "mode")
    ("simplify,s", po::value<bool>(&simplify)->default_value(true), "simplify foreign stars")
    ("kdop,k", po::value<int>(&NK)->default_value(0), "number of extreme point pairs (0=bbox)")
    ("points,p", po::value<int>(&NP)->default_value(10000), "number of points")
    ("log,l", po::value<int>(&loglevel)->default_value(0), "log level")
    ("threads,j", po::value<int>(&threads)->default_value(0), "number of threads (0=all)")
    ("tiles,t", po::value<std::vector<int>>(&NT), "number of tiles")
    ("range,r", po::value<double>(&range)->default_value(1), "range")
    ("iter", po::value<std::string>(&iter), "prefix for iteration dumps (vrt)")
    ("out,o", po::value<std::string>(&out), "output directory")
    ;
    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if ( vm.count("help")  )
        {
            std::cout << "Distributed Delaunay Triangulation" << std::endl
                      << desc << std::endl;
            return 0;
        }
        po::notify(vm);
        if ( NT.size() > D )
        {
            std::cout << "Discarding excess tile grid dimension(s) : ";
            std::copy(NT.begin()+D, NT.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << std::endl;
            NT.resize(D);
        }
    }
    catch(po::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }
    ddt::Bbox<D> bbox(range);
    ddt::grid_partitioner<Traits> partitioner(bbox, NT.begin(), NT.end());
    Scheduler sch(threads);
    std::cout << "- Loglevel : " << loglevel << std::endl;
    std::cout << "- Range    : " << range << std::endl;
    std::cout << "- Points   : " << NP << std::endl;
    std::cout << "- KDOP     : " << NK << " (0=>bbox)" << std::endl;
    std::cout << "- Threads  : " << sch.number_of_threads() << std::endl;
    std::cout << "- Out dir  : " << (out.empty() ? "[no output]" : out) << std::endl;
    std::cout << "- Tiles    : " << partitioner.size() << " ( ";
    std::copy(partitioner.begin(), partitioner.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << ")" << std::endl;
    DDT tri;
    Random_points points(D, range);
    {
        ddt::logging log("Preprocess   ", loglevel);
        log.step("Generate Points");
        std::cout << tri.send_points(sch, points, NP, partitioner) << "\t";
    }
    {
        ddt::logging log("DDT          ", loglevel);
        log.step("Insert Points");
        std::cout << tri.insert(sch, false) << "\t";
        log.step("Send Extremes");
        if(NK == 0) std::cout << tri.send_all(sch,ddt::get_bbox_points()) << "\t";
        else std::cout << tri.send_all(sch,ddt::get_kdop_points(NK)) << "\t"; // , NK);
        /*
        log.step("Insert Points");
        std::cout << tri.insert(sch, false) << "\t";

        */
        int count = 1;
        for(int i=0; count; ++i)
        {
            std::cout << "iter :" << i << std::endl;
            if (vm.count("iter"))
            {
                tri.for_each(sch, [&iter,i](const Tile& t)
                {
                    std::ostringstream oss;
                    oss << iter << i << "_" << size_t(t.id()) << "_c.vrt";
                    ddt::write_tile_vrt_cells(t, oss.str());
                });
                tri.for_each(sch, [&iter,i](const Tile& t)
                {
                    std::ostringstream oss;
                    oss <<  iter << i << "_" << size_t(t.id()) << "_v.vrt";
                    ddt::write_tile_vrt_vertices(t, oss.str());
                });
            }
            log.step("Send   Neighb");
            switch(mode)
            {
            case 0:
                count = tri.splay<ddt::get_neighbors>(sch, simplify);
                break;
            case 1:
                count = tri.insert_splay(sch, simplify);
                break;
            case 2:
                count = tri.insert_simplified<ddt::get_neighbors_vertices_v1>(sch, simplify);
                break;
            case 3:
                count = tri.insert_simplified<ddt::get_neighbors_vertices_v2>(sch, simplify);
                break;
            case 4:
                count = tri.insert_simplified<ddt::get_neighbors_vertices_v3>(sch, simplify);
                break;
            default:
                exit(-1);
            }
            std::cout << count << "\t";
        }
    }
    {
        ddt::logging log("Post process ", loglevel);
        log.step("Splay  Neighb");
        std::cout << tri.splay_rec2<ddt::get_neighbors>(sch) << "\t";
        log.step("Finalize  V  ");
        tri.finalize_vertices(sch);
        std::cout << tri.number_of_vertices() << "\t";
        log.step("Finalize  F  ");
        tri.finalize_facets(sch);
        std::cout << tri.number_of_facets() << "\t";
        log.step("Finalize  C  ");
        tri.finalize_cells(sch);
        std::cout << tri.number_of_cells() << "\t";
        log.step("Sanity check ");
        bool sanity = tri.sanity_check(sch);
        std::cout << sanity << "\t";
        log.step("Simplify     ");
        std::cout << sch.transform_sum(tri.tiles_begin(), tri.tiles_end(), std::mem_fn(&Tile::simplify)) << "\t";
    }
    if ( vm.count("out")  )
    {
        ddt::logging log("Writing      ", loglevel);
        ddt::write_ply(tri, out+".ply");
        ddt::write_vrt_cell(tri, out+"_c.vrt");
        ddt::write_vrt_vert(tri, out+"_v.vrt");
    }
    if ( vm.count("check")  )
    {
        ddt::logging log("Validity     ", loglevel);
        std::cout << "Validity     \t" << (tri.is_valid(sch) ? "OK" : "ERROR!") << std::endl;
    }
    return 0;
}
