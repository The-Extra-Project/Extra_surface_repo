#include <CGAL/Simple_cartesian.h>
#include <CGAL/IO/read_las_points.h>
#include <CGAL/IO/write_ply_points.h>
#include <CGAL/jet_estimate_normals.h>
//#include <CGAL/scanline_orient_normals.h>
#include <scanline_orient_normals.hpp>

#define CGAL_SCANLINE_ORIENT_VERBOSE 1

using Kernel = CGAL::Simple_cartesian<double>;
using Point_3 = Kernel::Point_3;
using Vector_3 = Kernel::Vector_3;
using Point_with_info = std::tuple<Point_3, Vector_3, float, unsigned char>;
using Point_map = CGAL::Nth_of_tuple_property_map<0, Point_with_info>;
using Normal_map = CGAL::Nth_of_tuple_property_map<1, Point_with_info>;
using Scan_angle_map = CGAL::Nth_of_tuple_property_map<2, Point_with_info>;
using Scanline_id_map = CGAL::Nth_of_tuple_property_map<3, Point_with_info>;
void dump (const char* filename, const std::vector<Point_with_info>& points)
{
    std::ofstream ofile (filename, std::ios::binary);
    CGAL::IO::set_binary_mode(ofile);
    CGAL::IO::write_PLY
	(ofile, points,
	 CGAL::parameters::point_map (Point_map()).
	 normal_map (Normal_map()));
}
int main (int argc, char** argv)
{

    std::string fname = argv[1];
    std::string oname = argv[2];
    std::vector<Vector_3> lines_of_sight;  
    std::vector<Point_with_info> points;
    std::cerr << "Reading input file " << fname << std::endl;
    std::ifstream ifile (fname, std::ios::binary);
    if (!ifile ||
	!CGAL::IO::read_LAS_with_properties
	(ifile, std::back_inserter (points),
	 CGAL::IO::make_las_point_reader (Point_map()),
	 std::make_pair (Scan_angle_map(),
			 CGAL::IO::LAS_property::Scan_angle()),
	 std::make_pair (Scanline_id_map(),
			 CGAL::IO::LAS_property::Scan_direction_flag())))
	{
	    std::cerr << "Can't read " << fname << std::endl;
	    return EXIT_FAILURE;
	}


    std::cerr << "Estimating normals" << std::endl;
    CGAL::jet_estimate_normals<CGAL::Parallel_if_available_tag>
	(points, 50,
	 CGAL::parameters::point_map (Point_map()).
	 normal_map (Normal_map()));
    dump("/home/LCaraffa/code/spark-ddt/datas/lidar_hd/out_raw_normals.ply", points);
  
    std::cerr << "Orienting normals using scan angle and direction flag" << std::endl;
    CGAL::scanline_orient_normals
	(points,
	 lines_of_sight,
	 CGAL::parameters::point_map (Point_map()).
	 normal_map (Normal_map()).
	 scan_angle_map (Scan_angle_map()).
	 scanline_id_map (Scanline_id_map()));

    for (auto pp : points){
	break;
	auto vx = std::get<1>(pp)[0];
	auto vy = std::get<1>(pp)[1];
	auto vz = std::get<1>(pp)[2];
	std::cout << vx << " " << vy << " " << vz << " : ";
	std::cout << std::get<2>(pp) << " " << std::to_string(std::get<3>(pp))  << std::endl;
    }

  
    dump("/home/LCaraffa/code/spark-ddt/datas/lidar_hd/urban_fix.ply", points);
    std::cerr << "Orienting normals using scan direction flag only" << std::endl;
    CGAL::scanline_orient_normals
	(points,
	 lines_of_sight,
	 CGAL::parameters::point_map (Point_map()).
	 normal_map (Normal_map()).
	 scanline_id_map (Scanline_id_map()));
    dump("/home/LCaraffa/code/spark-ddt/datas/lidar_hd/out_flag.ply", points);
    std::cerr << "Orienting normals using scan angle only" << std::endl;
    CGAL::scanline_orient_normals
	(points,
	 lines_of_sight,
	 CGAL::parameters::point_map (Point_map()).
	 normal_map (Normal_map()).
	 scan_angle_map (Scan_angle_map()));
    dump("/home/LCaraffa/code/spark-ddt/datas/lidar_hd/out_angle.ply", points);
    std::cerr << "Orienting normals using no additional info" << std::endl;
    CGAL::scanline_orient_normals
	(points,
	 lines_of_sight,
	 CGAL::parameters::point_map (Point_map()).
	 normal_map (Normal_map()));
    dump("/home/LCaraffa/code/spark-ddt/datas/lidar_hd/out_nothing.ply", points);
    return EXIT_SUCCESS;
}
