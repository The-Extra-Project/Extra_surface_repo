#ifndef CREATE_DATAS_PARAMS
#define CREATE_DATAS_PARAMS

#include <string>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <getopt.h>
#include <list>
#include <random>
#include <iostream>

class create_datas_params
{
public :
  create_datas_params() : nb_sample(300),x_origin(0),y_origin(0),do_rand(false),do_flip(false),do_crop(false)
  {
  };


  
  int nb_sample;
  int x_origin;
  int y_origin;
  bool do_rand;
  bool do_flip;
  bool do_crop;
  std::string name_img;
  std::string name_ply_out;

  
    // ---
    // --- help function ----
    void help_param()
    {
        std::cerr << "--------------------------------------------------" << std::endl
                  << "INPUTS" << std::endl
                  << "--algo_step <string> : input_data_dir " << std::endl
                  << "--dim <int> : output_data_dir " << std::endl
                  << "[ --help ]  : print this message" << std::endl
                  << "--------------------------------------------------" << std::endl ;
    }


    int parse(int argc, char **argv)
    {
        int cc;

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

  return 0;
    }
};





#endif  // CREATE_DATAS_PARAMS

