#ifndef DATA_H
#define DATA_H

#include <map>
#include <algorithm>

#include "tinyply.h"

#include "io/number_parser.hpp"
#include "io/logging_stream.hpp"
#include "io/base64_new.hpp"

#define PLY_CHAR (';')
#define DATA_FLOAT_TYPE (tinyply::Type::FLOAT64)
#define NB_DIGIT_OUT (5)




template<typename Traits>
class ddt_data {
public :
  typedef typename Traits::Point                                    Point;
  typedef typename Traits::Delaunay_triangulation                                    Dt;
  typedef typename Traits::Vertex_const_handle                                    Vertex_const_handle;
  typedef typename Traits::Cell_const_handle                                    Cell_const_handle;
  typedef typename Traits::Cell_handle                                    Cell_handle;
  typedef typename Traits::Vertex_handle                                    Vertex_handle;


  class Data_ply {
  public:

    Data_ply(std::vector<std::string> vv, std::string pp, int dim, int vs, tinyply::Type tt) : vname(vv),part(pp),D(dim),vsize(vs),do_exist(false),type(tt) {};  
    Data_ply() : do_exist(false) {};
    
    std::vector<std::string> get_name(){
      return vname;
    }

    std::string get_name(int ii, bool do_suffix = false){
      if(vname.size() == 1 )
	return vname[0];
      return vname[0]  + "_" + std::to_string(ii);
    }


    bool is_init() const {
      return(output_vect.size() > 0);
    }

    
    int get_nbe_output(){
      return output_vect.size()/((tinyply::PropertyTable[type].stride)*vsize);
    }

    
    int get_nbe_input(){
      if(input_vect != nullptr)
	return get_nbe_input_shp();
      return 0;
    }


    int get_nbe(){
      if(input_vect != nullptr)
	return get_nbe_input();
      if(output_vect.size() != 0)
	return get_nbe_output();
      return 0;
    }
    
    int get_nbe_input_shp() const {
      if(input_vect == nullptr)
	return 0;
      else
	return input_vect->buffer.size_bytes()/((tinyply::PropertyTable[type].stride)*vsize);
    }



    bool has_label(std::string v1) const {
      for(auto v2 : vname){
	if( v1 == v2)
	  return true;
      }
      return false;
    }

    void print_elems(){
      int acc=0;
      std::cout << "==============" << std::endl;
      for(auto ee : vname){
	std::cout << ee << "\t";
      }
      std::cout << std::endl;
      for(int i = 0; i < output_vect.size(); i++){
	std::cout << output_vect[i] << "\t";
	if(i % vsize == vsize-1)
	  std::cout << std::endl;
      }
    }

    int get_vsize() const {
      return vsize;
    }
    int get_vnbb() const {
      return vsize*tinyply::PropertyTable[type].stride;
    }

    int size_bytes(){
      if(input_vect != nullptr)
	return input_vect->buffer.size_bytes();
      else
	return output_vect.size();
    }

    void input2output(bool do_clean = true){
      output_vect.resize(size_bytes());
      std::memcpy(output_vect.data(), input_vect->buffer.get(),size_bytes());
      if(do_clean)
	input_vect.reset();
    }

    Point extract_pts(int id){
      return traits.make_point( reinterpret_cast< double * >(input_vect->buffer.get())+id*D);
    }

    template<typename DT>
    Point extract_vect(std::vector<DT> & formated_data,int id){
      int vnbb =  get_vnbb();
      formated_data.resize(vsize);
      std::memcpy(formated_data.data(), (input_vect->buffer.get())+id*D ,vnbb);
    }

    template<typename DT>
    void  extract_value( int id, DT & vv, int i=0) const {
      int vnbb =  get_vnbb();
      int szd = get_nbe_input_shp()*vsize;
      if(szd > 0){
	vv =  reinterpret_cast<DT &>(input_vect->buffer.get()[id*vnbb+i*tinyply::PropertyTable[type].stride]);
      }else{
	std::memcpy(&vv,&output_vect[id*vnbb+i],tinyply::PropertyTable[type].stride);
      }
    }

    
    template<typename DT>
    void extract_full_input(std::vector<DT> & formated_data, bool do_clean = true){
      int szd = get_nbe_input_shp()*vsize;
      std::cerr << "nbe_i" << szd << std::endl;
      if(szd > 0){
	formated_data.resize(szd);
	std::memcpy(formated_data.data(), input_vect->buffer.get(),size_bytes());
	if(do_clean)
	  input_vect.reset();
	return;
      }
      szd = get_nbe_output()*vsize;
      std::cerr << "==> " << szd << std::endl;
      if(szd > 0){
	formated_data.resize(szd);
	std::cerr << "==> -- " << szd << std::endl;
	std::memcpy(formated_data.data(), &output_vect[0],size_bytes());
	std::cerr << "memcpy ok" << std::endl;
	if(do_clean)
	  output_vect.clear();
      }
      return;
    }


