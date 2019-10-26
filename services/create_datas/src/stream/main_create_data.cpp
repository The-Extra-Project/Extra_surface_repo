
#include "local.hpp"

#include "io/stream_api.hpp"
#include "io/write_stream.hpp"
#include "io/write_vrt.hpp"
#include "io/read_stream.hpp"
#include <getopt.h>
#include <list>

Traits traits;

Point flip_y(Point p, unsigned int height, Point pt ) {
  std::vector<double> coords = {p[0] + (((double) rand() / (RAND_MAX))*0.5 -0.25) + pt[0],
				height-p[1] + (((double) rand() / (RAND_MAX))*0.5 -0.25) + pt[1] };
  return traits.make_point(coords.begin());
}




bool is_inside(cimg_library::CImg<unsigned char> & img,int X,int Y){
  return (X > 0 && Y > 0 && X < img.width() && Y < img.height());
}

bool is_black(cimg_library::CImg<unsigned char> & img,int X,int Y){
  int lim=3;
  if(is_inside(img,X,Y))
    return ((img((int)X,(int)Y,0) < lim && img((int)X,(int)Y,1) < lim && img((int)X,(int)Y,2) < lim));
  else
    return false;
}



bool is_white(cimg_library::CImg<unsigned char> & img,int X,int Y){
  int lim=100;
  if(is_inside(img,X,Y))
    return ((img((int)X,(int)Y,0) > 255-lim && img((int)X,(int)Y,1) > 255-lim && img((int)X,(int)Y,2) > 255-lim));
  else
    return false;
}


bool is_red(cimg_library::CImg<unsigned char> & img,int X,int Y){
  if(is_inside(img,X,Y))
    return ((img((int)X,(int)Y,0) == 255 && img((int)X,(int)Y,1) == 0 && img((int)X,(int)Y,2) == 0));
  else
    return false;
}

bool is_green(cimg_library::CImg<unsigned char> & img,int X,int Y){
  if(is_inside(img,X,Y))
    return ((img((int)X,(int)Y,0) == 0 && img((int)X,(int)Y,1) == 255 && img((int)X,(int)Y,2) == 0));
  else
    return false;
}

bool is_blue(cimg_library::CImg<unsigned char> & img,int X,int Y){
  if(is_inside(img,X,Y))
    return ((img((int)X,(int)Y,0) == 0 && img((int)X,(int)Y,1) == 0 && img((int)X,(int)Y,2) == 255));
  else
    return false;
}



bool is_empty(cimg_library::CImg<unsigned char> & img,int X,int Y){
  return (is_black(img,X,Y));
}

bool is_neighbor_empty(cimg_library::CImg<unsigned char> & img,int X,int Y,int dist){
  for(int i = -dist; i <= dist ; i++){
    for(int j = -dist;j <= dist ; j++){
      if(is_empty(img,X+i,Y+j))
	return true;
    }
  }
  return false;
}



