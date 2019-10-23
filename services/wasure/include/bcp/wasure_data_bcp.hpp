#ifndef DATA_H
#define DATA_H

#include <map>
#include "tinyply.h"




using namespace tinyply;



template<typename Traits>
class wasure_data
{
public :
    typedef double d_type;
    typedef typename Traits::Point                                    Point;


    wasure_data()
    {



    }


    void write_geojson_point_stream(std::ostream & ofs, bool is_first = true)
    {
        bool do_ray = false;
        int D = Traits::D;
        std::vector<std::string> lab_color = {"\"red\"","\"green\"","\"blue\""};
        ofs << "{" << std::endl;
        ofs << "\"type\": \"FeatureCollection\"," << std::endl;
        ofs << "\"features\": [" << std::endl;
        std::cerr << "size:" << labs.size() << " " << raw_points.size() << std::endl;
        for(int id = 0; id < nb_pts(); id++)
        {
            int id_pts = id*D;
            int id_sigs = id*D;
            int id_norms = id*D*D;
            if(!is_first)
                ofs << "," << std::endl;
            is_first=false;

            // Points
            ofs << "{" << std::endl;
            ofs << "\"type\": \"Feature\"," << std::endl;
            ofs << "\"geometry\": {" << std::endl;
            ofs << "\"type\": \"Point\"," << std::endl;
            ofs << "\"coordinates\": [";
            for(int d=0; d<D-1; ++d)
                ofs << raw_points[id_pts +d] << ",";
            ofs << raw_points[id_pts + D-1] << "]" << std::endl;
            ofs << "}," << std::endl;
            ofs << "\"properties\": {" << std::endl;
            if(labs.size() > 0)
            {
                ofs << "\"lab\":" << labs[id] <<  "," << std::endl;
                ofs << "\"marker-color\":" << lab_color[labs[id]] <<  "," << std::endl;
            }
            ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
            ofs << "}" << std::endl;
            ofs << "}" << std::endl;


            // Center
            if(raw_centers.size() > 0)
            {
                ofs << "," << std::endl;
                ofs << "{" << std::endl;
                ofs << "\"type\": \"Feature\"," << std::endl;
                ofs << "\"geometry\": {" << std::endl;
                ofs << "\"type\": \"Point\"," << std::endl;
                ofs << "\"coordinates\": [";
                for(int d=0; d<D-1; ++d)
                    ofs << raw_centers[id_pts +d] << ",";
                ofs << raw_centers[id_pts + D-1] << "]" << std::endl;
                ofs << "}," << std::endl;
                ofs << "\"properties\": {" << std::endl;
                if(labs.size() > 0)
                {
                    ofs << "\"type\":\"point\"," << std::endl;
                    ofs << "\"lab\":" << labs[id] <<  "," << std::endl;
                    ofs << "\"marker-color\":" << lab_color[labs[id]] <<  "," << std::endl;
                }
                ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
                ofs << "}" << std::endl;
                ofs << "}" << std::endl;

                // Ray
                if(do_ray)
                {
                    ofs << "," << std::endl;
                    ofs << "{" << std::endl;
                    ofs << "\"type\": \"Feature\"," << std::endl;
                    ofs << "\"geometry\": {" << std::endl;
                    ofs << "\"type\": \"LineString\"," << std::endl;
                    ofs << "\"coordinates\": [[";
                    for(int d=0; d<D-1; ++d)
                        ofs << raw_centers[id_pts +d] << ",";
                    ofs << raw_centers[id_pts + D-1] << "]," << std::endl;
                    ofs << "[";
                    for(int d=0; d<D-1; ++d)
                        ofs << raw_points[id_pts +d] << ",";
                    ofs << raw_points[id_pts + D-1] << "]]" << std::endl;
                    ofs << "}," << std::endl;
                    ofs << "\"properties\": {" << std::endl;
                    if(raw_centers.size() > 0)
                    {
                        ofs << "\"type\":\"ray\"," << std::endl;
                    }
                    ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
                    ofs << "}" << std::endl;
                    ofs << "}" << std::endl;
                }
            }

            if(raw_norms.size() > 0)
            {
                ofs << "," << std::endl;
                ofs << "{" << std::endl;
                ofs << "\"type\": \"Feature\"," << std::endl;
                ofs << "\"geometry\": {" << std::endl;
                ofs << "\"type\": \"LineString\"," << std::endl;
                ofs << "\"coordinates\": [[";
                for(int d=0; d<D-1; ++d)
                    ofs << raw_points[id_pts +d] << ",";
                ofs << raw_points[id_pts + D-1] << "]," << std::endl;
                ofs << "[";
                for(int d=0; d<D-1; ++d)
                    ofs << (raw_points[id_pts +d] + (raw_sigs[id_sigs+ D-1]/3.0)*raw_norms[id_norms+D+d])  << ",";
                ofs << (raw_points[id_pts + D-1] + (raw_sigs[id_sigs+ D-1]/3.0)*raw_norms[id_norms+D+D-1]) << "]]" << std::endl;
                ofs << "}," << std::endl;
                ofs << "\"properties\": {" << std::endl;
                if(raw_norms.size() > 0)
                {
                    ofs << "\"type\":\"norms\"," << std::endl;
                }
                ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
                ofs << "}" << std::endl;
                ofs << "}" << std::endl;
            }
        }
        ofs << "]" << std::endl;
        ofs << "}" << std::endl;


//    fo << points[i][0] + dims_norms[i][1][0]*dims_scales[i][1]/3.0 << " " << points[i][1] + dims_norms[i][1][1]*dims_scales[i][1]/3.0 ;
    }



