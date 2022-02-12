#ifndef TBMRF_H
#define TBMRF_H
#define MULT (1.0)
#define GSPS_CONST (0)
//#include "wasure_typedefs.hpp"
#include <stdio.h>      /* printf */
#include <iostream>      /* printf */
#include <iterator>
#include <vector>
#include "io/write.hpp"
#include "io/read.hpp"
#include "partitioner/grid_partitioner.hpp"
#include "scheduler/scheduler.hpp"
#include "DDT.hpp"
#include "ddt_exeptions.hpp"
#include "graph.h"
#include "wasure_maths.hpp"
#include "QPBO.h"
#include "wasure_typedefs.hpp"

// Belief
#include <opengm/graphicalmodel/graphicalmodel.hxx>
#include <opengm/graphicalmodel/space/simplediscretespace.hxx>
#include <opengm/functions/potts.hxx>
#include <opengm/operations/adder.hxx>
#include <opengm/inference/messagepassing/messagepassing.hxx>
#include <opengm/inference/gibbs.hxx>
#include <opengm/opengm.hxx>
#include <opengm/graphicalmodel/graphicalmodel.hxx>
#include <opengm/operations/adder.hxx>




// struct g_edge {
//   int id1,id2;
//   double e0,e1;
// } ;
// struct g_vert {
//   int id1;
//   double e0,e1;
// } ;


inline int l2gid(int cid,std::vector<int> & tile_ids)
{
    return cid + tile_ids[2] + 2;
}

inline int g2lid(int cid,std::vector<int> & tile_ids)
{
    return cid - tile_ids[2] - 2;
}


inline int l2gidf(int cid,std::vector<int> & tile_ids)
{
    return cid + tile_ids[2];
}

inline int g2lidf(int cid,std::vector<int> & tile_ids)
{
    return cid - tile_ids[2];
}


inline int l2gid(int cid)
{
    return cid + 2;
}

inline int g2lid(int cid)
{
    return cid - 2;
}


inline int l2gidf(int cid)
{
    return cid + 2;
}

inline int g2lidf(int cid)
{
    return cid - 2;
}


template<typename DTW,typename D_MAP>
class tbmrf
{

public :

    typedef double coord_type;
    typedef double DATA_TYPE;
    typedef Graph<DATA_TYPE,DATA_TYPE,DATA_TYPE> GraphType;


    typedef typename DTW::Tile_cell_const_handle              Tile_cell_const_handle;
  //    typedef typename DTW::DT::Full_cell::Vertex_handle_iterator Vertex_h_iterator;
    typedef typename DTW::Cell_const_iterator                 Cell_const_iterator;
  typedef typename DTW::Facet_const_iterator                Facet_const_iterator;
      typedef typename DTW::Tile_cell_const_iterator                 Tile_cell_const_iterator;
    typedef typename DTW::Tile_facet_const_iterator                Tile_facet_const_iterator;
    typedef typename DTW::Traits                              Traits;
    typedef typename Traits::Point                            Point;
    typedef typename Traits::Vertex_const_handle              Vertex_const_handle;
    typedef typename Traits::Cell_const_handle                Cell_const_handle;
    typedef typename Traits::Cell_handle                      Cell_handle;
    typedef typename Traits::Vertex_handle                    Vertex_handle;


    tbmrf(int nblabs, DTW * t,D_MAP * dm) :  NBL(nblabs), lambda(1), labs(NBL),pLabs(NBL),ly_idx(0),ld_idx(0)
    {
        D = Traits::D;
        tri = t;
        data_map = dm;
        for(int i = 0; i < NBL; i++)
        {
            labs[i] = i;
            pLabs[i] = i*(255.0/((double)NBL));
        }
    }

    //std::vector<Point> parse_points(std::string namefile, int D);
    //void dump_points(std::vector<Point> & points, std::string namefile, int D);


    //  virtual void get_obs_cut_pursuit(Cell_const_iterator fch,double & obs, double & weight);
    virtual double get_score_linear(Cell_const_iterator fch,int label,D_MAP & data_map) = 0;


    void update_simplex()
    {
        // for( auto cit = tri->cells_begin();
        //      cit != tri->cells_end(); ++cit ){
        //   if( tri->is_infinite(cit) )
        //     continue;
        //   cit->data().update_lab();
        // }
        std::cout << "update simplex void" << std::endl;
    }


    void scale_simplex()
    {
        std::cout << "scale simpelx void" << std::endl;
    }


    void init_lab()
    {
        // for( auto cit = tri->cells_begin();
        //      cit != tri->cells_end(); ++cit ){
        //   // if( tri->is_infinite(cit) )
        //   //   continue;
        //   cit->data().lab = 0;
        // }
        return;

    }

    // Init lab function

    void init_idx()
    {
        // Vertex_iterator fvit;
        // int acc = 0;

        // for( auto cit = tri->cells_begin();
        //      cit != tri->cells_end(); ++cit ){
        //   cit->data().opt_idx = acc++;
        // }
        // acc = 0;
        // for(fvit = tri->vertices_begin(); fvit != tri->vertices_end() ; ++fvit){
        //   // if( tri->is_infinite(fvit) )
        //   //   continue;
        //   Vertex_handle v = fvit;
        //   //v->data().opt_idx = acc;
        //   acc++;
        // }
        return;
    }



    void hello()
    {
        std::cout << "hello tbmrf : " << D << std::endl;
    }



    double get_volume(Cell_const_iterator & cci)
    {
        Tile_cell_const_handle fch = cci->full_cell();
        
        std::list<Point> lp;
	std::list<Vertex_const_handle> lvh;
	cci->get_list_vertices(lvh);
        // for(auto vht = fch->vertices_begin() ;
        //         vht != fch->vertices_end() ;
        //         ++vht)
        // {
	for(auto vht : lvh){
	  Vertex_const_handle v = vht;
	  lp.push_back(v->point());

        }
        return n_volume(lp,D);
    }


    double get_goodshape_prior(Cell_const_iterator & cci, int idx)
    {
      return 1;
        Tile_cell_const_handle fch = cci->full_cell();

        std::list<Point> lp;
	std::list<Vertex_const_handle> lvh;
	cci->get_list_vertices(lvh);
	for(auto vht : lvh){
            Vertex_const_handle v = vht;
            if(fch->index(v) == idx)
                continue;
            lp.push_back(v->point());

        }
	double min_d = std::numeric_limits<double>::max();
	double max_d = 0.000001;
	for(int ii = 0; ii < lp.size(); ii++){
	  auto it1 = lp.begin();
	  std::advance(it1,ii);
	  for(int jj = ii+1; jj < lp.size();jj++){
	    auto it2 = lp.begin();
	    std::advance(it2,jj);
	    double dist = CGAL::squared_distance(*it2,*it1);
	    if(dist < min_d)
	      min_d = dist;
	    if(dist > max_d)
	      max_d = dist;
	  }
	}
	double eps = 0.0001;
	if(min_d <= eps)
	  min_d = eps;
        return (sqrt(max_d)/sqrt(min_d));
    }


  bool do_debug(std::list<Point> & lp){
    double xx = 14429.2;
    double yy = 20629.9;
    double zz = 133.75;
    for(auto pp : lp){
      if(abs(pp[0] - xx) < 0.5 &&
	 abs(pp[1] - yy) < 0.5 &&
	 abs(pp[2] - zz) < 0.5 
	 )
	return false;
    }
    return false;
  }
  
    double get_score_surface(Cell_const_iterator & cci, int idx)
    {
        Tile_cell_const_handle fch = cci->full_cell();
	Tile_cell_const_handle fchn = fch->neighbor(idx);
	int idx2 = fch->index(fch);
        std::list<Point> lp;
	std::list<Vertex_const_handle> lvh;
	cci->get_list_vertices(lvh);
	for(auto vht : lvh){
            Vertex_const_handle v = vht;
            if(fch->index(v) == idx)
                continue;
            lp.push_back(v->point());

        }
	double nff = n_surface<Point,Traits>(lp,D);
	double min_d = std::numeric_limits<double>::max();
	double max_d = 0.000001;
	bool do_deb = do_debug(lp);


	  double ccf = 1;
	if(true){
	  Sphere sp1(fch->vertex(0)->point(),
		    fch->vertex(1)->point(),
		    fch->vertex(2)->point(),
		    fch->vertex(3)->point());
	  Sphere sp2(fchn->vertex(0)->point(),
		    fchn->vertex(1)->point(),
		    fchn->vertex(2)->point(),
		    fchn->vertex(3)->point());
	  Plane pp(fch->vertex((idx+1)%4)->point(),
		   fch->vertex((idx+2)%4)->point(),
		   fch->vertex((idx+3)%4)->point());

	  auto center1 = sp1.center();
	  auto proj1 = pp.projection(center1);
	  auto center2 = sp2.center();
	  auto proj2 = pp.projection(center2);
	  auto pp1a = fch->vertex((idx+1)%4)->point();
	  double angle_deg1=CGAL::approximate_angle(center1,pp1a,proj1);
	  double angle_deg2=CGAL::approximate_angle(center2,pp1a,proj2);
	  double ang1 = (angle_deg1)*3.14/180.0;
	  double ang2 = (angle_deg2)*3.14/180.0;
	  ccf = std::min(cos(ang1),cos(ang2));
	
	}
	if(false){
	  if(do_deb)
	    std::cerr << "ddd =====" << std::endl;
	  for(int ii = 0; ii < lp.size(); ii++){
	    auto it1 = lp.begin();
	    std::advance(it1,ii);
	    for(int jj = ii+1; jj < lp.size();jj++){
	      auto it2 = lp.begin();
	      std::advance(it2,jj);
	      double dist = CGAL::squared_distance(*it2,*it1);
	      if(dist < min_d)
		min_d = dist;
	      if(dist > max_d)
		max_d = dist;
	      if(do_deb){

		std::cerr << "ddd " << (*it2) <<  " " << (*it1) << " dist:" << dist << std::endl;
	      }
	    }
	  }
	  double eps = 0.0001;
	  if(min_d <= eps)
	    min_d = eps;
	  eps = 10000;
	  if(max_d > eps)
	    max_d  =eps;
	   ccf = (sqrt(max_d)/sqrt(min_d));
	  if(do_deb){
	    std::cerr << "ddd " << max_d << " " << min_d << " " << ccf << std::endl;
	    std::cerr << "ddd surface:" << nff << std::endl;
	  }
	}
	
        return nff*ccf;
    }


