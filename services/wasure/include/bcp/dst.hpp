#ifndef DST_H
#define DST_H

//#include "triangulation/nd_functions.hpp"
//#include "utils/utils.hpp"
#include "wasure_typedefs.hpp"
#include <CGAL/Random.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

//#include "dst.hpp"
#include "wasure_maths.hpp"
#include "ANN/ANN.h"
//#include "input_params.hpp"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))



// typedef CGAL::Exact_predicates_inexact_constructions_kernel K3;
// typedef K3::Point_3    Point_3;
// typedef K3::Vector_3   Vector_3;


Point Pick_2d(const Point & v0,const Point & v1,const Point & v2)
{
    int D=2;
    std::vector<double> coords(D);
    double r1 = (((double) rand() / (RAND_MAX)));
    double r2 = (((double) rand() / (RAND_MAX)));
    for(int d = 0; d < D; d++)
        coords[d] = (1 - sqrt(r1)) * v0[d] + (sqrt(r1) * (1 - r2)) * v1[d] + (sqrt(r1) * r2) * v2[d];

    return Point(coords);

}

Point Pick_3d(const Point & v0,const Point & v1,const Point & v2,const Point & v3)
{
    int D = 3;
    double s = (((double) rand() / (RAND_MAX)));
    double t = (((double) rand() / (RAND_MAX)));
    double u = (((double) rand() / (RAND_MAX)));
    if(s+t>1.0)   // cut'n fold the cube into a prism
    {
        s = 1.0 - s;
        t = 1.0 - t;
    }
    if(t+u>1.0)   // cut'n fold the prism into a tetrahedron
    {
        double tmp = u;
        u = 1.0 - s - t;
        t = 1.0 - tmp;

    }
    else if(s+t+u>1.0)
    {
        double tmp = u;
        u = s + t + u - 1.0;
        s = 1 - t - tmp;
    }
    double a=1-s-t-u; // a,s,t,u are the barycentric coordinates of the random point.
    Point rp;
    std::vector<double> coords(D);
    coords[0] = (v0[0]*a + v1[0]*s + v2[0]*t + v3[0]*u);
    coords[1] = (v0[1]*a + v1[1]*s + v2[1]*t + v3[1]*u);
    coords[2] = (v0[2]*a + v1[2]*s + v2[2]*t + v3[2]*u);

    return Point(coords);;

}

Point  Pick(Full_cell_handle & ch, int D)
{
    if(D == 3)
        return Pick_3d(ch->vertex(0)->point(),ch->vertex(1)->point(),ch->vertex(2)->point(),ch->vertex(3)->point());
    if(D == 2)
        return Pick_2d(ch->vertex(0)->point(),ch->vertex(1)->point(),ch->vertex(2)->point());
}

void init_sample(DTW & tri,int nb_samples, int dim)
{
    std::cout << "init samples ...." << std::endl;
    for( auto cit = tri.full_cells_begin();
            cit != tri.full_cells_end(); ++cit )
    {
        Full_cell_handle ch = cit;
        ch->data().resize(nb_samples);
        if( tri.is_infinite(ch) )
        {

        }
        else
        {
            for(int x = 0; x < nb_samples; x++)
            {
                Point  C = (x == 0) ? iqlib::get_barycenter<DTW>(ch,dim) : Pick(ch,dim);
                ch->data().pts[x] = C;
            }
        }
    }
}


void finalize_sample(DTW & tri, int nb_samples)
{
    std::cout << "finalize samples ...." << std::endl;
    for( auto cit = tri.full_cells_begin();
            cit != tri.full_cells_end(); ++cit )
    {
        // if( tri.is_infinite(cit)  )
        //   continue;


        std::vector<double> & vpe = (cit->data()).vpe;
        std::vector<double> & vpo = (cit->data()).vpo;
        std::vector<double> & vpu = (cit->data()).vpu;


        double res_pe = 0, res_po = 0, res_pu = 0;
        for(int x = 0; x < nb_samples; x++)
        {
            res_pe += vpe[x];
            res_po += vpo[x];
            res_pu += vpu[x];
        }

        if(res_pe != res_pe || res_po != res_po || res_pu != res_pu)
        {
            std::cout << "/!\\ fch->data().lab NAN /!\\" << std::endl;
            res_pe = res_po = 0;
            res_pu = nb_samples;
        }

        (cit->data()).dat[0] = res_pe;
        (cit->data()).dat[1] = res_po;
        (cit->data()).dat[2] = res_pu;
        (cit->data()).dat[3] = nb_samples;
    }
}