    // template<typename Iterator>
    // void write_geojson_cell_range(Iterator begin, Iterator end, std::ostream & ofs,bool is_first = true)
    // {

    //   typedef typename Iterator::value_type Cell_const_iterator;
    //   typedef typename Cell_const_iterator::Traits Traits;
    //   std::map<Cell_const_iterator, int> cmap;
    //   int nextid = 0;
    //   int D = Traits::D;
    //   for(auto iit = begin; iit != end; ++iit)
    //     {
    // 	if(iit->is_infinite()) continue;
    // 	if(!is_first)
    // 	  ofs << "," << std::endl;
    // 	is_first=false;
    // 	ofs << "{" << std::endl;
    // 	ofs << "\"type\": \"Feature\"," << std::endl;
    // 	ofs << "\"geometry\": {" << std::endl;
    // 	ofs << "\"type\": \"Polygon\"," << std::endl;
    // 	ofs << "\"coordinates\": [" << std::endl;
    // 	int local = 0;
    // 	ofs << "[[";
    // 	for(int i=0; i<=D+1; ++i) // repeat first to close the polygon
    // 	  {
    // 	    auto v = iit->vertex(i % (D+1));
    // 	    if(i>0)
    // 	      {
    // 		ofs << "],[";
    // 		local += v->is_local();
    // 	      }
    // 	    auto p = v->point();
    // 	    for(int d=0; d<D-1; ++d) ofs << p[d] << ",";
    // 	    ofs << p[D-1];
    // 	  }
    // 	ofs << "]]";
    // 	ofs << "]";
    // 	ofs << "}," << std::endl;
    // 	ofs << "\"properties\": {" << std::endl;
    // 	switch(local)
    // 	  {
    // 	  case 0 :
    // 	    ofs << "\"fill\":\"red\"," << std::endl;
    // 	    break;
    // 	  case 1 :
    // 	    ofs << "\"fill\":\"green\"," << std::endl;
    // 	    break;
    // 	  case 2 :
    // 	    ofs << "\"fill\":\"blue\"," << std::endl;
    // 	    break;
    // 	  }
    // 	ofs << "\"stroke-width\":\"2\"," <<  std::endl;
    // 	ofs << "\"local\": " << local << "," << std::endl;
    // 	ofs << "\"tid\": " << int(iit->tile()->id())  << "," << std::endl;

