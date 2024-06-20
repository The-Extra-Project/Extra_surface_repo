import argparse
import pyproj
import trimesh
import os
def transform_coordinates(vertices, transformer,coords):
    transformed_vertices = []
    x_shift = float(coords.split("x")[0])
    y_shift = float(coords.split("x")[1])
    for x, y, z in vertices:
        x_new, y_new, z_new = transformer.transform(x+x_shift, y+y_shift, z)
        transformed_vertices.append([x_new, y_new, z_new])
    return transformed_vertices

def main(input_file, output_file,coords):
    # Load the .obj file
    mesh = trimesh.load_mesh(input_file,force='mesh')

    # Extract vertices
    vertices = mesh.vertices
    normals = mesh.vertex_normals
    # Define the transformer from EPSG:2154 to EPSG:4978
    transformer = pyproj.Transformer.from_crs("epsg:2154", "epsg:4978")

    # Transform the vertices
    transformed_vertices = transform_coordinates(vertices, transformer,coords)

    # Update the mesh with transformed vertices
    mesh.vertices = transformed_vertices
    #import pdb; pdb.set_trace()
    # Export the transformed .obj file
#    mesh.export(output_file)

        # Write the transformed mesh to the .obj file manually to include normals
    mtl_name="./" + str(os.path.basename(input_file)) + '.mtl'
    with open(output_file, 'w') as f:
        
        f.write(f"mtllib {mtl_name}\n")
        for i, v in enumerate(mesh.vertices):
            if len(normals) > 0:
                n = normals[i]
                f.write(f"vn {n[0]} {n[1]} {n[2]}\n")            
            f.write(f"v {v[0]} {v[1]} {v[2]}\n")
    
        
        for face in mesh.faces:
            f.write(f"f {' '.join([f'{idx+1}//{idx+1}' for idx in face])}\n")

    
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert a .obj file from EPSG:2154 to EPSG:4978")
    parser.add_argument('--input', type=str, required=True, help='Path to the input .obj file')
    parser.add_argument('--output', type=str, required=True, help='Path to the output .obj file')
    parser.add_argument('--coords', default='',help='translation coordinate')
    args = parser.parse_args()
    
    main(args.input, args.output,args.coords)
