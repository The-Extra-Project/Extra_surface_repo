//#include "utils.hpp"
#include "ANN/ANN.h"
#include "wasure_maths.hpp"

#include "wasure_algo.hpp"
#include "input_params.hpp"
#include <algorithm>
#include <random>
#include "wasure_typedefs.hpp"


// void insert_np(DT_raw  & tri, Point & pp, double * bbox_min, double * bbox_max, int D){
//   bool do_insert = true;
//   for(int d = 0; d < D; d++)
//     if(pp[d] > bbox_max[d] || pp[d] < bbox_min[d])
//       do_insert = false;
//   if(do_insert)
//     tri.insert(pp);
// }

// void wasure_algo::tessel_adapt(std::vector<Point> & points,std::vector<Point> & vps,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales, int maxit, double target_err, int D, int tid){
//   std::vector<int> lidx(points.size());
//   std::iota(lidx.begin(), lidx.end(), 0);
//   int nb_inserted = 0;
//   random_shuffle(std::begin(lidx), std::end(lidx));


  
//   //  typedef typename DT_raw::Vertex_handle                            Vertex_const_handle;
//   typedef DT_raw::Cell_handle    Cell_handle;
//   typedef DT_raw::Vertex_handle  Vertex_handle;
//   typedef DT_raw::Locate_type    Locate_type;
//   typedef DT_raw::Point          Point;
//   DT_raw  tri = traits_raw.triangulation(D) ;

  
  
//   for(int i = 0; i < points.size(); i++){
//     int pidx = lidx[i];
//     Point p1 = points[pidx];

//     std::vector<Point> & pts_norm = norms[pidx];
//     std::vector<double> & pts_scale = scales[pidx];

//     Locate_type lt;
//     int li, lj;
//     // DTW::Face f(D-1);
//     // Facet ft;

//     // DTW::Locate_type lt;
    
//     auto loc = tri.locate(p1,lt, li, lj);
//     if(lt != DT_raw::CELL){
//       tri.insert(p1,loc);
//       nb_inserted++;
//       continue;
//     }

//     bool do_insert = true;
//     // for(auto vht = loc->vertices_begin() ;
//     // 	vht != loc->vertices_end() ;
//     // 	++vht){
//     for(uint dd = 0; dd < D; dd++)
//       {
// 	Point & pii = loc->vertex(dd)->point();
// 	std::vector<double> pii_coefs = compute_base_coef<Point>(p1,pii,pts_norm,D);
// 	int is_close_enough = 0;
// 	for(int d = 0; d < D; d++){
// 	  if(fabs(pii_coefs[d]) < pts_scale[d]*target_err){
// 	    is_close_enough++;
// 	    break;
// 	  }
// 	}
// 	if(is_close_enough ){
// 	  do_insert = false;
// 	  break;
// 	}
//       }
//     if(do_insert){
//       tri.insert(p1,loc);
//       nb_inserted++;
//     }
//   }
//   tessel(tri,points,vps,norms,scales,maxit,D);
//   //std::cout << "nb inserted :" << nb_inserted << "/" << l1.size() << std::endl;
// }


void wasure_algo::tessel_adapt(std::vector<Point> & points,std::vector<Point> & vps,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales, int maxit, double target_err, int D, int tid){
  std::vector<int> lidx(points.size());
  std::iota(lidx.begin(), lidx.end(), 0);
  int nb_inserted = 0;
  random_shuffle(std::begin(lidx), std::end(lidx));


  
  //  typedef typename DT_raw::Vertex_handle                            Vertex_const_handle;
  typedef DT_raw::Cell_handle    Cell_handle;
  typedef DT_raw::Vertex_handle  Vertex_handle;
  typedef DT_raw::Locate_type    Locate_type;
  typedef DT_raw::Point          Point;
  DT_raw  tri = traits_raw.triangulation(D) ;

  
  
  for(int i = 0; i < points.size(); i++){
    int pidx = lidx[i];
    Point p1 = points[pidx];

    std::vector<Point> & pts_norm = norms[pidx];
    std::vector<double> & pts_scale = scales[pidx];

    Locate_type lt;
    int li, lj;
    // DTW::Face f(D-1);
    // Facet ft;

    // DTW::Locate_type lt;
    
    auto loc = tri.locate(p1,lt, li, lj);
    if(lt != DT_raw::CELL){
      tri.insert(p1,loc);
      nb_inserted++;
      continue;
    }

    bool do_insert = true;
    // for(auto vht = loc->vertices_begin() ;
    // 	vht != loc->vertices_end() ;
    // 	++vht){
    for(uint dd = 0; dd < D+1; dd++)
      {
	Point & pii = loc->vertex(dd)->point();
	std::vector<double> pii_coefs = compute_base_coef<Point>(p1,pii,pts_norm,D);
	int is_close_enough = 0;
	for(int d = 0; d < D; d++){
	  if(fabs(pii_coefs[d]) < pts_scale[d]*target_err){
	    is_close_enough++;
	    break;
	  }
	}
	if(is_close_enough > 0  ){
	  do_insert = false;
	  break;
	}
      }
    if(do_insert){
      tri.insert(p1,loc);
      nb_inserted++;
    }
  }
  tessel(tri,points,vps,norms,scales,maxit,tid);
  //std::cout << "nb inserted :" << nb_inserted << "/" << l1.size() << std::endl;
}


int dump_tessel(DT_raw  & tri, int it, int max_it,int tid, double * bbox_min,double * bbox_max){
  Traits_raw traits_raw;
  int nbp_face = 10;;
  typedef typename DT_raw::Vertex_handle                            Vertex_const_handle;  
  std::vector<std::vector<double>> l_cir;
  for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit){
  	auto tmp_fch = fit->first;
  	int tmp_idx = fit->second;
  	auto tmp_fchn = tmp_fch->neighbor(tmp_idx);
  	auto bb1 = traits_raw.circumcenter(tri,tmp_fch);
  	auto bb2 = traits_raw.circumcenter(tri,tmp_fchn);

  	for(int i = 0; i < nbp_face; i++){
	  std::vector<double> pp;
	  bool do_insert = true;
  	  for(int j = 0; j < 3;j++){
  	    pp.push_back((bb1[j] + (bb2[j]-bb1[j])*(i/((double)nbp_face))));
  	  }
	  for(int j = 0; j < 3;j++){
	    if(pp[j] < bbox_min[j] || pp[j] > bbox_max[j]){
	      do_insert = false;
	    }
	  }
	  if(do_insert)
	    l_cir.push_back(pp);
  	}
  }
  
  std::cerr << "dump" << std::endl;
  std::ofstream myfile;
  std::string filename("/home/laurent/shared_spark/tmp/tessel_" + std::to_string(it) + "_" + std::to_string(tid) + ".ply");
  myfile.open (filename);
  myfile << "ply" <<  std::endl;
  myfile << "format ascii 1.0" << std::endl;
  myfile << "element vertex "  << tri.number_of_vertices() + l_cir.size()  << std::endl;
  myfile << "property float x" << std::endl;
  myfile << "property float y" << std::endl;
  myfile << "property float z" << std::endl;
  myfile << "property uchar red " << std::endl;
  myfile << "property uchar green" << std::endl;
  myfile << "property uchar blue" << std::endl;
  myfile << "element face " << tri.number_of_finite_facets() << std::endl;
  myfile << "property list uchar int vertex_index " << std::endl;
  myfile << "end_header                " << std::endl;


  double ccol = ((double)it)/((double)max_it);
  CGAL::Unique_hash_map<Vertex_const_handle, int> vertex_map;
  int acc = 0;
  for(auto vv = traits_raw.vertices_begin(tri); vv != traits_raw.vertices_end(tri) ; ++vv){
    if(tri.is_infinite(vv))
      continue;
    myfile << vv->point() << " " << ((int)(255*ccol)) << " " << ((int)(255*ccol)) << " " << ((int)(255*ccol)) << std::endl;
    vertex_map[vv] = acc++;
  }

  for(auto pp : l_cir)
    myfile << pp[0] << " " << pp[1] << " " << pp[2] << " 0 255 0" << std::endl;;

  
  for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit){
    if(tri.is_infinite(*fit))
      continue;
    auto fch = fit->first;
    int id_cov = fit->second;
    myfile << "3 ";
    for(int i = 0; i < 4; ++i)
      {
	if(i != id_cov)
	  {
	    auto v = fch->vertex(i);
	    myfile << vertex_map[v] << " ";
	  }
      }
    myfile << std::endl;

  }
  
  std::cerr << "close" << std::endl;
  myfile.close();
  std::cerr << "dump done" << std::endl;
  return 0;
}