    // 	if(true)
    // 	  {

    // 	    if(!cmap.count(*iit)) cmap[*iit] = nextid++;
    // 	    ofs << "\"id\": " << cmap[*iit] << "," << std::endl;
    // 	    for(int i = 0 ; i < D+1; i++)
    // 	      {
    // 		int iid = -1;
    // 		// Maybe neighbors does not exists if not loaded completly
    // 		try {
    // 		  auto nb0 = iit->neighbor(i);
    // 		  auto n0 = nb0->main();
    // 		  if(!cmap.count(n0)) cmap[n0] = nextid++;
    // 		  iid = cmap[n0];
    // 		} catch (...) {

    // 		}
    // 		ofs << "\"neigbhor " << i << "\": " << iid << "," << std::endl;
    // 	      }
    // 	  }
    // 	ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
    // 	ofs << "}" << std::endl;
    // 	ofs << "}" << std::endl;
    //     }
    // }


    void write_ply_stream( std::ostream & outputFile)
    {

        try
        {
            std::ostringstream outputStream;
            PlyFile myFile;

            std::vector<std::string>  vname(verts_name.begin(),verts_name.begin()+D);
            std::vector<std::string>  cname(centers_name.begin(),centers_name.begin()+D);
            std::vector<std::string>  lname(labs_name.begin(),labs_name.begin()+1);
            std::vector<std::string>  sname(sig_name.begin(),sig_name.begin()+D);
            std::vector<std::string>  egvname(egv_name[D].begin(),egv_name[D].end());

            if(raw_points.size() > 0)
                myFile.add_properties_to_element("vertex", vname, raw_points);
            if(raw_centers.size() > 0)
                myFile.add_properties_to_element("vertex", cname, raw_centers);
            if(labs.size() > 0)
                myFile.add_properties_to_element("vertex", lname, labs);
            if(raw_sigs.size() > 0)
                myFile.add_properties_to_element("vertex", sname, raw_sigs);
            if(raw_norms.size() > 0)
                myFile.add_properties_to_element("vertex", egvname, raw_norms);
            myFile.comments.push_back("[wasure_data]");
            myFile.write(outputStream, false);
            outputFile << outputStream.str();

        }
        catch (const std::exception & e)
        {
            std::cerr << "Caught exception: " << e.what() << std::endl;
        }
    }

    void read_ply_stream(std::istream & ss)
    {
        try
        {
            PlyFile file(ss);
            uint32_t vertexCount, centersCount, labsCount,sigCount,egvCount;

            std::vector<std::string>  vname(verts_name.begin(),verts_name.begin()+D);
            std::vector<std::string>  cname(centers_name.begin(),centers_name.begin()+D);
            std::vector<std::string>  lname(labs_name.begin(),labs_name.begin()+1);

            std::vector<std::string>  sname(sig_name.begin(),sig_name.begin()+D);
            std::vector<std::string>  egvname(egv_name[D].begin(),egv_name[D].end());

            vertexCount = centersCount = labsCount = 0;
            vertexCount = file.request_properties_from_element("vertex", vname, raw_points);
            centersCount = file.request_properties_from_element("vertex", cname, raw_centers);
            labsCount = file.request_properties_from_element("vertex", lname, labs);
            sigCount = file.request_properties_from_element("vertex", sname, raw_sigs );
            egvCount = file.request_properties_from_element("vertex", egvname, raw_norms );
            file.read(ss);

        }
        catch (const std::exception & e)
        {
            std::cerr << "Caught exception: " << e.what() << std::endl;
        }
    }