bool ray_tracing(Point a, Point b,cimg_library::CImg<unsigned char> & img,cimg_library::CImg<unsigned char> & img_out, Point & pr, int & clab){
  //scene.line_dbg.push_back(std::make_pair(a,b));
  int max_x = img.width();
  int max_y = img.height(); 

  std::vector<double> coords = {b[0]-a[0],b[1]-a[1]};
  Point v = traits.make_point(coords.begin());
  double norm = sqrt(v[0]*v[0] + v[1]*v[1]);
  double tMaxX,tMaxY,stepX,stepY,X,Y;
  X = ((int)a[0]);
  Y = ((int)a[1]);


  stepX = (b[0]-a[0])/fabs((b[0]-a[0]));
  stepY = (b[1]-a[1])/fabs((b[1]-a[1]));

  double tDeltaX = norm/fabs(b[0] - a[0]);
  double tDeltaY = norm/fabs(b[1] - a[1]);
  tMaxX = ((stepX > 0) ? (1-(a[0]-floor(a[0]))) : (a[0]-floor(a[0])))*tDeltaX;
  tMaxY = ((stepY > 0) ? (1-(a[1]-floor(a[1]))) : (a[1]-floor(a[1])))*tDeltaY;

  int nbr_size = 1;
  
  do {
    if(tMaxX < tMaxY) {
      X= X + stepX;
      if(X > max_x-1 - nbr_size || X < 0 + nbr_size)
	return false; /* outside grid */
      tMaxX= tMaxX + tDeltaX;
    } else {
      Y= Y + stepY;
      if(Y > max_y-1 - nbr_size || Y < 0 + nbr_size)
	return false;
      tMaxY= tMaxY + tDeltaY;
    }

    if(!is_empty(img,X,Y) && is_inside(img,X,Y)){
      if(is_neighbor_empty(img,X,Y,1) && !is_red(img_out,X,Y) ){
	for(int c = 0; c < 3; c++)
	  img_out((int)X,(int)Y,c) = img((int)X,(int)Y,c);
	std::vector<double> coords = {X,Y};      
	pr = traits.make_point(coords.begin());

	if(is_red(img_out,X,Y))
	  clab = 0;
	else if(is_green(img_out,X,Y))
	  clab = 1;
	else if(is_blue(img_out,X,Y))
	  clab = 2;
	return true;
      }
      return false;
    }else
      if(img_out((int)X,(int)Y,0) < 200){
	for(int c = 0 ; c < 3; c++)
	  img_out((int)X,(int)Y,c) = img_out((int)X,(int)Y,c) + 10;
      }

    //scene.pts_dbg.push_back(Point_3(a[0] + tMaxX*stepX,a[1] +tMaxY*stepY,a.z() +tMaxZ*stepZ));                                                                                                                                            
  } while(true);
  return false;
}
  


