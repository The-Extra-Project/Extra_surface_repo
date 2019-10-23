#ifndef WASURE_PARAMS
#define WASURE_PARAMS

#include <string>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <getopt.h>
#include <list>
#include <random>
#include <iostream>

class wasure_params
{
public :
    wasure_params() : verbose_flag(0),nbp(0),log_level(2),show_ghost(false),do_stream(true),area_processed(0),coef_mult(1),
        input_dir(std::string("")),output_dir(std::string("")),  algo_step(std::string("")),slabel(std::string("")),mode("surface"),rat_ray_sample(0),pscale(1),nb_samples(1),lambda(1),nb_labs(2),graph_type(0),skip_app_header(false)
    {



    };
    int verbose_flag,seed,nbp,log_level,id_padding,graph_type,area_processed;
    bool show_ghost,skip_app_header,do_stream,process_only_shared;
    double lambda,terr,rat_ray_sample,rat_extra_pts,min_scale,pscale,coef_mult;
    int nb_samples,max_it,nb_labs,nb_threads;
    int center_type,tile_id;



    std::string bbox_string,input_dir,output_dir,algo_step,slabel,mode;
    std::ostream& operator<<(std::ostream& os)
    {
        std::default_random_engine er((unsigned int)time(0));
        seed = er();
        os << "input_dir : " << input_dir << std::endl;
        os << "algo_step      : " << algo_step << std::endl;
        return os;
    }

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
        int errflg = 2;
        static struct option long_options[] =
        {
            // /* These options set a flag. */
            {"step",  required_argument, 0, 's'},
            {"nbp",  required_argument, 0, 'n'},
            {"graph_type",  required_argument, 0, 'i'},
            {"seed",  required_argument, 0, 'a'},
            {"input_dir",  required_argument, 0, 'r'},
            {"mode",  required_argument, 0, 'm'},
            {"output_dir",  required_argument, 0, 'w'},
            {"label",  required_argument, 0, 'l'},
            {"lambda",  required_argument, 0, 'c'},
            {"coef_mult",  required_argument, 0, 'k'},
            {"rat_ray_sample",  required_argument, 0, 'u'},
            {"bbox",  required_argument, 0, 'b'},
            {"dim",  required_argument, 0, 'd'},
            {"nbt_side",  required_argument, 0, 't'},
            {"nb_samples",  required_argument, 0, 'j'},
            {"area_processed",  required_argument, 0, 'p'},
            {"pscale", required_argument,0, 'f'},
            {"show_ghost", no_argument,0, 'g'},
            {"skip_app_header", no_argument,0, 'e'},
            {"do_stream", no_argument,0, 'x'},
            {"verbose", no_argument, &verbose_flag, 1},
            {"help",  no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };


        int option_index = 0;

        while ((cc = getopt_long(argc, argv, "s:a:c:k:n:i:d:u:m:t:f:j:l:b:p:r:w:gexh",long_options,&option_index)) != -1)
        {
            switch (cc)
            {
            case 's':
                algo_step = std::string(optarg);
                errflg--;
                break;
            case 'r':
                input_dir = std::string(optarg);
                break;
            case 'w':
                output_dir = std::string(optarg);
                break;
            case 'm':
                mode = std::string(optarg);
                break;
            case 'l':
                slabel = std::string(optarg);
                break;
            case 'b':
                bbox_string = std::string(optarg);
                std::replace( bbox_string.begin(), bbox_string.end(), ':', ' '); // replace all 'x' to 'y'
                std::replace( bbox_string.begin(), bbox_string.end(), 'x', ' ');
                break;
            case 'n':
                nbp = atoi(optarg);
                break;
            case 'c':
                lambda = atof(optarg);
                break;
            case 'k':
                coef_mult = atof(optarg);
                break;
            case 'u':
                rat_ray_sample = atof(optarg);
                break;
            case 'd':
                std::cerr << "void" << std::endl;
                break;
            case 'p':
                area_processed = atoi(optarg);
                break;
            case 'f':
                pscale = atof(optarg);
                break;
            case 'a':
                seed = atoi(optarg);
                break;
            case 'i':
                graph_type = atoi(optarg);
                break;
            case 't':
                std::cerr << "void" << std::endl;
                break;
            case 'j':
                nb_samples = atoi(optarg);
                break;
            case 'g':
                show_ghost=true;
                break;
            case 'e':
                skip_app_header=true;
                break;
            case 'x':
                do_stream=true;
                break;
            case 'h':
                break;
                char * optc = (argv[optind]);
                std::cout << "value:" << (char)cc << "  :  " << optc  << std::endl;
                help_param();
                return 0;
            }
        }

        if(errflg != 0)
        {
            help_param();
            return 1;
        }
        return 0;
    }

};





#endif  // WASURE_PARAMS