    void format_datas(bool do_clear = true)
    {
        double coords[Traits::D];
        for(int i = 0 ; i < raw_points.size(); i++)
        {
            coords[i%D] = raw_points[i];
            if(i%D == D-1)
            {
                format_points.push_back(traits.make_point(coords));
            }
        }
        if(do_clear)
            raw_points.clear();

        for(int i = 0 ; i < raw_centers.size(); i++)
        {
            coords[i%D] = raw_centers[i];
            if(i%D == D-1)
            {
                format_centers.push_back(traits.make_point(coords));
            }
        }
        if(do_clear)
            raw_centers.clear();


        std::vector<Point> act_vect;
        for(int i = 0 ; i < raw_norms.size(); i++)
        {
            coords[i%D] = raw_norms[i];
            if(i%D == D-1)
            {
                act_vect.push_back(traits.make_point(coords));
            }
            if(i % ((int)D*D) == D-1)
            {
                format_norms.push_back(act_vect);
                act_vect.clear();
            }
        }
        if(do_clear)
            raw_norms.clear();


        std::vector<double> act_vsig;
        for(int i = 0 ; i < raw_sigs.size(); i++)
        {
            act_vsig.push_back(raw_sigs[i]);
            if(i%D == D-1)
            {
                format_sigs.push_back(act_vsig);
                act_vsig.clear();
            }
        }
        if(do_clear)
            raw_sigs.clear();
    }


    void unformat_data(bool do_clear = true)
    {

        for(int i = 0 ; i < format_points.size(); i++)
        {
            for(int d = 0 ; d < D; d++)
                raw_points.push_back(format_points[i][d]);
        }
        if(do_clear)
            format_points.clear();


        for(int i = 0 ; i < format_centers.size(); i++)
        {
            for(int d = 0 ; d < D; d++)
                raw_centers.push_back(format_centers[i][d]);
        }
        if(do_clear)
            format_centers.clear();


        for(int i = 0 ; i < format_norms.size(); i++)
        {
            for(int d1 = 0 ; d1 < D; d1++)
            {
                for(int d2 = 0 ; d2 < D; d2++)
                {
                    raw_norms.push_back(format_norms[i][d1][d2]);
                }
            }
        }
        if(do_clear)
            format_norms.clear();



        for(int i = 0 ; i < format_sigs.size(); i++)
        {
            for(int d = 0 ; d < D; d++)
                raw_sigs.push_back(format_sigs[i][d]);
        }
        if(do_clear)
            raw_sigs.clear();
    }


    void insert_point(Point & p)
    {
        format_points.push_back(p);
    }


    void insert_raw_point(Point & p)
    {
        for(int d = 0 ; d < D; d++)
            raw_points.push_back(p[d]);
    }


    // void copy_point(wasure_data & wd, int id){
    //   for(int n = 0 ; n < nbe_points ; n++)
    //     raw_points[id*nbe_points + n] = wd
    // }

    Point get_pts(int id)
    {
        return format_points[id];
    }

    int nb_pts()
    {
        return std::max(raw_points.size()/D,format_points.size());
    }

    Traits traits;
    int D = Traits::D;
    std::vector<d_type> raw_points;
    std::vector<d_type> raw_centers;
    std::vector<d_type> raw_norms;
    std::vector<d_type> raw_sigs;
    std::vector<int> labs;

    std::vector<Point>  format_points;
    std::vector<Point>  format_centers;
    std::vector<std::vector<Point> >  format_norms;
    std::vector<std::vector<double>>  format_sigs ;

    int nbe_points = D;
    int nbe_centers = D;
    int nbe_norms = D;
    int nbe_sigs = D;
    int nbe_labs = 1;

    std::vector<std::string>  verts_name = {"x","y","z"};
    std::vector<std::string>  centers_name = {"x_origin","y_origin","z_origin"};

    std::vector<std::string> labs_name = {"lab"};
    std::vector<std::string> sig_name = {"sigma1","sigma2","sigma3"};
    std::vector<std::vector<std::string> > egv_name = {{"eigenVector1x"},
        {"eigenVector1x","eigenVector1y","eigenVector2x","eigenVector2y"},
        {
            "eigenVector1x","eigenVector1y","eigenVector1z",
            "eigenVector2x","eigenVector2y","eigenVector2z",
            "eigenVector3x","eigenVector3y","eigenVector3z"
        }
    };




};

#endif
