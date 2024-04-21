#include <CGAL/Simple_cartesian.h>
#include <CGAL/IO/read_las_points.h>
#include <CGAL/IO/write_las_points.h>
#include <CGAL/IO/write_ply_points.h>
#include <CGAL/jet_estimate_normals.h>


#include <scanline_orient_normals_ori.hpp>
#include <typeinfo>

#include <iostream>
#include <string>
#include <filesystem>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>


#include <iostream>
#include <fstream>
#include <filesystem>

using Kernel = CGAL::Simple_cartesian<double>;
using Point_3 = Kernel::Point_3;
using Vector_3 = Kernel::Vector_3;
using Point_with_info = std::tuple<Point_3, Vector_3, float, unsigned char>;
using Point_with_info_2 = std::tuple<Point_3, Vector_3, Vector_3>;
using Point_map = CGAL::Nth_of_tuple_property_map<0, Point_with_info>;
using Normal_map = CGAL::Nth_of_tuple_property_map<1, Point_with_info>;
using Scan_angle_map = CGAL::Nth_of_tuple_property_map<2, Point_with_info>;
using Scanline_id_map = CGAL::Nth_of_tuple_property_map<3, Point_with_info>;
using Point_map_2 = CGAL::Nth_of_tuple_property_map<0, Point_with_info_2>;
using Normal_map_2 = CGAL::Nth_of_tuple_property_map<1, Point_with_info_2>;
using Ori_map = CGAL::Nth_of_tuple_property_map<2, Point_with_info_2>;

void dump_2 (const char* filename, const std::vector<Point_with_info_2>& points)
{
    std::ofstream ofile (filename, std::ios::binary);
    CGAL::IO::set_binary_mode(ofile);
    CGAL::IO::write_PLY_with_properties
    (ofile, points,
     CGAL::make_ply_point_writer (Point_map_2()),
     std::make_tuple(Normal_map_2(),
                     CGAL::IO::PLY_property<double>("nx"),
                     CGAL::IO::PLY_property<double>("ny"),
                     CGAL::IO::PLY_property<double>("nz")),
     std::make_tuple(Ori_map(),
                     CGAL::IO::PLY_property<double>("x_origin"),
                     CGAL::IO::PLY_property<double>("y_origin"),
                     CGAL::IO::PLY_property<double>("z_origin")));

}


std::string extractDirectoryPath(const std::string& filePath)
{
    size_t found = filePath.find_last_of("/\\"); // Find the last occurrence of '/' or '\'
    if (found != std::string::npos)
    {
        return filePath.substr(0, found);
    }
    return ""; // If no directory separator found, return an empty string
}



void dump_xml(CGAL::Bbox_3 & bbox, int nbp, Point_3 & cc, std::string fname)
{
    // Create XML writer context
    xmlTextWriterPtr writer = xmlNewTextWriterFilename(fname.c_str(), 0);

    xmlTextWriterSetIndent(writer, 1);
    xmlTextWriterSetIndentString(writer, BAD_CAST "  ");

    //    xmlTextWriterPtr writer = xmlNewTextWriterToStdout();
    if (writer == NULL)
    {
        fprintf(stderr, "Error creating the xml writer\n");
        return 1;
    }

    // Start the document with the root element
    if (xmlTextWriterStartDocument(writer, NULL, NULL, NULL) < 0)
    {
        fprintf(stderr, "Error at xmlTextWriterStartDocument\n");
        return 1;
    }

    // Start the root element <env>
    if (xmlTextWriterStartElement(writer, BAD_CAST "env") < 0)
    {
        fprintf(stderr, "Error at xmlTextWriterStartElement\n");
        return 1;
    }

    // Start <datasets> element
    if (xmlTextWriterStartElement(writer, BAD_CAST "datasets") < 0)
    {
        fprintf(stderr, "Error at xmlTextWriterStartElement\n");
        return 1;
    }

    // Start <austin> element
    if (xmlTextWriterStartElement(writer, BAD_CAST "austin") < 0)
    {
        fprintf(stderr, "Error at xmlTextWriterStartElement\n");
        return 1;
    }

    // Write elements and their values
    //std::string bbstring = std::to_string(bbox.xmin())+"x"+std::to_string(bbox.xmax()) + ":"+ std::to_string(bbox.xmin())+"x"+std::to_string(bbox.ymax()) + ":" + std::to_string(bbox.zmin())+"x"+std::to_string(bbox.zmax());
    std::string bbstring = std::to_string(bbox.xmin())+"x"+std::to_string(bbox.xmax()) + ":"+ std::to_string(bbox.xmin())+"x"+std::to_string(bbox.ymax()) + ":0x100000";
    std::string shift=std::to_string(cc[0]) + "x" + std::to_string(cc[1]) + "x" + std::to_string(cc[2]);
    xmlTextWriterWriteElement(writer, BAD_CAST "plot_lvl", BAD_CAST "1");
    xmlTextWriterWriteElement(writer, BAD_CAST "do_stats", BAD_CAST "false");
    xmlTextWriterWriteElement(writer, BAD_CAST "datatype", BAD_CAST "files");
    xmlTextWriterWriteElement(writer, BAD_CAST "dim", BAD_CAST "3");
    xmlTextWriterWriteElement(writer, BAD_CAST "ndtree_depth", BAD_CAST "7");
    xmlTextWriterWriteElement(writer, BAD_CAST "bbox", BAD_CAST bbstring.c_str());
    xmlTextWriterWriteElement(writer, BAD_CAST "shift", BAD_CAST shift.c_str());
    xmlTextWriterWriteElement(writer, BAD_CAST "max_ppt", BAD_CAST "500000");
    xmlTextWriterWriteElement(writer, BAD_CAST "mode", BAD_CAST "1");
    xmlTextWriterWriteElement(writer, BAD_CAST "pscale", BAD_CAST "0.05");
    xmlTextWriterWriteElement(writer, BAD_CAST "nb_samples", BAD_CAST "50");
    xmlTextWriterWriteElement(writer, BAD_CAST "algo_opt", BAD_CAST "seg_lagrange_weight");
    xmlTextWriterWriteElement(writer, BAD_CAST "lambda", BAD_CAST "2");
    xmlTextWriterWriteElement(writer, BAD_CAST "coef_mult", BAD_CAST "5");
    xmlTextWriterWriteElement(writer, BAD_CAST "max_opt_it", BAD_CAST "50");
    xmlTextWriterWriteElement(writer, BAD_CAST "StorageLevel", BAD_CAST "DISK_ONLY");
    xmlTextWriterWriteElement(writer, BAD_CAST "do_process", BAD_CAST "true");
    xmlTextWriterWriteElement(writer, BAD_CAST "do_expand", BAD_CAST "false");
    xmlTextWriterWriteElement(writer, BAD_CAST "dump_debug", BAD_CAST "false");

    // End <austin> element
    if (xmlTextWriterEndElement(writer) < 0)
    {
        fprintf(stderr, "Error at xmlTextWriterEndElement\n");
        return 1;
    }

    // End <datasets> element
    if (xmlTextWriterEndElement(writer) < 0)
    {
        fprintf(stderr, "Error at xmlTextWriterEndElement\n");
        return 1;
    }

    // End <env> element
    if (xmlTextWriterEndElement(writer) < 0)
    {
        fprintf(stderr, "Error at xmlTextWriterEndElement\n");
        return 1;
    }

    // End the document
    if (xmlTextWriterEndDocument(writer) < 0)
    {
        fprintf(stderr, "Error at xmlTextWriterEndDocument\n");
        return 1;
    }

    // Free the writer context
    xmlFreeTextWriter(writer);

    printf("XML file written successfully.\n");

    return 0;

}

