import pymeshlab
import argparse
import subprocess
import os


def bounding_boxes_intersect(bbox1, bary):
    """ Check if two bounding boxes intersect """
    for dim in range(3):  # Check each dimension (x, y, z)
        if bbox1.min()[dim] > bary[dim] or bbox1.max()[dim] < bary[dim]:
            return False
    return True

def transfer_colors(input_pts, input_mesh_dir, output_colorized_dir):
    # Charger le nuage de points avec couleurs
    ms = pymeshlab.MeshSet()
    ms.load_new_mesh(input_pts)
    bb1 = ms.mesh(0).bounding_box()
    acc = 1
    for filename in os.listdir(input_mesh_dir):
        if filename.endswith("ply"):
            # Split the filename into name and extension
            name, extension = os.path.splitext(filename)
            file_path = os.path.join(input_mesh_dir, filename)
            # Create the new filename by inserting '_col' before the extension
            new_filename = f'{output_colorized_dir}/{filename}'
            ms.load_new_mesh(file_path)
            bary = ms .apply_filter('compute_geometric_measures')['barycenter']
            intersect = bounding_boxes_intersect(bb1, bary)
            if intersect : 
                ms.vertex_attribute_transfer(sourcemesh=0,targetmesh=acc,colortransfer=True,upperbound=pymeshlab.Percentage(75))
                ms.set_current_mesh(acc)
                ms.save_current_mesh(new_filename)
                command = f"sed -n '3p' {file_path} | sed -i '3r /dev/stdin'  {new_filename}"
                subprocess.run(command, shell=True, check=True)
            acc = acc+1

def main():
    parser = argparse.ArgumentParser(description="Transférer les couleurs d'un nuage de points vers un maillage.")
    parser.add_argument('--input_pts', required=True, help="Chemin vers le fichier du nuage de points avec couleurs (PLY).")
    parser.add_argument('--input_mesh_dir', required=True, help="Chemin vers le fichier du maillage sans couleur (PLY).")
    parser.add_argument('--output_colorized_mesh', required=True, help="Chemin vers le fichier de sortie du maillage coloré (PLY).")
    
    args = parser.parse_args()

    transfer_colors(args.input_pts, args.input_mesh_dir, args.output_colorized_mesh)

if __name__ == "__main__":
    main()