int 
wasure_algo::tessel(DT_raw  & tri,
		    std::vector<Point> & points,  std::vector<Point> & vps,
		    std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales, int max_it, Id tid){

  typedef typename DT_raw::Vertex_handle                            Vertex_const_handle;  
  double bbox_min[Traits::D];
  double bbox_max[Traits::D];
  for(int d = 0; d < D; d++){
    bbox_min[d] = 100000;
    bbox_max[d] = -100000;
  }
  for(auto p1 : points){
    for(int d = 0; d < D; d++){
      if(p1[d] < bbox_min[d])
	bbox_min[d] = p1[d];
      if(p1[d] > bbox_max[d])
	bbox_max[d] = p1[d];
    }
  }
  
  vps.clear();

  std::vector<Point> extra_pts;
  for(int it = 0; it < max_it; it++){
    std::cerr << "tessel:" << it << std::endl;
    CGAL::Unique_hash_map<Vertex_const_handle, std::vector<double>> vertex_map;
    CGAL::Unique_hash_map<Vertex_const_handle, std::vector<double>> norm_map;
    CGAL::Unique_hash_map<Vertex_const_handle, std::vector<double>> scale_map;

    for(auto vv = traits_raw.vertices_begin(tri); vv != traits_raw.vertices_end(tri) ; ++vv){
      if(tri.is_infinite(vv))
	continue;
      vertex_map[vv] = std::vector<double>(D+1,0);
      norm_map[vv] = std::vector<double>(D,0);
      scale_map[vv] = std::vector<double>(D,0);
    }
    for(int ii = 0; ii < points.size();ii++){
      Point pp = points[ii];
      double ss = 1;//exp(-(scales[ii][D-1]*scales[ii][D-1])/0.1);
      auto vv = tri.nearest_vertex(pp);
      for(int d = 0; d < D; d++){
	vertex_map[vv][d] += ss*pp[d];
	norm_map[vv][d] += ss*norms[ii][D-1][d];
	scale_map[vv][d] += ss*scales[ii][d];
      }

      vertex_map[vv][D]+=ss;
    }

    for(auto vv = traits_raw.vertices_begin(tri); vv != traits_raw.vertices_end(tri) ; ++vv){
      if(tri.is_infinite(vv))
	continue;
      auto vpo = vv->point();
      auto vpn = vertex_map[vv];
      auto vn = norm_map[vv];
      auto vs = norm_map[vv];
      if(vpn[D] == 0)
	continue;
      for(int d = 0; d < D; d++){
	vpn[d]=vpn[d]/vpn[D];
	vn[d]=vn[d]/vpn[D];
	vs[d]=vs[d]/vpn[D];
      }
      //auto pp = Point(vpn[0],vpn[1],vpn[2]);
      auto pp = Point(vpo[0] +(vpn[0]-vpo[0]),
		      vpo[1] +(vpn[1]-vpo[1]),
		      vpo[2] +(vpn[2]-vpo[2]));
      //std::cerr << "move:" << vv->point() << " -> " << pp << "(" << vp[D] << ")"<<std::endl;

      bool do_insert = true;
      for(int d = 0; d < D; d++)
	if(pp[d] > bbox_max[d] || pp[d] < bbox_min[d])
	  do_insert = false;
      if(do_insert){
	tri.move(vv,pp);
      }

      if(((double) rand() / (RAND_MAX)) > 0.9 && it == max_it-1){
	auto pp2 = Point(vpn[0]-vn[0]*vs[0]*3,vpn[1]-vn[1]*vs[1]*3,vpn[2]-vn[2]*vs[2]*3);
	extra_pts.push_back(pp2);
      }

            
      // if(((double) rand() / (RAND_MAX)) > 0.9 && it == max_it-1){
      // 	auto pp2 = Point(vp[0]-vn[0],vp[1]-vn[1],vp[2]-vn[2]);
      // 	std::cerr << "insert new " << pp2 <<  std::endl;
      // 	insert_np(tri,pp2,bbox_min,bbox_max,D);
      // 	std::cerr << "insert done " <<  std::endl;
      // }
    }
     if(false)
       dump_tessel(tri,it,max_it,tid,bbox_min,bbox_max);

  }    


  std::cerr << "filter out bbox" << std::endl;
    
  for(auto vv = traits_raw.vertices_begin(tri); vv != traits_raw.vertices_end(tri) ; ++vv){
    bool do_insert = true;
    for(int d = 0; d < D; d++)
      if(vv->point()[d] > bbox_max[d] || vv->point()[d] < bbox_min[d])
	do_insert = false;
    if(do_insert)
      vps.push_back(vv->point());
  }
  
  for(auto pp : extra_pts){
    bool do_insert = true;
    for(int d = 0; d < D; d++)
      if(pp[d] > bbox_max[d] || pp[d] < bbox_min[d])
  	do_insert = false;
    if(do_insert)
      vps.push_back(pp);
  }
  
  return 0;
}




int 
wasure_algo::simplify(std::vector<Point> & points, std::vector<bool> & do_keep, double dist ){
  int D = Traits::D;  

  int nbp = points.size();
  double eps = 0;
  int K_T = 150;
  if(K_T > points.size() -1)
    K_T = points.size() - 1;
  //std::cerr << "step1" << std::endl;
  ANNpointArray	dataPts;
  ANNpoint	queryPt;
  ANNidxArray	nnIdx;
  ANNdistArray	dists;
  ANNkd_tree*	kdTree;

  queryPt = annAllocPt(D);
  dataPts = annAllocPts(nbp, D);
  nnIdx = new ANNidx[K_T];				
  dists = new ANNdist[K_T];

  int npts =0;
  for(std::vector<Point>::iterator pit = points.begin(); pit != points.end(); ++pit){
    Point p1 = *pit;
    for(int d = 0; d < D; d++){
      dataPts[npts][d] = p1[d];
    }
    do_keep[npts] = false;
    npts++;
  }

  kdTree = new ANNkd_tree(dataPts,npts,D);
  int cur_id = 0;
  for(std::vector<Point>::iterator pit = points.begin(); pit != points.end(); ++pit){
    Point p1 = *pit;
    for(int d = 0; d < D; d++)
      queryPt[d] = p1[d];
    kdTree->annkSearch(queryPt,K_T, nnIdx,dists,eps);
    for(int k = 1; k < K_T; k++){
      double kdist = dists[k];
      //  std::cerr << kdist << std::endl;
      if(kdist > dist){
	do_keep[cur_id] = true;
	break;
      }
      if(do_keep[nnIdx[k]]){
	break;
      }
    }
    cur_id++;
  }
  std::cerr << "done!" << std::endl;
  delete kdTree;
  delete [] nnIdx;                                                  // clean things up
  delete [] dists;
  return 0;
  //std::cerr << "step3" << std::endl;

}



// =======================  DIM ================================
void
wasure_algo::compute_svd(int K_T, const  ANNidxArray & nnIdx, const std::vector<Point> & points,std::vector<Point> & pts_norms, std::vector<double> &coords_scale){
  int D = Traits::D;  
  
  std::vector<Point> loc_pts;

  for(int i = 0; i < K_T; i++){
    int idx = nnIdx[i];
    loc_pts.push_back(points[idx]);
  }
  // std::pair<Point,Point> norm = get_norm(loc_pts,D);


  // -----------------------------
  // ------- Compute svd ---------
  // -----------------------------
  int N = loc_pts.size();
  Eigen::MatrixXf mat(N,D);

  int acc = 0;
  for(std::vector<Point>::iterator pit = loc_pts.begin(); pit != loc_pts.end(); ++pit){
    Point p = *pit;
    for(int d = 0 ; d < D ; d++)
      mat(acc,d) = p[d];
    acc++;
  }

  Eigen::MatrixXf m = mat.rowwise() - mat.colwise().mean();
  Eigen::JacobiSVD<Eigen::MatrixXf> svd(m, Eigen::ComputeThinU | Eigen::ComputeThinV);
  Eigen::MatrixXf sv = svd.singularValues() ;
  Eigen::MatrixXf ev =  svd.matrixV();
    
  for(int d1 = 0; d1 < D; d1++){
    double coords_norm[Traits::D];
    for(int d2 = 0; d2 < D; d2++){
      coords_norm[d2] = ev(d2,d1);
    }
    pts_norms.push_back(traits.make_point(coords_norm));
  }
  double v_min = 0.0000001;
  for(int d = 0; d < D; d++){
    coords_scale[d] = sv(d);
    if(coords_scale[d] <= v_min || std::isnan(coords_scale[d]))
      coords_scale[d] = v_min;
  }
}