int main (int argc, char** argv)
{

    std::string fname = argv[1];
    std::string oname = argv[2];

    std::string xml_fname = extractDirectoryPath(oname) + std::string("/wasure_metadata.xml");

    std::cout << fname << std::endl;
    std::cout << oname << std::endl;
    std::cout << xml_fname << std::endl;

    std::vector<Point_with_info> points;
    std::vector<Point_with_info_2> points_2;
    //    std::vector<Vector_3> lines_of_sight;
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
    (points, 100,
     CGAL::parameters::point_map (Point_map()).
     normal_map (Normal_map()));
    std::cerr << "Orienting normals using scan angle and direction flag" << std::endl;
    CGAL::scanline_orient_normals
    (points,
     CGAL::parameters::point_map (Point_map()).
     normal_map (Normal_map()).
     scan_angle_map (Scan_angle_map()).
     scanline_id_map (Scanline_id_map()));

    double alpha = 200.0;
    CGAL::Bbox_3 bbox;
    CGAL::Bbox_3 bbox2;

    for (auto pp : points)
    {
        auto vx = std::get<0>(pp)[0];
        auto vy = std::get<0>(pp)[1];
        auto vz = std::get<0>(pp)[2];
        bbox = bbox +  CGAL::Bbox_3(vx,vy,vz,
                                    vx,vy,vz);
    }
    // auto bbox_center = Point_3((bbox.xmin() + bbox.xmax()) / 2.0,
    //                (bbox.ymin() + bbox.ymax()) / 2.0,
    //                0);
    auto bbox_center = Point_3(0,0,0);
    int acc = 0;
    for (auto pp : points)
    {
        auto lx = std::get<1>(pp)[0];
        auto ly = std::get<1>(pp)[1];
        auto lz = std::get<1>(pp)[2];

        Kernel::Vector_3 ori(
            std::get<0>(pp)[0] + alpha*lx - bbox_center[0],
            std::get<0>(pp)[1] + alpha*ly - bbox_center[1],
            std::get<0>(pp)[2] + alpha*lz - bbox_center[2]
        );
        Kernel::Point_3 pp_new(
            std::get<0>(pp)[0] - bbox_center[0],
            std::get<0>(pp)[1] - bbox_center[1],
            std::get<0>(pp)[2] - bbox_center[2]
        );


        points_2.push_back(std::make_tuple(pp_new,std::get<1>(pp),ori));

        double vx = pp_new[0];
        double vy = pp_new[1];
        double vz = pp_new[2];
        bbox2 = bbox2 +  CGAL::Bbox_3(vx,vy,vz,
                                      vx,vy,vz);
        acc++;

    }
    dump_2(oname.c_str(), points_2);
    dump_xml(bbox2,points.size(),bbox_center,xml_fname);

    return EXIT_SUCCESS;
}

