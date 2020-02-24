#!/bin/bash

mesh_fullfile=$1 #fichier avec chemin relatif
filename=$(basename -- "$mesh_fullfile") # avec extension
path=$(dirname "${mesh_fullfile}")
surface_mesh="${path}/${filename%.*}" # avec chemin mais sans extensionpath=$(dirname "${fullfile}")

echo "Generation de la surface implicite (verite terrain) a partir du mesh :"
echo "----------------------------------------------------------------------"
implicit_surface="${surface_mesh}_implicit.mpu"
min_samples=3 # defaut : 6
fit_epsilon=0.01 # defaut : 0.0075
covering=1.0 # defaut : 1.0
./bin/mesh_to_implicit ${surface_mesh}.ply ${implicit_surface} $min_samples $fit_epsilon $covering
echo "./bin/mesh_to_implicit ${surface_mesh}.ply ${implicit_surface} $min_samples $fit_epsilon $covering"

echo "Generation d_une version .obj de la surface implicite juste pour la visualisation :"
echo "-----------------------------------------------------------------------------------"
resolution=256
output_surface="${surface_mesh}_ground_truth.obj"
./bin/isosurface ${implicit_surface} $resolution $output_surface

echo "Scan synthetique :"
echo "-----------------"
config_base="${surface_mesh}_config"
resolution=200 # par defaut : 300
nb_de_scans=5 # par defaut : 10
./bin/pc_generator ${implicit_surface} ${config_base} res $resolution scans $nb_de_scans
python scripts/RunSampler.py ${config_base}_0.cnf

# Pour ne garder que les coordonnees xyz des points :
cat ${config_base}_0.npts | cut -d " " -f 1-3 > ${config_base}_point_cloud.xyz
cat ${config_base}_0.npts | cut -d " " -f 4-6 > ${config_base}_normals.xyz


echo "EVALUATION"
echo "----------"
num_samples=10000
./bin/implicit_uniform ${implicit_surface} ${num_samples}
# Le probleme de cette methode est qu_elle renvoie le dense_uniform_sampling sous forme d_un fichier :
# particles.npts qui se trouve dans la racine par defaut