int 
wasure_algo::compute_dim(std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales){
  int D = Traits::D;  

  int nbp = points.size();
  double eps = 0;
  int K_T = 150;
  if(K_T > points.size() -1)
    K_T = points.size() -1;

  int K_T_min = 3;
  // if(K_T_min > points.size() -1)
  //   K_T_min = points.size() - 1;
  
  //std::cerr << "step1" << std::endl;
  ANNpointArray	dataPts;
  ANNpoint	queryPt;
  ANNidxArray	nnIdx;
  ANNdistArray	dists;
  ANNkd_tree*	kdTree;

  queryPt = annAllocPt(D);
  dataPts = annAllocPts(nbp, D);
  nnIdx = new ANNidx[K_T];				
  dists = new ANNdist[K_T];


  
  int npts =0;
  for(std::vector<Point>::iterator pit = points.begin(); pit != points.end(); ++pit){
    Point p1 = *pit;
    for(int d = 0; d < D; d++)
      dataPts[npts][d] = p1[d];
    npts++;
  }

  kdTree = new ANNkd_tree(dataPts,npts,D);
  //std::cerr << "step2" << std::endl;


  for(std::vector<Point>::iterator pit = points.begin(); pit != points.end(); ++pit){
    Point p1 = *pit;
    std::vector<Point> pts_norms;
    std::vector<double> coords_scale(D);


    for(int d1 = 0; d1 < D; d1++){
      double coords_norm[Traits::D];
      for(int d2 = 0; d2 < D; d2++){
	if(d1 != d2)
	  coords_norm[d2] = 0;
	else
	  coords_norm[d2] = 1;
	
      }
      pts_norms.push_back(traits.make_point(coords_norm));
      coords_scale[d1] = 1;
    }

    
    double entropy = 1000000000;
    for(int d = 0; d < D; d++)
      queryPt[d] = p1[d];

    kdTree->annkSearch(queryPt,K_T, nnIdx,dists,eps);
    
    for(int k = K_T_min; k < K_T; k++){
      if(dists[k] < 0.02 && k < 30)
	continue;
      std::vector<Point> cur_pts_norms;
      std::vector<double> cur_coords_scale(D);
      compute_svd(k, nnIdx, points,cur_pts_norms,cur_coords_scale);
      double cst = 0;
      std::vector<double> dim(D);      
      for(int d = 0 ; d < D; d++){
	cst += cur_coords_scale[d];
      }
      for(int d = 0;d < D; d++){
	dim[d] = (d == D-1) ? cur_coords_scale[d]/cst : (cur_coords_scale[d] - cur_coords_scale[d+1])/cst;
      }
      double cur_entropy = 0;
      for(int d = 0; d < D; d++){
	cur_entropy += -dim[d]*log(dim[d]);
      }
      if(cur_entropy < entropy){
	entropy = cur_entropy;
	pts_norms = cur_pts_norms;
	coords_scale = cur_coords_scale;
      }
    }


    // -----------------------------
    // ------- Dump values ---------
    // -----------------------------
    if(pts_norms.size() < D ){
      std::cerr << "error !! " << "dim:" << D << " kt:" << K_T << " entropy:" << entropy  << std::endl;

      std::vector<Point> loc_pts;
      for(int i = 0; i < K_T; i++){
	int idx = nnIdx[i];
	loc_pts.push_back(points[idx]);
      }
      bool are_equal = true;
      for(int i = 1; i < loc_pts.size(); i++)
	if(loc_pts[i] != loc_pts[i-1]){
	  are_equal = false;
	  break;
	}
      if(are_equal){
	std::cerr << "Point " << loc_pts[0] << " duplicated " << K_T << " time, svd not defined!" << std::endl;
	std::cerr << "remove duplicted point to continue" << std::endl;
	return 1;
      }
	
    }else{
      norms.push_back(pts_norms);
      scales.push_back(coords_scale);
    }
  }
  std::cerr << "done!" << std::endl;
  delete kdTree;
  delete [] nnIdx;                                                  // clean things up
  delete [] dists;
  return 0;
  //std::cerr << "step3" << std::endl;

}


int 
wasure_algo::compute_dim_with_simp(std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales,std::vector<Point> & simp,double pscale){
  int D = Traits::D;  

  std::vector<double> entropy_vect;
  int nbp = points.size();
  double eps = 0;
  int K_T = 150;
  if(K_T > points.size() -1)
    K_T = points.size() - 1;
  //std::cerr << "step1" << std::endl;
  ANNpointArray	dataPts;
  ANNpoint	queryPt;
  ANNidxArray	nnIdx;
  ANNdistArray	dists;
  ANNkd_tree*	kdTree;

  queryPt = annAllocPt(D);
  dataPts = annAllocPts(nbp, D);
  nnIdx = new ANNidx[K_T];				
  dists = new ANNdist[K_T];

  double bbox_min[Traits::D];
  double bbox_max[Traits::D];
  for(int d = 0; d < D; d++){
    bbox_min[d] = 100000;
    bbox_max[d] = -100000;
  }
  int npts =0;
  for(std::vector<Point>::iterator pit = points.begin(); pit != points.end(); ++pit){
    Point p1 = *pit;
    for(int d = 0; d < D; d++){
      dataPts[npts][d] = p1[d];
      if(p1[d] < bbox_min[d])
	bbox_min[d] = p1[d];
      if(p1[d] > bbox_max[d])
	bbox_max[d] = p1[d];
    }
    npts++;
  }

  kdTree = new ANNkd_tree(dataPts,npts,D);
  //std::cerr << "step2" << std::endl;


  for(std::vector<Point>::iterator pit = points.begin(); pit != points.end(); ++pit){
    Point p1 = *pit;
    std::vector<Point> pts_norms;
    std::vector<double> coords_scale(D);
    double entropy = 1000000000;
    for(int d = 0; d < D; d++)
      queryPt[d] = p1[d];

    kdTree->annkSearch(queryPt,K_T, nnIdx,dists,eps);
    
    for(int k = 3; k < K_T; k++){
      
      std::vector<Point> cur_pts_norms;
      std::vector<double> cur_coords_scale(D);
      compute_svd(k, nnIdx, points,cur_pts_norms,cur_coords_scale);
      double cst = 0;
      std::vector<double> dim(D);      
      for(int d = 0 ; d < D; d++){
	cst += cur_coords_scale[d];
      }
      for(int d = 0;d < D; d++){
	dim[d] = (d == D-1) ? cur_coords_scale[d]/cst : (cur_coords_scale[d] - cur_coords_scale[d+1])/cst;
      }
      double cur_entropy = 0;
      for(int d = 0; d < D; d++){
	cur_entropy += -dim[d]*log(dim[d]);
      }
      if(cur_entropy < entropy){
	entropy = cur_entropy;
	pts_norms = cur_pts_norms;
	coords_scale = cur_coords_scale;
      }
    }
    entropy_vect.push_back(entropy);

    // -----------------------------
    // ------- Dump values ---------
    // -----------------------------
    if(pts_norms.size() < D){
      std::cerr << "error !! " << "dim:" << D << " kt:" << K_T << " entropy:" << entropy  << std::endl;

      std::vector<Point> loc_pts;
      for(int i = 0; i < K_T; i++){
	int idx = nnIdx[i];
	loc_pts.push_back(points[idx]);
      }
      bool are_equal = true;
      for(int i = 1; i < loc_pts.size(); i++)
	if(loc_pts[i] != loc_pts[i-1]){
	  are_equal = false;
	  break;
	}
      if(are_equal){
	std::cerr << "Point " << loc_pts[0] << " duplicated " << K_T << " time, svd not defined!" << std::endl;
	std::cerr << "remove duplicted point to continue" << std::endl;
	return 1;
      }
	
    }else{
      norms.push_back(pts_norms);
      scales.push_back(coords_scale);
    }
  }

  if(false){
    //  if(pscale > 0){
    int K_T2 = K_T;
    for(int ii = 0; ii < nbp; ii++){
      Point p1 = points[ii];
      for(int d = 0; d < D; d++)
	queryPt[d] = p1[d];
      kdTree->annkSearch(queryPt,K_T2, nnIdx,dists,eps);
      bool is_min = true;
      int dd = -1;
      double coords_new_pts[Traits::D];
      for(int d = 0; d < D;d++)
	  coords_new_pts[d] = 0;
      double ww_acc = 0;
      int k;

      for(k = 0; k < K_T2; k++){
	if(entropy_vect[ii] > entropy_vect[nnIdx[k]]){
	  is_min = false;
	  break;
	}
	double ww = exp(-(dists[k]*dists[k])/scales[ii][D-1]);
	ww_acc += ww;
	for(int d = 0; d < D;d++){
	  coords_new_pts[d] += points[nnIdx[k]][d]*ww;

	}
	
	if(dists[k] > scales[ii][D-1]*pscale){
	  dd = dists[k];
	  break;
	}
      }
      
      
      if(is_min){
	bool is_in_bbox = true;
	for(int d = 0; d < D;d++){
	  if(coords_new_pts[d] > bbox_max[d] || coords_new_pts[d] < bbox_min[d])
	    is_in_bbox = false;
	  coords_new_pts[d] /= ww_acc;
	}
	if(is_in_bbox)
	  simp.push_back(traits.make_point(coords_new_pts));
	//	simp.push_back(p1);
	  //kdTree->annkSearch(queryPt,K_T2, nnIdx,dists,eps);

	// Adding extra points
	if(((double) rand() / (RAND_MAX)) > 0.9){
	  double coords_extra_pts[Traits::D];
	  bool is_in_bbox = true;
	  for(int d = 0; d < D;d++){
	    //coords_extra_pts[d] = coords_new_pts[d] + scales[ii][Traits::D-1]*norms[ii][D-1][d]*6;
	    coords_extra_pts[d] = coords_new_pts[d] + scales[ii][Traits::D-1]*norms[ii][D-1][d]*6;
	    queryPt[d] = coords_extra_pts[d];
	    if(coords_extra_pts[d] > bbox_max[d] || coords_extra_pts[d] < bbox_min[d])
	      is_in_bbox = false;
	  }

	  if(is_in_bbox){
	    simp.push_back(traits.make_point(coords_extra_pts));
	  }
	}
      }
    }
  }else{
    // int acc = 0;
    // if(nbp*pscale < 10)
    //   pscale = 10/nbp;
    // for(int ii = 0; ii < nbp; ii++){
    //   if(acc++ % ((int)(1.0/(pscale)))  == 0){
    // 	simp.push_back(points[ii]);
    //   }	
    // }
    // if(simp.size() < 50){
    //   simp.clear();
    //   simp.insert(simp.end(),points.begin(),points.end());
    // }
      
  }
  std::cerr << "done!" << std::endl;
  delete kdTree;
  delete [] nnIdx;                                                  // clean things up
  delete [] dists;
  return 0;
  //std::cerr << "step3" << std::endl;

}


