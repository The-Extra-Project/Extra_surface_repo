// #ifndef UTILS_HPP
// #define UTILS_HPP

// #include "wasure_typedefs.hpp"

// //namespace iqlib {

// template <typename Point>
// class Bbox {
//  public:
//  Bbox(int dim) : D(dim),cmin(std::vector<double>(D)),cmax(std::vector<double>(D)),dlen(std::vector<double>(D)) {
//     for(int d = 0; d <  D; d++){
//       cmin[d] = std::numeric_limits<double>::max();
//       cmax[d] = std::numeric_limits<double>::min();
//       dlen[d] = 0;
//     }

//   }



//   int D;
//   std::vector<Point> lp;
//   std::vector<double> cmin;
//   std::vector<double> cmax;
//   std::vector<double> dlen;


//   bool
//   is_inside(Point & p){
//     for(int d = 0; d < D ; d++){
//       if(p[d] < cmin[d] || p[d] > cmax[d] )
// 	return false;
//     }
//     return true;

//   }

//   void read(std::string namefile){
//     parse_points(namefile,lp,D);
//     for(auto pit = lp.begin(); pit != lp.end(); ++pit){
//       Point p = *pit;
//       for(int d = 0; d < D ; d++){
// 	if(p[d] < cmin[d]) cmin[d] = p[d];
// 	if(p[d] > cmax[d]) cmax[d] = p[d];
//       }
//     }
//     for(int d = 0; d < D ; d++){
//       //      std::cout << " " << cmax[d] << " " <<  cmin[d] << std::endl;;
//       dlen[d] = cmax[d] - cmin[d];
//     }
// }


//   void write(std::string namefile){
//     dump_points(lp,namefile,D);
//   }


//   friend std::ostream& operator<< (std::ostream& stream, const Bbox& bb){
//     stream << "bbox:" << std::endl;
//     for(int i = 0; i < bb.lp.size(); i++)
//       stream << (bb.lp[i]) << std::endl;
//     return stream;
//   }



// };


// void parse_string(const std::string& str, std::vector<std::string>& elements, char delimeter = ' ');


// std::string remove_path(const std::string & filename);
// std::string remove_extension(const std::string& filename) ;
// std::string switch_extension(const std::string& filename,const std::string& ext);
// std::string extract_last_digit(const std::string & str) ;
// int get_tri_idx(const std::string & str) ;


// template <typename DT>
// void init_idx_2(DT & tri){
//   using Vertex_handle = typename DT::Full_cell_handle;
//   int acc = 0;
//   for(auto fvit = tri.vertices_begin(); fvit != tri.vertices_end() ; ++fvit){
//     if( tri.is_infinite(fvit) )
//       continue;
//     Vertex_handle v = fvit;
//     v->data().idx = acc;
//     acc++;
//   }
// }



// inline void dump_double(float  num,std::ofstream & fo){
//   fo.write(reinterpret_cast<const char *>(&num), sizeof(num));
// }

// template <typename Point>
// Point get_rand_point(Point p,int D){

//   std::vector<double> coords(D);
//   for(int d = 0; d < D; d++){
//     coords[d]  = p[d] + ((double) rand() / (RAND_MAX))/0.5;
//   }
//   Point np(coords);
//   return np;
// }


// inline int get_byte(int n, int k){
//   return ((n & ( 1 << k )) >> k);
// }


// template <typename Point>
// void add_rand_point(std::vector<Point> & lp, int nbp,Bbox<Point> & bb,  int D){

//   for(int n = 0; n < nbp; n++){
//     std::vector<double> coords(D);
//     for(int d = 0; d < D; d++){
//       coords[d]  = (bb.cmax[d] - bb.cmin[d])*((double) rand() / (RAND_MAX))  + bb.cmin[d];
//     }
//     lp.push_back(Point(coords));
//   }
// }

// template <typename Point>
// void add_bbox_point(std::vector<Point> & lp, Bbox<Point> & bb,  int D){
//   for(auto  pit = bb.lp.begin();
//       pit != bb.lp.end();
//       pit++){
//     lp.push_back(*pit);
//   }
// }



// std::string double_2_string(const double dbl);


// template <typename Point>
// int
// parse_points(std::string namefile,   std::vector<Point> & points, int D){
//   std::fstream fs_in(namefile.c_str(), std::ios_base::in);
//   bool is_reading = true;
//   while (is_reading){
//     std::vector<double> coords(D);
//     for(int d = 0; d < D; d++){
//       if(!(fs_in >> coords[d]))
// 	is_reading = false;
//     }
//     if(!is_reading)
//       break;
//     points.push_back(Point(coords));
//   }
//   fs_in.close();
//   return points;
// }


// template <typename Point>
// void
// dump_points(std::vector<Point> & points, std::string namefile, int D){
//   std::ofstream fo;
//   fo.open (namefile.c_str());
//   for(auto pit = points.begin(); pit != points.end(); ++pit){
//     Point p1 = *pit;
//     for(int d = 0; d < D; d++)
//       fo << p1[d] << " ";
//     fo << std::endl;
//   }
//   fo.close();
// }


// template <typename Point>
// void dump_3d_ply(std::string namefile, std::vector<Point> & points,std::vector<std::vector<Point> > & norms){


//   int nbv = points.size();
//   std::cout << "motherfucking money!" << std::endl;

//   std::ofstream fo;
//   fo.open (namefile.c_str());
// #ifdef VERBOSE
//   std::cout << "\t Writing " << fileName << "..." << std::endl;
// #endif
//   fo << "ply" << std::endl;
//   fo << "format ascii 1.0" << std::endl;
//   fo << "comment VCGLIB generated" << std::endl;
//   fo << "element vertex " << nbv << std::endl;
//   fo << "property float x" << std::endl;
//   fo << "property float y" << std::endl;
//   fo << "property float z" << std::endl;
//   fo << "property float nx" << std::endl;
//   fo << "property float ny" << std::endl;
//   fo << "property float nz" << std::endl;
//   // fo << "property uchar red" << std::endl;
//   // fo << "property uchar green" << std::endl;
//   // fo << "property uchar blue" << std::endl;
//   fo << "end_header" << std::endl;

//   std::cout << "dumping point...." << std::endl;


//   for(int n = 0; n < nbv; n++){
//     Point pi = points[n];
//     Point norm = norms[n][2];
//     Point cols = norms[n][2];
//     fo << pi[0] << " " <<  pi[1]  << " " <<  pi[2] << " " << norm[0] << " " <<  norm[1]  << " " <<  norm[2]    << std::endl;
//     //fo << pi[0] << " " <<  pi[1]  << " " <<  pi[2] << " " << norm[0] << " " <<  norm[1]  << " " <<  norm[2]   <<  " " << cols[0] << " " <<  cols[1]  << " " <<  cols[2]   << std::endl;

//   }

//   fo.close();


// }



// //}


// #endif
