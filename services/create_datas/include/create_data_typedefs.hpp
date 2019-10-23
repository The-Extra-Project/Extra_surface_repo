#ifndef WASURE_TYPEDEFS_H
#define WASURE_TYPEDEFS_H






#include <CGAL/Random.h>

#include <stdio.h>      /* printf */
#include <math.h>


#include "tile.hpp"
#include "traits/traits.hpp"
#include "traits/data_cell_base.hpp"
#include "scheduler/scheduler.hpp"
#include "partitioner/grid_partitioner.hpp"
#include "partitioner/const_partitioner.hpp"

#include "DDT.hpp"



typedef int Id;
typedef int FlagV;
typedef int FlagC;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef ddt::Data<Id,FlagV>                                  Data_V;
typedef ddt::Data<Id,FlagC>                                     Data_C;
typedef ddt::Traits<Data_V,Data_C> Traits;

typedef ddt::Tile<Traits> Tile;
typedef ddt::Scheduler<Tile> Scheduler;
typedef ddt::DDT<Traits> DTW;


typedef ddt::grid_partitioner<Traits> Grid_partitioner;


typedef Traits::Random_points_in_box Random_points;
typedef typename DTW::Tile_const_iterator  Tile_const_iterator ;
typedef typename DTW::Tile_cell_const_handle Tile_cell_const_handle;
typedef typename DTW::Tile_iterator  Tile_iterator ;

typedef typename Traits::Delaunay_triangulation DT;
typedef typename Traits::Point       Point;
typedef typename Traits::Id          Id;
typedef typename Traits::Point_id    Point_id;
typedef typename Traits::Point_id_id Point_id_id;


typedef typename Traits::Point                                    Point;
typedef typename Traits::Point                                    Facet;
typedef typename Traits::Cell_handle                                    Cell_handle;
typedef typename Traits::Vertex_handle                                    Vertex_handle;


typedef typename DTW::Facet_const_iterator  Facet_const_iterator;
typedef typename DTW::Vertex_const_iterator  Vertex_const_iterator;
typedef typename DTW::Tile_cell_const_handle              Tile_cell_const_handle;
typedef typename DTW::DT::Full_cell::Vertex_handle_iterator Vertex_h_iterator;
typedef typename DTW::Cell_const_iterator                 Cell_const_iterator;
typedef typename DTW::Facet_const_iterator                Facet_const_iterator;




typedef typename Tile::Vertex_const_handle_and_id Vertex_const_handle_and_id;
typedef typename Tile::Vertex_const_handle Vertex_const_handle;




typedef typename Tile::Point_id Point_id;
typedef std::tuple<Point,Id,Id>                  Point_id_source;



#endif