void wasure_algo::flip_dim_ori( std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<Point> &  ori){
  int nbp = points.size();
  int D = Traits::D;  


  int nb_flip = 0;
  for(int n = 0; n < nbp; n++){
    Point pts_norm = norms[n][D-1];
    Point pi  = ori[n];
    Point p1 = points[n];
    std::vector<double> pts_coefs = compute_base_coef<Point>(p1,pi,norms[n],D);
    if(pts_coefs[D-1] < 0){
      double coords_norm[Traits::D];
      for(int d = 0; d < D; d++){
	coords_norm[d] = -norms[n][D-1][d];
      }
      norms[n][D-1] = traits.make_point(coords_norm);
      nb_flip++;
    }else{

    }
  }
  std::cerr << "nb flips:" << nb_flip << "/" << nbp << std::endl;
}

void wasure_algo::flip_dim( std::vector<Point> & points, std::vector<std::vector<Point> > & norms, Point p1){
  int nbp = points.size();
  int D = Traits::D;  
  for(int n = 0; n < nbp; n++){
    Point pts_norm = norms[n][D-1];
    Point pi  = points[n];
    std::vector<double> pts_coefs = compute_base_coef<Point>(p1,pi,norms[n],D);
    if(pts_coefs[D-1] < 0){
      double coords_norm[Traits::D];
      for(int d = 0; d < D; d++){
	coords_norm[d] = -norms[n][D-1][d];
      }
      norms[n][D-1] = traits.make_point(coords_norm);
    }else{

    }
  }
}




// ====================================================
// ======================== DST =======================
// ====================================================




std::vector<double>  wasure_algo::Pick_2d(const Point & v0,const Point & v1,const Point & v2){
  int D=2;
  std::vector<double> coords(D);
  double r1 = (((double) rand() / (RAND_MAX)));
  double r2 = (((double) rand() / (RAND_MAX)));
  for(int d = 0; d < D; d++)
    coords[d] = (1 - sqrt(r1)) * v0[d] + (sqrt(r1) * (1 - r2)) * v1[d] + (sqrt(r1) * r2) * v2[d];

  return coords;

}

std::vector<double>  wasure_algo::Pick_3d(const Point & v0,const Point & v1,const Point & v2,const Point & v3) {
  int D = Traits::D;  
  double s = (((double) rand() / (RAND_MAX)));
  double t = (((double) rand() / (RAND_MAX)));
  double u = (((double) rand() / (RAND_MAX)));
  if(s+t>1.0) { // cut'n fold the cube into a prism
    s = 1.0 - s;
    t = 1.0 - t;
  }
  if(t+u>1.0) { // cut'n fold the prism into a tetrahedron
    double tmp = u;
    u = 1.0 - s - t;
    t = 1.0 - tmp;

  } else if(s+t+u>1.0) {
    double tmp = u;
    u = s + t + u - 1.0;
    s = 1 - t - tmp;
  }
  double a=1-s-t-u; // a,s,t,u are the barycentric coordinates of the random point.
  std::vector<double> coords(D);
  coords[0] = (v0[0]*a + v1[0]*s + v2[0]*t + v3[0]*u);
  coords[1] = (v0[1]*a + v1[1]*s + v2[1]*t + v3[1]*u);
  coords[2] = (v0[2]*a + v1[2]*s + v2[2]*t + v3[2]*u);
  return coords;

}




void wasure_algo::init_sample(DT & tri,int nb_samples, int dim){
  // std::cout << "init samples ...." << std::endl;
  // for( auto cit = tri.full_cells_begin();
  //      cit != tri.full_cells_end(); ++cit ){
  //   Cell_handle ch;
  //   cit->data().resize(nb_samples);
  //   if( tri.is_infinite(cit) ){

  //   }else{
  //     for(int x = 0; x < nb_samples; x++){
  // 	std::vector<double> & C = (x == 0) ? get_cell_barycenter(cit) : Pick(cit,dim);
  // 	cit->data().pts[x] = C;
  //     }
  //   }
  // }
}


void wasure_algo::finalize_sample(DT & tri, int nb_samples){
  //  std::cout << "finalize samples ...." << std::endl;
  // for( auto cit = tri.full_cells_begin();
  //      cit != tri.full_cells_end(); ++cit ){
  //   // if( tri.is_infinite(cit)  )
  //   //   continue;


  //   std::vector<double> & vpe = (cit->data()).vpe;
  //   std::vector<double> & vpo = (cit->data()).vpo;
  //   std::vector<double> & vpu = (cit->data()).vpu;


  //   double res_pe = 0, res_po = 0, res_pu = 0;
  //   for(int x = 0; x < nb_samples; x++){
  //     res_pe += vpe[x];
  //     res_po += vpo[x];
  //     res_pu += vpu[x];
  //   }

  //   if(res_pe != res_pe || res_po != res_po || res_pu != res_pu){
  //     std::cout << "/!\\ fch->data().lab NAN /!\\" << std::endl;
  //     res_pe = res_po = 0;
  //     res_pu = nb_samples;
  //   }
    
  //   (cit->data()).dat[0] = res_pe;
  //   (cit->data()).dat[1] = res_po;
  //   (cit->data()).dat[2] = res_pu;
  //   (cit->data()).dat[3] = nb_samples;
  // }
}

double get_min_scale(std::vector<double> &v)
{
  size_t n = v.size() / 10;
  return v[n];
}

double get_median_scale(std::vector<double> &v)
{
  size_t n = v.size()*3 / 4;
  return v[n];
}





void wasure_algo::ds_score(double v_e1,double v_o1,double v_u1,double v_e2,double v_o2,double v_u2,double & v_e3,double & v_o3,double & v_u3){
  double vK = v_o1*v_e2 + v_e1*v_o2;
  v_e3 = (v_e1*v_e2 + v_e1*v_u2 + v_u1*v_e2)/(1-vK);
  v_o3 = (v_o1*v_o2 + v_o1*v_u2 + v_u1*v_o2)/(1-vK);
  v_u3 = (v_u1*v_u2)/(1-vK);
}






void wasure_algo::compute_dst_mass_norm(std::vector<double> coefs, std::vector<double> scales, double coef_conf, double pdfs_e,double pdfs_o, double & v_e1, double & v_o1, double & v_u1){
  int D = scales.size();
  double c3 = coefs[D-1];
  double nscale = scales[D-1];
  if(nscale <= 0) nscale = 0.000001;
  if(c3 > 0){
    v_e1 =  1-0.5*(exp(-fabs(c3)/nscale));
    v_o1 = 0.5*(exp(-fabs(c3)/nscale));
  }else if (c3 < 0){
    v_e1 = 0.5*(exp(-fabs(c3)/nscale));
    v_o1 = 1-0.5*(exp(-fabs(c3)/nscale));
  }else{
    v_e1 = v_o1 = 0.5;
  }
  for(int d = 0; d < D-1; d++){
    if(scales[d] <= 0){
      v_e1 = v_o1 = 0;
    }else{
      // v_e1 = v_e1*exp(-fabs(coefs[d]/(scales[d]*3)));
      // v_o1 = v_o1*exp(-fabs(coefs[d]/(scales[d]*3)));
      v_e1 = v_e1*score_pdf(coefs[d],scales[d]/9);
      v_o1 = v_o1*score_pdf(coefs[d],scales[d]/9);
    }
  }

  v_e1 = v_e1*exp(-(fabs(c3)/(pdfs_e))*(fabs(c3)/(pdfs_e)));
  v_o1 = v_o1*exp(-(fabs(c3)/(pdfs_o))*(fabs(c3)/(pdfs_o)));
  v_e1 = v_e1*coef_conf;
  v_o1 = v_o1*coef_conf;

  v_u1 = 1-v_e1-v_o1;
  regularize(v_e1,v_o1,v_u1);

}





//void compute_dst_tri(DT & tri,std::vector<Point> points_dst,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales,int D){


double
get_conf_volume(const std::vector<double> & pts_scales, int dim){
  //double c1 = *std::max_element(pts_scales.begin(),pts_scales.end()--)/(*pts_scales.end());
  double c1 = MIN(pts_scales[dim-1]/pts_scales[0],1);
  return 1-c1;
}

void
wasure_algo::get_params_surface_dst(const std::vector<double> & pts_scales,double glob_scale,double min_scale, double & pdf_smooth,double & coef_conf, int D){

  double data_scale = *std::max_element(pts_scales.begin(),pts_scales.end());
  //double data_scale = 10;
  if(glob_scale > 0){
    pdf_smooth = glob_scale*3;

  }else{
    //double data_scale = *std::max_element(pts_scales.begin(),pts_scales.end());
    pdf_smooth = data_scale;
  }
  //  coef_conf = 1- (*std::min_element(pts_scales.begin(),pts_scales.end()))/(*std::max_element(pts_scales.begin(),pts_scales.end()));
  double mins = *std::min_element(pts_scales.begin(),pts_scales.end());
  double maxs = *std::max_element(pts_scales.begin(),pts_scales.end());
  double rat = (mins/maxs);
  coef_conf = 1;
  //coef_conf = exp(-(rat*rat)/0.05);
  //coef_conf = exp(-(rat*rat)/0.002);
  if(min_scale > 0)
    coef_conf = coef_conf*MIN(MAX(min_scale/data_scale,0.000001),1);//*get_conf_volume(pts_scales,D);

 
}


