//#include "utils.hpp"
#include "ANN/ANN.h"
#include "wasure_maths.hpp"

#include "wasure_algo.hpp"
#include "input_params.hpp"





int 
wasure_algo::simplify(std::vector<Point> & points, std::vector<bool> & do_keep, double dist ){
  int D = Traits::D;  

  int nbp = points.size();
  double eps = 0;
  int K_T = 100;
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

  for(int d = 0; d < D; d++){
    coords_scale[d] = sv(d);
    if(coords_scale[d] <= 0)
      coords_scale[d] = 0.000001;
  }
}



int 
wasure_algo::compute_dim(  std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales){
  int D = Traits::D;  

  int nbp = points.size();
  double eps = 0;
  int K_T = 50;
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
      std::cerr << "entropy:" << entropy << std::endl;
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
wasure_algo::compute_dim_with_simp(  std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales,std::vector<Point> & simp,double pscale){
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

  
  if(pscale > 0){
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
	for(int d = 0; d < D;d++){
	  coords_new_pts[d] /= ww_acc;
	}
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
    for(int ii = 0; ii < nbp; ii++){
      simp.push_back(points[ii]);
    }
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






void wasure_algo::compute_dst_mass_norm(std::vector<double> coefs, std::vector<double> scales, double coef_conf, double pdfs, double & v_e1, double & v_o1, double & v_u1){
  int D = scales.size();
  double c3 = coefs[D-1];
  double nscale = scales[D-1];
  double pdf_smooth =  pdfs;
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

  v_e1 = v_e1*exp(-(fabs(c3)/(pdf_smooth))*(fabs(c3)/(pdf_smooth)));
  v_o1 = v_o1*exp(-(fabs(c3)/(pdf_smooth))*(fabs(c3)/(pdf_smooth)));
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
  coef_conf = exp(-(rat*rat)/0.01);
  //  coef_conf = exp(-(rat*rat)/0.002);
  //coef_conf = 1;//MIN(min_scale/data_scale,1);//*get_conf_volume(pts_scales,D);

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
  coef_conf = exp(-(rat*rat)/0.01);
  //coef_conf = 1;//MIN(min_scale/data_scale,1);//*get_conf_volume(pts_scales,D);

}




void
wasure_algo::compute_dst_tri(DTW & tri, wasure_data<Traits>  & datas_tri, wasure_data<Traits>  & datas_pts, wasure_params & params){
  std::cerr << "    compute dst norm ..." << std::endl;
  std::vector<Point> & points_dst =  datas_pts.format_points;
  std::vector<std::vector<Point>> & norms = datas_pts.format_egv;
  std::vector<std::vector<double>> & scales = datas_pts.format_sigs;
  std::vector<std::vector<double>> & v_dst = datas_tri.format_dst;

  std::cerr << points_dst.size() << " " << norms.size() << " " << scales.size() << " " << v_dst.size() << std::endl;
  
  int D = datas_pts.D;
  // ---------------------------------------
  //           KD-tree creation
  // --------------------------------------
  int nbp = points_dst.size();
  int K_T = 100;
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
  for(int i = 0; i < scales.size() ; i++){
    v_scale[i] = scales[i][D-1];
  }
  std::sort(v_scale.begin(), v_scale.end());
  params.min_scale = get_min_scale(v_scale);
  double gbl_scale = get_median_scale(v_scale);
  // ---------------------------------------
  //           DST computation
  // --------------------------------------
  bool do_debug = false;
  std::cerr << "    dst computation" << std::endl;

  int accid = 0;
  for( auto cit = tri.cells_begin();
       cit != tri.cells_end(); ++cit ){
    // Cell_handle ch = cit;
    if( cit->is_infinite() )
      continue;

    int cid = cit->lid();
    //    std::cerr << "cid:" << cid << " " << "vdst.size :" << v_dst.size() << std::endl;
    if(cid == 5786){

    }
    double  vpe = v_dst[cid][0];
    double  vpo = v_dst[cid][1];
    double  vpu = v_dst[cid][2];

    //    std::cerr << "    id : " << vpe << " " << "vop:" << vpo <<  std::endl;
    for(int x = 0; x < params.nb_samples; x++){
      //Point PtSample = traits.make_point((cit->data()).pts[x].begin());
      std::vector<double>  C =  (x == 0) ? cit->barycenter() : Pick(cit->full_cell(),D);
      //std::vector<double>  C = (x == 0) ? cit->barycenter() : Pick(cit->full_cell(),D);
      Point  PtSample = traits.make_point(C.begin());


      for(int d = 0; d < D; d++){
	  queryPt[d] = PtSample[d];
      }


      if(cid == 5786 && do_debug){
	std::cerr << "=============" <<  std::endl;
	std::cerr << "id : " << cid <<  std::endl;
	std::cerr << "bary  :";
	for(int d = 0; d < D; d++){
	  std::cerr << PtSample[d] << " ";
	}
	std::cerr << std::endl;
      }
      
      kdTree->annkSearch(queryPt,K_T, nnIdx,dists,eps);
      for(int k = 0; k < K_T; k++){
       
  	double pe1,po1,pu1,pe2,po2,pu2;
  	int idx = nnIdx[k];
  	Point Pt3d = points_dst[idx];
  	std::vector<Point> pts_norms = norms[idx];
  	std::vector<double> pts_scales = scales[idx];

      if(cid == 5786 && do_debug){
	std::cerr << "----------- " << std::endl;
	std::cerr << "   kidx : " << idx << std::endl;
	std::cerr << "   pt3D  :";
	for(int d = 0; d < D; d++){
	  std::cerr << Pt3d[d] << " ";
	}
	std::cerr << std::endl;
	std::cerr << "   Norm  :";
	for(int d = 0; d < D; d++){
	  std::cerr << pts_norms[0][d] << " ";
	}
	std::cerr << std::endl;
	std::cerr << "   scale  :";
	for(int d = 0; d < D; d++){
	  std::cerr << pts_scales[d] << " ";
	}
	std::cerr << std::endl;
      }

	
  	double pdf_smooth = -1;
  	double coef_conf = -1;
	double gbl_scale = -1;//(datas.glob_scale.size() > 0)  ? datas.glob_scale[idx] : -1;

	if(params.mode == std::string("surface")){
	  get_params_surface_dst(pts_scales,gbl_scale,params.min_scale,pdf_smooth,coef_conf,D);
	  std::vector<double> pts_coefs = compute_base_coef(Pt3d,PtSample,pts_norms,D);
	  compute_dst_mass_norm(pts_coefs,pts_scales,coef_conf, pdf_smooth,pe2, po2,pu2);
	}else if(params.mode == std::string("conflict")){
	  get_params_surface_dst(pts_scales,gbl_scale,params.min_scale,pdf_smooth,coef_conf,D);
	  std::vector<double> pts_coefs = compute_base_coef(Pt3d,PtSample,pts_norms,D);
	  compute_dst_mass_norm(pts_coefs,pts_scales,coef_conf, pdf_smooth,pe2, po2,pu2);

	}



  	pe1=vpe;
  	po1=vpo;
  	pu1=vpu;

	// std::cerr << "  K  "<< k << " " << pe1 << " " << po1 << " " << pu1 <<  std::endl;
	// std::cerr << "  K  "<< k << " " << pe2 << " " << po2 << " " << pu2 <<  std::endl;

  	ds_score(pe1,po1,pu1,pe2,po2,pu2,pe1,po1,pu1);
  	regularize(pe1,po1,pu1);

	if(cid == 5786 && do_debug){
	  for(int d = 0; d < D; d++){
	    std::cerr << Pt3d[d] << " ";
	  }
	  std::cerr << std::endl;
	  std::cerr << "  pe2  "<< k << " " << pe2 << " " << po2 << " " << pu2 <<  std::endl;
	  std::cerr << "  pe1  "<< k << " " << pe1 << " " << po1 << " " << pu1 <<  std::endl;
	  std::cerr << "  new  "<< k << " " << vpe << " " << vpo << " " << vpu <<  std::endl;
	  std::cerr << " " << std::endl;
	}

	
  	vpe = pe1;
  	vpo = po1;
  	vpu = pu1;	  
      }
    }
    //std::cerr << "  cid  "<< cid << " " << vpe << " " << vpo << " " << vpu <<  std::endl;

    // NAN check
    if(vpe == vpe &&
       vpo == vpo &&
       vpu == vpu){
    v_dst[cid][0] = vpe;
    v_dst[cid][1] = vpo ;
    v_dst[cid][2] = vpu ;
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
  if(c3 > 0){
    v_o1 =  0;
    v_e1 = 1-0.5*score_pdf(fabs(c3),sig);
    double score = fabs((angle/angle_scale)*(angle/angle_scale));
    v_e1 = v_e1*exp(-score);

  }else{
    v_o1 = v_e1 = 0;
  }
  v_u1 = 1-v_e1-v_o1;

  v_e1 = v_e1*coef_conf;
  v_o1 = v_o1*coef_conf;

  regularize(v_e1,v_o1,v_u1);
  // std::cerr << std::endl;
}


#ifdef DDT_CGAL_TRAITS_D

std::vector<double> get_cell_barycenter(Cell_handle ch)
    {
        int D = ch->maximal_dimension();
        std::vector<double> coords;// = get_vect_barycenter(ch);
        for(uint d = 0; d < D; d++)
            coords[d] = 0;
        for(auto vht = ch->vertices_begin() ;
                vht != ch->vertices_end() ;
                ++vht)
        {
            Vertex_handle v = *vht;
            for(uint d = 0; d < D; d++)
            {
                coords[d] += (v->point())[d];
            }
        }
        for(uint d = 0; d < D; d++)
            coords[d] /= ((double)D+1);
        return coords;
    }


void
wasure_algo::sample_cell(Cell_handle & ch,Point &  Pt3d, Point & PtCenter, wasure_data<Traits>  & datas_tri, wasure_data<Traits>  & datas_pts, wasure_params & params, int cid, int dim){
  std::vector<Point> & pts_norm = datas_pts.format_egv[cid];
  std::vector<double> & pts_scale = datas_pts.format_sigs[cid];
  std::vector<std::vector<double>> & v_dst = datas_tri.format_dst;
  
  // //  if((ch->data()).seg == cid) return;
  // std::vector<double> & vpe = (ch->data()).vpe;
  // std::vector<double> & vpo = (ch->data()).vpo;
  // std::vector<double> & vpu = (ch->data()).vpu;

  
  for(int x = 0; x < params.nb_samples; x++){
    std::vector<double>  C = (x == 0) ? get_cell_barycenter(ch) : Pick(ch,D);
    Point  PtSample = traits.make_point(C.begin());
    double pe1,po1,pu1,pe2,po2,pu2;

    pe1 = v_dst[cid][0];
    po1 = v_dst[cid][1];
    pu1 = v_dst[cid][2];

	  
    std::vector<double> center_coefs = compute_base_coef<Point>(Pt3d,PtCenter,pts_norm,dim);
    std::vector<double> pts_coefs = compute_base_coef<Point>(Pt3d,PtSample,pts_norm,dim);
    double angle = compute_angle_rad(PtSample,Pt3d,PtCenter,dim);

    double pdf_smooth = -1;
    double coef_conf = -1;
    double gbl_scale = -1;//(datas.glob_scale.size() > 0)  ? datas.glob_scale[cid] : -1;
    get_params_surface_dst(pts_scale,gbl_scale,params.min_scale,pdf_smooth,coef_conf,dim);

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

#endif


Cell_handle wasure_algo::walk_locate(DT & tri,
				     Point & Pt3d,  Point & Ptcenter,
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
      if( CGAL::EQUAL != geom_traits.compare_lexicographically_d_object()(Pt3d, vit->point()) )
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
	    spivi(s->vertex(i), &Pt3d);

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


  #else
  return start_cell;
#endif
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
    v_dst[cid][0] = 1;
    v_dst[cid][1] = 0;
    v_dst[cid][2] = 0;
  }

}


//void compute_dst_ray(DT & tri,std::vector<Point> & points,std::vector<Point> & centers,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales,double rat_ray_sample, int D){

void wasure_algo::compute_dst_ray(DT & tri, wasure_data<Traits>  & datas_tri,wasure_data<Traits>  & datas_pts, wasure_params & params){
  std::cerr << "    compute dst ray ..." << std::endl;
  std::vector<Point> & points = datas_pts.format_points;
  std::vector<Point> & centers = datas_pts.format_centers;
  std::vector<std::vector<Point>> & norms = datas_pts.format_egv;
  std::vector<std::vector<double>> & scales = datas_pts.format_sigs;
  double rat_ray_sample = params.rat_ray_sample;
  int D = Traits::D;  


  int nb_pts = points.size();
  int acc = 0;
  int max_walk = 100000;
  Cell_handle  start_walk = Cell_handle();
  for(int n = 1 ; n < nb_pts; n++){

    if(acc++ % ((int)(1.0/(rat_ray_sample)))  == 0){
      Point & Pt3d = points[n];
      Point & Ptcenter = centers[n];
      std::cerr << n << " -- pt:" << Pt3d << " -- center:" << Ptcenter << std::endl;
      //Cell_handle loc = walk_locate(tri,Pt,Ptcenter,norms[n],scales[n],acc++);
      Cell_handle loc = walk_locate(tri,Pt3d,Ptcenter,datas_tri,datas_pts,params,n,start_walk);
    }
  }
}


void wasure_algo::compute_dst_with_center(DTW & tri, wasure_data<Traits>  & datas_tri, wasure_data<Traits>  & datas_pts, wasure_params & params, Id tid){

  //std::vector<Point> & points_3d,std::vector<Point> & centers,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales, double rat_ray_sample, int dim){

  compute_dst_tri(tri,datas_tri,datas_pts,params);
  //  compute_dst_ray(tri,datas_tri,datas_pts,params);
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
