#ifndef DATA_H
#define DATA_H

#include <map>
#include "tinyply.h"
#include <algorithm>

//#define PLY_CHAR '\n'
#define PLY_CHAR ';'
#define IS_BINARY false
#define DATA_FLOAT_TYPE tinyply::Type::FLOAT64
#define NB_DIGIT_OUT (5)


template<typename Traits>
class ddt_data
{
public :
    typedef typename Traits::Point                                    Point;
    typedef typename Traits::Delaunay_triangulation                                    Dt;
    typedef typename Traits::Vertex_const_handle                                    Vertex_const_handle;
    typedef typename Traits::Cell_const_handle                                    Cell_const_handle;
    typedef typename Traits::Cell_handle                                    Cell_handle;
    typedef typename Traits::Vertex_handle                                    Vertex_handle;




    class Data_ply
    {
    public:

        Data_ply(std::vector<std::string> vv, std::string pp, int dim, int vs, tinyply::Type tt) : vname(vv),part(pp),D(dim),vsize(vs),do_exist(false),type(tt) {};
        Data_ply() : do_exist(false) {};

        std::vector<std::string> get_name()
        {
            return vname;
        }


        std::string get_name(int ii, bool do_suffix = false)
        {
            if(vname.size() == 1 )
                return vname[0];
            return vname[0]  + "_" + std::to_string(ii);
        }


        bool is_init() const
        {
            return(output_vect.size() > 0);
        }

        int get_nbe_output()
        {
            return output_vect.size()/((tinyply::PropertyTable[type].stride)*vsize);
        }

        int get_nbe_input()
        {
            if(input_vect == nullptr)
                return 0;
            else
                return input_vect->buffer.size_bytes()/((tinyply::PropertyTable[type].stride)*vsize);
        }


        bool has_label(std::string v1) const
        {
            for(auto v2 : vname)
            {
                if( v1 == v2)
                    return true;
            }
            return false;
        }

        void print_elems()
        {
            int acc=0;
            std::cout << "==============" << std::endl;
            for(auto ee : vname)
            {
                std::cout << ee << "\t";
            }
            std::cout << std::endl;
            for(int i = 0; i < output_vect.size(); i++)
            {
                std::cout << output_vect[i] << "\t";
                if(i % vsize == vsize-1)
                    std::cout << std::endl;
            }
        }

        int get_vsize() const
        {
            return vsize;
        }
        int get_vnbb() const
        {
            return vsize*tinyply::PropertyTable[type].stride;
        }

        int size_bytes()
        {
            return input_vect->buffer.size_bytes();
        }



        void input2output(bool do_clean = true)
        {
            output_vect.resize(size_bytes());
            std::memcpy(output_vect.data(), input_vect->buffer.get(),size_bytes());
            if(do_clean)
                input_vect.reset();
        }


        Point extract_pts(int id)
        {
            return traits.make_point( reinterpret_cast< double * >(input_vect->buffer.get())+id*D);
        }

        template<typename DT>
        Point extract_vect(std::vector<DT> & formated_data,int id)
        {
            int vnbb =  get_vnbb();
            formated_data.resize(vsize);
            std::memcpy(formated_data.data(), (input_vect->buffer.get())+id*D,vnbb);
        }


        template<typename DT>
        void  extract_value( int id, DT & vv, int i=0) const
        {
            int vnbb =  get_vnbb();
            vv =  reinterpret_cast<DT &>(input_vect->buffer.get()[id*vnbb+i*tinyply::PropertyTable[type].stride]);
        }




        template<typename DT>
        void extract_full_input(std::vector<DT> & formated_data, bool do_clean = true)
        {
            int szd = get_nbe_input()*vsize;
            if(szd > 0)
            {
                formated_data.resize(szd);
                std::memcpy(formated_data.data(), input_vect->buffer.get(),size_bytes());
            }
            if(do_clean)
                input_vect.reset();
        }


        template<typename DT>
        void fill_full_output(std::vector<DT> & formated_data, bool do_clean = true)
        {
            int szb = sizeof(DT)*formated_data.size();
            output_vect.resize(szb);
            std::memcpy(output_vect.data(), formated_data.data(),szb);
            if(do_clean)
                formated_data.clear();
            do_exist = true;
        }


        void clean_input()
        {
            input_vect.reset();
        }

        void clean_output()
        {
            output_vect.clear();
        }

        void set_exist(bool bb)
        {
            do_exist = bb;
        }

        bool do_exist;
        std::string part;
        std::vector<std::string>  vname;
        std::shared_ptr<tinyply::PlyData> input_vect;
        std::vector<uint8_t> output_vect;
        tinyply::Type type;
    protected :
        int D,vsize;
    };