void
wasure_algo::get_params_conflict_dst(const std::vector<double> & pts_scales,double glob_scale,double min_scale, double & pdf_smooth,double & coef_conf, int D){
  double data_scale = *std::max_element(pts_scales.begin(),pts_scales.end());
  if(glob_scale > 0){
    pdf_smooth = glob_scale/3;
  }else{
    pdf_smooth = data_scale/3;
  }
  double mins = *std::min_element(pts_scales.begin(),pts_scales.end());
  double maxs = *std::max_element(pts_scales.begin(),pts_scales.end());
  double rat = (mins/maxs);

  //  coef_conf = exp(-(rat/0.01)*(rat/0.01));
  //  coef_conf = exp(-(rat*rat)/0.01);
  coef_conf = 1;//MIN(min_scale/data_scale,1);//*get_conf_volume(pts_scales,D);

}




void
wasure_algo::compute_dst_tri(DTW & tri, wasure_data<Traits>  & datas_tri, wasure_data<Traits>  & datas_pts, wasure_params & params){
  std::cerr << "    compute dst norm ..." << std::endl;
  std::vector<Point> & points_dst =  datas_pts.format_points;
  std::vector<std::vector<Point>> & norms = datas_pts.format_egv;
  std::vector<std::vector<double>> & scales = datas_pts.format_sigs;
  std::vector<int> & format_flags = datas_pts.format_flags;
  std::vector<Point> & centers = datas_pts.format_centers;
  std::vector<std::vector<double>> & v_dst = datas_tri.format_dst;

  std::cerr << points_dst.size() << " " << norms.size() << " " << scales.size() << " " << v_dst.size() << std::endl;
  
  int D = datas_pts.D;
  // ---------------------------------------
  //           KD-tree creation
  // --------------------------------------
  int nbp = points_dst.size();
  int K_T = 30;
  if(K_T >= nbp)
    K_T = nbp-1;
  double eps = 0;


  ANNpointArray	dataPts;
  ANNpoint	queryPt;
  ANNidxArray	nnIdx;
  ANNdistArray	dists;
  ANNkd_tree*	kdTree;

  queryPt = annAllocPt(D);
  dataPts = annAllocPts(nbp, D);
  nnIdx = new ANNidx[K_T];
  dists = new ANNdist[K_T];

  int npts =0;
  for(std::vector<Point>::iterator pit = points_dst.begin(); pit != points_dst.end(); ++pit){
    Point p1 = *pit;
    for(int d = 0; d < D; d++)
      dataPts[npts][d] = p1[d];
    npts++;
  }

  std::cerr << "    init" << std::endl;
  kdTree = new ANNkd_tree(dataPts,npts,D);

  std::vector<double> v_scale(scales.size());
  std::cerr << "print_scale:";
  for(int i = 0; i < scales.size() ; i++){
    v_scale[i] = scales[i][D-1];
    std::cerr << v_scale[i] << " ";
  }
  std::sort(v_scale.begin(), v_scale.end());

  std::cerr << "dst_scale : " << params.dst_scale << std::endl;
  if(params.dst_scale < 0){
    //    params.min_scale = get_min_scale(v_scale);
    params.min_scale = -1;
    // for(auto ss : v_scale)
    //   std::cerr << "sss:" << ss << std::endl;
  }
  else{
    params.min_scale = params.dst_scale;
  }


  // ---------------------------------------
  //           DST computation
  // --------------------------------------
  bool do_debug = false;
  std::cerr << "    dst computation" << std::endl;




  int debug_acc=0;  
  int accid = 0;
  for( auto cit = tri.cells_begin();
       cit != tri.cells_end(); ++cit ){
    // Cell_handle ch = cit;
    if( cit->is_infinite() )
      continue;
 
    int cid = cit->lid();

    double  vpe = v_dst[cid][0];
    double  vpo = v_dst[cid][1];
    double  vpu = v_dst[cid][2];


    //Find close ray

    // std::vector<int> ray_id;

    // std::vector<double> C =   cit->barycenter();
    // Point  PtSample = traits.make_point(C.begin());
    // for(int id = 0;  id < points_dst.size();id++){
    // 	Point & Pt3d = points_dst[id];
    // 	Point & PtCenter = centers[id];
    // 	double angle = compute_angle_rad(PtSample,Pt3d,PtCenter,D);
    // 	if(angle < 0.01){
    // 	  ray_id.push_back(id);
    // 	}
    // }
    


    //    std::cerr << "    id : " << vpe << " " << "vop:" << vpo <<  std::endl;
    double pe_acc=0;
    double po_acc=0;
    double pu_acc=0;
    for(int x = 0; x < params.nb_samples; x++){
      //Point PtSample = traits.make_point((cit->data()).pts[x].begin());
      std::vector<double>  C =  (x == 0) ? cit->barycenter() : Pick(cit->full_cell(),D);
      //std::vector<double>  C = (x == 0) ? cit->barycenter() : Pick(cit->full_cell(),D);
      Point  PtSample = traits.make_point(C.begin());


      if(C[0] > 4100 && C[0] < 4102 &&
	 C[1] > 4987 && C[1] < 4988 &&
	 C[2] > 150.22 && C[2] < 150.8){
	do_debug = false;
	debug_acc++;
      }
      else
	do_debug = false;

      // // ================= RAY DST =======================
      // //std::cerr << "NBRAY:" << ray_id.size() << std::endl;
      // for(auto rid : ray_id){
      //   double pe1,po1,pu1,pe2,po2,pu2;
      //   Point & Pt3d = points_dst[rid];
      //   Point & PtCenter = centers[rid];
      //   std::vector<Point> & pts_norm = datas_pts.format_egv[rid];
      //   std::vector<double> & pts_scale = datas_pts.format_sigs[rid];
      //   pe1 = vpe;
      //   po1 = vpe;
      //   pu1 = vpe;	  
      //   std::vector<double> center_coefs = compute_base_coef<Point>(Pt3d,PtCenter,pts_norm,D);
      //   std::vector<double> pts_coefs = compute_base_coef<Point>(Pt3d,PtSample,pts_norm,D);
      //   double angle = compute_angle_rad(PtSample,Pt3d,PtCenter,D);

      //   double pdf_smooth = -1;
      //   double coef_conf = -1;
      //   double gbl_scale = -1;// (format_glob_scale.size() > 0)  ? format_glob_scale[rid] : -1;
      //   get_params_surface_dst(pts_scale,gbl_scale,params.min_scale,pdf_smooth,coef_conf,D);

      //   compute_dst_mass_beam(pts_coefs,pts_scale,angle,ANGLE_SCALE,coef_conf,pe2,po2,pu2);
      //   ds_score(pe1,po1,pu1,pe2,po2,pu2,pe1,po1,pu1);
      //   regularize(pe1,po1,pu1);
      //   // NAN check
      //   if(pe1 == pe1 &&
      //      po1 == po1 &&
      //      pu1 == pu1){
      //     vpe = pe1;
      //     vpo = po1 ;
      //     vpu = pu1 ;
      //   }	
      // }
      
    
      
      // ================= NORM DST =======================
      for(int d = 0; d < D; d++){
	queryPt[d] = PtSample[d];
      }

      std::ofstream myfile;
      if(do_debug){

	std::string filename("/home/laurent/shared_spark/tmp/bary_" + std::to_string(debug_acc) + ".ply");
      
	myfile.open (filename);
	myfile << "ply" <<  std::endl;
	myfile << "format ascii 1.0" << std::endl;
	myfile << "element vertex "  << std::to_string(K_T + 1)  << std::endl;
	myfile << "property float x" << std::endl;
	myfile << "property float y" << std::endl;
	myfile << "property float z" << std::endl;
	myfile << "property float nx" << std::endl;
	myfile << "property float ny" << std::endl;
	myfile << "property float nz" << std::endl;
	myfile << "property uchar red                   { start of vertex color }" << std::endl;
	myfile << "property uchar green" << std::endl;
	myfile << "property uchar blue" << std::endl;
	myfile << "end_header" << std::endl;

      

	std::cerr << "=============" <<  std::endl;
	std::cerr << "debug acc " << debug_acc <<  std::endl;
	std::cerr << "id : " << cid <<  std::endl;
	std::cerr << "bary  :";
	for(int d = 0; d < D; d++){
	  std::cerr << C[d] << " ";
	}
	std::cerr << std::endl;
      }
      
      kdTree->annkSearch(queryPt,K_T, nnIdx,dists,eps);
      vpe = vpo = 0;
      vpu = 1;
      for(int k = 0; k < K_T; k++){
       
  	double pe1,po1,pu1,pe2,po2,pu2;
  	int idx = nnIdx[k];
  	Point Pt3d = points_dst[idx];
  	std::vector<Point> pts_norms = norms[idx];
  	std::vector<double> pts_scales = scales[idx];

	if(do_debug){
	  std::cerr << "----------- " << std::endl;
	  std::cerr << "   kidx : " << idx << std::endl;
	  std::cerr << "   pt3D  :";
	  for(int d = 0; d < D; d++){
	    std::cerr << Pt3d[d] << " ";
	    myfile << Pt3d[d] << " ";
	  }
	  std::cerr << std::endl;
	  std::cerr << "   Norm  :";
	  for(int d = 0; d < D; d++){
	    std::cerr << pts_norms[D-1][d] << " ";
	    myfile << pts_norms[D-1][d] << " ";
	  }
	  myfile << " 0 0 0 " << std::endl;
	  std::cerr << std::endl;
	  std::cerr << "   scale  :";
	  for(int d = 0; d < D; d++){
	    std::cerr << pts_scales[d] << " ";
	  }
	  std::cerr << std::endl;
	}

	if(((int)format_flags[idx]) < 0)
	  continue;
	
  	double pdf_smooth = -1;
  	double coef_conf = -1;
	double gbl_scale = -1; 
	//	std::cerr << "glob scale:" << gbl_scale << std::endl;
	if(params.mode == std::string("surface")){
	  get_params_surface_dst(pts_scales,gbl_scale,params.min_scale,pdf_smooth,coef_conf,D);
	  std::vector<double> pts_coefs = compute_base_coef(Pt3d,PtSample,pts_norms,D);

	  if(params.dst_scale > 0){
	    if(((int)format_flags[idx]) > 0){
	      coef_conf = 0.4*coef_conf;
	    }
	  }
	  compute_dst_mass_norm(pts_coefs,pts_scales,coef_conf,pdf_smooth,pdf_smooth,pe2, po2,pu2);
	}else if(params.mode == std::string("conflict")){
	  // get_params_surface_dst(pts_scales,gbl_scale,params.min_scale,pdf_smooth,coef_conf,D);
	  // std::vector<double> pts_coefs = compute_base_coef(Pt3d,PtSample,pts_norms,D);
	  // compute_dst_mass_norm(pts_coefs,pts_scales,coef_conf, pdf_smooth,pe2, po2,pu2);

	}



  	pe1=vpe;
  	po1=vpo;
  	pu1=vpu;

	// std::cerr << "  K  "<< k << " " << pe1 << " " << po1 << " " << pu1 <<  std::endl;
	// std::cerr << "  K  "<< k << " " << pe2 << " " << po2 << " " << pu2 <<  std::endl;

  	ds_score(pe1,po1,pu1,pe2,po2,pu2,pe1,po1,pu1);
  	regularize(pe1,po1,pu1);

	if(do_debug){
	  for(int d = 0; d < D; d++){
	    std::cerr << Pt3d[d] << " ";
	  }
	  std::cerr << std::endl;
	  std::cerr << "  pe2  "<< k << " " << pe2 << " " << po2 << " " << pu2 <<  std::endl;
	  std::cerr << "  pe1  "<< k << " " << pe1 << " " << po1 << " " << pu1 <<  std::endl;
	  std::cerr << "  new  "<< k << " " << vpe << " " << vpo << " " << vpu <<  std::endl;
	  std::cerr << " " << std::endl;

	  for(int d = 0; d < D; d++){
	    std::cerr << C[d] << " ";
	    myfile << C[d] << " ";
	  }
	  myfile << " 0 0 0 " << ((int)(pe1*255)) << " " << ((int)(po1*255)) << " " << ((int)(pu1*255)) << std::endl;
	  
	}

      vpe = pe1;
      vpo = po1;
      vpu = pu1;	         
      }

      pe_acc += vpe;
      po_acc += vpo;
      pu_acc += vpu;	  
      if(do_debug)
	myfile.close();

      
    }
    //std::cerr << "  cid  "<< cid << " " << vpe << " " << vpo << " " << vpu <<  std::endl;

    // NAN check
    if(vpe == vpe &&
       vpo == vpo &&
       vpu == vpu){
      v_dst[cid][0] = pe_acc/params.nb_samples;
      v_dst[cid][1] = po_acc/params.nb_samples;
      v_dst[cid][2] = pu_acc/params.nb_samples ;
    }else{
      // v_dst[cid][0] = v_dst[cid][1] = 0;
      // v_dst[cid][2] = 1;
      std::cerr << "NAN ERROR DST" << std::endl;
    }
  }
  std::cerr << "    done" << std::endl;
}