int main(int argc, char **argv)
{


  /* initialize random seed: */
  srand (time(NULL));


  int nb_sample = 300;
  int x_origin = 0;
  int y_origin = 0;
  bool do_rand = false;
  bool do_flip = false;
  bool do_crop = false;
  std::string name_img;
  std::string name_ply_out;


  
  const char* const short_opts = "i:o:n:x:y:crvh";
  const option long_opts[] = {
    {"name_img_in", 1, nullptr, 'i'},
    {"name_ply_out", 1, nullptr, 'o'},
    {"x_origin", 1, nullptr, 'x'},
    {"y_origin", 1, nullptr, 'y'},
    {"nb_sample", 1, nullptr, 'n'},
    {"do_crop", 0, nullptr, 'c'},
    {"do_rand", 0, nullptr, 'r'},
    {"do_invert", 0, nullptr, 'v'},
    {"help", 0, nullptr, 'h'},
    {nullptr, 0, nullptr, 0}
  };
  int count=2;
  while (true)
    {
      const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

      if (-1 == opt)
	break;

      switch (opt)
	{
	case 'i':
	  name_img=std::string(optarg);
	  count--;
	  break;
	case 'o':
	  name_ply_out=std::string(optarg);
	  count--;
	  break;
	case 'n':
	  nb_sample=atoi(optarg);
	  break;
	case 'x':
	  x_origin=atoi(optarg);
	  break;
	case 'y':
	  y_origin=atoi(optarg);
	  break;
	case 'c':
	  do_crop=true;
	  break;
	case 'r':
	  do_rand=true;
	  break;
	case 'v':
	  do_flip=true;
	  break;


	case 'h': // -h or --help
	case '?': // Unrecognized option
	default:
	  //PrintHelp();
	  break;
	}
    }

  
  if(count != 0){
    std::cout << "error" << std::endl;
    return 1;
  }


  std::vector<double> coords_ori = {x_origin,y_origin};
  Point p_ori = traits.make_point(coords_ori.begin());

  std::string name_img_out(remove_extension(name_ply_out) + ".tif");
  cimg_library::CImg<unsigned char> img_load(name_img.c_str());
  cimg_library::CImg<unsigned char> img(img_load.width(), img_load.height(), 1, 3, 0);

  std::cout << "name_ply_out : " << name_ply_out << std::endl;
  std::cout << "name_img_out : " << name_img_out << std::endl;
  std::cout << "do rand: " << do_rand << std::endl;
  std::cout << "spectrum:" << img_load.spectrum() << std::endl;

  cimg_forXYC(img, x, y, c){
    if(img_load.spectrum() == 3)
      img(x,y,c) = img_load(x,y,c);
    else
      img(x,y,c) = img_load(x,y,0);
  }

  if(do_flip){
    cimg_forXYC(img, x, y, c){
      img(x,y,c) = 255-img(x,y,c);
    }
  }

  if(do_crop)
    img.crop(0,0,1024,1024);
  
  std::cout << "spectrum:" << img.spectrum() << " " << img.height() << " " << std::endl;

  
  std::vector<DT::Point> points,ori;
  std::vector<DT::Point> center_rot,center_line;
  std::vector<int> labs;
  int nbp = img.height()*img.width();
  int acc = 0;
  for (int y=0; y < img.height() ; ++y){
    for (int x=0; x < img.width() ; ++x){

      std::vector<double> coords_center = {x,y};
      if(do_rand){
	if(is_empty(img,x,y)){
	  center_rot.push_back(traits.make_point(coords_center.begin()));
	}
      }else{
	if(img(x,y,0) == 255 &&
	   img(x,y,1) == 0 &&
	   img(x,y,2) == 0){
	  center_rot.push_back(traits.make_point(coords_center.begin()));
	}
	if(img(x,y,0) == 0 &&
	   img(x,y,1) == 0 &&
	   img(x,y,2) == 255){
	  center_line.push_back(traits.make_point(coords_center.begin()));
	}
      }
      acc++;
    }
  }


  // obtain a time-based seed:
  if(do_rand){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    shuffle (center_rot.begin(), center_rot.end(), std::default_random_engine(seed));
    center_rot.erase(center_rot.begin()+nb_sample,center_rot.end());
  }


  cimg_library::CImg<unsigned char> img_out(img_load.width(), img_load.height(), 1, 3, 0);
  for(auto pit = center_rot.begin(); pit != center_rot.end();pit++){
    std::vector<double> coords_center = {(*pit)[0],(*pit)[1]};
    std::vector<double> coords_2 = {0,0};
    Point pr;
    int clab = -1;
    bool is_touched = false;
    double nb_s = 200;
    for(double i = -3.14 ; i <3.14 ; i += 3.14*2/nb_s){
      coords_2[0] = coords_center[0] + cos(i)*img.width();
      coords_2[1] = coords_center[1] + sin(i)*img.width();	  
      if(ray_tracing(traits.make_point(coords_center.begin()),traits.make_point(coords_2.begin()),img,img_out,pr,clab )){
	points.push_back(flip_y(pr,img.height(),p_ori));
	ori.push_back(flip_y(traits.make_point(coords_center.begin()),img.height(),p_ori));
	labs.push_back(clab);
      }
      clab = -1;
    }
  }

  for(auto pit = center_line.begin(); pit != center_line.end();pit++){
    std::vector<double> coords_center = {(*pit)[0],(*pit)[1]};
    std::vector<double> coords_2 = {0,0};
    Point pr;
    int clab = -1;
    bool is_touched = false;
    double nb_s = 200;
    coords_2[0] = coords_center[0];
    coords_2[1] = coords_center[1] + img.height();
    if(ray_tracing(traits.make_point(coords_center.begin()),traits.make_point(coords_2.begin()),img,img_out,pr,clab)){
      points.push_back(flip_y(pr,img.height(),p_ori));
      ori.push_back(flip_y(traits.make_point(coords_center.begin()),img.height(),p_ori));
      labs.push_back(clab);
    }
    clab = -1;
  }


  std::cout << "size:" << points.size() << " " << ori.size() << std::endl;
  dump_points_ply_ori_lab(points,ori,labs,name_ply_out,2);
  img_out.save(name_img_out.c_str());

  return 0;
}