double get_min_scale(std::vector<double> &v)
{
    size_t n = v.size() / 10;
    nth_element(v.begin(), v.begin()+n, v.end());
    return v[n];
}

double get_median_scale(std::vector<double> &v)
{
    size_t n = v.size() / 2;
    nth_element(v.begin(), v.begin()+n, v.end());
    return v[n];
}





void ds_score(double v_e1,double v_o1,double v_u1,double v_e2,double v_o2,double v_u2,double & v_e3,double & v_o3,double & v_u3)
{
    double vK = v_o1*v_e2 + v_e1*v_o2;
    v_e3 = (v_e1*v_e2 + v_e1*v_u2 + v_u1*v_e2)/(1-vK);
    v_o3 = (v_o1*v_o2 + v_o1*v_u2 + v_u1*v_o2)/(1-vK);
    v_u3 = (v_u1*v_u2)/(1-vK);
}






void compute_dst_mass_norm(std::vector<double> coefs, std::vector<double> scales, double coef_conf, double pdfs, double & v_e1, double & v_o1, double & v_u1)
{
    int D = scales.size();
    double c3 = coefs[D-1];
    double nscale = scales[D-1];
    double pdf_smooth =  pdfs;
    if(nscale <= 0) nscale = 0.000001;
    if(c3 > 0)
    {
        v_e1 =  1-0.5*(exp(-fabs(c3)/nscale));
        v_o1 = 0.5*(exp(-fabs(c3)/nscale));
    }
    else if (c3 < 0)
    {
        v_e1 = 0.5*(exp(-fabs(c3)/nscale));
        v_o1 = 1-0.5*(exp(-fabs(c3)/nscale));
    }
    else
    {
        v_e1 = v_o1 = 0.5;
    }
    for(int d = 0; d < D-1; d++)
    {
        if(scales[d] <= 0)
        {
            v_e1 = v_o1 = 0;
        }
        else
        {
            // v_e1 = v_e1*exp(-fabs(coefs[d]/(scales[d]*3)));
            // v_o1 = v_o1*exp(-fabs(coefs[d]/(scales[d]*3)));
            v_e1 = v_e1*score_pdf(coefs[d],scales[d]/6);
            v_o1 = v_o1*score_pdf(coefs[d],scales[d]/6);
        }
    }

    v_e1 = v_e1*exp(-(fabs(c3)/(pdf_smooth))*(fabs(c3)/(pdf_smooth)));
    v_o1 = v_o1*exp(-(fabs(c3)/(pdf_smooth))*(fabs(c3)/(pdf_smooth)));
    v_e1 = v_e1*coef_conf;
    v_o1 = v_o1*coef_conf;

    v_u1 = 1-v_e1-v_o1;
    regularize(v_e1,v_o1,v_u1);

}





//void compute_dst_tri(DTW & tri,std::vector<Point> points_dst,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales,int D){


double get_conf_volume(const std::vector<double> & pts_scales, int dim)
{
    //double c1 = *std::max_element(pts_scales.begin(),pts_scales.end()--)/(*pts_scales.end());
    double c1 = MIN(pts_scales[dim-1]/pts_scales[0],1);
    return 1-c1;
}

void  get_params_pts(const std::vector<double> & pts_scales,double glob_scale,double min_scale, double & pdf_smooth,double & coef_conf, int D)
{

    double data_scale = *std::min_element(pts_scales.begin(),pts_scales.end());
    if(glob_scale > 0)
    {
        pdf_smooth = glob_scale;

    }
    else
    {
        //    double data_scale = *std::max_element(pts_scales.begin(),pts_scales.end());
        pdf_smooth = data_scale*3;
    }

    coef_conf = 1;//MIN(min_scale/data_scale,1);//*get_conf_volume(pts_scales,D);

}