    double get_score_quad(int ch1lab,int chnlab)
    {
        return fabs(ch1lab-chnlab);

    }




    // void opt_gc(int lalpha){


    //   int N = tri->number_of_cells();
    //   int NF = 0;

    //   for(Facet_const_iterator fit = tri->facets_begin();  fit != tri->facets_end(); ++fit){
    //     NF++;
    //   }



    //   GraphType *g = new GraphType(N,NF*2 );
    //   double e0,e1;

    //   int acc = 0;
    //   for( auto cit = tri->cells_begin();
    //        cit != tri->cells_end(); ++cit ){
    //     // if(tri->is_infinite(cit))
    //     //   continue;
    //     cit->data().opt_idx = acc++;
    //     g -> add_node();
    //   }


    //   for( auto cit = tri->cells_begin();
    //        cit != tri->cells_end(); ++cit ){
    //     Cell_const_iterator fch = cit;

    //     // if(tri->is_infinite(fch))
    //     //   continue;
    //     int lcurr = fch->data().lab;
    //     int cid = fch->data().opt_idx;


    //     e0 = get_score_linear(fch,lcurr);
    //     e1 = get_score_linear(fch,lalpha);

    //     //    if(1 == lcurr)

    //     if(tri->is_infinite(fch)){
    //       e0 = 0;
    //       e1 = 0;
    //     }
    //     g->add_tweights(cid, (e0 * MULT), (e1 * MULT));

    //   }



    //   DATA_TYPE E[4];
    //   for(Facet_const_iterator fit = tri->facets_begin();  fit != tri->facets_end(); ++fit){
    //     Cell_const_iterator fch = fit.full_cell();
    //     int idx = fit.index_of_covertex();
    //     Cell_const_iterator fchn = fch->neighbor(idx);
    //     if(tri->is_infinite(fch) && tri->is_infinite(fchn) )
    //       continue;
    //     Vertex_h_iterator vht;

    //     int c1Id = fch->data().opt_idx;
    //     int cnId = fchn->data().opt_idx;

    //     double surface = get_score_surface(fch,idx);
    //     double coef = lambda*surface;

    //     int ch1lab = fch->data().lab;
    //     int chnlab = fchn->data().lab;

    //     E[3] = get_score_quad(ch1lab,chnlab);
    //     E[2] = get_score_quad(ch1lab,lalpha);
    //     E[1] = get_score_quad(lalpha,chnlab);
    //     E[0] = get_score_quad(lalpha,lalpha);

    //     double E_x1 = E[0] - E[2];
    //     double E_bx2 = E[3] - E[2];
    //     // Quadratic term should be positif
    //     double E_quad = -E[0] + E[1] + E[2] - E[3];


    //     if(E_x1 > 0)
    //       g->add_tweights(c1Id, 0 , MULT*E_x1*coef);
    //     else
    //       g->add_tweights(c1Id,-1*MULT*E_x1*coef, 0);
    //     if(E_bx2 > 0)
    //       g->add_tweights(cnId, MULT*E_bx2*coef, 0 );
    //     else
    //       g->add_tweights(cnId,0, -1*MULT*E_bx2*coef);
    //     g->add_edge(c1Id, cnId,    /* capacities */ MULT*E_quad*coef ,0);
    //   }


    //   //std::cout << "\t Max flow algorithm ..." << std::endl;
    //   double flow = g->maxflow();
    //   //std::cout << "\t\t flow value : " << flow << std::endl;
    //   //std::cout << "\t\t Wow! such flow! much cut! good optimizing!" << std::endl;
    //   int nb_merge = 0;

    //   for( auto cit = tri->cells_begin();
    //        cit != tri->cells_end(); ++cit ){
    //     Cell_const_iterator fch = cit;
    //     if(g->what_segment(fch->data().opt_idx) == GraphType::SOURCE){
    //       fch->data().lab = lalpha;
    //       nb_merge++;
    //     }else{

    //     }

    //   }
    //   //std::cerr << "nb merges :" << nb_merge << std::endl;
    //   delete g;

    // }




    // void alpha_exp(){

    //   int IT_MAX = 10;
    //   std::vector<int>::iterator labsit;
    //   int acc = 0;
    //   for(int it = 0; it < IT_MAX;it++){
    //     for(labsit = labs.begin(); labsit != labs.end();++labsit){
    //       int lalpha = *labsit;
    //       //std::cout << "\t\t alpha exp label:" << lalpha << std::endl;
    //       opt_gc(lalpha);
    //       acc++;
    //     }
    //     if(NBL == 2) break;
    //   }
    // }


    void do_cut_pursuit()
    {
        return ;

    }







