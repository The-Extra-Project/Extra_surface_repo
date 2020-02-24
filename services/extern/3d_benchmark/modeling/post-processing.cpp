#include <OpenMesh/Core/IO/MeshIO.hh>
#include "shape_loader.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

#define MAX_DIM_MM 75

int main(int argc, char** argv)  {
	// if(argc != 6)  {
	// 	cerr << "usage: " << argv[0] << " mesh_in implicit_out min_samples fit_eps covering" << endl;
	// 	return 0;
	// }
	std::cout << "hello post processing" << std::endl;

	int arg_num=1;
	string mesh_global = argv[1];
	string mesh_local = argv[2];
	string output_filename = argv[3];

	BasicTriMesh surface_mesh_global;
	OpenMesh::IO::Options opt;
	OpenMesh::IO::Options opt_bin;
	opt_bin += OpenMesh::IO::Options::Binary; 
	OpenMesh::IO::read_mesh(surface_mesh_global, mesh_global, opt_bin);

	BasicTriMesh surface_mesh_local;
	OpenMesh::IO::read_mesh(surface_mesh_local, mesh_local, opt_bin);
	if (!OpenMesh::IO::read_mesh(surface_mesh_local, mesh_local, opt_bin)) 
	{
  		std::cerr << "read error\n";
  		exit(1);
	}

	// resize mesh to largest dimension being 75mm
	BasicTriMesh::Point ll, ur;
	for(int i = 0; i < 3; i++)  {
		ll[i] = 1e10;
		ur[i] = -1e10;
	}

	for(int p = 0; p < surface_mesh_global.n_vertices(); p++)  {
		BasicTriMesh::Point pt = surface_mesh_global.point(BasicTriMesh::VertexHandle(p));
		for(int i = 0; i < 3; i++)  {
			ll[i] = pt[i] < ll[i] ? pt[i] : ll[i];
			ur[i] = pt[i] > ur[i] ? pt[i] : ur[i];
		}
	}

	BasicTriMesh::Point bounds = ur-ll;
	double max_bound = -1e10;
	int max_dim = -1;
	for(int i = 0; i < 3; i++)  {
		if(bounds[i] > max_bound)  {
			max_dim = i;
			max_bound = bounds[i];
		}
	}

	BasicTriMesh::Point rescaled_bounds;
	for(int i = 0; i < 3; i++)
		rescaled_bounds[i] = (MAX_DIM_MM/max_bound)*bounds[i];

	BasicTriMesh::Point center = (ur+ll)*0.5;
	for(int p = 0; p < surface_mesh_global.n_vertices(); p++)  {
		BasicTriMesh::Point pt = surface_mesh_global.point(BasicTriMesh::VertexHandle(p));
		BasicTriMesh::Point normalized_pt = pt-center;
		for(int i = 0; i < 3; i++)
			normalized_pt[i] = normalized_pt[i] / (0.5*bounds[i]);
		BasicTriMesh::Point new_pt;
		for(int i = 0; i < 3; i++)
			new_pt[i] = normalized_pt[i] * (0.5*rescaled_bounds[i]);
		surface_mesh_global.set_point(BasicTriMesh::VertexHandle(p), new_pt);
	}

	for(int p = 0; p < surface_mesh_local.n_vertices(); p++)  {
		BasicTriMesh::Point pt = surface_mesh_local.point(BasicTriMesh::VertexHandle(p));
		BasicTriMesh::Point new_pt2;
		for(int i = 0; i < 3; i++){
			new_pt2[i] = pt[i] / (0.5*rescaled_bounds[i]);
			new_pt2[i] = new_pt2[i] * (0.5*bounds[i]);
		}
		new_pt2 = new_pt2 + center;
		surface_mesh_local.set_point(BasicTriMesh::VertexHandle(p), new_pt2);
	}

	OpenMesh::IO::write_mesh(surface_mesh_local, output_filename);

	if (!OpenMesh::IO::write_mesh(surface_mesh_local, output_filename)) 
	{
  		std::cerr << "write error\n";
  		exit(1);
	}



	std::cout << "rescaling completed" << std::endl;

}