void compute_dst_tri(DTW & tri, wasure_data  & datas, algo_params & params)
{
    std::cout << "    compute dst norm ..." << std::endl;
    std::vector<Point> & points_dst =  datas.points;
    std::vector<std::vector<Point>> & norms = datas.dims_norms;
    std::vector<std::vector<double>> & scales = datas.dims_scales;

    int D = datas.D;
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
    for(std::vector<Point>::iterator pit = points_dst.begin(); pit != points_dst.end(); ++pit)
    {
        Point p1 = *pit;
        for(int d = 0; d < D; d++)
            dataPts[npts][d] = p1[d];
        npts++;
    }

    kdTree = new ANNkd_tree(dataPts,npts,D);

    std::vector<double> v_scale(scales.size());
    for(int i = 0; i < scales.size() ; i++)
    {
        v_scale[i] = scales[i][D-1];
    }
    params.min_scale = get_min_scale(v_scale);

    // ---------------------------------------
    //           DST computation
    // --------------------------------------

    for( Full_cell_iterator cit = tri.full_cells_begin();
            cit != tri.full_cells_end(); ++cit )
    {
        Full_cell_handle ch = cit;
        if( tri.is_infinite(cit) )
            continue;

        std::vector<double> & vpe = (cit->data()).vpe;
        std::vector<double> & vpo = (cit->data()).vpo;
        std::vector<double> & vpu = (cit->data()).vpu;

        for(int x = 0; x < params.nb_samples; x++)
        {
            Point PtSample = (cit->data()).pts[x];
            //std::cout << p1 << " - nbsamplesv:" << params.nb_samples << std::endl;


            for(int d = 0; d < D; d++)
                queryPt[d] = PtSample[d];

            kdTree->annkSearch(queryPt,K_T, nnIdx,dists,eps);
            for(int k = 0; k < K_T; k++)
            {
                double pe1,po1,pu1,pe2,po2,pu2;
                int idx = nnIdx[k];
                Point Pt3d = points_dst[idx];
                std::vector<Point> pts_norms = norms[idx];
                std::vector<double> pts_scales = scales[idx];

                //
                double pdf_smooth = -1;
                double coef_conf = -1;
                double gbl_scale = (datas.glob_scale.size() > 0)  ? datas.glob_scale[idx] : -1;

                get_params_pts(pts_scales,gbl_scale,params.min_scale,pdf_smooth,coef_conf,D);

                std::vector<double> pts_coefs = compute_base_coef<Point>(Pt3d,PtSample,pts_norms,D);
                compute_dst_mass_norm(pts_coefs,pts_scales,coef_conf, pdf_smooth,pe2, po2,pu2);


                pe1=vpe[x];
                po1=vpo[x];
                pu1=vpu[x];

                ds_score(pe1,po1,pu1,pe2,po2,pu2,pe1,po1,pu1);
                regularize(pe1,po1,pu1);

                vpe[x] = pe1;
                vpo[x] = po1;
                vpu[x] = pu1;

            }
        }
    }

}

// Point_3 inline pointd23(const Point  & a){
//   return Point_3(a[0],a[1],a[2]);
// }


// double compute_angle_rad_3D(const Point_3 & a, const Point_3 & b, const Point_3 & c ){

//   CGAL::Vector_3<K3> v1 = c - a;
//   CGAL::Vector_3<K3> v2 = c - b;
//   double cosine = v1 * v2 / CGAL::sqrt(v1*v1) / CGAL::sqrt(v2 * v2);
//   return std::acos(cosine);
// }

double compute_angle_rad_2D(const Point & a, const Point & b, const Point & c )
{
    std::vector<double> v1 = {c[0]-a[0], c[1]-a[1]};
    std::vector<double> v2 = {c[0]-b[0], c[1]-b[1]};
    double cosine = (v1[0]*v2[0]+v1[1]*v2[1])/sqrt(v1[0]*v1[0]+v1[1]*v1[1])/sqrt(v2[0]*v2[0]+v2[1]*v2[1]);
    return std::acos(cosine);

}

double compute_angle_rad(const Point & a, const Point & b, const Point & c, int dim)
{
    std::vector<double> v1(dim);
    std::vector<double> v2(dim);
    for(int d = 0; d < dim; d++)
    {
        v1[d] = c[d]-a[d];
        v2[d] = c[d]-b[d];
    }
    double num=0,accv1=0,accv2=0;
    for(int d = 0; d < dim; d++)
    {
        num += v1[d]*v2[d];
        accv1 += v1[d]*v1[d];
        accv2 += v2[d]*v2[d];
    }
    double cosine = num/sqrt(accv1)/sqrt(accv2);
    return std::acos(cosine);

}


// double compute_angle_rad(const Point  & a, const Point & b, const Point & c, int dim ){

//   if(dim == 3){
//     Point_3 a3 = pointd23(a);
//     Point_3 b3 = pointd23(b);
//     Point_3 c3 = pointd23(c);
//     return compute_angle_rad_3D(a3,b3,c3);
//   }else if(dim == 2){
//     return compute_angle_rad_2D(a,b,c);
//   }

// }


