#ifndef DATA_H
#define DATA_H

#include <map>
#include <algorithm>

#include "tinyply.h"

#include "io/number_parser.hpp"
#include "io/logging_stream.hpp"
#include "io/base64_new.hpp"


#define DATA_FLOAT_TYPE tinyply::Type::FLOAT64
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
      if(input_vect_uint.size() != 0)
	return get_nbe_input_uint();
      return 0;
    }
    
    int get_nbe_input_shp() const {
      if(input_vect == nullptr)
	return 0;
      else
	return input_vect->buffer.size_bytes()/((tinyply::PropertyTable[type].stride)*vsize);
    }

    int get_nbe_input_uint() const {
      if(input_vect_uint.size() == 0)
	return 0;
      else	
	return input_vect_uint.size()/((tinyply::PropertyTable[type].stride)*vsize);
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
	return input_vect_uint.size();
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
	std::memcpy(&vv,&input_vect_uint[id*vnbb+i],tinyply::PropertyTable[type].stride);
      }
    }

    
    template<typename DT>
    void extract_full_input(std::vector<DT> & formated_data, bool do_clean = true){
      int szd = get_nbe_input_shp()*vsize;
      if(szd > 0){
	formated_data.resize(szd);
	std::memcpy(formated_data.data(), input_vect->buffer.get(),size_bytes());
	if(do_clean)
	  input_vect.reset();
	return;
      }
      szd = get_nbe_input_uint()*vsize;
      if(szd > 0){
	formated_data.resize(szd);
	std::memcpy(formated_data.data(), &input_vect_uint[0],size_bytes());
	std::cerr << "memcpy ok" << std::endl;
	if(do_clean)
	  input_vect_uint.clear();
	return;
      }
      szd = get_nbe_output()*vsize;
      if(szd > 0){
	formated_data.resize(szd);
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


    template<typename DT>
    void fill_full_input(std::vector<DT> & formated_data, bool do_clean = true){
      int szb = sizeof(DT)*formated_data.size();
      input_vect_uint.resize(szb);
      std::memcpy(input_vect_uint.data(), formated_data.data(),szb);
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
    std::vector<uint8_t> input_vect_uint;
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
    int D = Traits::D;
    std::vector<std::string> lab_color = {"\"red\"","\"green\"","\"blue\""};
    bool is_first = is_full;

    if(is_full){
      write_geojson_header(ofs_pts);
      write_geojson_header(ofs_spx);
    }

   
    std::vector<double> v_xyz;
    std::vector<int> v_simplex;
    dmap[xyz_name].extract_full_input(v_xyz,false);
    dmap[simplex_name].extract_full_input(v_simplex,false);

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
    uint num_c = dmap[simplex_name].get_nbe_input();///(D+1);
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



  void read_b64_data_fast(Data_ply & dp,std::istream & ifile){

    int bufflen_in;
    long unsigned int bufflen_out;
    char cc;
    ifile >> bufflen_in;
    ifile.get(cc);
    unsigned char * buffer_char_in = new unsigned char[bufflen_in];
    unsigned char * buffer_char_out;
    ifile.read(((char *)buffer_char_in),bufflen_in);
    dp.do_exist = true;
    buffer_char_out = fast_base64_decode(buffer_char_in,bufflen_in,&bufflen_out);
    dp.input_vect_uint.resize(bufflen_out);
    std::memcpy(&dp.input_vect_uint[0], buffer_char_out, bufflen_out);
    delete [] buffer_char_in;
    delete [] buffer_char_out;
  }

  void read_b64_data(Data_ply & dp,std::istream & ifile){
    int nbc;
    std::string  buffer_char;
    char cc;
    std::cerr << "nbc:" << nbc << std::endl;
    ifile >> nbc;
    ifile >> buffer_char;
    dp.do_exist = true;
    dp.input_vect_uint = base64_decode(buffer_char);
  }

  
  void read_b64_stream(std::istream & ifile)
  {
    int do_datav_int,do_datac_int,num_v,num_c;

    ifile >> do_datav_int;// >> num_v >> num_c;
    ifile >> do_datac_int;// >> num_v >> num_c;

    bool do_datav = (do_datav_int == 1);
    bool do_datac = (do_datac_int == 1);

    dmap[xyz_name] = Data_ply(xyz_name,"vertex",D,D,get_float_type());
    read_b64_data_fast(dmap[xyz_name],ifile);

    dmap[simplex_name] = Data_ply(simplex_name,"face",D+1,D+1,get_int_type());
    read_b64_data_fast(dmap[simplex_name],ifile);

    dmap[nb_name] = Data_ply(nb_name,"face",D+1,D+1,get_int_type());
    read_b64_data_fast(dmap[nb_name],ifile);

    if(do_datav){
      dmap[vid_name] = Data_ply(vid_name,"vertex",1,1,get_int_type());
      read_b64_data_fast(dmap[vid_name],ifile);
      
      dmap[flag_vertex_name] = Data_ply(flag_vertex_name,"vertex",1,1,get_int_type());
      read_b64_data_fast(dmap[flag_vertex_name],ifile);
    }
    if(do_datac){
      dmap[flag_simplex_name] = Data_ply(flag_simplex_name,"face",1,1,tinyply::Type::INT8);
      read_b64_data_fast(dmap[flag_simplex_name],ifile);
    }
    // std::cerr << "Read face simplex" << std::endl;
    // ifile >> buffer_char;
    // dmap[simplex_name] = Data_ply(simplex_name,"face",D+1,D+1,get_int_type());
    // dmap[simplex_name].do_exist = true;
    // dmap[simplex_name].input_vect_uint = base64_decode(buffer_char);
    // std::cerr << dmap[simplex_name].input_vect_uint.size() << std::endl;
    // buffer_char.clear();
    
  };
    
    

  void write_dataply_stream(Data_ply & dp,std::ostream & ss){
    int bufflen = dp.output_vect.size();
    std::vector<BYTE> myData(bufflen);
    std::memcpy(&myData[0], &dp.output_vect[0], bufflen);
    std::string encodedData = base64_encode(&myData[0], myData.size());
    ss << encodedData.length() << " ";
    ss << encodedData << " ";
    
  }


  void write_dataply_stream_fast(Data_ply & dp,std::ostream & ss,ddt::logging_stream & log){
    log.step("[write]write_b64_stream_init");
    int bufflen_in = dp.output_vect.size();
    unsigned long int bufflen_out;
    unsigned char * buff_in = new unsigned char[bufflen_in];
    log.step("[write]write_b64_stream_memcpy");
    std::memcpy(buff_in, &dp.output_vect[0], bufflen_in);
    dp.output_vect.clear();
    log.step("[write]write_b64_stream_encode");
    unsigned char * buff_out = fast_base64_encode(buff_in, bufflen_in,&bufflen_out);
    log.step("[write]write_b64_dump_size");
    ss << bufflen_out << " ";
    log.step("[write]write_b64_dump_write_" + std::to_string(bufflen_out) + "_" + std::to_string(bufflen_in));
    ss.write(reinterpret_cast<char*>(buff_out),bufflen_out);
    ss << " ";
    log.step("[write]write_b64_dump_finalize");
    delete [] buff_in;
    delete [] buff_out;
  }


  void write_b64_stream( std::ostream & ss,ddt::logging_stream & log)
  {
    int do_data_int,num_v,num_c;
    bool do_data_v = dmap[vid_name].do_exist;
    bool do_data_c = dmap[flag_simplex_name].do_exist;
    ss << (do_data_v ? 1 : 0) << " ";
    ss << (do_data_c ? 1 : 0) << " "; 

    write_dataply_stream_fast(dmap[xyz_name],ss,log);
    write_dataply_stream_fast(dmap[simplex_name],ss,log);
    write_dataply_stream_fast(dmap[nb_name],ss,log);
    if(do_data_v){
      write_dataply_stream_fast(dmap[vid_name],ss,log);
      write_dataply_stream_fast(dmap[flag_vertex_name],ss,log);
    }
    if(do_data_c){
      //      write_dataply_stream_fast(dmap[cid_name],ss,log);
      write_dataply_stream_fast(dmap[flag_simplex_name],ss,log);
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



  void write_dataply_stream_block(Data_ply & dp,std::ostream & ss, char * full_buff, int & full_pos){
    std::cerr << "--1" << std::endl;
    int bufflen_in = dp.output_vect.size();
    unsigned long int bufflen_out;
    unsigned char * buff_in = new unsigned char[bufflen_in];
    std::cerr << "--2" << std::endl;
    std::memcpy(buff_in, &dp.output_vect[0], bufflen_in);
    char * buff_out = reinterpret_cast<char*>(fast_base64_encode(buff_in, bufflen_in,&bufflen_out));
    std::memcpy(full_buff + full_pos, buff_out, bufflen_out);
    full_pos += bufflen_out;
    std::cerr << "--3" << std::endl;
    delete [] buff_in;
    delete [] buff_out;
  }




  void write_b64_stream_block( std::ostream & ss,ddt::logging_stream & log)
  {
    int do_data_int,num_v,num_c;
    bool do_data = dmap[vid_name].do_exist;
    ss << (do_data ? 1 : 0) << " "; 

    char * full_buff = new char[200000000];
    int full_pos = 0;
    log.step("[write]b64_convert_data");
    write_dataply_stream_block(dmap[xyz_name],ss,full_buff,full_pos);
    write_dataply_stream_block(dmap[simplex_name],ss,full_buff,full_pos);
    write_dataply_stream_block(dmap[nb_name],ss,full_buff,full_pos);
    if(do_data){
      write_dataply_stream_block(dmap[vid_name],ss,full_buff,full_pos);
      write_dataply_stream_block(dmap[flag_vertex_name],ss,full_buff,full_pos);
    }
    log.step("[write]b64_dump_data");
    ss.write(full_buff,full_pos);
    log.step("[write]b64_done");
    delete [] full_buff;
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

  //   //std::cerr << "number of cells to read.." << num_c << std::endl;

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


  //   //std::cerr << "number of cells created : " << tri.number_of_full_cells() << std::endl;
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



  tinyply::Type get_int_type(){
    return tinyply::Type::INT32;
  }

  tinyply::Type get_float_type(){
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