    template<typename DT>
    void fill_full_output(std::vector<DT> & formated_data, bool do_clean = true){
      int szb = sizeof(DT)*formated_data.size();
      output_vect.resize(szb);
      std::memcpy(output_vect.data(), formated_data.data(),szb);
      if(do_clean)
	formated_data.clear();
      do_exist = true;
    }


    void clean_input(){
      input_vect.reset();
    }

    void clean_output(){
      output_vect.clear();
    }

    void set_exist(bool bb){
      do_exist = bb;
    }

    int get_dim(){
      return D;
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


  std::vector<std::string> subvect(std::vector<std::string> vname, int dd){
    return std::vector<std::string>(vname.begin(),vname.begin()+dd);
  }




  void init_map(){
    int D = Traits::D;
    dmap[xyz_name] = Data_ply(xyz_name,"vertex",D,D,tinyply::Type::INVALID);
    dmap[simplex_name] = Data_ply(simplex_name,"face",D+1,D+1,tinyply::Type::INT32);
    dmap[nb_name] = Data_ply(nb_name,"face",D+1,D+1,tinyply::Type::INT32);
  }



  void init_name(){
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
    gid_name = {"gid"};
  }
  
  ddt_data<Traits>(){
    int D = Traits::D;
    init_name();
    init_map();
  }



  
  ddt_data<Traits>(std::map<std::vector<std::string>, Data_ply > & init_dmap)  {
    init_name();
    for ( const auto &ee : init_dmap ) {
      dmap[ee.first] =  Data_ply(ee.first,ee.second.part,D,ee.second.get_vsize(),ee.second.type);	    
    }
  }


  void clear ()  {
    for ( const auto &ee : dmap ) {
      std::vector<uint8_t>().swap(dmap[ee.first].output_vect);
    }
  }



  void write_geojson_header(std::ostream & ss){
    ss << "{" << std::endl;
    ss << "\"type\": \"FeatureCollection\"," << std::endl;
    ss << "\"features\": [" << std::endl;
  }

  void write_geojson_footer(std::ostream & ss){
    ss << "]" << std::endl;
    ss << "}" << std::endl;
  }


  
  void write_geojson_tri(std::ostream & ofs_pts,std::ostream & ofs_spx, bool is_full = true){
    //    ofs_spx << std::fixed << std::setprecision(12);
    std::cerr << "vcyz ==>" << std::endl;
    int D = Traits::D;
    std::vector<std::string> lab_color = {"\"red\"","\"green\"","\"blue\""};
    bool is_first = is_full;

    if(is_full){
      write_geojson_header(ofs_pts);
      write_geojson_header(ofs_spx);
    }

    std::cerr << "vcyz <<<" << std::endl;
    std::vector<double> v_xyz;
    std::vector<int> v_simplex;
    dmap[xyz_name].extract_full_input(v_xyz,false);
    dmap[simplex_name].extract_full_input(v_simplex,false);
    std::cerr << "vcyz :::" << std::endl;
    std::cerr << v_xyz.size() << std::endl;
    int nb_pts = v_xyz.size()/D;
    std::cerr << "nbpts:" << nb_pts << std::endl;
    for(int id = 0; id < nb_pts; id++){
      std::cerr << "id:" << id << std::endl;
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

      for ( const auto &ee : dmap ) {
      	if(dmap[ee.first].part == "vertex" && ee.second.do_exist){
      	  for(int nn = 0 ; nn < dmap[ee.first].get_vsize();nn++){
      	    if(dmap[ee.first].type == tinyply::Type::INT32){
      	      int vv;
      	      dmap[ee.first].extract_value(id,vv,nn);
      	      ofs_pts << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
      	    }else  if(dmap[ee.first].type == tinyply::Type::UINT32){
      	      uint vv;
      	      dmap[ee.first].extract_value(id,vv,nn);
      	      ofs_pts << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
      	    }else  if(dmap[ee.first].type == DATA_FLOAT_TYPE){
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
    uint num_c = dmap[simplex_name].get_nbe();///(D+1);
    for(int id = 0; id < num_c; id++){
      int local = 0;
      bool is_infinite = false;
      for(int i=0; i<=D+1; ++i){
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

      for ( const auto &ee : dmap ) {
      	if(dmap[ee.first].part == "face" && ee.second.do_exist){

	  for(int nn = 0 ; nn < dmap[ee.first].get_vsize();nn++){
	    if(dmap[ee.first].type == tinyply::Type::INT32){
	      int vv;
	      dmap[ee.first].extract_value(id,vv,nn);
	      ofs_spx << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
	    }else  if(dmap[ee.first].type == tinyply::Type::UINT32){
	      uint vv;
	      dmap[ee.first].extract_value(id,vv,nn);
	      ofs_spx << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
	    }else  if(dmap[ee.first].type == DATA_FLOAT_TYPE){
	      double vv;
	      dmap[ee.first].extract_value(id,vv,nn);
	      ofs_spx << "\"" << dmap[ee.first].get_name(nn,true) << "\":" << vv << "," << std::endl;
	    }
	  }
      	}
      }
      
      ofs_spx << "\"stroke-width\":\"2\"," <<  std::endl;	
      ofs_spx << "\"prop1\": { \"this\": \"that\" }" << std::endl;
      ofs_spx << "}" << std::endl;
      ofs_spx << "}" << std::endl;
      
    }

    if(is_full){
      write_geojson_footer(ofs_pts);
      write_geojson_footer(ofs_spx);
    }
  }


  void write_serialized_stream( std::ostream & ss) 
  {
    int nn = 0;
    for ( const auto &ee : dmap ) {
      if(ee.second.do_exist){
	nn++;
      }
    }

    ss << nn << " ";
    
    for ( const auto &ee : dmap ) {
      if(ee.second.do_exist){
	std::cerr << "do_exist" << std::endl;
	// if(dmap[ee.first].get_nbe_output() == 0 &&
	//    dmap[ee.first].get_nbe_input() != 0){
	//   std::cerr << "i2o" << std::endl;
	//   dmap[ee.first].input2output();
	// }

	
	int nbe = dmap[ee.first].get_nbe_output();
	auto vv = dmap[ee.first].output_vect;
	ss << dmap[ee.first].vname.size() << " ";
	for(auto nn : dmap[ee.first].vname){
	  ss << nn << " ";
	}
	ss << ee.second.part << " ";
	ss << ee.second.get_vsize() << " ";
	ss << ((int) ee.second.type) << " ";
	serialize_b64_vect(vv,ss);
	ss << " ";
	std::cerr << "done" << std::endl;
      }
    }
  }

  void read_serialized_stream(std::istream & ss){
    int nbe;
    ss >> nbe;
    std::cerr << "nbe:" << nbe << std::endl;
    for(int i = 0 ; i < nbe;i++){
      std::vector<std::string> data_name;
      std::string tt_name("vertex");
      int dim,vs,dn_size;
      tinyply::Type tt;

      ss >> dn_size;
      std::cerr << "dn_size:" << dn_size << std::endl;
      std::cerr << ":";
      for(int i = 0; i < dn_size; i++){
	std::string nnn;
	ss >> nnn;
	std::cerr << nnn << " ";
	data_name.push_back(nnn);
      }
      std::cerr << std::endl;
      ss >> tt_name;
      ss >> vs;
      int ttti;
      ss >> ttti;
      dmap[xyz_name] = Data_ply(xyz_name,tt_name,dim,vs,static_cast<tinyply::Type>(ttti));
      deserialize_b64_vect(dmap[xyz_name].output_vect,ss);
    }
  }
  
  void write_ply_stream( std::ostream & ss,char nl_char = '\n',bool is_binary = false,bool do_elem_newline = false)
  {
    try
      {
	tinyply::PlyFile file_out;
	for ( const auto &ee : dmap ) {
	  if(dmap[ee.first].part == "vertex"){
	    if(ee.second.do_exist){
	      if(dmap[ee.first].get_nbe_output() == 0 &&
		 dmap[ee.first].get_nbe_input() != 0){
		dmap[ee.first].input2output();
	      }
	      int nbe = dmap[ee.first].get_nbe_output();
	      uint8_t * vv = dmap[ee.first].output_vect.data();
	      if(nbe > 0){
		file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name() , 
						   dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::INVALID, 0);
	      
	      }
	    }
	  }
	}
	for ( const auto &ee : dmap ) {
	  if(dmap[ee.first].part != "vertex"){
	    if(ee.second.do_exist){
	      if(dmap[ee.first].get_nbe_output() == 0 &&
		 dmap[ee.first].get_nbe_input() != 0){
		dmap[ee.first].input2output();
	      }
	      int nbe = dmap[ee.first].get_nbe_output();
	      uint8_t * vv = dmap[ee.first].output_vect.data();
	      if(nbe > 0){
		if(dmap[ee.first].part == "face"){
		  if(ee.first[0] == simplex_name[0] || ee.first[0] == nb_name[0])
		    file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name() , 
						       dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::UINT8, dmap[ee.first].get_vsize());
		  else
		    file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name() , 
						       dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::INVALID, 0);
		}
	      
	      }
	    }
	  }
	}
	file_out.write(ss,is_binary,nl_char,do_elem_newline);
      }
    catch (const std::exception & e)
      {
	std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
      }
  }



  void write_dataset_stream( std::ostream & ss,char nl_char,int tid)
  {


    
    std::vector<int> raw_ids_vertex(std::max(nb_pts_output(),nb_pts_input()),tid);
    std::vector<int> raw_ids_simplex(std::max(nb_simplex_output(),nb_simplex_input()),tid);


    
    dmap[vtileid_name] = Data_ply(vtileid_name,"vertex",1,1,tinyply::Type::INT32);
    dmap[ctileid_name] = Data_ply(ctileid_name,"face",1,1,tinyply::Type::INT32);
    dmap[vtileid_name].fill_full_output(raw_ids_vertex);
    dmap[ctileid_name].fill_full_output(raw_ids_simplex);
    dmap[vtileid_name].do_exist = true;
    dmap[ctileid_name].do_exist = true;

    try
      {
	//	ss << std::fixed << std::setprecision(12);
	//	ss << std::scientific << std::endl;
	tinyply::PlyFile file_out;
	//std::cerr << "db 1" << std::endl;
	for ( const auto &ee : dmap ) {
	  //std::cerr << "db 2 : "  << std::endl;
	  if(dmap[ee.first].part == "vertex"){
	    if(ee.second.do_exist){
	      if(dmap[ee.first].get_nbe_output() == 0 &&
		 dmap[ee.first].get_nbe_input() != 0){
		dmap[ee.first].input2output();
	      }
	      int nbe = dmap[ee.first].get_nbe_output();
	      uint8_t * vv = dmap[ee.first].output_vect.data();
	      if(nbe > 0){
		file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name() , 
						   dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::INVALID, 0);
	      
	      }
	    }
	  }
	}
	for ( const auto &ee : dmap ) {
	  if(dmap[ee.first].part != "vertex"){
	    //std::cerr << "db 3 : " << std::endl;
	    if(ee.second.do_exist){
	      if(dmap[ee.first].get_nbe_output() == 0 &&
		 dmap[ee.first].get_nbe_input() != 0){
		dmap[ee.first].input2output();
	      }
	      int nbe = dmap[ee.first].get_nbe_output();
	      uint8_t * vv = dmap[ee.first].output_vect.data();
	      if(nbe > 0){
		if(dmap[ee.first].part == "face"){
		  if(ee.first[0] == simplex_name[0] || ee.first[0] == nb_name[0])
		    file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name() , 
						       dmap[ee.first].type, nbe, reinterpret_cast<uint8_t*>(vv), tinyply::Type::UINT8, dmap[ee.first].get_vsize());
		  else
		    file_out.add_properties_to_element(dmap[ee.first].part, dmap[ee.first].get_name() , 
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
	    if(true){
	      vsize = e.size;

	      std::cerr << "element - " << e.name << " (" << e.size << ")" << std::endl;	      
	      for (auto p : e.properties){
		std::vector<std::string> pname({p.name});
		bool do_exist = false;

		// if(tinyply::PropertyTable[p.propertyType].str != "double")
		//   continue;
		
		for(auto vp : pname)
		   std::cerr << "\tproperty - " << vp << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;

		for ( const auto &ee : dmap ) {
		  if(ee.second.has_label(p.name)){
		    std::cerr << p.name << " existe!" << std::endl;
		    std::cerr << "[" << ee.first.size() << "] => ";
		     for(auto ie : ee.first)
		       std::cerr << ie << " - ";
		    std::cerr << std::endl;
		    do_exist = true;
		    dmap[ee.first].do_exist = true;
		    dmap[ee.first].type = p.propertyType;
		    // if(!dmap[ee.first].do_exist)
		    //   dmap[ee.first].init(vsize);
		    break;
		  }
		}
		if(!do_exist){
		  std::cerr << p.name << " create!" << std::endl;
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
	for ( const auto &ee : dmap ) {
	  if(ee.second.do_exist){
	    //std::cerr << "[" << ee.first.size() << "] => ";
	    // for(auto ie : ee.first)
	    //   std::cerr << ie << " - ";  
	    // std::cerr << std::endl;
	    std::cerr << "ok parse" <<  ee.first[0] << std::endl;
	    try { dmap[ee.first].input_vect = file.request_properties_from_element(ee.second.part, dmap[ee.first].get_name() ); }
	    catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }
	  }
	}
	//std::cerr << "reading done" << std::endl;


	// std::string input_buff;
	// std::getline(ss, input_buff);
	// std::stringstream iss(input_buff);

	
	file.read(ss);

	// std::cerr << "file convert" << std::endl;
	// for ( const auto &ee : dmap ) {
	//   if(ee.second.do_exist){
	//     dmap[ee.first].format(vsize);
	//   }
	//   //dmap[ee.first].print_elems();
	// }
	std::cerr << "read tri done" << std::endl;
      }

    catch (const std::exception & e)
      {
	std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
      }
  }






  void write_b64_generic_stream( std::ostream & ss,ddt::logging_stream & log)
  {
    int acc = 0;
    for ( const auto &ee : dmap ) {
      if(ee.second.do_exist){
	acc++;
      }
    }
    ss << acc << " ";
    for ( const auto &ee : dmap ) {
      if(ee.second.do_exist){
	std::vector<std::string> cname = dmap[ee.first].get_name();
	std::string part = dmap[ee.first].part;
	int dim = dmap[ee.first].get_dim();
	int vsize = dmap[ee.first].get_vsize();
	int tt =  static_cast<int>(dmap[ee.first].type);
	ss << cname.size() << " ";
	for(auto ccn : cname)
	  ss << ccn << " ";
	ss <<  " " << part << " " << dim << " " << vsize << " " << tt << " ";
	write_dataply_stream_fast(dmap[cname],ss,log);
      }
    }
  }

  void read_b64_generic_stream(std::istream & ifile)
  {
    std::cerr << "READ GENERIC STREAM" << std::endl;
    int acc;
    ifile >> acc;
    for(int i = 0;i < acc; i++){
      std::vector<std::string> cname;
      int name_size;
      std::string part; 
      int dim,vsize,tt;
      ifile >> name_size;
      std::cerr << "cname  :";
      for(int j = 0; j < name_size;j++){
	std::string lname;
	ifile >> lname;
	std::cerr  << lname << " ";
	cname.emplace_back(lname);
      }
      std::cerr << std::endl;
      ifile >> part >> dim >> vsize >> tt;
      std::cerr << "part:" << part << " dim:" << dim << " vsize:" << vsize << " tt" << tt << std::endl;
      dmap[cname] = Data_ply(cname,part,dim,vsize,static_cast<tinyply::Type>(tt));
      read_b64_data_fast(dmap[cname],ifile);
      //      dmap[xyz_name] = Data_ply(xyz_name,"vertex",D,D,get_float_type());
    }
    
  };



  tinyply::Type get_int_type(){
    return tinyply::Type::INT32;
  }

  tinyply::Type get_float_type(){
    return DATA_FLOAT_TYPE;
  }

  

  std::shared_ptr<tinyply::PlyData> & get_raw_points_ref(){
    return dmap[xyz_name].input_vect;
  }


  void copy_attribute(ddt_data & wd, int id, std::string ll){
    for ( const auto &ee : wd.dmap ) {
      if(dmap[ee.first].has_label(ll)){
	if(ee.second.do_exist){
	  int vnbb =  ee.second.get_vnbb();
	  for(int i = 0 ; i < vnbb; i++){
	    dmap[ee.first].output_vect.push_back(wd.dmap[ee.first].input_vect->buffer.get()[id*vnbb+i]);
	  }
	  dmap[ee.first].do_exist = true;
	}
      }
    }
  }

  
  void copy_point(ddt_data & wd, int id){
    for ( const auto &ee : wd.dmap ) {
      if(ee.second.do_exist){
	int vnbb =  ee.second.get_vnbb();
	for(int i = 0 ; i < vnbb; i++){
	  dmap[ee.first].output_vect.push_back(wd.dmap[ee.first].input_vect->buffer.get()[id*vnbb+i]);
	}
	dmap[ee.first].do_exist = true;
      }
    }
  }


  Point get_pts(int id,std::vector<std::string> & name){
    std::shared_ptr<tinyply::PlyData> & rp = dmap[name].input_vect;
    return traits.make_point( reinterpret_cast< double * >(rp->buffer.get())+id*D);
  }

  //  std::copy(a, a + 5, b);
  Point get_pts(int id){
    std::shared_ptr<tinyply::PlyData> & rp = get_raw_points_ref();
    return traits.make_point( reinterpret_cast< double * >(rp->buffer.get())+id*D);
  }
  
  int nb_pts_input(){
    return dmap[xyz_name].get_nbe_input();
  }

  int nb_pts_output(){
    return dmap[xyz_name].get_nbe_output();
  }



  template<typename DT>
  int extract_full_input(std::vector<std::string> & name,std::vector<DT> & formated_data, bool do_clean = true){
    dmap[name].extract_full_input(formated_data,do_clean);
  }

  template<typename DT>
  int fill_full_output(std::vector<std::string> & name,std::vector<DT> & formated_data, bool do_clean = true){
    dmap[name].fill_full_output(formated_data,do_clean);
  }

  int nb_simplex_input(){
    return dmap[simplex_name].get_nbe_input();
  }

  int nb_simplex_output(){
    return dmap[simplex_name].get_nbe_output();
  }



  void extract_ptsvect(std::vector<std::string> & name,std::vector<Point> & formated_data, bool do_clean = true){
    std::cerr << "start extract ptsvect" << std::endl;
    int nbv = dmap[name].get_nbe_input();

    for(int nn = 0; nn < nbv ; nn++)
      formated_data.push_back(get_pts(nn,name));
    if(do_clean)
      dmap[name].clean_input();
  }


  void fill_ptsvect(std::vector<std::string> & name,std::vector<Point> & formated_data, bool do_clean = true){
    dmap[name] = Data_ply(xyz_name,"vertex",D,D,DATA_FLOAT_TYPE);
    std::vector<double> raw_pts;
    for(int n = 0; n < formated_data.size();n++){
      for(int d = 0; d < D; d++){
	raw_pts.push_back(formated_data[n][d]);
      }
    }

    dmap[name].fill_full_output(raw_pts);
    //    dmap[name].do_exist = true;
  }

    void fill_gids(std::vector<int> & format_gids,bool do_clear = true)
    {
        ddt_data<Traits>::dmap[gid_name] = typename ddt_data<Traits>::Data_ply(gid_name,"face",1,1,tinyply::Type::INT32);
        std::vector<int> raw_gids;
        for(int i = 0 ; i < format_gids.size(); i++)
        {
            raw_gids.push_back(format_gids[i]);
        }

        ddt_data<Traits>::dmap[gid_name].fill_full_output(raw_gids);
        ddt_data<Traits>::dmap[gid_name].do_exist = true;

        if(do_clear)
        {
            format_gids.clear();
        }
        raw_gids.clear();
    }

    void extract_gids(std::vector<int> & format_gids,bool do_clear = true)
    {
        int D = Traits::D;
        std::vector<int> raw_gids;
        ddt_data<Traits>::dmap[gid_name].extract_full_input(raw_gids,false);

        uint num_s = ddt_data<Traits>::dmap[gid_name].get_nbe_input();
        for(int i = 0 ; i < raw_gids.size(); i++)
        {
            format_gids.push_back(raw_gids[i]);
        }
        if(do_clear)
            raw_gids.clear();
    }  
  
  Traits traits;  
  int D = Traits::D;
    std::vector<int> tile_ids;
  std::vector<int>  format_gids ;
  std::vector<int>  format_gidv ;
  std::map<std::vector<std::string>, Data_ply > dmap;
  std::vector<std::string> xyz_name,
    vtileid_name,
    ctileid_name,
    vid_name,
    cid_name,
    flag_vertex_name,
    flag_simplex_name,
    gid_name,
    simplex_name,nb_name;      
};

#endif