    void init_lab(DTW & tri)
    {
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            (*cit)->data().lab = 0;
        }

    }


    void init_idx(DTW & tri)
    {
        //  Vertex_iterator fvit;
        //  int acc = 0;

        //  for( auto cit = tri.cells_begin();
        //       cit != tri.cells_end(); ++cit ){
        //    (*cit)->data().opt_idx = acc++;
        // }
        //  acc = 0;
        return;
    }


    void  do_cut_pursuit(DTW & tri)
    {

        return ;

    }


    void alpha_exp(DTW & tri,D_MAP & data_map)
    {

        for(auto lit = labs.begin(); lit != labs.end(); ++lit)
            std::cerr << "ll:"<< *lit << std::endl;

        if(NBL == 2)
        {
            opt_gc(labs.back(),tri,data_map);
            return;
        }

        int IT_MAX = 2;
        std::vector<int>::iterator labsit;
        int acc = 0;
        for(int it = 0; it < IT_MAX; it++)
        {
            for(labsit = labs.begin(); labsit != labs.end(); ++labsit)
            {
                int lalpha = *labsit;
                //std::cerr << "\t\t alpha exp label:" << lalpha << "/" << labs.size() << " it:" << it << "/" << IT_MAX <<  std::endl;
                opt_gc(lalpha,tri,data_map);
                acc++;
            }
        }
        ////std::cerr << "alpha exp finish" << std::endl;
    }


    double get_energy(DTW & tri,D_MAP & data_map)
    {

      
      // recupération nb cells (== nb sommets du graph) et nb triangles (nb edges du graph)
        int  N =   tri.number_of_cells();
        int NF = 0;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

	double acc_energy = 0;
        double e0,e1;
        int acc = 0;

	// ID map est une structure map d'un pointeur de tetraèdre => id
	// ou id = indice des tétraèdres dans le du graph local.
        std::map<Cell_const_iterator,int> id_map;
        std::cerr << "init cell " << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
	  // incrémentaiton de l'indice
            id_map[cit] = acc++;
	    // Construction du noeud.
        }


	// Création des aretes (termes unaires) entre  (S => noeuds) et (noeuds => T)
	// On boucle sur les main => local + shared (id min)
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;

	    // recupération de l'id local du tétraèdre dans le graph
            int cid = id_map[cit];
	    // Lid : l'id local (mais dans la structure "data"
	    int cccid = cit->lid();
	    // En théorie lcurr = 0, mais ici on a une fonction générique.
            int lcurr = data_map[fch->tile()->id()].format_labs[cccid];
	    // Lcurr = 0 et lAlpha = 1 dans le cas binaire.
            acc_energy+=get_score_linear(fch,lcurr,data_map);


        }

	// Ajout des termes quadratiques (on boucle sur les triangles main)
        std::cerr << " ~~~~~ score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
	  // is infinite => pour faire sauter les triangles infinis
            if(fit->is_infinite())
            {
                continue;
            }

            try
            {
	      // recuperation des 2 tetraèdres de la facet
                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
		   !tri.tile_is_loaded(tmp_fchn->main_id()))
                    continue;

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();


                int c1Id = id_map[fch];
                int cnId = id_map[fchn];

		int cccid = fch->lid();
                int cccidn = fchn->lid();
		double gsps = get_goodshape_prior(tmp_fch,tmp_idx);
                double surface = get_score_surface(tmp_fch,tmp_idx);
                double coef = lambda*surface+GSPS_CONST*gsps;

		
                int ch1lab = data_map[fch->tile()->id()].format_labs[cccid];
                int chnlab = data_map[fchn->tile()->id()].format_labs[cccidn];
		acc_energy+= coef*fabs(ch1lab - chnlab);
		

            }
            catch (...)
            {
                continue;
            }
        }



	return acc_energy;

    }


  
  // cherche l'ensemble des labels qui minimisent la fonction d'énergie définie par "get_score_linear & get_score_quad"
  // Lalpha (ici label 1),
  // Tri, la structure de la triangulation de Delaunay
  // Les informations (label courant, dempster shafer, etc) dans chaque tetraèdre.
  // Les étapes sont :
    void opt_gc(int lalpha,DTW & tri,D_MAP & data_map)
    {

      
      // recupération nb cells (== nb sommets du graph) et nb triangles (nb edges du graph)
        int  N =   tri.number_of_cells();
        int NF = 0;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

	// Création du graph (N et NF ne servent qu'à la "pre-alocation")
	// ./services/extern/graphcut/include/graph.h
        GraphType *g = new GraphType(N,NF*2 );
        double e0,e1;
        int acc = 0;

	// ID map est une structure map d'un pointeur de tetraèdre => id
	// ou id = indice des tétraèdres dans le du graph local.
        std::map<Cell_const_iterator,int> id_map;
        std::cerr << "init cell " << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
	  // incrémentaiton de l'indice
            id_map[cit] = acc++;
	    // Construction du noeud.
            g->add_node();
        }


	// Création des aretes (termes unaires) entre  (S => noeuds) et (noeuds => T)
	// On boucle sur les main => local + shared (id min)
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;

	    // recupération de l'id local du tétraèdre dans le graph
            int cid = id_map[cit];

	    // Lid : l'id local (mais dans la structure "data"
	    int cccid = cit->lid();
	    // En théorie lcurr = 0, mais ici on a une fonction générique.
            int lcurr = data_map[fch->tile()->id()].format_labs[cccid];

	    // Lcurr = 0 et lAlpha = 1 dans le cas binaire.
            e0 = get_score_linear(fch,lcurr,data_map);
            e1 = get_score_linear(fch,lalpha,data_map);

	    // Construction des termes unaires
	    // add_tweights(id du tétraèdre, score S->node,score node->T);
            g->add_tweights(cid, (e0 * MULT), (e1 * MULT));

        }

	// Ajout des termes quadratiques (on boucle sur les triangles main)
        std::cerr << " ~~~~~ score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
	  // is infinite => pour faire sauter les triangles infinis
            if(fit->is_infinite())
            {
                continue;
            }

            try
            {
	      // recuperation des 2 tetraèdres de la facet
                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
		   !tri.tile_is_loaded(tmp_fchn->main_id()))
                    continue;

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();


                int c1Id = id_map[fch];
                int cnId = id_map[fchn];

		int cccid = fch->lid();
                int cccidn = fchn->lid();
		double gsps = get_goodshape_prior(tmp_fch,tmp_idx);
                double surface = get_score_surface(tmp_fch,tmp_idx);
                double coef = lambda*surface+GSPS_CONST*gsps;

		
                int ch1lab = data_map[fch->tile()->id()].format_labs[cccid];
                int chnlab = data_map[fchn->tile()->id()].format_labs[cccidn];


                E[3] = get_score_quad(ch1lab,chnlab);
                E[2] = get_score_quad(ch1lab,lalpha);
                E[1] = get_score_quad(lalpha,chnlab);
                E[0] = get_score_quad(lalpha,lalpha);

                double E_x1 = E[0] - E[2];
                double E_bx2 = E[3] - E[2];

                // Quadratic term should be positif
                double E_quad = -E[0] + E[1] + E[2] - E[3];
 
                if(E_x1 > 0)
                    g->add_tweights(c1Id, 0, MULT*E_x1*coef);
                else
                    g->add_tweights(c1Id,-1*MULT*E_x1*coef, 0);
                if(E_bx2 > 0)

                    g->add_tweights(cnId, MULT*E_bx2*coef, 0 );
                else
                    g->add_tweights(cnId,0, -1*MULT*E_bx2*coef);
                g->add_edge(c1Id, cnId,    /* capacities */ MULT*E_quad*coef,0);

            }
            catch (...)
            {
                continue;
            }
        }


        std::cerr << "\t Max flow algorithm ..." << std::endl;
        double flow = g->maxflow();
        std::cerr << "\t\t flow value : " << flow << std::endl;
        int nb_merge = 0;


	// On reboucle sur les tets main
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;
	    // recuperation de l'id
            int cid = id_map[fch];
	    // Ici si le noeud (cid) est connecté à la source, c'est que la coupe est entre le noeud (cid) et le puits (score minimum, inclus dans la mincut)
            if(g->what_segment(cid) == GraphType::SOURCE)
            {
	      int cccid = cit->lid();
	      if( data_map.find(fch->tile()->id()) == data_map.end()){
		std::cerr << "ERROR, NO CELL LOAD, WHY??" << std::endl;
                    continue;
	      }
                data_map[fch->tile()->id()].format_labs[cccid] = lalpha;
                nb_merge++;
            }
            else
            {

            }

        }
        std::cerr << "nb merges :" << nb_merge << "/"<< N << std::endl;
        delete g;

    }




    // cherche l'ensemble des labels qui minimisent la fonction d'énergie définie par "get_score_linear & get_score_quad"
  // Lalpha (ici label 1),
  // Tri, la structure de la triangulation de Delaunay
  // Les informations (label courant, dempster shafer, etc) dans chaque tetraèdre.
  // Les étapes sont :


  //
  typedef std::tuple<Id,double,double,double>                                EdgeData;
  void opt_gc_lagrange(int lalpha,DTW & tri,D_MAP & data_map,std::map<Id,std::map<Id,EdgeData> > shared_data_map,int tid_k, bool use_weight = true) 
    {


      //  Tile_iterator  tile_k  = tri.get_tile(tid_k);
      Tile_const_iterator tile_k = tri.get_const_tile(tid_k);
      int D  = tile_k->current_dimension();
      const DT & ttri1 = tile_k->triangulation();
      // recupération nb cells (== nb sommets du graph) et nb triangles (nb edges du graph)
      int  N =   ttri1.number_of_cells();
      int NF = 0;
      for(auto fit = tile_k->facets_begin();  fit != tile_k->facets_end(); ++fit)
        {
	  NF++;
        }

      	// Création du graph (N et NF ne servent qu'à la "pre-alocation")
      	// ./services/extern/graphcut/include/graph.h
        GraphType *g = new GraphType(N,NF*2 );
        double e0,e1,c_i,c_j;
        int acc = 0;

      	// ID map est une structure map d'un pointeur de tetraèdre => id
      	// ou id = indice des tétraèdres dans le du graph local.
        std::map<Tile_cell_const_iterator,int> id_map;
        for( auto cit = tile_k->cells_begin();
                cit != tile_k->cells_end(); ++cit )
        {
      	  // incrémentaiton de l'indice
            id_map[cit] = acc++;
      	    // Construction du noeud.
            g->add_node();
        }


      	// Création des aretes (termes unaires) entre  (S => noeuds) et (noeuds => T)
      	// On boucle sur les main => local + shared (id min)
        for( auto cit = tile_k->cells_begin();
                cit != tile_k->cells_end(); ++cit )
        {

      	  Cell_const_iterator fch = Cell_const_iterator(tile_k,tile_k, tile_k, cit);

	  // recupération de l'id local du tétraèdre dans le graph
	  int cid = id_map[cit];
	  // Lid : l'id local (mais dans la structure "data"
	  int cccid = tile_k->lid(cit);
	  // Get current label
	  int lcurr = data_map[tid_k].format_labs[cccid];
	  lcurr = 0;


	  // === Lagrangian stuff for mixed cell ===
	  double lag_acc = 0;
	  int card_shared = 1;


	  if(shared_data_map.size() > 0 && tile_k->cell_is_mixed(cit)){
	    std::unordered_set<Id> idSet ;
	    // Number of time the cell is duplicated
	    if(use_weight){
	      for(int l=0; l<=D ; ++l){
		Id tid_l = tile_k->id(tile_k->vertex(cit,l));
		idSet.insert(tid_l);
	      }
	      card_shared = idSet.size();
	    }
	    idSet.clear();
	    for(int l=0; l<=D; ++l){
	      // Get current lagrangian
	      Id tid_l = tile_k->id(tile_k->vertex(cit,l));
	      if (idSet.find(tid_l) != idSet.end()){
		continue;
	      }
	      idSet.insert(tid_l);
	      double lag_kl = std::get<1>(shared_data_map[tid_l][cccid]);
	      if(tid_l == tid_k)
		continue;
	      lag_acc += (tid_k < tid_l ? -1 : 1)*lag_kl;
	    }
	  }

	  // Lcurr = 0 et lAlpha = 1 dans le cas binaire.
	  c_i = get_score_linear(fch,lcurr,data_map);
	  c_j = get_score_linear(fch,lalpha,data_map);

	  // Update with the lagrangian

	  
	  e0 = c_i/((double)card_shared); 
	  e1 = c_j/((double)card_shared);
	  // if(tid_k == 3 && tile_k->cell_is_mixed(cit) )
	  //   std::cerr << "lid_k:" << cccid << " e0: " << e0 << " e1:" << e1 << " lag:" << lag_acc << " card" << card_shared <<  std::endl;

	  
	  if(lag_acc > 0)
	    e0 += lag_acc;
	  else
	    e1 += -lag_acc;

	  // if(tid_k == 3 && tile_k->cell_is_mixed(cit) )
	  //   std::cerr << "lid_k:" << cccid << " new_e0: " << e0 << " new_e1:" << e1  <<  std::endl;
	  // Construction des termes unaires
	  // add_tweights(id du tétraèdre, score S->node,score node->T);
	  g->add_tweights(cid, (e0 * MULT), (e1 * MULT));

        }

      	// Ajout des termes quadratiques (on boucle sur les triangles main)
        std::cerr << " ~~~~~ score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tile_k->facets_begin();  fit != tile_k->facets_end(); ++fit)
        {
      	  // is infinite => pour faire sauter les triangles infinis
            if(tile_k->facet_is_foreign(fit))
            {
                continue;
            }

            try
            {
	      
      	      // recuperation des 2 tetraèdres de la facet
                auto tmp_fch = tile_k->full_cell(fit);
                int tmp_idx = tile_k->index_of_covertex(fit);
                auto tmp_fchn = tmp_fch->neighbor(tmp_idx);
		// Nombre fe fois la facet est dupliquée
		double card_shared = 1; 
		if(tile_k->facet_is_mixed(fit) && use_weight){
		  std::unordered_set<Id> idSet ; 
		  for(int i=0; i<=D; ++i)
		    {
		      if(i == tmp_idx)
			continue;
		      Id tid_l = tile_k->id(tile_k->vertex(tmp_fch,i));
		      idSet.insert(tid_l);
		    }
		   card_shared = idSet.size();
		  //		  std::cerr << "card" << card_shared << std::endl;
		}
		//		card_shared=1;

                Cell_const_iterator fch = Cell_const_iterator(tile_k,tile_k, tile_k, tmp_fch);
                int idx = tmp_idx;
                Cell_const_iterator fchn = Cell_const_iterator(tile_k,tile_k, tile_k,tmp_fchn);


                int c1Id = id_map[tmp_fch];
                int cnId = id_map[tmp_fchn];

      		int cccid = fch->lid();
                int cccidn = fchn->lid();
		double gsps = get_goodshape_prior(fch,tmp_idx);
                double surface = get_score_surface(fch,tmp_idx);
                double coef = (lambda*surface+GSPS_CONST*gsps)/((double)card_shared);
                int ch1lab = data_map[fch->tile()->id()].format_labs[cccid];
                int chnlab = data_map[fchn->tile()->id()].format_labs[cccidn];
		ch1lab = chnlab = 0;

                E[3] = get_score_quad(ch1lab,chnlab) ;
                E[2] = get_score_quad(ch1lab,lalpha) ;
                E[1] = get_score_quad(lalpha,chnlab) ;
                E[0] = get_score_quad(lalpha,lalpha) ;


		
                double E_x1 = E[0] - E[2];
                double E_bx2 = E[3] - E[2];

                // Quadratic term should be positif
                double E_quad = -E[0] + E[1] + E[2] - E[3];
 
                if(E_x1 > 0)
                    g->add_tweights(c1Id, 0, MULT*E_x1*coef);
                else
                    g->add_tweights(c1Id,-1*MULT*E_x1*coef, 0);
                if(E_bx2 > 0)
                    g->add_tweights(cnId, MULT*E_bx2*coef, 0 );
                else
                    g->add_tweights(cnId,0, -1*MULT*E_bx2*coef);
                g->add_edge(c1Id, cnId,    /* capacities */ MULT*E_quad*coef,0);

            }
            catch (...)
            {
                continue;
            }
        }


        std::cerr << "\t Max flow algorithm ..." << std::endl;
        double flow = g->maxflow();
        std::cerr << "\t\t flow value : " << flow << std::endl;
        int nb_merge = 0;


      	// On reboucle sur les tets main
        for( auto cit = tile_k->cells_begin();
                cit != tile_k->cells_end(); ++cit )
        {
	  Cell_const_iterator fch = Cell_const_iterator(tile_k,tile_k, tile_k,cit);
	  // recuperation de l'id
            int cid = id_map[cit];
      	    // Ici si le noeud (cid) est connecté à la source, c'est que la coupe est entre le noeud (cid) et le puits (score minimum, inclus dans la mincut)

	    int cccid = fch->lid();
	    if( data_map.find(fch->tile()->id()) == data_map.end()){
	      std::cerr << "ERROR, NO CELL LOAD, WHY??" << std::endl;
	      continue;
	    }
            if(g->what_segment(cid) == GraphType::SOURCE){	    
	      data_map[fch->tile()->id()].format_labs[cccid] = 1;
	      nb_merge++;
	    }else{
	      data_map[fch->tile()->id()].format_labs[cccid] = 0;
	    }

	

        }
        std::cerr << "nb merges :" << nb_merge << "/"<< N << std::endl;
        delete g;

    }




      void opt_qpbo(int lalpha,DTW & tri,D_MAP & data_map)
    {

        int  N =   tri.number_of_cells();
        int NF = 0;


        std::cerr << "init facet QPBO " << std::endl;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

	typedef double FLOAT;
	QPBO<FLOAT>* q;
	q = new QPBO<FLOAT>(N, NF); 
	//	q->AddNode(N); // add two nodes	  

        double e0,e1;

        int acc = 0;
        std::map<Cell_const_iterator,int> id_map;
        std::cerr << "init cell " << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            // if(tri.is_infinite(cit))
            //   continue;
	  q->AddNode(); // add two nodes	  
	  id_map[cit] = acc++;
        }


        std::cerr << "score cell " << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;

            // if(tri->is_infinite(fch))
            //   continue;

            int cid = id_map[cit];
	    //            int cccid = cit->cell_data().id;
	    int cccid = cit->lid();

            int lcurr = data_map[fch->tile()->id()].format_labs[cccid];

            e0 = get_score_linear(fch,lcurr,data_map);
            e1 = get_score_linear(fch,lalpha,data_map);
            q->AddUnaryTerm(cid, (e0 * MULT), (e1 * MULT));

        }


        std::cerr << " ~~~~~ score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            if(fit->is_infinite())
            {
	      //                std::cerr << " ~~~~~ is infinit" << std::endl;
                continue;
            }

            try
            {


                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
                        !tri.tile_is_loaded(tmp_fchn->main_id()))
                    continue;

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();

                int c1Id = id_map[fch];
                int cnId = id_map[fchn];

                // if( c1Id ==  cnId  || c1Id == 0 || cnId == 0 ){
                //   continue;
                // }


                // int cccid = fch->cell_data().id;
                // int cccidn = fchn->cell_data().id;
		int cccid = fch->lid();
                int cccidn = fchn->lid();
		double gsps = get_goodshape_prior(tmp_fch,tmp_idx);
                double surface = get_score_surface(tmp_fch,tmp_idx);
                double coef = lambda*surface+GSPS_CONST*gsps;

                int ch1lab = data_map[fch->tile()->id()].format_labs[cccid];
                int chnlab = data_map[fchn->tile()->id()].format_labs[cccidn];



                E[3] = get_score_quad(ch1lab,chnlab);
                E[2] = get_score_quad(ch1lab,lalpha);
                E[1] = get_score_quad(lalpha,chnlab);
                E[0] = get_score_quad(lalpha,lalpha);
		q->AddPairwiseTerm(c1Id,cnId,E[0]*coef,E[1]*coef,E[2]*coef,E[3]*coef); // add term (x+1)*(y+2)

            }
            catch (...)
            {
                continue;
            }
        }


        std::cerr << "\t Start qpbo ..." << std::endl;
	q->MergeParallelEdges();
	std::cerr << "\t Solve ..." << std::endl;
	q->Solve();
	std::cerr << "\t Weak.. ..." << std::endl;
	q->ComputeWeakPersistencies();
	// q->Solve();

        int nb_merge = 0;

        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;
            int cid = id_map[fch];
	    int lab = q->GetLabel(cid);
	    
            if(lab == 1)
            {
	      int cccid = cit->lid();
                if( data_map.find(fch->tile()->id()) == data_map.end())
                    continue;
                data_map[fch->tile()->id()].format_labs[cccid] = lalpha;
                nb_merge++;
            }
            else
            {

            }

        }
        std::cerr << "nb merges :" << nb_merge << "/"<< N << std::endl;
        delete q;

    }






      void opt_belief(int lalpha,DTW & tri,D_MAP & data_map)
    {


      typedef double                                                               ValueType;          // type used for values
      typedef size_t                                                               IndexType;          // type used for indexing nodes and factors (default : size_t)
      typedef size_t                                                               LabelType;          // type used for labels (default : size_t)
      typedef opengm::Adder                                                        OpType;             // operation used to combine terms
      typedef opengm::ExplicitFunction<ValueType,IndexType,LabelType>              ExplicitFunction;   // shortcut for explicite function
      typedef opengm::meta::TypeListGenerator<ExplicitFunction>::type              FunctionTypeList;   // list of all function the model cal use (this trick avoids virtual methods) - here only one
      typedef opengm::DiscreteSpace<IndexType, LabelType>                          SpaceType;          // type used to define the feasible statespace
      typedef opengm::GraphicalModel<ValueType,OpType,FunctionTypeList,SpaceType>  Model;              // type of the model
      typedef Model::FunctionIdentifier                                            FunctionIdentifier; // type of the function identifier    

      
        int  N =   tri.number_of_cells();
        int NF = 0;


        std::cerr << "init facet BELIEF " << std::endl;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

	
	LabelType * numbersOfLabels = new LabelType[N];
	for(int i = 0; i < N;i++)
	  numbersOfLabels[i] = 2;

	Model gm(SpaceType(numbersOfLabels, numbersOfLabels + N));
	std::cerr << "num var belief :" << N << "," << gm.numberOfVariables() << std::endl;;
	  

	// construct a graphical model with 
	// - addition as the operation (template parameter Adder)
	// - support for Potts functions (template parameter PottsFunction<double>)

	
	
        double e0,e1;

        int acc = 0;
        std::map<Cell_const_iterator,int> id_map;
        std::cerr << "init cell " << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            // if(tri.is_infinite(cit))
            //   continue;
	  //	  q->AddNode(); // add two nodes

	  id_map[cit] = acc++;
        }


        std::cerr << "score cell " << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;

            // if(tri->is_infinite(fch))
            //   continue;

            int cid = id_map[cit];
	    //            int cccid = cit->cell_data().id;
	    int cccid = cit->lid();

            int lcurr = data_map[fch->tile()->id()].format_labs[cccid];

            e0 = get_score_linear(fch,lcurr,data_map);
            e1 = get_score_linear(fch,lalpha,data_map);

	    // =========
	    const LabelType shape[] = {2};
	    ExplicitFunction f(shape, shape + 1);

	    // f(0) = reg(e0);
	    // f(1) = reg(e1);
	    f(0) = -e0;
	    f(1) = -e1;

	    // add function
	    FunctionIdentifier id = gm.addFunction(f);
	    // add factor
	    IndexType variableIndex[] = {cid};
	    gm.addFactor(id, variableIndex, variableIndex + 1);
	    
	    //            q->AddUnaryTerm(cid, (e0 * MULT), (e1 * MULT));

        }


        std::cerr << " ~~~~~ score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            if(fit->is_infinite())
            {
	      //                std::cerr << " ~~~~~ is infinit" << std::endl;
                continue;
            }

            try
            {


                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
                        !tri.tile_is_loaded(tmp_fchn->main_id()))
                    continue;

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();

                int c1Id = id_map[fch];
                int cnId = id_map[fchn];

                // if( c1Id ==  cnId  || c1Id == 0 || cnId == 0 ){
                //   continue;
                // }


                // int cccid = fch->cell_data().id;
                // int cccidn = fchn->cell_data().id;
		int cccid = fch->lid();
                int cccidn = fchn->lid();
		double gsps = get_goodshape_prior(tmp_fch,tmp_idx);
                double surface = get_score_surface(tmp_fch,tmp_idx);
                double coef = lambda*surface+GSPS_CONST*gsps;

                int ch1lab = data_map[fch->tile()->id()].format_labs[cccid];
                int chnlab = data_map[fchn->tile()->id()].format_labs[cccidn];



                E[3] = get_score_quad(ch1lab,chnlab);
                E[2] = get_score_quad(ch1lab,lalpha);
                E[1] = get_score_quad(lalpha,chnlab);
                E[0] = get_score_quad(lalpha,lalpha);



		IndexType vars[]  = {c1Id,cnId}; 
		LabelType shape[] = {2,2};
		LabelType state[] = {0,0};
		ExplicitFunction f(shape, shape + 2);

		int cum = 0;
		for(state[0] = 0; state[0] < gm.numberOfLabels(0); ++state[0]){
		  for(state[1] = 0; state[1] < gm.numberOfLabels(1); ++state[1]) {
		    f(state[0], state[1]) = -coef*E[cum++]; 
		  }
		}

		
		// add function
		FunctionIdentifier fid = gm.addFunction(f);
		// add factor

		std::sort(vars, vars + 2);
		gm.addFactor(fid, vars, vars + 2);
		
		//q->AddPairwiseTerm(c1Id,cnId,E[0]*coef,E[1]*coef,E[2]*coef,E[3]*coef); // add term (x+1)*(y+2)

            }
            catch (...)
            {
                continue;
            }
        }


	std::cerr << " ~~~~~ start solving " << std::endl;

	
	// q->Solve();
	// set up the optimizer (loopy belief propagation)
	typedef opengm::BeliefPropagationUpdateRules<Model, opengm::Maximizer> UpdateRules;
	typedef opengm::MessagePassing<Model, opengm::Maximizer, UpdateRules, opengm::MaxDistance> BeliefPropagation;
	const size_t maxNumberOfIterations = 50;
	const double convergenceBound = 1e-7;
	const double damping = 0;
	BeliefPropagation::Parameter parameter(maxNumberOfIterations, convergenceBound, damping);
	BeliefPropagation bp(gm, parameter);
	std::cerr << " ~~~~~ graph created " << std::endl;	
	//	optimize (approximately)
	BeliefPropagation::VerboseVisitorType visitor;
	bp.infer(visitor);
	std::cerr << " ~~~~~ solving done " << std::endl;
	// obtain the (approximate) argmin
	std::vector<size_t> labeling(N);
	bp.arg(labeling);


	std::cerr << " ~~~~~ solving done " << std::endl;
        int nb_merge = 0;

        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;
            int cid = id_map[fch];
	    //	    	    int lab = q->GetLabel(cid);
	    int lab = labeling[cid];
            if(lab == 1)
            {
	      int cccid = cit->lid();
                if( data_map.find(fch->tile()->id()) == data_map.end())
                    continue;
                data_map[fch->tile()->id()].format_labs[cccid] = lalpha;
                nb_merge++;
            }
            else
            {

            }

        }

	
	delete [] numbersOfLabels;
        std::cerr << "nb merges :" << nb_merge << "/"<< N << std::endl;
	//        delete q;

    }


  

    // int gc_on_stream(std::istream & ifile,std::ostream & ofile){

    //   int nb_dat,tt;
    //   std::vector<g_edge> v_edges;
    //   std::vector<g_vert> v_verts;

    //   int acc = 0;
    //   while(ifile  >> tt){
    //     std::cerr << "acc:" << acc++ << "tt:" << tt << std::endl;
    //     //      for(int i = 0; i < nb_dat;i++){
    //     //	ifile >> tt;
    // 	switch(tt) {
    // 	case 1 : {
    // 	  g_vert gv;
    // 	  ifile >> gv.id1 >> gv.e0 >> gv.e1;
    // 	  v_verts.push_back(gv);
    // 	  break;
    // 	}
    // 	case 2 : {
    // 	  g_edge ge;
    // 	  ifile >> ge.id1 >> ge.id2 >> ge.e0 >> ge.e1;
    // 	  v_edges.push_back(ge);
    // 	  break;
    // 	}
    // 	}
    //     }
    //   //    }
    //   int N = v_verts.size();
    //   int NF = v_edges.size();
    //   std::cerr << "N:" << N << " NF:" << NF <<  std::endl;
    //   GraphType *g = new GraphType(N,NF*2 );
    //   std::cerr << "init graph struct " << std::endl;
    //   for(int i = 0; i < v_verts.size(); i++){
    //     g -> add_node();
    //   }

    //   for(auto vv = v_verts.begin();
    // 	vv != v_verts.end(); vv++){
    //     g->add_tweights(vv->id1, vv->e0, vv->e1);
    //   }

    //   for(auto ee = v_edges.begin();
    // 	ee != v_edges.end(); ee++){
    //     g->add_edge(ee->id1, ee->id2,ee->e0 ,ee->e1);
    //   }

    //   std::cerr << "\t Max flow algorithm ..." << std::endl;
    //   double flow = g->maxflow();
    //   std::cerr << "\t\t flow value : " << flow << std::endl;
    //   int nb_merge = 0;

    //   for(int i = 0; i < N; i++){
    //     int lab = g->what_segment(i);
    //     ofile << i << " " << lab << std::endl;
    //   }


    //   delete g;
    //   std::cerr << "perfect!" << std::endl;
    //   return 0;
    // }

    // void extract_gc_graph(int lalpha,DTW & tri,D_MAP & data_map, std::vector<int> & tile_ids,std::ostream & ofile){
    //   int MULT_2 = 1;

    //   int sourceId = 0;
    //   int targetId = 1;

    //   int  N = tri.number_of_cells();
    //   int NF = 0;
    //   std::cerr << "init graph" << std::endl;
    //   for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit){
    //     NF++;
    //   }

    //   if(D == 3)
    //     MULT_2=1000000;


    //   double e0,e1;

    //   int acc = 0;
    //   std::map<Cell_const_iterator,int> id_map;

    //   std::cerr << "create ids" << std::endl;
    //   for( auto cit = tri.cells_begin();
    // 	 cit != tri.cells_end(); ++cit ){
    //     // if(tri.is_infinite(cit))
    //     //    continue;
    //     id_map[cit] = acc++;
    //   }


    //       std::cerr << "score simplex" << std::endl;
    //   for( auto cit = tri.cells_begin();
    // 	 cit != tri.cells_end(); ++cit ){
    //     Cell_const_iterator fch = *cit;

    //     if(cit->main_id() != main_tile_id)
    // 	  continue;
    //     // if(tri->is_infinite(fch))
    //     //    continue;
    //     int tid = cit->tile()->id();
    //     int lid = cit->cell_data().id;
    //     int gid = data_map[tid].format_gids[lid];

    //     int lcurr = 0; //data_map[fch->tile()->id()].format_labs[cccid];
    //     e0 = get_score_linear(fch,lcurr,data_map);
    //     e1 = get_score_linear(fch,lalpha,data_map);

    //     ofile << sourceId << " " <<  l2gid(gid) << " " << (e0 * MULT_2) << std::endl;
    //     ofile << l2gid(gid) << " " << targetId << " " <<  (e1 * MULT_2) << std::endl;
    //     //ofile << "v" << " " <<  l2gidf(cid,tile_ids) << " " << cid << " " <<  cid << std::endl;
    //   }



    //   std::cerr << "score facet " << std::endl;
    //   DATA_TYPE E[4];

    //       for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit){
    //     if(fit->is_infinite())
    //     	continue;
    //     try {
    // 	if(fit->main_id() != main_tile_id)
    // 	  continue;


    // 	Cell_const_iterator tmp_fch = fit.full_cell();
    // 	int tmp_idx = fit.index_of_covertex();
    // 	Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);


    // 	if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
    // 	    !tri.tile_is_loaded(tmp_fchn->main_id())){
    // 	   std::cerr << "ERROR : CELL NOT LOADED" << std::endl;
    // 	   return 1;
    // 	   continue;
    // 	 }

    // 	Cell_const_iterator fch = tmp_fch->main();
    // 	int idx = tmp_idx;
    // 	Cell_const_iterator fchn = tmp_fchn->main();


    // 	Vertex_h_iterator vht;


    // 	int lidc = fch->cell_data().id;
    // 	int lidn = fchn->cell_data().id;

    // 	int tidc = fch->tile()->id();
    // 	int tidn = fchn->tile()->id();

    // 	int gidc = data_map[tidc].format_gids[lidc];
    // 	int gidn = data_map[tidn].format_gids[lidn];

    // 	double surface = get_score_surface(tmp_fch,tmp_idx);
    // 	double coef = lambda*surface+GSPS_CONST*gsps;


    // 	int ch1lab = data_map[tidc].format_labs[lidc];
    // 	int chnlab = data_map[tidn].format_labs[lidn];



    // 	E[3] = get_score_quad(ch1lab,chnlab);
    // 	E[2] = get_score_quad(ch1lab,lalpha);
    // 	E[1] = get_score_quad(lalpha,chnlab);
    // 	E[0] = get_score_quad(lalpha,lalpha);

    // 	double E_x1 = E[0] - E[2];
    // 	double E_bx2 = E[3] - E[2];
    // 	// Quadratic term should be positif
    // 	double E_quad = -E[0] + E[1] + E[2] - E[3];
    // 	ofile << "e " << gidc << " " << gidn  << " ";

    // 	if(E_x1 > 0){
    // 	  //g->add_tweights(c1Id, 0 , MULT*E_x1*coef);
    // 	  ofile << l2gid(gidc) << " "  << targetId  << " " << MULT_2*E_x1*coef  << std::endl;
    // 	}else{
    // 	  //g->add_tweights(c1Id,-1*MULT_2*E_x1*coef, 0);
    // 	  ofile << sourceId << " " << l2gid(gidc) << " "  << -1*MULT_2*E_x1*coef  << std::endl;
    // 	}if(E_bx2 > 0){
    // 	  //g->add_tweights(cnId, MULT_2*E_bx2*coef, 0 );
    // 	  ofile << sourceId << " " << l2gid(gidn) << " "  << MULT_2*E_bx2*coef   << std::endl;
    // 	}else{
    // 	  // g->add_tweights(cnId,0, -1*MULT_2*E_bx2*coef);
    // 	  ofile <<  l2gid(gidn) << " "  << targetId << " " << -1*MULT_2*E_bx2*coef  << std::endl;

    // 	}
    // 	// g->add_edge(c1Id, cnId,    /* capacities */ MULT_2*E_quad*coef ,0);
    // 	ofile  << l2gid(gidc) << " " << l2gid(gidn) << " " << MULT_2*E_quad*coef <<  std::endl;
    //     }
    //     catch (...) {
    // 	continue;
    //     }
    //   }
    // }
  double reg(double v,double mm = -1){
    return -v;
    //return -(v/mm)*4.60 - 0.01;
    //return -log(1+v)
  }

  
  double reg2(double v,double mm){
    //return(-(v/mm)*(0.69) - 0.02);
    //return -log(1+v);
        return reg(v,mm);
  }

  double reg3(double v,double mm){
    //return(-(v/mm)*(0.69) - 0.02);
    //return -log(1+v);
    return v;
  }

  double reg1(double v,double mm){
    //return(-(v/mm)*(4.60) - 0.02);
    return reg(v,mm);
  }

  
  int extract_stream_graph_v2(int lalpha,DTW & tri,D_MAP & data_map, std::map<int,std::vector<int>> & tile_ids,std::ostream & ofile, int main_tile_id, int gtype, int area_processed, double coef_mult)
  {
    ofile << std::fixed << std::setprecision(15);

    int chunk_size = 10;
    int sourceId = 0;
    int targetId = 1;
    double MULT_2 = coef_mult;
    int  N = tri.number_of_cells();
    int NF = 0;
    double mv = 1000;
    std::cerr << "COEF_MULT" << MULT_2 << " LAMBDA:" << lambda << std::endl;
    switch(gtype)
      {
      case 0 :
        {
	  chunk_size = 1;
	  break;
        }

      case 1 :
        {
	  chunk_size = 1;
	  break;
        }
      case 2 :
        {
	  break;
        }
      }



    std::cerr << "init graph" << std::endl;
    for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
      {
	NF++;
      }

    

    GraphType *g = new GraphType(N,NF*2 );


    double e0,e1,e2,e3;
    int acc = 0;

    std::map<Cell_const_iterator,int> id_map;
    std::map<int,int> gid_map;
    std::vector<int> id2gid_vec;
    std::cerr << "create ids" << std::endl;

    if(gtype == 1){
    for( auto cit = tri.cells_begin();
	 cit != tri.cells_end(); ++cit )
      {

	
	id_map[cit] = acc++;
	gid_map[id_map[cit]] = cit->gid();
	g->add_node();
      }
    }

    std::cerr << "score simplex" << std::endl;
    std::vector<std::vector<double> > v_vertex;
    std::vector<std::vector<double> > v_edge;
    double v_max = 0;


    
    for( auto cit = tri.cells_begin();
	 cit != tri.cells_end(); ++cit )
      {
	Cell_const_iterator fch = *cit;
	if(cit->main_id() != main_tile_id)
	  continue;
	if(area_processed > 1)
	  continue;


	// if(tri->is_infinite(fch))
	//    continue;
	int tid = cit->tile()->id();
	//int lid = cit->cell_data().id;
	int lid = cit->lid();
	int gid = cit->gid();//data_map[tid].format_gids[lid];

	int linit = 0;
	int lcurr = data_map[tid].format_labs[lid];
	e0 = get_score_linear(fch,linit,data_map);
	e1 = get_score_linear(fch,lalpha,data_map);
	// e2 = (lcurr == linit) ? e2 : e2*1000;
	// e3 = (lcurr == linit) ? e3: e3*1000;
	 if(e0 > v_max)
	   v_max = e0;
	 if(e1 > v_max)
	   v_max = e1;
	switch(gtype)
	  {
	  case 0 :
            {
	      // Belief spark
	      //ofile << "v " <<   gid  << " " << e0*MULT_2 << " " <<  e1*MULT_2 ;
	      v_vertex.push_back(std::vector<double>({(double)gid,e0,e1}));
	      //	      if(++acc % chunk_size == 0) ofile << std::endl;
	      break;       // and exits the switch
            }

	  case 1 :
            {
	      g->add_tweights(id_map[cit],e0*MULT_2,e1*MULT_2);
	      // v_vertex.push_back(std::vector<double>({(double)l2gid(gid),0}));
	      // v_edge.push_back(std::vector<double>({(double)sourceId,(double)l2gid(gid),e0}));
	      // v_edge.push_back(std::vector<double>({(double)l2gid(gid),(double)targetId,e1}));
	      break;
            }
	  case 2 :
            {
	      // Graph cut c++

	      // 1 : vertex
	      ofile << "1 " <<   gid  << " " << (e0 * MULT_2) << " " <<  (e1 * MULT_2) << " ";
	      if(++acc % chunk_size == 0) ofile << std::endl;
	      break;
            }
	  }

      }

    std::cerr << "score facet " << std::endl;
    DATA_TYPE E[4];

    // std::ofstream myfile;
    // std::string filename("/home/laurent/shared_spark/tmp/voronoi.xyz");
    // myfile.open (filename);

    
    for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
      {
	if(fit->is_infinite())
	  continue;
	try
	  {

	    Cell_const_iterator tmp_fch = fit.full_cell();
	    int tmp_idx = fit.index_of_covertex();
	    Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

	    if(
	       (area_processed == 1 && tmp_fch->main_id() != tmp_fchn->main_id()) ||
	       (area_processed == 2 && tmp_fch->main_id() == tmp_fchn->main_id()))
	      {
		continue;
	      }
	    // if(
	    //    (area_processed == 1 && !fit->is_local() ) ||
	    //    (area_processed == 2 && fit->is_local()))
	    //   continue;

	    if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
	       !tri.tile_is_loaded(tmp_fchn->main_id()))
	      {
		//std::cerr << "ERROR : CELL NOT LOADED" << std::endl;
		//	   return 0;
		continue;
	      }

	    auto bb1 = tmp_fch->barycenter();
	    auto bb2 = tmp_fchn->barycenter();

	    // for(int i = 0; i < 10; i++){
	    //   for(int j = 0; j < 3;j++){
	    // 	myfile << (bb1[j] + (bb2[j]-bb1[j])*(i/10.0)) << " ";
	    //   }
	    //   myfile << std::endl;
	    // }
	    

	    Cell_const_iterator fch = tmp_fch->main();
	    int idx = tmp_idx;
	    Cell_const_iterator fchn = tmp_fchn->main();


	    int lidc = fch->lid();//cell_data().id;
	    int lidn = fchn->lid();//cell_data().id;

	    int tidc = fch->tile()->id();
	    int tidn = fchn->tile()->id();


	    int gidc = fch->gid();//data_map[tidc].format_gids[lidc];
	    int gidn = fchn->gid();//data_map[tidn].format_gids[lidn];

	    // std::cerr << "lidc" << lidc << " lidn:" << lidn << std::endl;
	    // std::cerr << "gidc" << gidc << " gidn:" << gidn << std::endl;
	    // std::cerr << "lab.size : " << data_map[tidn].format_labs.size();
	    double gsps = get_goodshape_prior(tmp_fch,tmp_idx);
	    double surface = get_score_surface(tmp_fch,tmp_idx);
	    double coef = lambda*surface+GSPS_CONST*gsps;

	    int ch1lab = data_map[tidc].format_labs[lidc];
	    int chnlab = data_map[tidn].format_labs[lidn];


	    E[3] = get_score_quad(ch1lab,chnlab);
	    E[2] = get_score_quad(ch1lab,lalpha);
	    E[1] = get_score_quad(lalpha,chnlab);
	    E[0] = get_score_quad(lalpha,lalpha);

	    double E_x1 = E[0] - E[2];
	    double E_bx2 = E[3] - E[2];
	    // Quadratic term should be positif
	    double E_quad = -E[0] + E[1] + E[2] - E[3];

		
	    switch(gtype)
	      {
	      case 0 :
                {
		  for(int i = 0 ; i < 4;i++){
		    E[i] = E[i]*coef;
		    if(E[i] > v_max)
		      v_max = E[i];
		  }
		  // Belief spark

		  v_edge.push_back(std::vector<double>({(double)gidc,(double)gidn,E[0],E[1],E[2],E[3]}));
		  // ofile << "e " << gidc << " " << gidn  << " ";
		  // ofile << MULT_2*E[0] << " ";
		  // ofile << MULT_2*E[1] << " ";
		  // ofile << MULT_2*E[2] << " ";
		  // ofile << MULT_2*E[3];
		  // for(int i = 0 ; i < 4; i++)
		  // {
		  //   ofile << MULT_2*E[i]*coef;
		  //     if(i <3)
		  //         ofile << " ";
		  // }
		  //		  if(++acc % chunk_size == 0) ofile << std::endl;
		  break;
                }
	      case 1 :
                {
		  // if(E_x1 > 0)
		  //     v_edge.push_back(std::vector<double>({(double)l2gid(gidc),(double)targetId,E_x1*coef}));
		  // else
		  //     v_edge.push_back(std::vector<double>({(double)sourceId,(double)l2gid(gidc),-1*E_x1*coef}));
		  // if(E_bx2 > 0)
		  //     v_edge.push_back(std::vector<double>({(double)sourceId,(double)l2gid(gidn),E_bx2*coef}));
		  // else
		  //     v_edge.push_back(std::vector<double>({(double)l2gid(gidn),(double)targetId,-1*E_bx2*coef}));
		  // v_edge.push_back(std::vector<double>({(double)l2gid(gidc),(double)l2gid(gidn),E_quad*coef}));
		  if(E_x1 > 0)
                    g->add_tweights(id_map[fch], 0, MULT_2*E_x1*coef);
		  else
                    g->add_tweights(id_map[fch],-1*MULT_2*E_x1*coef, 0);
		  if(E_bx2 > 0)
                    g->add_tweights(id_map[fchn], MULT_2*E_bx2*coef, 0 );
		  else
                    g->add_tweights(id_map[fchn],0, -1*MULT_2*E_bx2*coef);
		  g->add_edge(id_map[fch], id_map[fchn],    /* capacities */ MULT_2*E_quad*coef,0);
		  break;
                }
	      case 2 :
                {
		  // Graph cut c++
		  // 1 : vertex
		  // 2 : edges
		  if(E_x1 > 0)
                    {
		      ofile << "1 " << gidc << " " << 0                   << " " << MULT_2*E_x1*coef  << " ";;
                    }
		  else
                    {
		      ofile << "1 " << gidc << " " << -1*MULT_2*E_x1*coef << " " << 0  << " ";;
                    }
		  if(E_bx2 > 0)
                    {
		      ofile << "1 " << gidn << " " << MULT_2*E_bx2*coef   << " " << 0 << " ";;
                    }
		  else
                    {
		      ofile << "1 " << gidn << " " << 0                   << " " << -1*MULT_2*E_bx2*coef  << " ";;
                    }
		  if(++acc % chunk_size == 0) ofile << std::endl;
		  ofile << "2 " << gidc << " " <<  gidn <<  " " <<  MULT*E_quad*coef << " " << 0 << " ";;;
		  if(++acc % chunk_size == 0) ofile << std::endl;
		  break;
                }
	      }
	  }
	catch (ddt::DDT_exeption& e)
	  {
	    std::cerr << "!! WARNING !!!" << std::endl;
	    std::cerr << "Exception catched : " << e.what() << std::endl;
	    continue;
	  }
      }
    //    myfile.close();

    double flow = g->maxflow();


    switch(gtype)
      {
      case 0 :
        {
	  for(auto vv : v_vertex){
	    ofile << "v " <<   (int)vv[0]  << " " << MULT_2*reg1(vv[1],v_max) << " " <<  MULT_2*reg1(vv[2],v_max) << std::endl;
	  }
	  for(auto ee : v_edge){
	    ofile << "e " << (int)ee[0] << " " << (int)ee[1]  << " ";
	    ofile << MULT_2*reg2(ee[2],v_max) << " " << MULT_2*reg2(ee[3],v_max) << " " << MULT_2*reg2(ee[4],v_max) << " " <<  MULT_2*reg2(ee[5],v_max) << " ";
	    ofile << std::endl;
	  }
	  break;
        }

      case 1 :
        {

	  if(area_processed < 2){
	    for (auto a= g->get_first_node(); a < g->get_last_node(); a++){
	      ofile << "v " <<  gid_map[(int)(a - g->get_first_node())] + 2  << " " << 0  <<  std::endl;
	    }
	  }
	  for (auto a= g->get_first_node(); a < g->get_last_node(); a++)
	    {

	      double cap = a->tr_cap;
	      if(abs(cap) > 0.000001){
		if(cap > 0)
		  std::cout  << "e " << 0 << " " << gid_map[(int)(a - g->get_first_node())] + 2 << " " << cap << std::endl;
		else
		  std::cout  << "e " << gid_map[(int)(a - g->get_first_node())] + 2 << " " << 1 << " " << -cap << std::endl;
	      }
	    }

	  // for (auto a= g->get_first_arc(); a < g->get_last_arc(); a++)
	  //   {
	  //     if(abs(a->r_cap) > 0.0001){
	  //     std::cout << "e " << gid_map[(int)(a->head - g->get_first_node())] + 2 << " " 
	  // 		<< gid_map[(int)(a->sister->head - g->get_first_node())] + 2 << " "
	  // 		<< a->r_cap << std::endl;
	  //     }
	  //   }


	  delete g;
	  // for(auto ee : v_edge){
	  //   ofile << "e " << (int)ee[0] << " " << (int)ee[1]  << " " << MULT_2*reg3(ee[2],v_max) ;
	  //   ofile << std::endl;
	  // }
        }
      case 2 :
        {
	  break;
        }
      }
    
    std::cerr << "acc = " << acc << std::endl;
    return acc;
  }




    int extract_stream_graph_v1(int lalpha,DTW & tri,D_MAP & data_map, std::map<int,std::vector<int>> & tile_ids,std::ostream & ofile, int main_tile_id, int gtype, double coef_mult)
    {
        ofile << std::fixed << std::setprecision(15);

        int chunk_size = 10;
        int sourceId = 0;
        int targetId = 1;
        double MULT_2 = coef_mult;
        int  N = tri.number_of_cells();
        int NF = 0;

        std::cerr << "COEF_MULT" << MULT_2 << " LAMBDA:" << lambda << std::endl;

        switch(gtype)
        {
        case 0 :
        {
            chunk_size = 1;
            break;
        }

        case 1 :
        {
            break;
        }
        case 2 :
        {
            break;
        }
        }


        std::cerr << "init graph" << std::endl;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

        double e0,e1;

        int acc = 0;
        //    std::map<int,int> id_map;
        std::map<Cell_const_iterator,int> id_map;
        std::vector<int> id2gid_vec;
        std::cerr << "create ids" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            id_map[cit];
        }


        std::cerr << "score simplex" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;
            if(cit->main_id() != main_tile_id)
                continue;
            // if(tri->is_infinite(fch))
            //    continue;
            int tid = cit->tile()->id();
            int lid = cit->lid();//cell_data().id;
            int gid = cit->gid();

            int lcurr = 0; //data_map[fch->tile()->id()].format_labs[cccid];
            e0 = get_score_linear(fch,lcurr,data_map);
            e1 = get_score_linear(fch,lalpha,data_map);

            switch(gtype)
            {
            case 0 :
            {
                // Belief spark
                ofile << "v " <<   gid  << " " << (e0 * MULT_2) << " " <<  (e1 * MULT_2) << " ";
                if(++acc % chunk_size == 0) ofile << std::endl;
                break;       // and exits the switch
            }

            case 1 :
            {
                // Graph cut spark (only edge with +2 id)
                ofile << sourceId << " " <<  l2gid(gid) << " " << (e0 * MULT_2) << " " ;
                if(++acc % chunk_size == 0) ofile << std::endl;
                ofile << l2gid(gid) << " " << targetId << " " <<  (e1 * MULT_2) << " " ;
                if(++acc % chunk_size == 0) ofile << std::endl;
                break;
            }
            case 2 :
            {
                // Graph cut c++

                // 1 : vertex
                ofile << "1 " <<   gid  << " " << (e0 * MULT_2) << " " <<  (e1 * MULT_2) << " ";
                if(++acc % chunk_size == 0) ofile << std::endl;
                break;
            }
            }

        }


        std::cerr << "score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            if(fit->is_infinite())
                continue;
            try
            {
                if(fit->main_id() != main_tile_id)
                    continue;


                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);


                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
                        !tri.tile_is_loaded(tmp_fchn->main_id()))
                {
                    std::cerr << "ERROR : CELL NOT LOADED" << std::endl;
                    return 1;
                    continue;
                }

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();





                int lidc = fch->lid();//>cell_data().id;
                int lidn = fchn->lid();//cell_data().id;

                int tidc = fch->tile()->id();
                int tidn = fchn->tile()->id();

                int gidc = fch->gid();//data_map[tidc].format_gids[lidc];
                int gidn = fchn->gid();//data_map[tidn].format_gids[lidn];
		// std::cerr << "lidc" << lidc << " lidn:" << lidn << std::endl;
		// std::cerr << "gidc" << gidc << " gidn:" << gidn << std::endl;
		double gsps = get_goodshape_prior(tmp_fch,tmp_idx);
                double surface = get_score_surface(tmp_fch,tmp_idx);
                double coef = lambda*surface+GSPS_CONST*gsps;


                int ch1lab = data_map[tidc].format_labs[lidc];
                int chnlab = data_map[tidn].format_labs[lidn];



                E[3] = get_score_quad(ch1lab,chnlab);
                E[2] = get_score_quad(ch1lab,lalpha);
                E[1] = get_score_quad(lalpha,chnlab);
                E[0] = get_score_quad(lalpha,lalpha);

                double E_x1 = E[0] - E[2];
                double E_bx2 = E[3] - E[2];
                // Quadratic term should be positif
                double E_quad = -E[0] + E[1] + E[2] - E[3];

                switch(gtype)
                {
                case 0 :
                {
                    // Belief spark
                    ofile << "e " << gidc << " " << gidn  << " ";
                    for(int i = 0 ; i < 4; i++)
                    {
                        ofile << MULT_2*E[i]*coef;
                        if(i <3)
                            ofile << " ";
                    }
                    if(++acc % chunk_size == 0) ofile << std::endl;
                    break;
                }
                case 1 :
                {
                    // Graph cut spark (only edge with +2 id)
                    if(E_x1 > 0)
                    {
                        ofile << l2gid(gidc) << " "  << targetId  << " " << MULT_2*E_x1*coef  << " ";;
                    }
                    else
                    {
                        ofile << sourceId << " " << l2gid(gidc) << " "  << -1*MULT_2*E_x1*coef  << " ";;
                    }
                    if(E_bx2 > 0)
                    {
                        ofile << sourceId << " " << l2gid(gidn) << " "  << MULT_2*E_bx2*coef   << " ";;
                    }
                    else
                    {
                        ofile <<  l2gid(gidn) << " "  << targetId << " " << -1*MULT_2*E_bx2*coef  << " ";;
                    }
                    if(++acc % chunk_size == 0) ofile << std::endl;
                    break;
                }
                case 2 :
                {
                    // Graph cut c++
                    // 1 : vertex
                    // 2 : edges
                    if(E_x1 > 0)
                    {
                        ofile << "1 " << gidc << " " << 0                   << " " << MULT_2*E_x1*coef  << " ";;
                    }
                    else
                    {
                        ofile << "1 " << gidc << " " << -1*MULT_2*E_x1*coef << " " << 0  << " ";;
                    }
                    if(E_bx2 > 0)
                    {
                        ofile << "1 " << gidn << " " << MULT_2*E_bx2*coef   << " " << 0 << " ";;
                    }
                    else
                    {
                        ofile << "1 " << gidn << " " << 0                   << " " << -1*MULT_2*E_bx2*coef  << " ";;
                    }
                    if(++acc % chunk_size == 0) ofile << std::endl;
                    ofile << "2 " << gidc << " " <<  gidn <<  " " <<  MULT*E_quad*coef << " " << 0 << " ";;;
                    if(++acc % chunk_size == 0) ofile << std::endl;
                    break;
                }
                }

            }
            catch (ddt::DDT_exeption& e)
            {
                std::cerr << "!! WARNING !!!" << std::endl;
                std::cerr << "Exception catched : " << e.what() << std::endl;
                continue;
            }

        }
        return acc;
    }



    int extract_factor_graph(int lalpha,DTW & tri,D_MAP & data_map, std::map<int,std::vector<int>> & tile_ids,std::ostream & ofile, int main_tile_id)
    {
        ofile << std::fixed << std::setprecision(10);
        int sourceId = 0;
        int targetId = 1;
        int MULT_2 = 1;
        int  N = tri.number_of_cells();
        int NF = 0;

        if(D == 3)
            MULT_2=1000000;

        std::cerr << "init graph" << std::endl;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

        double e0,e1;

        int acc = 0;
        //    std::map<int,int> id_map;
        std::map<Cell_const_iterator,int> id_map;
        std::vector<int> id2gid_vec;
        std::cerr << "create ids" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            id_map[cit];
        }



        std::cerr << "score simplex" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;

            if(cit->main_id() != main_tile_id)
                continue;
            // if(tri->is_infinite(fch))
            //    continue;
            int tid = cit->tile()->id();
            int lid = cit->lid();//cell_data().id;
            int gid = cit->gid(); //data_map[tid].format_gids[lid];

            int lcurr = 0; //data_map[fch->tile()->id()].format_labs[cccid];
            e0 = get_score_linear(fch,lcurr,data_map);
            e1 = get_score_linear(fch,lalpha,data_map);
            ofile << "v " <<   gid  << " " << (e0 * MULT_2) << " " <<  (e1 * MULT_2) << std::endl;
            //ofile << "v" << " " <<  l2gidf(cid,tile_ids) << " " << cid << " " <<  cid << std::endl;
        }


        std::cerr << "score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            if(fit->is_infinite())
                continue;
            try
            {
                if(fit->main_id() != main_tile_id)
                    continue;


                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);


                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
                        !tri.tile_is_loaded(tmp_fchn->main_id()))
                {
                    std::cerr << "ERROR : CELL NOT LOADED" << std::endl;
                    return 1;
                    continue;
                }

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();





                int lidc = fch->lid();//cell_data().id;
                int lidn = fchn->lid();//cell_data().id;

                int tidc = fch->tile()->id();
                int tidn = fchn->tile()->id();

                int gidc = fch->gid(); //data_map[tidc].format_gids[lidc];
                int gidn = fchn->gid();//data_map[tidn].format_gids[lidn];

		double gsps = get_goodshape_prior(tmp_fch,tmp_idx);		
                double surface = get_score_surface(tmp_fch,tmp_idx);
                double coef = lambda*surface+GSPS_CONST*gsps;


                int ch1lab = data_map[tidc].format_labs[lidc];
                int chnlab = data_map[tidn].format_labs[lidn];



                E[3] = get_score_quad(ch1lab,chnlab);
                E[2] = get_score_quad(ch1lab,lalpha);
                E[1] = get_score_quad(lalpha,chnlab);
                E[0] = get_score_quad(lalpha,lalpha);

                double E_x1 = E[0] - E[2];
                double E_bx2 = E[3] - E[2];
                // Quadratic term should be positif
                double E_quad = -E[0] + E[1] + E[2] - E[3];
                ofile << "e " << gidc << " " << gidn  << " ";
                for(int i = 0 ; i < 4; i++)
                {
                    ofile << MULT_2*E[i]*coef;
                    if(i <3)
                        ofile << " ";
                }
                ofile << std::endl;

            }
            catch (...)
            {
                continue;
            }
        }
        return acc;
    }



    int D,NBL;
    double lambda;
    int ly_idx,ld_idx;
    DTW * tri;
    D_MAP * data_map;



    std::vector<int>  labs;
    std::vector<double>  pLabs;



};



#endif