void
compute_dst_mass_beam(std::vector<double> & coefs, std::vector<double> & scales,double angle,double angle_scale, double coef_conf, double & v_e1, double & v_o1, double & v_u1)
{

    int D = scales.size();
    double sig = scales[D-1];
    double c3 = coefs[D-1];
    double nscale = scales[D-1];
    if(c3 > 0)
    {
        v_o1 =  0;
        v_e1 = 1-0.5*score_pdf(fabs(c3),sig);
        double score = fabs((angle/angle_scale)*(angle/angle_scale));
        v_e1 = v_e1*exp(-score);

    }
    else
    {
        v_o1 = v_e1 = 0;
    }
    v_u1 = 1-v_e1-v_o1;

    v_e1 = v_e1*coef_conf;
    v_o1 = v_o1*coef_conf;

    regularize(v_e1,v_o1,v_u1);
    // std::cout << std::endl;
}



void sample_cell(Full_cell_handle & ch,Point &  Pt3d, Point & PtCenter, wasure_data  & datas, algo_params & params, int idr, int dim)
{
    std::vector<Point> & pts_norm = datas.dims_norms[idr];
    std::vector<double> & pts_scale = datas.dims_scales[idr];

    if((ch->data()).seg == idr) return;
    std::vector<double> & vpe = (ch->data()).vpe;
    std::vector<double> & vpo = (ch->data()).vpo;
    std::vector<double> & vpu = (ch->data()).vpu;

    for(int x = 0; x < params.nb_samples; x++)
    {
        //Point C = get_barycenter(ch,D);
        Point & PtSample = (ch->data()).pts[x];


        double pe1,po1,pu1,pe2,po2,pu2;
        pe1 = vpe[x];
        po1 = vpo[x];
        pu1 = vpu[x];


        std::vector<double> center_coefs = compute_base_coef<Point>(Pt3d,PtCenter,pts_norm,dim);
        std::vector<double> pts_coefs = compute_base_coef<Point>(Pt3d,PtSample,pts_norm,dim);
        // if(center_coefs[dim-1] <  pts_coefs[dim-1])
        //   return;
        double angle = compute_angle_rad(PtSample,Pt3d,PtCenter,dim);
        //double coef_conf  = MIN(0.1/pts_scale[dim-1],1);

        double pdf_smooth = -1;
        double coef_conf = -1;
        double gbl_scale = (datas.glob_scale.size() > 0)  ? datas.glob_scale[idr] : -1;

        get_params_pts(pts_scale,gbl_scale,params.min_scale,pdf_smooth,coef_conf,dim);

        compute_dst_mass_beam(pts_coefs,pts_scale,angle,ANGLE_SCALE,coef_conf,pe2,po2,pu2);

        ds_score(pe1,po1,pu1,pe2,po2,pu2,pe1,po1,pu1);
        regularize(pe1,po1,pu1);


        vpe[x] = pe1;
        vpo[x] = po1;
        vpu[x] = pu1;
    }
    (ch->data()).seg = idr;

}