// Point_3 inline pointd23(const Point  & a){
//   return Point_3(a[0],a[1],a[2]);
// }

// double
// wasure_algo::compute_angle_rad_3D(const Point_3 & a, const Point_3 & b, const Point_3 & c ){

//   CGAL::Vector_3<K3> v1 = c - a;
//   CGAL::Vector_3<K3> v2 = c - b;
//   double cosine = v1 * v2 / CGAL::sqrt(v1*v1) / CGAL::sqrt(v2 * v2);
//   return std::acos(cosine);
// }

// double
// wasure_algo::compute_angle_rad_2D(const Point & a, const Point & b, const Point & c ){
//   std::vector<double> v1 = {c[0]-a[0], c[1]-a[1]};
//   std::vector<double> v2 = {c[0]-b[0], c[1]-b[1]};
//   double cosine = (v1[0]*v2[0]+v1[1]*v2[1])/sqrt(v1[0]*v1[0]+v1[1]*v1[1])/sqrt(v2[0]*v2[0]+v2[1]*v2[1]);
//   return std::acos(cosine);

// }

double
wasure_algo::compute_angle_rad(const Point & a, const Point & b, const Point & c , int dim){
  std::vector<double> v1(dim);
  std::vector<double> v2(dim);
  for(int d = 0; d < dim; d++){
    v1[d] = c[d]-a[d];
    v2[d] = c[d]-b[d];
  }
  double num=0,accv1=0,accv2=0;
  for(int d = 0; d < dim; d++){
    num += v1[d]*v2[d];
    accv1 += v1[d]*v1[d];
    accv2 += v2[d]*v2[d];
  }
  double cosine = num/sqrt(accv1)/sqrt(accv2);
  return std::acos(cosine);

}




void
wasure_algo::compute_dst_mass_beam(std::vector<double> & coefs, std::vector<double> & scales,double angle,double angle_scale, double coef_conf, double & v_e1, double & v_o1, double & v_u1){

  int D = scales.size();
  double sig = scales[D-1];
  double c3 = coefs[D-1];
  double nscale = scales[D-1];
  double score = fabs((angle/angle_scale)*(angle/angle_scale));
  if(c3 > 0){
    v_o1 =  0;
    v_e1 = 1-0.5*score_pdf(fabs(c3),sig);
    v_e1 = v_e1*exp(-score);

  }else{
    v_o1 = 1-0.5*score_pdf(fabs(c3),sig);
    v_e1 =  0;
    v_o1 = v_e1*exp(-score);
  }
  v_u1 = 1-v_e1-v_o1;

  v_e1 = v_e1*coef_conf;
  v_o1 = v_o1*coef_conf;

  // if(v_u1 != 1)
  //   std::cerr << " massbeam" << v_e1 << " " << v_o1 << " " << v_u1 << std::endl;
  
  regularize(v_e1,v_o1,v_u1);
  // std::cerr << std::endl;
}


//#ifdef DDT_CGAL_TRAITS_D

// std::vector<double> get_cell_barycenter_new(Cell_handle ch,int D)
//     {
//         std::vector<double> coords;// = get_vect_barycenter(ch);
//         for(uint d = 0; d < D; d++)
//             coords[d] = 0;
//         for(auto vht = ch->vertices_begin() ;
//                 vht != ch->vertices_end() ;
//                 ++vht)
//         {
//             Vertex_handle v = *vht;
//             for(uint d = 0; d < D; d++)
//             {
//                 coords[d] += (v->point())[d];
//             }
//         }
//         for(uint d = 0; d < D; d++)
//             coords[d] /= ((double)D+1);
//         return coords;
//     }

//#endif

void
wasure_algo::sample_cell(Cell_handle & ch,Point &  Pt3d, Point & PtCenter, wasure_data<Traits>  & datas_tri, wasure_data<Traits>  & datas_pts, wasure_params & params, int rid, int dim){
  Id cid = traits.gid(ch);

  
  std::vector<Point> & pts_norm = datas_pts.format_egv[rid];
  std::vector<double> & pts_scale = datas_pts.format_sigs[rid];
  std::vector<std::vector<double>> & v_dst = datas_tri.format_dst;
  std::vector<int> & format_flags = datas_pts.format_flags;
  // if(cid < 0 || cid > v_dst.size())
  //   return;

  
  // //  if((ch->data()).seg == cid) return;
  // std::vector<double> & vpe = (ch->data()).vpe;
  // std::vector<double> & vpo = (ch->data()).vpo;
  // std::vector<double> & vpu = (ch->data()).vpu;
  Traits  traits;
 
  for(int x = 0; x < params.nb_samples; x++){
    std::vector<double>  C = (x == 0) ? traits.get_cell_barycenter(ch) : Pick(ch,D);
    Point  PtSample = traits.make_point(C.begin());
    double pe1,po1,pu1,pe2,po2,pu2;

    pe1 = v_dst[cid][0];
    po1 = v_dst[cid][1];
    pu1 = v_dst[cid][2];

	  
    //    std::vector<double> center_coefs = compute_base_coef<Point>(Pt3d,PtCenter,pts_norm,dim);
    std::vector<double> pts_coefs = compute_base_coef<Point>(Pt3d,PtSample,pts_norm,dim);
    double angle = compute_angle_rad(PtSample,Pt3d,PtCenter,dim);

    double pdf_smooth = -1;
    double coef_conf = -1;
    double gbl_scale = -1;//(datas.glob_scale.size() > 0)  ? datas.glob_scale[cid] : -1;
    get_params_surface_dst(pts_scale,gbl_scale,params.min_scale,pdf_smooth,coef_conf,dim);

    // if(((int)format_flags[rid]) > 0)
    //   coef_conf = 0.2*coef_conf;
	  
    
    compute_dst_mass_beam(pts_coefs,pts_scale,angle,ANGLE_SCALE,coef_conf,pe2,po2,pu2);
    ds_score(pe1,po1,pu1,pe2,po2,pu2,pe1,po1,pu1);
    regularize(pe1,po1,pu1);

    
  // NAN check
    if(pe1 == pe1 &&
       po1 == po1 &&
       pu1 == pu1){
      v_dst[cid][0] = pe1;
      v_dst[cid][1] = po1 ;
      v_dst[cid][2] = pu1 ;
    }

  }


}