    std::vector<std::string> subvect(std::vector<std::string> vname, int dd)
    {
        return std::vector<std::string>(vname.begin(),vname.begin()+dd);
    }


    void init_map()
    {
        int D = Traits::D;
        dmap[xyz_name] = Data_ply(xyz_name,"vertex",D,D,tinyply::Type::INVALID);
        dmap[simplex_name] = Data_ply(simplex_name,"face",D+1,D+1,tinyply::Type::INT32);
        dmap[nb_name] = Data_ply(nb_name,"face",D+1,D+1,tinyply::Type::INT32);
    }



    void init_name()
    {
        int D = Traits::D;
        xyz_name = subvect({"x","y","z","t"},D);
        simplex_name = {"vertex_index"};
        vid_name = {"vid"};
        vtileid_name = {"vtid"};
        ctileid_name = {"ctid"};
        cid_name = {"cid"};
        flag_vertex_name = {"flag_v"};
        flag_simplex_name = {"flag_s"};
        nb_name = {"nb_indices"};
    }

    ddt_data<Traits>()
    {
        int D = Traits::D;
        init_name();
        init_map();
    }



    ddt_data<Traits>(std::map<std::vector<std::string>, Data_ply > & init_dmap)
    {
        init_name();
        for ( const auto &ee : init_dmap )
        {
            dmap[ee.first] =  Data_ply(ee.first,ee.second.part,D,ee.second.get_vsize(),ee.second.type);
        }
    }


