#ifndef DIM_H
#define DIM_H

#include "wasure_typedefs.hpp"





// int
// compute_dim(  std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales, int D);


// void
// compute_dim_nb(  std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales, int K_T, int D);

// void flip_dim( std::vector<Point> & points, std::vector<std::vector<Point> > & norms, Point pi, int D);

// void flip_dim_ori( std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<Point> &  ori, int D);


#include "utils.hpp"
#include "ANN/ANN.h"
#include "wasure_maths.hpp"
void
compute_svd(int K_T, const  ANNidxArray & nnIdx, const std::vector<Point> & points,std::vector<Point> & pts_norms, std::vector<double> &coords_scale, Traits & traits)
{
    int D = Traits::D;

    std::vector<Point> loc_pts;

    for(int i = 0; i < K_T; i++)
    {
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
    for(std::vector<Point>::iterator pit = loc_pts.begin(); pit != loc_pts.end(); ++pit)
    {
        Point p = *pit;
        for(int d = 0 ; d < D ; d++)
            mat(acc,d) = p[d];
        acc++;
    }


    Eigen::MatrixXf m = mat.rowwise() - mat.colwise().mean();
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(m, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::MatrixXf sv = svd.singularValues() ;
    Eigen::MatrixXf ev =  svd.matrixV();

    for(int d1 = 0; d1 < D; d1++)
    {
        double coords_norm[Traits::D];
        for(int d2 = 0; d2 < D; d2++)
        {
            coords_norm[d2] = ev(d2,d1);
        }

        pts_norms.push_back(traits.make_point(coords_norm));
    }

    for(int d = 0; d < D; d++)
    {
        coords_scale[d] = sv(d);
        if(coords_scale[d] <= 0)
            coords_scale[d] = 0.000001;
    }
}

int
compute_dim(  std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<std::vector<double>> & scales, Traits & traits)
{

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
    for(std::vector<Point>::iterator pit = points.begin(); pit != points.end(); ++pit)
    {
        Point p1 = *pit;
        for(int d = 0; d < D; d++)
            dataPts[npts][d] = p1[d];
        npts++;
    }

    kdTree = new ANNkd_tree(dataPts,npts,D);
    //std::cerr << "step2" << std::endl;


    for(std::vector<Point>::iterator pit = points.begin(); pit != points.end(); ++pit)
    {
        Point p1 = *pit;
        std::vector<Point> pts_norms;
        std::vector<double> coords_scale(D);
        double entropy = 1000000000;
        for(int d = 0; d < D; d++)
            queryPt[d] = p1[d];

        kdTree->annkSearch(queryPt,K_T, nnIdx,dists,eps);

        for(int k = 3; k < K_T; k++)
        {

            std::vector<Point> cur_pts_norms;
            std::vector<double> cur_coords_scale(D);
            compute_svd(k, nnIdx, points,cur_pts_norms,cur_coords_scale,traits);
            double cst = 0;
            std::vector<double> dim(D);
            for(int d = 0 ; d < D; d++)
            {
                cst += cur_coords_scale[d];
            }
            for(int d = 0; d < D; d++)
            {
                dim[d] = (d == D-1) ? cur_coords_scale[d]/cst : (cur_coords_scale[d] - cur_coords_scale[d+1])/cst;
            }
            double cur_entropy = 0;
            for(int d = 0; d < D; d++)
            {
                cur_entropy += -dim[d]*log(dim[d]);
            }
            if(cur_entropy < entropy)
            {
                entropy = cur_entropy;
                pts_norms = cur_pts_norms;
                coords_scale = cur_coords_scale;
            }
        }

        // -----------------------------
        // ------- Dump values ---------
        // -----------------------------
        if(pts_norms.size() < D)
        {
            std::cout << "error !! " << "dim:" << D << " kt:" << K_T << " entropy:" << entropy  << std::endl;

            std::vector<Point> loc_pts;
            for(int i = 0; i < K_T; i++)
            {
                int idx = nnIdx[i];
                loc_pts.push_back(points[idx]);
            }
            bool are_equal = true;
            for(int i = 1; i < loc_pts.size(); i++)
                if(loc_pts[i] != loc_pts[i-1])
                {
                    are_equal = false;
                    break;
                }
            if(are_equal)
            {
                std::cout << "Point " << loc_pts[0] << " duplicated " << K_T << " time, svd not defined!" << std::endl;
                std::cout << "remove duplicted point to continue" << std::endl;
                return 1;
            }

        }
        else
        {
            norms.push_back(pts_norms);
            scales.push_back(coords_scale);
        }
    }
    std::cout << "done!" << std::endl;
    delete kdTree;
    delete [] nnIdx;                                                  // clean things up
    delete [] dists;
    return 0;
    //std::cerr << "step3" << std::endl;

}


void flip_dim_ori( std::vector<Point> & points, std::vector<std::vector<Point> > & norms, std::vector<Point> &  ori, Traits & traits)
{
    int nbp = points.size();
    int D = Traits::D;


    int nb_flip = 0;
    for(int n = 0; n < nbp; n++)
    {
        Point pts_norm = norms[n][D-1];
        Point pi  = ori[n];
        Point p1 = points[n];
        std::vector<double> pts_coefs = compute_base_coef<Point>(p1,pi,norms[n],D);
        if(pts_coefs[D-1] < 0)
        {

            double coords_norm[Traits::D];
            for(int d = 0; d < D; d++)
            {
                coords_norm[d] = -norms[n][D-1][d];
            }
            norms[n][D-1] = traits.make_point(coords_norm);
            nb_flip++;
        }
        else
        {

        }
    }
    std::cout << "nb flips:" << nb_flip << "/" << nbp << std::endl;
}

void flip_dim( std::vector<Point> & points, std::vector<std::vector<Point> > & norms, Point p1, Traits & traits)
{
    int nbp = points.size();
    int D = Traits::D;
    for(int n = 0; n < nbp; n++)
    {
        Point pts_norm = norms[n][D-1];
        Point pi  = points[n];
        std::vector<double> pts_coefs = compute_base_coef<Point>(p1,pi,norms[n],D);
        if(pts_coefs[D-1] < 0)
        {
            double coords_norm[Traits::D];
            for(int d = 0; d < D; d++)
            {
                coords_norm[d] = -norms[n][D-1][d];
            }
            norms[n][D-1] = traits.make_point(coords_norm);
        }
        else
        {

        }
    }
}



#endif