Full_cell_handle walk_locate(DTW & tri,
                             Point & Pt3d,  Point & Ptcenter,
                             wasure_data  & datas, algo_params & params,
                             int idr,
                             Full_cell_handle & start_cell
                            )
{

    DTW::Locate_type  loc_type;// type of result (full_cell, face, vertex)
    Face face(tri.maximal_dimension());// the face containing the query in its interior (when appropriate)
    Facet  facet;// the facet containing the query in its interior (when appropriate)

    Full_cell_handle start = tri.locate(Ptcenter,start_cell);
    start_cell = start;
    DTW::Geom_traits geom_traits;
    CGAL::Random                      rng_;
    K::Orientation_d orientation_pred = geom_traits.orientation_d_object();

    int cur_dim = tri.current_dimension();
    std::vector<CGAL::Oriented_side>  orientations_(cur_dim+1);


    for(int i = 0 ; i < start_cell->data().vpe.size(); i++)
    {
        start_cell->data().vpe[i] = 1;
        start_cell->data().vpo[i] = 0;
        start_cell->data().vpu[i] = 0;
    }

    if( cur_dim == -1 )
    {
        loc_type = DTW::OUTSIDE_AFFINE_HULL;
        return Full_cell_handle();
    }
    else if( cur_dim == 0 )
    {
        Vertex_handle vit = tri.infinite_full_cell()->neighbor(0)->vertex(0);
        if( CGAL::EQUAL != geom_traits.compare_lexicographically_d_object()(Pt3d, vit->point()) )
        {
            loc_type = DTW::OUTSIDE_AFFINE_HULL;
            return Full_cell_handle();
        }
        else
        {
            loc_type = DTW::ON_VERTEX;
            face.set_full_cell(vit->full_cell());
            face.set_index(0, 0);
            return vit->full_cell();
        }
    }

    Full_cell_handle s;

    // if we don't know where to start, we start from any bounded full_cell
    if( Full_cell_handle() == start )
    {
        // THE HACK THAT NOBODY SHOULD DO... BUT DIFFICULT TO WORK AROUND
        // THIS... TODO: WORK AROUND IT
        Full_cell_handle inf_c = tri.infinite_full_cell();
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
    // 	  loc_type = DTW::OUTSIDE_AFFINE_HULL;
    //         return Full_cell_handle();
    //     }
    // }

    // we remember the |previous|ly visited full_cell to avoid the evaluation
    // of one |orientation| predicate
    Full_cell_handle previous = Full_cell_handle();
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
            Full_cell_handle next = s->neighbor(i);
            if( previous == next )
            {
                // no need to compute the orientation, we already know it
                orientations_[i] = CGAL::POSITIVE;
                continue; // go to next full_cell's facet
            }

            CGAL::Substitute_point_in_vertex_iterator<
            DTW::Full_cell::Vertex_handle_const_iterator>
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

            if( tri.is_infinite(next) )
            {
                // we have arrived OUTSIDE the convex hull of the triangulation,
                // so we stop the search
                full_cell_not_found = false;
                loc_type = DTW::OUTSIDE_CONVEX_HULL;
                face.set_full_cell(s);
            }
            else
            {
                sample_cell(s,Pt3d,Ptcenter,datas,params,idr, cur_dim);
                for(int ii = 0; ii <= cur_dim; ii++)
                {
                    Full_cell_handle s_nbr = s->neighbor(ii);
                    if(!tri.is_infinite(s_nbr))
                        sample_cell(s_nbr,Pt3d,Ptcenter,datas,params,idr, cur_dim);
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
                facet = Facet(s, i);
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
            facet = Facet(s, cur_dim);
        }
        else if( verts < cur_dim )
            face.set_index(verts, cur_dim);
        //-- end of remark above //
        if( 0 == num )
        {
            loc_type = DTW::IN_FULL_CELL;
            face.clear();
        }
        else if( cur_dim == num )
            loc_type = DTW::ON_VERTEX;
        else if( 1 == num )
            loc_type = DTW::IN_FACET;
        else
            loc_type = DTW::IN_FACE;
    }
    return s;
}


void
center_dst(DTW & tri, std::vector<Point> & points_3d)
{

    int D = 3;
    Full_cell_handle cloc =  Full_cell_handle();
    Full_cell_handle cloc_start =  Full_cell_handle();
    for(auto pit : points_3d)
    {
        Full_cell_handle cloc = tri.locate(pit,cloc_start);
        cloc_start = cloc;
        for(int i = 0 ; i < cloc->data().vpe.size(); i++)
        {
            cloc->data().vpe[i] = 1;
            cloc->data().vpo[i] = 0;
            cloc->data().vpu[i] = 0;
        }
    }

}


//void compute_dst_ray(DTW & tri,std::vector<Point> & points,std::vector<Point> & centers,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales,double rat_ray_sample, int D){

void compute_dst_ray(DTW & tri, wasure_data  & datas, algo_params & params)
{
    std::cout << "    compute dst ray ..." << std::endl;
    std::vector<Point> & points = datas.points;
    std::vector<Point> & centers = datas.centers_pts;
    //std::vector<std::vector<Point>> & norms = datas.dims_norms;
    //std::vector<std::vector<double>> & scales = datas.dims_scales;
    double rat_ray_sample = params.rat_ray_sample;
    int D = datas.D;


    int nb_pts = points.size();
    int acc = 0;
    int max_walk = 100000;
    Full_cell_handle  start_walk = Full_cell_handle();
    for(int n = 0 ; n < nb_pts; n++)
    {

        if(acc++ % ((int)(1.0/(rat_ray_sample)))  == 0)
        {
            Point & Pt3d = points[n];
            Point & Ptcenter = centers[n];
            //Full_cell_handle loc = walk_locate(tri,Pt,Ptcenter,norms[n],scales[n],acc++);
            Full_cell_handle loc = walk_locate(tri,Pt3d,Ptcenter,datas,params,n,start_walk);
        }
    }
}

void compute_dst_with_center(DTW & tri, wasure_data  & datas, algo_params & params)
{

    //std::vector<Point> & points_3d,std::vector<Point> & centers,std::vector<std::vector<Point>> & norms,std::vector<std::vector<double>> & scales, double rat_ray_sample, int dim){

    compute_dst_tri(tri,datas,params);
    compute_dst_ray(tri,datas,params);
    //center_dst(tri,datas.centers_pts);
}


#endif