    void write_geojson_stream(std::ostream & ofs, bool is_first = true)
    {


        int D = Traits::D;
        std::vector<std::string> lab_color = {"\"red\"","\"green\"","\"blue\""};
        ofs << "{" << std::endl;
        ofs << "\"type\": \"FeatureCollection\"," << std::endl;
        ofs << "\"features\": [" << std::endl;


        std::vector<double> v_xyz;
        std::vector<int> v_simplex;
        dmap[xyz_name].extract_full_input(v_xyz,false);
        dmap[simplex_name].extract_full_input(v_simplex,false);

        ofs << std::fixed << std::setprecision(12);
        for(int id = 0; id < nb_pts_input(); id++)
        {
            int id_pts = id*D;
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
                ofs << v_xyz[id_pts +d] << ",";
            ofs << v_xyz[id_pts + D-1] << "]" << std::endl;
            ofs << "}," << std::endl;
            ofs << "\"properties\": {" << std::endl;

            for ( const auto &ee : dmap )
            {
                if(dmap[ee.first].part == "vertex" && ee.second.do_exist)
                {
                    for(int nn = 0 ; nn < dmap[ee.first].get_vsize(); nn++)
                    {
                        if(dmap[ee.first].type == tinyply::Type::INT32)
                        {
                            int vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                        else  if(dmap[ee.first].type == tinyply::Type::UINT32)
                        {
                            uint vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                        else  if(dmap[ee.first].type == DATA_FLOAT_TYPE)
                        {
                            double vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                    }
                }
            }

            ofs << "\"datatype\":\"point\"," << std::endl;
            ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
            ofs << "}" << std::endl;
            ofs << "}" << std::endl;
        }

        uint num_c = dmap[simplex_name].get_nbe_input();///(D+1);
        for(int id = 0; id < num_c; id++)
        {
            int local = 0;
            bool is_infinite = false;
            for(int i=0; i<=D+1; ++i)
            {
                if(v_simplex[id*(D+1)+(i%(D+1))] == 0)
                    is_infinite = true;
            }
            if(is_infinite)
                continue;
            ofs << "," << std::endl;
            ofs << "{" << std::endl;
            ofs << "\"type\": \"Feature\"," << std::endl;
            ofs << "\"geometry\": {" << std::endl;
            ofs << "\"type\": \"Polygon\"," << std::endl;
            ofs << "\"coordinates\": [" << std::endl;
            ofs << "[[";
            for(int i=0; i<=D+1; ++i) // repeat first to close the polygon
            {
                if(i>0)
                {
                    ofs << "],[";
                }


                int id_pp = v_simplex[id*(D+1)+(i%(D+1))];

                for(int d=0; d<D-1; ++d) ofs << v_xyz[id_pp*D + d] << ",";
                ofs << v_xyz[id_pp*D + D-1];
            }
            ofs << "]]";
            ofs << "]";
            ofs << "}," << std::endl;
            ofs << "\"properties\": {" << std::endl;
            switch(local)
            {
            case 0 :
                ofs << "\"fill\":\"red\"," << std::endl;
                break;
            case 1 :
                ofs << "\"fill\":\"green\"," << std::endl;
                break;
            case 2 :
                ofs << "\"fill\":\"blue\"," << std::endl;
                break;
            }


            for ( const auto &ee : dmap )
            {
                if(dmap[ee.first].part == "face" && ee.second.do_exist)
                {
                    for(int nn = 0 ; nn < dmap[ee.first].get_vsize(); nn++)
                    {
                        if(dmap[ee.first].type == tinyply::Type::INT32)
                        {
                            int vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                        else  if(dmap[ee.first].type == tinyply::Type::UINT32)
                        {
                            uint vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                        else  if(dmap[ee.first].type == DATA_FLOAT_TYPE)
                        {
                            double vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                    }
                }
            }

            ofs << "\"stroke-width\":\"2\"," <<  std::endl;
            //	iit->data().dump_geojson(ofs);
            ofs << "\"prop1\": { \"this\": \"that\" }" << std::endl;
            ofs << "}" << std::endl;
            ofs << "}" << std::endl;

        }


        ofs << "]" << std::endl;
        ofs << "}" << std::endl;

    }



    void write_geojson_header(std::ostream & ss)
    {
        ss << "{" << std::endl;
        ss << "\"type\": \"FeatureCollection\"," << std::endl;
        ss << "\"features\": [" << std::endl;
    }

    void write_geojson_footer(std::ostream & ss)
    {
        ss << "]" << std::endl;
        ss << "}" << std::endl;
    }




    void write_geojson_tri(std::ostream & ofs_pts,std::ostream & ofs_spx, bool is_full = true)
    {
        ofs_spx << std::fixed << std::setprecision(12);
        int D = Traits::D;
        std::vector<std::string> lab_color = {"\"red\"","\"green\"","\"blue\""};
        bool is_first = is_full;

        if(is_full)
        {
            write_geojson_header(ofs_pts);
            write_geojson_header(ofs_spx);
        }


        std::vector<double> v_xyz;
        std::vector<int> v_simplex;
        dmap[xyz_name].extract_full_input(v_xyz,false);
        dmap[simplex_name].extract_full_input(v_simplex,false);


        for(int id = 0; id < nb_pts_input(); id++)
        {
            int id_pts = id*D;
            if(!is_first)
                ofs_pts << "," << std::endl;
            is_first=false;

            // Points
            ofs_pts << "{" << std::endl;
            ofs_pts << "\"type\": \"Feature\"," << std::endl;
            ofs_pts << "\"geometry\": {" << std::endl;
            ofs_pts << "\"type\": \"Point\"," << std::endl;
            ofs_pts << "\"coordinates\": [";
            for(int d=0; d<D-1; ++d)
                ofs_pts << v_xyz[id_pts +d] << ",";
            ofs_pts << v_xyz[id_pts + D-1] << "]" << std::endl;
            ofs_pts << "}," << std::endl;
            ofs_pts << "\"properties\": {" << std::endl;

            for ( const auto &ee : dmap )
            {
                if(dmap[ee.first].part == "vertex" && ee.second.do_exist)
                {
                    for(int nn = 0 ; nn < dmap[ee.first].get_vsize(); nn++)
                    {
                        if(dmap[ee.first].type == tinyply::Type::INT32)
                        {
                            int vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs_pts << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                        else  if(dmap[ee.first].type == tinyply::Type::UINT32)
                        {
                            uint vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs_pts << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                        else  if(dmap[ee.first].type == DATA_FLOAT_TYPE)
                        {
                            double vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs_pts << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                    }
                }
            }


            ofs_pts << "\"datatype\":\"point\"," << std::endl;
            ofs_pts << "\"prop1\": { \"this\": \"that\" }" << std::endl;
            ofs_pts << "}" << std::endl;
            ofs_pts << "}" << std::endl;
        }

        is_first=true;
        uint num_c = dmap[simplex_name].get_nbe_input();///(D+1);
        for(int id = 0; id < num_c; id++)
        {
            int local = 0;
            // if(id > 3)
            // 	break;
            bool is_infinite = false;
            for(int i=0; i<=D+1; ++i)
            {
                if(v_simplex[id*(D+1)+(i%(D+1))] == 0)
                    is_infinite = true;
            }
            if(is_infinite)
                continue;

            if(!is_first)
                ofs_spx << "," << std::endl;
            is_first = false;
            ofs_spx << "{" << std::endl;
            ofs_spx << "\"type\": \"Feature\"," << std::endl;
            ofs_spx << "\"geometry\": {" << std::endl;
            ofs_spx << "\"type\": \"Polygon\"," << std::endl;
            ofs_spx << "\"coordinates\": [" << std::endl;
            ofs_spx << "[[";
            for(int i=0; i<=D+1; ++i) // repeat first to close the polygon
            {
                if(i>0)
                {
                    ofs_spx << "],[";
                }


                int id_pp = v_simplex[id*(D+1)+(i%(D+1))];

                for(int d=0; d<D-1; ++d) ofs_spx << v_xyz[id_pp*D + d] << ",";
                ofs_spx << v_xyz[id_pp*D + D-1];
            }
            ofs_spx << "]]";
            ofs_spx << "]";
            ofs_spx << "}," << std::endl;
            ofs_spx << "\"properties\": {" << std::endl;
            switch(local)
            {
            case 0 :
                ofs_spx << "\"fill\":\"red\"," << std::endl;
                break;
            case 1 :
                ofs_spx << "\"fill\":\"green\"," << std::endl;
                break;
            case 2 :
                ofs_spx << "\"fill\":\"blue\"," << std::endl;
                break;
            }

            for ( const auto &ee : dmap )
            {
                if(dmap[ee.first].part == "face" && ee.second.do_exist)
                {
                    for(int nn = 0 ; nn < dmap[ee.first].get_vsize(); nn++)
                    {
                        if(dmap[ee.first].type == tinyply::Type::INT32)
                        {
                            int vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs_spx << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                        else  if(dmap[ee.first].type == tinyply::Type::UINT32)
                        {
                            uint vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs_spx << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                        else  if(dmap[ee.first].type == DATA_FLOAT_TYPE)
                        {
                            double vv;
                            dmap[ee.first].extract_value(id,vv,nn);
                            ofs_spx << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
                        }
                    }
                }
            }

            ofs_spx << "\"stroke-width\":\"2\"," <<  std::endl;
            //	iit->data().dump_geojson(ofs_spx);
            ofs_spx << "\"prop1\": { \"this\": \"that\" }" << std::endl;
            ofs_spx << "}" << std::endl;
            ofs_spx << "}" << std::endl;

        }

        if(is_full)
        {
            write_geojson_footer(ofs_pts);
            write_geojson_footer(ofs_spx);
        }
    }





    void write_ply_stream( std::ostream & ss,char nl_char = '\n',bool is_binary = false,bool do_elem_newline = false)
    {
        try
        {

            ss << std::fixed << std::setprecision(NB_DIGIT_OUT);
            //	ss << std::scientific << std::endl;
            tinyply::PlyFile file_out;
            //std::cerr << "db 1" << std::endl;
            for ( const auto &ee : dmap )
            {
                //std::cerr << "db 2 : "  << std::endl;
                if(dmap[ee.first].part == "vertex")
                {
                    if(ee.second.do_exist)
                    {
                        if(dmap[ee.first].get_nbe_output() == 0 &&
                                dmap[ee.first].get_nbe_input() != 0)
                        {
                            dmap[ee.first].input2output();
                        }
                        int nbe = dmap[ee.first].get_nbe_output();
                        uint8_t * vv = dmap[ee.first].output_vect.data();
                        if(nbe > 0)
                        {
                            file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name(),
                                                               dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::INVALID, 0);

                        }
                    }
                }
            }
            for ( const auto &ee : dmap )
            {
                if(dmap[ee.first].part != "vertex")
                {
                    //std::cerr << "db 3 : " << std::endl;
                    if(ee.second.do_exist)
                    {
                        if(dmap[ee.first].get_nbe_output() == 0 &&
                                dmap[ee.first].get_nbe_input() != 0)
                        {
                            dmap[ee.first].input2output();
                        }
                        int nbe = dmap[ee.first].get_nbe_output();
                        uint8_t * vv = dmap[ee.first].output_vect.data();
                        if(nbe > 0)
                        {
                            if(dmap[ee.first].part == "face")
                            {
                                if(ee.first[0] == simplex_name[0] || ee.first[0] == nb_name[0])
                                    file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name(),
                                                                       dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::UINT8, dmap[ee.first].get_vsize());
                                else
                                    file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name(),
                                                                       dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::INVALID, 0);
                            }

                        }
                    }
                }
            }
            //std::cerr << "db 4 : " << std::endl;
            file_out.write(ss,is_binary,nl_char,do_elem_newline);
            //std::cerr << "db 5 : " << std::endl;

        }
        catch (const std::exception & e)
        {
            std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
        }
    }


    void remove_infinite()
    {


    }

    void write_dataset_stream( std::ostream & ss,char nl_char,int tid)
    {



        std::vector<int> raw_ids_vertex(std::max(nb_pts_output(),nb_pts_input()),tid);
        std::vector<int> raw_ids_simplex(std::max(nb_simplex_output(),nb_simplex_input()),tid);

        std::cerr << "raw_ids_vertex.size() => " << raw_ids_vertex.size() << std::endl;

        dmap[vtileid_name] = Data_ply(vtileid_name,"vertex",1,1,tinyply::Type::INT32);
        dmap[ctileid_name] = Data_ply(ctileid_name,"face",1,1,tinyply::Type::INT32);
        dmap[vtileid_name].fill_full_output(raw_ids_vertex);
        dmap[ctileid_name].fill_full_output(raw_ids_simplex);
        dmap[vtileid_name].do_exist = true;
        dmap[ctileid_name].do_exist = true;

        try
        {
            ss << std::fixed << std::setprecision(12);
            //	ss << std::scientific << std::endl;
            tinyply::PlyFile file_out;
            //std::cerr << "db 1" << std::endl;
            for ( const auto &ee : dmap )
            {
                //std::cerr << "db 2 : "  << std::endl;
                if(dmap[ee.first].part == "vertex")
                {
                    if(ee.second.do_exist)
                    {
                        if(dmap[ee.first].get_nbe_output() == 0 &&
                                dmap[ee.first].get_nbe_input() != 0)
                        {
                            dmap[ee.first].input2output();
                        }
                        int nbe = dmap[ee.first].get_nbe_output();
                        uint8_t * vv = dmap[ee.first].output_vect.data();
                        if(nbe > 0)
                        {
                            file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name(),
                                                               dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::INVALID, 0);

                        }
                    }
                }
            }
            for ( const auto &ee : dmap )
            {
                if(dmap[ee.first].part != "vertex")
                {
                    //std::cerr << "db 3 : " << std::endl;
                    if(ee.second.do_exist)
                    {
                        if(dmap[ee.first].get_nbe_output() == 0 &&
                                dmap[ee.first].get_nbe_input() != 0)
                        {
                            dmap[ee.first].input2output();
                        }
                        int nbe = dmap[ee.first].get_nbe_output();
                        uint8_t * vv = dmap[ee.first].output_vect.data();
                        if(nbe > 0)
                        {
                            if(dmap[ee.first].part == "face")
                            {
                                if(ee.first[0] == simplex_name[0] || ee.first[0] == nb_name[0])
                                    file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name(),
                                                                       dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::UINT8, dmap[ee.first].get_vsize());
                                else
                                    file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name(),
                                                                       dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::INVALID, 0);
                            }

                        }
                    }
                }
            }
            //std::cerr << "db 4 : " << std::endl;
            file_out.write(ss,false,nl_char,true);
            //std::cerr << "db 5 : " << std::endl;

        }
        catch (const std::exception & e)
        {
            std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
        }
    }

    void read_ply_stream(std::istream & ss,char nl_char = '\n')
    {
        try
        {
            tinyply::PlyFile file;
            file.parse_header(ss,nl_char);

            int vsize = 0;
            //for (auto c : file.get_comments()) std::cerr << "Comment: " << c << std::endl;
            for (auto e : file.get_elements())
            {
                if(true)
                {
                    vsize = e.size;

                    std::cerr << "element - " << e.name << " (" << e.size << ")" << std::endl;
                    for (auto p : e.properties)
                    {
                        std::vector<std::string> pname({p.name});
                        bool do_exist = false;

                        // if(tinyply::PropertyTable[p.propertyType].str != "double")
                        //   continue;

                        // for(auto vp : pname)
                        //   std::cerr << "\tproperty - " << vp << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;

                        for ( const auto &ee : dmap )
                        {
                            if(ee.second.has_label(p.name))
                            {
                                // std::cerr << p.name << " existe!" << std::endl;
                                // std::cerr << "[" << ee.first.size() << "] => ";
                                // for(auto ie : ee.first)
                                //   std::cerr << ie << " - ";
                                // std::cerr << std::endl;
                                do_exist = true;
                                dmap[ee.first].do_exist = true;
                                dmap[ee.first].type = p.propertyType;
                                // if(!dmap[ee.first].do_exist)
                                //   dmap[ee.first].init(vsize);
                                break;
                            }
                        }
                        if(!do_exist)
                        {
                            //std::cerr << p.name << " create!" << std::endl;
                            dmap[pname] = Data_ply(pname,e.name,D,1,p.propertyType);
                            dmap[pname].do_exist = true;
                            // if(!dmap[pname].do_exist)
                            //   dmap[pname].init(vsize);
                        }

                    }
                }
            }
            // std::cerr << "vsize:" << vsize << std::endl;
            // std::cerr << "........................................................................\n";


            //std::cerr << "file read" << std::endl;
            for ( const auto &ee : dmap )
            {
                if(ee.second.do_exist)
                {
                    //std::cerr << "[" << ee.first.size() << "] => ";
                    // for(auto ie : ee.first)
                    //   std::cerr << ie << " - ";
                    // std::cerr << std::endl;
                    try { dmap[ee.first].input_vect = file.request_properties_from_element(ee.second.part, dmap[ee.first].get_name() ); }
                    catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }
                }
            }
            //std::cerr << "reading done" << std::endl;
            file.read(ss);

            // std::cerr << "file convert" << std::endl;
            // for ( const auto &ee : dmap ) {
            //   if(ee.second.do_exist){
            //     dmap[ee.first].format(vsize);
            //   }
            //   //dmap[ee.first].print_elems();
            // }

        }
        catch (const std::exception & e)
        {
            std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
        }
    }


    // void read_tri(Dt & tri){
    //   std::cerr << "reading triangulation" << std::endl;
    //   std::vector<double> v_xyz;
    //   std::vector<int> v_simplex,v_nb,v_vid,v_cid,v_flagv,v_flags;
    //   dmap[xyz_name].extract_full_input(v_xyz,false);
    //   dmap[vid_name].extract_full_input(v_vid,false);
    //   dmap[cid_name].extract_full_input(v_cid,false);
    //   dmap[flag_vertex_name].extract_full_input(v_flagv,false);
    //   dmap[flag_simplex_name].extract_full_input(v_flags,false);
    //   dmap[simplex_name].extract_full_input(v_simplex,false);
    //   dmap[nb_name].extract_full_input(v_nb,false);

    //   std::cerr << "initialization done" << std::endl;

    //   tri.set_current_dimension(D);

    //   auto cit = tri.full_cells_begin();
    //   Cell_handle inf_ch = cit;
    //   tri.tds().delete_full_cell(inf_ch);

    //   // 4) read the number of vertices
    //   uint num_v = dmap[xyz_name].get_nbe_input();
    //   std::cerr << "num_v:" << num_v << std::endl;
    //   std::vector<Vertex_handle> vertex_map(num_v);
    //   vertex_map[0] = tri.infinite_vertex();

    //   // 5) read and create the vertices

    //   for(uint i = 1; i < num_v; ++i)
    //     {
    // 	int ii = i;
    // 	std::vector<double> coords_v(D);
    // 	for(uint d = 0; d < D; d++)
    // 	  {
    // 	    coords_v[d] = v_xyz[ii*D +d];
    // 	  }
    // 	Point p(D,coords_v.begin(),coords_v.end());
    // 	vertex_map[ii] = tri.new_vertex(p);
    // 	vertex_map[ii]->data().id = v_vid[ii];
    // 	vertex_map[ii]->data().flag = v_flagv[ii];
    //     }


    //   // 6) read the number of cells
    //   uint num_c = dmap[simplex_name].get_nbe_input();///(D+1);
    //   std::cerr << "num_c:" << num_c << std::endl;
    //   // 7) read and create the cells
    //   std::vector<Cell_handle> cell_map(num_c);
    //   uint ik;

    //   //std::cout << "number of cells to read.." << num_c << std::endl;

    //   for(uint i = 0; i < num_c; ++i)
    //     {
    // 	int ii = i;
    // 	Cell_handle ch = tri.new_full_cell();
    // 	for (uint d = 0; d < D+1; d++)
    // 	  {

    // 	    ik = v_simplex[i*(D+1)+d];
    // 	    ch->set_vertex(d, vertex_map[ik]);
    // 	    vertex_map[ik]->set_full_cell(ch);
    // 	  }

    // 	cell_map[ii] = ch;
    // 	ch->data().flag = v_flags[ii];
    // 	ch->data().id = ii;//v_cid[ii];
    //     }


    //   //std::cout << "number of cells created : " << tri.number_of_full_cells() << std::endl;
    //   // 8) read and construct neighbourhood relationships for cells
    //   for(uint j = 0; j < num_c; ++j)
    //     {
    // 	Cell_handle ch  = cell_map[j];
    // 	for(uint d = 0; d < D+1; d++)
    // 	  {
    // 	    ik = v_nb[j*(D+1)+d];
    // 	    ch->set_neighbor(d, cell_map[ik]);
    // 	  }
    //     }


    //   // compute the mirror indices
    //   for(uint j = 0; j < num_c; ++j)
    //     {
    // 	Cell_handle s  = cell_map[j];
    // 	for( uint j = 0; j <= D; ++j )
    // 	  {
    // 	    if( -1 != s->mirror_index(j) )
    // 	      continue;
    // 	    Cell_handle n = s->neighbor(j);
    // 	    int k = 0;
    // 	    Cell_handle nn = n->neighbor(k);
    // 	    while( s != nn )
    // 	      nn = n->neighbor(++k);
    // 	    s->set_mirror_index(j,k);
    // 	    n->set_mirror_index(k,j);
    // 	  }
    //     }

    // }



    tinyply::Type get_int_type()
    {
        return tinyply::Type::INT32;
    }

    tinyply::Type get_float_type()
    {
        return DATA_FLOAT_TYPE;
    }


    // void dump_tri(Dt & tri, bool do_init_id = false){
    //   dmap[xyz_name] = Data_ply(xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
    //   dmap[vid_name] = Data_ply(vid_name,"vertex",1,1,tinyply::Type::INT32);
    //   dmap[cid_name] = Data_ply(cid_name,"face",1,1,tinyply::Type::INT32);
    //   dmap[flag_vertex_name] = Data_ply(flag_vertex_name,"vertex",1,1,tinyply::Type::INT32);
    //   dmap[flag_simplex_name] = Data_ply(flag_simplex_name,"face",1,1,tinyply::Type::INT32);
    //   dmap[simplex_name] = Data_ply(simplex_name,"face",D+1,D+1,tinyply::Type::INT32);
    //   dmap[nb_name] = Data_ply(nb_name,"face",D+1,D+1,tinyply::Type::INT32);


    //   std::vector<double> v_xyz;
    //   std::vector<int> v_simplex,v_nb,v_vid,v_cid,v_flagv,v_flags;

    //   uint n = tri.number_of_vertices();
    //   std::map<Vertex_const_handle, uint> vertex_map;

    //   vertex_map[tri.infinite_vertex()] = 0;
    //   v_vid.push_back(0);
    //   v_flagv.push_back(0);
    //   for(int d = 0; d < D; d++)
    //     {
    // 	v_xyz.push_back(0.0);
    //     }
    //   uint i = 1;

    //   // Infinite hack
    //   // for(int d = 0; d < D; d++)
    //   //   {
    //   // 	v_xyz.push_back(0);
    //   //   }
    //   for(auto vit = tri.vertices_begin(); vit != tri.vertices_end(); ++vit)
    //     {
    // 	if(tri.is_infinite(vit))
    // 	  {
    // 	    // vertex_map[vit] = 0;
    // 	    // v_vid.push_back(0);
    // 	    // v_flagv.push_back(0);
    // 	    // for(int d = 0; d < D; d++)
    // 	    //   {
    // 	    // 	v_xyz.push_back(0.0);
    // 	    //   }
    // 	    // i++;
    // 	    continue;
    // 	  }


    // 	int ii = i;
    // 	// if(!do_init_id)
    // 	//   ii = vit->data().id;


    // 	for(int d = 0; d < D; d++)
    // 	  {
    // 	    double pv = vit->point()[d];
    // 	    v_xyz.push_back(pv);
    // 	  }
    // 	v_vid.push_back(vit->data().id);
    // 	v_flagv.push_back(vit->data().flag);

    // 	vertex_map[vit] = ii;
    // 	i++;
    //     }
    //   // write the number of cells
    //   n = tri.number_of_full_cells();
    //   // write the cells
    //   std::map<Cell_const_handle, uint> cell_map;
    //   i = 0;
    //   for(auto it = tri.full_cells_begin(); it != tri.full_cells_end(); ++it)
    //     {
    // 	// if(tri.is_infinite(it))
    // 	// 	continue;

    // 	int ii = i;
    // 	if(!do_init_id)
    // 	  int ii =  it->data().id;

    // 	cell_map[it] = ii;
    // 	++i;
    // 	for(int d = 0; d < D+1; d++)
    // 	  {
    // 	    int vertex_id = vertex_map[it->vertex(d)] ;
    // 	    v_simplex.push_back(vertex_id);

    // 	    // write the id
    // 	  }
    // 	v_flags.push_back(it->data().flag);
    // 	v_cid.push_back(ii);
    //     }

    //   // write the neighbors of the cells
    //   for(auto it = tri.full_cells_begin(); it != tri.full_cells_end(); ++it){
    //     for(int j = 0; j < D+1; j++)
    // 	{
    // 	  int nb_id = cell_map[it->neighbor(j)];
    // 	  v_nb.push_back(nb_id);
    // 	}
    //   }

    //   dmap[xyz_name].fill_full_output(v_xyz);
    //   dmap[simplex_name].fill_full_output(v_simplex);
    //   dmap[nb_name].fill_full_output(v_nb);
    //   dmap[vid_name].fill_full_output(v_vid);
    //   dmap[cid_name].fill_full_output(v_cid);
    //   dmap[flag_vertex_name].fill_full_output(v_flagv);
    //   dmap[flag_simplex_name].fill_full_output(v_flags);

    //   dmap[xyz_name].do_exist = true;
    //   dmap[simplex_name].do_exist = true;
    //   dmap[nb_name].do_exist = true;
    //   dmap[vid_name].do_exist = true;
    //   dmap[cid_name].do_exist = true;
    //   dmap[flag_vertex_name].do_exist = true;
    //   dmap[flag_simplex_name].do_exist = true;
    // }


    std::shared_ptr<tinyply::PlyData> & get_raw_points_ref()
    {
        return dmap[xyz_name].input_vect;
    }


    void copy_attribute(ddt_data & wd, int id, std::string ll)
    {
        for ( const auto &ee : wd.dmap )
        {
            if(dmap[ee.first].has_label(ll))
            {
                if(ee.second.do_exist)
                {
                    int vnbb =  ee.second.get_vnbb();
                    for(int i = 0 ; i < vnbb; i++)
                    {
                        dmap[ee.first].output_vect.push_back(wd.dmap[ee.first].input_vect->buffer.get()[id*vnbb+i]);
                    }
                    dmap[ee.first].do_exist = true;
                }
            }
        }
    }


    void copy_point(ddt_data & wd, int id)
    {
        for ( const auto &ee : wd.dmap )
        {
            if(ee.second.do_exist)
            {
                int vnbb =  ee.second.get_vnbb();
                for(int i = 0 ; i < vnbb; i++)
                {
                    dmap[ee.first].output_vect.push_back(wd.dmap[ee.first].input_vect->buffer.get()[id*vnbb+i]);
                }
                dmap[ee.first].do_exist = true;
            }
        }
    }


    Point get_pts(int id,std::vector<std::string> & name)
    {
        std::shared_ptr<tinyply::PlyData> & rp = dmap[name].input_vect;
        return traits.make_point( reinterpret_cast< double * >(rp->buffer.get())+id*D);
    }

    //  std::copy(a, a + 5, b);
    Point get_pts(int id)
    {
        std::shared_ptr<tinyply::PlyData> & rp = get_raw_points_ref();
        return traits.make_point( reinterpret_cast< double * >(rp->buffer.get())+id*D);
    }

    int nb_pts_input()
    {
        return dmap[xyz_name].get_nbe_input();
    }

    int nb_pts_output()
    {
        return dmap[xyz_name].get_nbe_output();
    }

    template<typename DT>
    int extract_full_input(std::vector<std::string> & name,std::vector<DT> & formated_data, bool do_clean = true)
    {
        dmap[name].extract_full_input(formated_data,do_clean);
    }

    template<typename DT>
    int fill_full_output(std::vector<std::string> & name,std::vector<DT> & formated_data, bool do_clean = true)
    {
        dmap[name].fill_full_output(formated_data,do_clean);

    }

    int nb_simplex_input()
    {
        return dmap[simplex_name].get_nbe_input();
    }

    int nb_simplex_output()
    {
        return dmap[simplex_name].get_nbe_output();
    }



    void extract_ptsvect(std::vector<std::string> & name,std::vector<Point> & formated_data, bool do_clean = true)
    {
        int nbv = dmap[name].get_nbe_input();
        for(int nn = 0; nn < nbv ; nn++)
            formated_data.push_back(get_pts(nn,name));
        if(do_clean)
            dmap[name].clean_input();
    }


    void fill_ptsvect(std::vector<std::string> & name,std::vector<Point> & formated_data, bool do_clean = true)
    {
        dmap[name] = Data_ply(xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
        std::vector<double> raw_pts;
        for(int n = 0; n < formated_data.size(); n++)
        {
            for(int d = 0; d < D; d++)
            {
                raw_pts.push_back(formated_data[n][d]);
            }
        }

        dmap[name].fill_full_output(raw_pts);
        //    dmap[name].do_exist = true;
    }



    Traits traits;
    int D = Traits::D;

    std::map<std::vector<std::string>, Data_ply > dmap;
    std::vector<std::string> xyz_name,
        vtileid_name,
        ctileid_name,
        vid_name,
        cid_name,
        flag_vertex_name,
        flag_simplex_name,
        simplex_name,nb_name;
};

#endif