Cell_handle wasure_algo::walk_locate(DT & tri,
				     Point & Pt3d,  Point & Ptcenter, Point & Ptcenter_mir,
				     wasure_data<Traits>  & datas_tri,
				     wasure_data<Traits>  & datas_pts,
				     wasure_params & params,
				     int idr,
				     Cell_handle & start_cell
				     )
{


#if defined(DDT_CGAL_TRAITS_D)
  
  std::cerr << "start walk begin" << std::endl;
  typedef typename DT::Face Face;
  DT::Locate_type  loc_type;// type of result (full_cell, face, vertex)
  Face face(tri.maximal_dimension());// the face containing the query in its interior (when appropriate)
  DT::Facet  facet;// the facet containing the query in its interior (when appropriate)

  std::cerr << "start locate" << std::endl;
  Cell_handle start = tri.locate(Ptcenter,start_cell);
  std::cerr << "located" << std::endl;
  start_cell = start;
  DT::Geom_traits geom_traits;
  CGAL::Random                      rng_;
  Traits::K::Orientation_d orientation_pred = geom_traits.orientation_d_object();

  int cur_dim = tri.current_dimension();
  std::vector<CGAL::Oriented_side>  orientations_(cur_dim+1);

  std::cerr << "start walk" << std::endl;


  if( cur_dim == -1 )
    {
      loc_type = DT::OUTSIDE_AFFINE_HULL;
      return Cell_handle();
    }
  else if( cur_dim == 0 )
    {
      Vertex_handle vit = tri.infinite_full_cell()->neighbor(0)->vertex(0);
      if( CGAL::EQUAL != geom_traits.compare_lexicographically_d_object()(Ptcenter_mir, vit->point()) )
        {
	  loc_type = DT::OUTSIDE_AFFINE_HULL;
	  return Cell_handle();
        }
      else
        {
	  loc_type = DT::ON_VERTEX;
	  face.set_full_cell(vit->full_cell());
	  face.set_index(0, 0);
	  return vit->full_cell();
        }
    }

   Cell_handle s;

  // if we don't know where to start, we start from any bounded full_cell
  if( Cell_handle() == start )
    {
      // THE HACK THAT NOBODY SHOULD DO... BUT DIFFICULT TO WORK AROUND
      // THIS... TODO: WORK AROUND IT
      Cell_handle inf_c = tri.infinite_full_cell();
      int inf_v_index = inf_c->index(tri.infinite_vertex());
      s = inf_c->neighbor(inf_v_index);
    }
  else
    {
      s = start;
      if( tri.is_infinite(s) )
        {
	  int inf_v_index = s->index(tri.infinite_vertex());
	  s = s->neighbor(inf_v_index);
        }
    }

  // Check if query |p| is outside the affine hull
  // if( cur_dim < tri.maximal_dimension() )
  // {
  //     if( ! geom_traits.contained_in_affine_hull_d_object()(
  // 							      Point_const_iterator(s->vertices_begin()),
  // 							      Point_const_iterator(s->vertices_end()) + tri.current_dimension() + 1,
  // 							      p) )
  //     {
  // 	  loc_type = DT::OUTSIDE_AFFINE_HULL;
  //         return Cell_handle();
  //     }
  // }

  // we remember the |previous|ly visited full_cell to avoid the evaluation
  // of one |orientation| predicate
  Cell_handle previous = Cell_handle();
  bool full_cell_not_found = true;
  while(full_cell_not_found) // we walk until we locate the query point |p|
    {
#ifdef CGAL_TRIANGULATION_STATISTICS
      ++walk_size_;
#endif
      // For the remembering stochastic walk, we need to start trying
      // with a random index:
      int j, i = rng_.get_int(0, cur_dim);
      // we check |p| against all the full_cell's hyperplanes in turn

      for(j = 0; j <= cur_dim; ++j, i = (i + 1) % (cur_dim + 1) )
        {
	  Cell_handle next = s->neighbor(i);
	  if( previous == next )
            {   // no need to compute the orientation, we already know it
	      orientations_[i] = CGAL::POSITIVE;
	      continue; // go to next full_cell's facet
            }

	  CGAL::Substitute_point_in_vertex_iterator<
	    DT::Full_cell::Vertex_handle_const_iterator>
	    spivi(s->vertex(i), &Ptcenter_mir);

	  orientations_[i] = orientation_pred(boost::make_transform_iterator(s->vertices_begin(), spivi), boost::make_transform_iterator(s->vertices_begin() + cur_dim + 1, spivi));

	  // orientations_[i] = 	K::orientation_d_object(
	  //   boost::make_transform_iterator(s->vertices_begin(), spivi),
	  //   boost::make_transform_iterator(s->vertices_begin() + cur_dim + 1,
	  //                                  spivi));



	  if( orientations_[i] != CGAL::NEGATIVE )
            {
	      // from this facet's point of view, we are inside the
	      // full_cell or on its boundary, so we continue to next facet
	      continue;
            }

	  // At this point, we know that we have to jump to the |next|
	  // full_cell because orientation_[i] == NEGATIVE
	  previous = s;
	  s = next;

	  if( tri.is_infinite(next) ){
	    // we have arrived OUTSIDE the convex hull of the triangulation,
	    // so we stop the search
	    full_cell_not_found = false;
	    loc_type = DT::OUTSIDE_CONVEX_HULL;
	    face.set_full_cell(s);
	  }else{
	    sample_cell(s,Pt3d,Ptcenter,datas_tri,datas_pts,params,idr, D);
	    for(int ii = 0; ii <= cur_dim; ii++){
	      Cell_handle s_nbr = s->neighbor(ii);
	      if(!tri.is_infinite(s_nbr))
		sample_cell(s_nbr,Pt3d,Ptcenter,datas_tri,datas_pts,params,idr, D);
	    }
	  }


	  break;
        } // end of the 'for' loop
      if( ( cur_dim + 1 ) == j ) // we found the full_cell containing |p|
	full_cell_not_found = false;
    }
  // Here, we know in which full_cell |p| is in.
  // We now check more precisely where |p| landed:
  // vertex, facet, face or full_cell.
  if( ! tri.is_infinite(s) )
    {
      face.set_full_cell(s);
      int num(0);
      int verts(0);
      for(int i = 0; i < cur_dim; ++i)
        {
	  if( orientations_[i] == CGAL::COPLANAR )
            {
	      ++num;
	      facet = DT::Facet(s, i);
            }
	  else
	    face.set_index(verts++, i);
        }
      //-- We could put the if{}else{} below in the loop above, but then we would
      // need to test if (verts < cur_dim) many times... we do it only once
      // here:
      if( orientations_[cur_dim] == CGAL::COPLANAR )
        {
	  ++num;
	  facet = DT::Facet(s, cur_dim);
        }
      else if( verts < cur_dim )
	face.set_index(verts, cur_dim);
      //-- end of remark above //
      if( 0 == num )
        {
	  loc_type = DT::IN_FULL_CELL;
	  face.clear();
        }
      else if( cur_dim == num )
	loc_type = DT::ON_VERTEX;
      else if( 1 == num )
	loc_type = DT::IN_FACET;
      else
	loc_type = DT::IN_FACE;
    }
  return s;
#endif

  
#if defined(DDT_CGAL_TRAITS_3)

  int n_of_turns = 10000;
  if(tri.dimension() < 3)
    return start_cell;

  Cell_handle start = tri.locate(Ptcenter,start_cell);

  start_cell = start;
  
  // Make sure we continue from here with a finite cell.
  if(start == Cell_handle())
    start = tri.infinite_cell();



  int ind_inf;
  if(start->has_vertex(tri.infinite_vertex(), ind_inf))
    start = start->neighbor(ind_inf);

    CGAL_triangulation_precondition(start != Cell_handle());
  //     CGAL_triangulation_precondition(! start->has_vertex(infinite));

  // We implement the remembering visibility walk.
  // In this phase, no need to be stochastic

  // Remembers the previous cell to avoid useless orientation tests.
  Cell_handle previous = Cell_handle();
  Cell_handle c = start;


  // Now treat the cell c.
try_next_cell:
  n_of_turns--;


  
  // We know that the 4 vertices of c are positively oriented.
  // So, in order to test if p is seen outside from one of c's facets,
  // we just replace the corresponding point by p in the orientation
  // test.  We do this using the array below.
  const Point* pts[4] = { &(c->vertex(0)->point()),
                          &(c->vertex(1)->point()),
                          &(c->vertex(2)->point()),
                          &(c->vertex(3)->point()) };


  
  // (non-stochastic) visibility walk
  for(int i=0; i != 4; ++i)
  {
    Cell_handle next = c->neighbor(i);
    if(previous == next) continue;

    if(!next->has_vertex(tri.infinite_vertex()))
      sample_cell(next,Pt3d,Ptcenter,datas_tri,datas_pts,params,idr, D);
    
    // We temporarily put p at i's place in pts.
    const Point* backup = pts[i];
    pts[i] = &Ptcenter_mir;
    if(tri.inexact_orientation(*pts[0], *pts[1], *pts[2], *pts[3]) != CGAL::NEGATIVE)
    {
      pts[i] = backup;
      continue;
    }

    if(next->has_vertex(tri.infinite_vertex()))
    {
      // We are outside the convex hull.
      return start_cell;
    }


    // for(int ii = 0; ii <= cur_dim; ii++){
    //   Cell_handle s_nbr = s->neighbor(ii);
    //   if(!tri.is_infinite(s_nbr))
    // 	sample_cell(s_nbr,Pt3d,Ptcenter,datas_tri,datas_pts,params,idr, D);
    
    previous = c;
    c = next;


    if(n_of_turns) goto try_next_cell;
  }

#endif
  return start_cell;  
}


