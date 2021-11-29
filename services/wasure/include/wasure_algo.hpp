#ifndef WASURE_ALGO
#define WASURE_ALGO

#include "wasure_typedefs.hpp"
#include "wasure_params.hpp"
#include "wasure_data.hpp"


#include "ANN/ANN.h"

class wasure_algo
{
public :
    typedef double d_type;

    wasure_algo()
    {
        int D = Traits::D;

    }

    // ============ Dim ====================
    void compute_svd(int K_T, const  ANNidxArray & nnIdx, const std::vector<Point> & points,std::vector<Point> & pts_norms, std::vector<double> &coords_scale);
    int  compute_dim(  std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales,ddt::logging_stream & log);
    int  compute_dim_with_simp(  std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales,std::vector<Point> & simp, double pscale);
    int simplify(std::vector<Point> & points, std::vector<bool> & do_keep, double dist );

  void tessel_adapt(std::vector<Point> & points,std::vector<Point> & vps,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales, int maxit, double target_err, int D, int tid);
  int  tessel(DT_raw  & tri, std::vector<Point> & points, std::vector<Point> & vps,  std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales, int max_it, Id tid);

    void flip_dim_ori( std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<Point> &  ori);
    void flip_dim( std::vector<Point> & points, std::vector<std::vector<Point> > & norms, Point p1);


    // ============ DST Computation  =========

    void sample_cell(Cell_handle & ch,Point &  Pt3d, Point & PtCenter, wasure_data<Traits>  & datas,wasure_data<Traits>  & datas_pts, wasure_params & params, int idr, int dim);

    void compute_dst_mass(std::vector<double> coefs, std::vector<double> scales, double & v_e1, double & v_o1, double & v_u1) ;

    void compute_dst_ray(DT & tri, wasure_data<Traits>  & datas_pts,wasure_data<Traits>  & datas_tri, wasure_params & params);
    void compute_dst_tri(DTW & tri, wasure_data<Traits>  & datas, wasure_data<Traits>  & datast, wasure_params & params);
  void compute_dst_with_center(DTW & tri, wasure_data<Traits>  & datas_tri, wasure_data<Traits>  & datas_pts, wasure_params & params, Id tid);
    void finalize_sample(DT & tri, int nb_samples);

  //   std::vector<double>  get_cell_barycenter(Cell_handle ch);
  // std::vector<double>  get_cell_barycenter(Cell_const_iterator ch);
    std::vector<double>  Pick_3d(const Point & v0,const Point & v1,const Point & v2,const Point & v3);
    std::vector<double>  Pick_2d(const Point & v0,const Point & v1,const Point & v2);
  //    std::vector<double>   Pick(Tile_cell_const_handle  ch, int D);


    void init_sample(DT & tri,int nb_samples, int dim);
    double compute_angle_rad(const Point  & a, const Point & b, const Point & c, int dim );

    void compute_dst_mass_norm(std::vector<double> coefs, std::vector<double> scales, double coef_conf, double pdfs_e,double pdfs_o, double & v_e1, double & v_o1, double & v_u1);

    void  get_params_surface_dst(const std::vector<double> & pts_scales,double glob_scale,double min_scale, double & pdf_smooth,double & coef_conf, int D);
    void  get_params_conflict_dst(const std::vector<double> & pts_scales,double glob_scale,double min_scale, double & pdf_smooth,double & coef_conf, int D);

    void compute_dst_mass_beam(std::vector<double> & coefs, std::vector<double> & scales,double angle,double angle_scale, double coef_conf, double & v_e1, double & v_o1, double & v_u1);

    void ds_score(double v_e1,double v_o1,double v_u1,double v_e2,double v_o2,double v_u2,double & v_e3,double & v_o3,double & v_u3);
    // dst_img.cpp
    void dump_dst_in_img(  wasure_data<Traits>  & datas, wasure_params & params,std::string & name_img);

    Cell_handle walk_locate(DT & tri,
                            Point & Pt3d,  Point & Ptcenter, Point & Ptcenter_mir,
                            wasure_data<Traits>  & datas,wasure_data<Traits>  & datas_pts, wasure_params & params,
                            int idr,
                            Cell_handle & start_cell
                           );

  void center_dst(DTW & tri, wasure_data<Traits>  & datas_tri,std::vector<Point> & center_pts, Id tid);


    // Surface extraction

    void extract_surface(DTW & tri,  std::map<Id,wasure_data<Traits> >  & datas_tri);


//   template<typename CH>
// std::vector<double>  wasure_algo::get_cell_barycenter(CH ch)
// {

//   std::vector<double> coords(D);
//   for(uint d = 0; d < D; d++)
//     coords[d] = 0;
//   for(auto vht = ch->vertices_begin() ;
//       vht != ch->vertices_end() ;
//       ++vht)
//     {
//       Vertex_handle v = *vht;
//       for(uint d = 0; d < D; d++)
// 	{
// 	  coords[d] += (v->point())[d];
// 	}
//     }
//   for(uint d = 0; d < D; d++)
//     coords[d] /= ((double)D+1);
//   return coords;
// }


  template<typename CH>
  std::vector<double>   Pick(CH  ch, int D){
    if(D == 3)
      return Pick_3d(ch->vertex(0)->point(),ch->vertex(1)->point(),ch->vertex(2)->point(),ch->vertex(3)->point());
    if(D == 2)
      return Pick_2d(ch->vertex(0)->point(),ch->vertex(1)->point(),ch->vertex(2)->point());
  }

  
  Traits  traits  ;
  Traits_raw  traits_raw  ;
  int D = Traits::D;

};

#endif