void wasure_algo::center_dst(DTW & ttri, wasure_data<Traits>  & datas_tri,std::vector<Point> & center_pts, Id tid){

  auto & tri = ttri.get_tile(tid)->tri(); 
  auto  tile = ttri.get_tile(tid);
  std::vector<std::vector<double>> & v_dst = datas_tri.format_dst;
  Cell_handle cloc =  Cell_handle();
  Cell_handle cloc_start =  Cell_handle();
  for(auto pit : center_pts){
    Cell_handle cloc = tri.locate(pit,cloc_start);
    cloc_start = cloc;
    // Quick and dity hack
    int cid = tile->lid(cloc);

    double pe1,po1,pu1,pe2,po2,pu2;
    pe1 = v_dst[cid][0];
    po1 = v_dst[cid][1];
    pu1 = v_dst[cid][2];
    pe2 = 0.05;
    po2 = 0;
    pu2 = 0.95;
    ds_score(pe1,po1,pu1,pe2,po2,pu2,pe1,po1,pu1);
    regularize(pe1,po1,pu1);
    
    v_dst[cid][0] = pe1;
    v_dst[cid][1] = po1;
    v_dst[cid][2] = pu1;
  }

}


//void compute_dst_ray(DT & tri,std::vector<Point> & points,std::vector<Point> & centers,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales,double rat_ray_sample, int D){

void wasure_algo::compute_dst_ray(DT & tri, wasure_data<Traits>  & datas_tri,wasure_data<Traits>  & datas_pts, wasure_params & params){
  std::cerr << "    compute dst ray ..." << std::endl;
  Traits traits;
  std::vector<Point> & points = datas_pts.format_points;
  std::vector<Point> & centers = datas_pts.format_centers;
  std::vector<std::vector<Point>> & norms = datas_pts.format_egv;
  std::vector<std::vector<double>> & scales = datas_pts.format_sigs;
  std::vector<int> & format_flags = datas_pts.format_flags;
  double rat_ray_sample = params.rat_ray_sample;
  if(rat_ray_sample == 0)
    return;
  int D = Traits::D;  


  int nb_pts = points.size();
  int acc = 0;
  int max_walk = 100000;
  Cell_handle  start_walk = Cell_handle();
  for(int n = 1 ; n < nb_pts; n++){

    if(acc++ % ((int)(1.0/(rat_ray_sample)))  == 0){
      Point & Pt3d = points[n];
      Point & Ptcenter = centers[n];
      double coords[Traits::D];
      for(int d = 0;d < D;d++)
	coords[d] = Pt3d[d] -(Ptcenter[d] - Pt3d[d]);
      Point Ptcenter_mir = traits.make_point(coords);
      //std::cerr << n << " -- pt:" << Pt3d << " -- center:" << Ptcenter << std::endl;
      //Cell_handle loc = walk_locate(tri,Pt,Ptcenter,norms[n],scales[n],acc++);
      if(((int)format_flags[n]) ==0){
	walk_locate(tri,Pt3d,Ptcenter,Pt3d,datas_tri,datas_pts,params,n,start_walk);
      }else if(((int)format_flags[n]) ==1){
	walk_locate(tri,Pt3d,Ptcenter,Ptcenter_mir,datas_tri,datas_pts,params,n,start_walk);
      }
    }
  }
}


void wasure_algo::compute_dst_with_center(DTW & tri, wasure_data<Traits>  & datas_tri, wasure_data<Traits>  & datas_pts, wasure_params & params, Id tid){

  //std::vector<Point> & points_3d,std::vector<Point> & centers,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales, double rat_ray_sample, int dim){

  compute_dst_tri(tri,datas_tri,datas_pts,params);
  DT & tri_tile  = tri.get_tile(tid)->triangulation();
  //compute_dst_ray(tri_tile,datas_tri,datas_pts,params);
  center_dst(tri,datas_tri,datas_pts.format_centers,tid);
} 



void wasure_algo::extract_surface(DTW & tri, std::map<Id,wasure_data<Traits> >  & w_datas_tri){

  int dim = Traits::D;  
  
  std::vector<Point>  format_points;
  std::vector<int> v_simplex;
  std::map<Vertex_const_iterator, uint> vertex_map;
  int acc = 0;
  std::cerr << " extract step1" << std::endl;
  for(auto fit = tri.facets_begin(); fit != tri.facets_end(); fit++){
    Cell_const_iterator fch = fit.full_cell();
    int id_cov = fit.index_of_covertex();
    Cell_const_iterator fchn = fch->neighbor(id_cov);    
    //Vertex_h_iterator vht;

    // if(fch->is_infinite() && fchn->is_infinite())
    //   continue;
	
    // if(!fit->is_local())
    //   continue;

    // if(fit->main_id() != tid)
    //   continue;

    // if(fit->is_infinite())
    //   continue;

    
    int cccid = fch->cell_data().id;
    int cccidn = fchn->cell_data().id;


    if( w_datas_tri.find(fch->main_id()) == w_datas_tri.end() ||
	w_datas_tri.find(fchn->main_id()) == w_datas_tri.end()
	){
	continue;
    }
    
    int ch1lab = w_datas_tri[fch->main_id()].format_labs[cccid];
    int chnlab = w_datas_tri[fchn->main_id()].format_labs[cccidn];

    
    if(ch1lab != chnlab){
      for(int i = 0; i < dim+1; ++i){
	if(i != id_cov){
	  Vertex_const_iterator v = fch->vertex(i);	  
	  if(vertex_map.find(v) == vertex_map.end() && v->is_main() ){
	    vertex_map[v] = acc++;
	    format_points.push_back(v->point());
	  }
	}
      }
    }
    
  }

  std::cerr << " extract step2" << std::endl;
  for(auto mit = vertex_map.begin(); mit != vertex_map.end();mit++){
    //format_points.push_back(mit->first->point());
  }

  std::cerr << " extract step3" << std::endl;
  for(auto fit = tri.facets_begin(); fit != tri.facets_end(); fit++){
    Cell_const_iterator fch = fit.full_cell();
    int id_cov = fit.index_of_covertex();
    Cell_const_iterator fchn = fch->neighbor(id_cov);    
    //    Vertex_h_iterator vht;
    int cccid = fch->cell_data().id;
    int cccidn = fchn->cell_data().id;

    // if(fch->is_infinite() && fchn->is_infinite())
    //   continue;

   
    // if(!fch->is_main() || !fchn->is_main())
    //   continue;

    // if(!fit->is_local())
    //   continue;


    
    if( w_datas_tri.find(fch->main_id()) == w_datas_tri.end() ||
	w_datas_tri.find(fchn->main_id()) == w_datas_tri.end()
	){
	continue;
    }

    bool do_skip = false;
    // for(int i = 0; i < dim+1; ++i){
    //   for(int j = 0; j < dim+1; ++j){
    // 	if(vertex_map[fch->vertex(i)] == 0)
    // 	  do_skip = true;
    // 	if(i != j && vertex_map[fch->vertex(i)] == vertex_map[fch->vertex(j)])
    // 	  do_skip = true;
    //   }
    // }

    
    int ch1lab = w_datas_tri[fch->main_id()].format_labs[cccid];
    int chnlab = w_datas_tri[fchn->main_id()].format_labs[cccidn]; 


    
    if(ch1lab != chnlab && !do_skip){
      for(int i = 0; i < dim+1; ++i){
	if(i != id_cov){
	  Vertex_const_iterator v = fch->vertex(i);
	  v_simplex.push_back(vertex_map[v]);
	}
      }
    }
  }

  std::cerr << " extract step4" << std::endl;
  ddt_data<Traits> datas_out;

  std::cerr << "==>" << format_points.size() << " " << v_simplex.size() << " " << std::endl;
  datas_out.dmap[datas_out.xyz_name] = ddt_data<Traits>::Data_ply(datas_out.xyz_name,"vertex",dim,dim,DATA_FLOAT_TYPE);
  datas_out.dmap[datas_out.simplex_name] = ddt_data<Traits>::Data_ply(datas_out.simplex_name,"face",dim,dim,tinyply::Type::INT32);
  datas_out.dmap[datas_out.xyz_name].fill_full_uint8_vect(format_points);
  datas_out.dmap[datas_out.simplex_name].fill_full_uint8_vect(v_simplex);

}
