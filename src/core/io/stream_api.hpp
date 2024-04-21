#ifndef IQ_HEADER_H
#define IQ_HEADER_H

#include <iomanip>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <utility>
#include <sstream>
#include <string>
#include <stdio.h>      // printf, scanf, puts, NULL
#include <stdlib.h>     // srand, rand
#include <time.h>

#include <chrono>
#include <thread>
#include <sys/stat.h>

#include <random>
#include <algorithm>
#include <ctime>
#include <chrono>

#include <limits>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "io/logging_stream.hpp"

#include "conf_header/conf.hpp"

// #ifdef DDT_USE_HDFS
// #include "hdfs.h"
// #endif

#define MAXBUFLEN (10000000)
#define SPARK_BUF_SIZE (65536)
#define IS_BINARY false

typedef std::numeric_limits< double > dbl;

#define OUTPUT_DIR ("/mnt/shared_spark/out")
namespace ddt
{

class stream_data_header
{
public :

    stream_data_header() : fis(NULL),fos(NULL),ifile(NULL),ofile(NULL),bool_dump(true),log(NULL)
    {

    }

    stream_data_header(std::string ll,std::string tt,std::vector<int> v) : stream_data_header()
    {
        lab = ll;
        type = tt;
        lidx = v;
    };

    stream_data_header(std::string ll,std::string tt,int id)  : stream_data_header()
    {
        lab = ll;
        type = tt;
        lidx = std::vector<int>(1,id);
    };


    std::string random_string( size_t length );

    void finalize();

    std::string  get_lab()
    {
        return lab;
    }

    void  set_lab(std::string ll )
    {
        lab = ll;
    }


    std::string get_ext()
    {
        return filename.substr(filename.find_last_of(".") + 1);
    }


    int get_id(int i)
    {
        return lidx[i];
    }

    int get_id()
    {
        return lidx[0];
    }

    bool is_stream();
    bool is_serialized();
    void serialize(bool);

    bool is_file() ;

    bool is_hdfs() ;

    bool fexists(const std::string& filename)
    {
        std::ifstream iff(filename.c_str());
        return (bool)iff;
    }


    char get_nl_char()
    {
        if(is_file())
            return '\n';
        return ';';
    }

    std::string get_file_name()
    {
        //    assert(is_file());
        return filename;
    }

    void write_into_file(std::string root_dir,std::string ext, bool rand_ext = true);
    void set_file_name(std::string fname);


    std::istream & parse_header(std::istream & ifs, bool is_binary = true);

    std::ostream & write_header(std::ostream & ost, bool is_binary = true);

    void set_logger(  ddt::logging_stream * ll)
    {
        log = ll;
    }

    std::istream & get_input_stream()
    {
        if(is_file()) // File
            return *fis;
        else  if(is_hdfs()) // Hdfs
            return *ifile;
        else if(is_stream())
            return *ifile;
        else return std::cin;
    }

    std::ostream & get_output_stream()
    {
        if(is_file())
        {
            fos->precision(dbl::max_digits10);
            return *fos;
        }
        else if(is_hdfs())
        {
            ofile->precision(dbl::max_digits10);
            return *ofile;
        }
        else
        {
            std::cout.precision(dbl::max_digits10);
            return std::cout;
        }
    }


private:
    std::string lab,type,filename;
    std::vector<int> lidx;
    std::ifstream * fis;
    std::ofstream * fos;
    std::stringstream * ifile;
    std::ostringstream * ofile;
    bool bool_dump;
    ddt::logging_stream * log;
};



class stream_app_header
{
public :

    stream_app_header() : tile_id(-1),nbd(-1)
    {

    }

    bool is_void()
    {
        return tile_id == -1;
    }
    int get_nb_dat()
    {
        return nbd;
    }
    std::istream & parse_header(std::istream & ist)
    {
        std::string fip;
        ist >> fip;
        if(fip.empty())
        {
            return ist;
        }
        std::cerr << "read_stream_api" << std::endl;
        std::cerr << fip << std::endl;
        tile_id = stoi(fip);
        std::cerr << "done" << std::endl;
        ist >> nbd;
        return ist;
    }
    int tile_id,nbd;
};

void regularize_sspath(std::string & ss)
{
    boost::replace_all(ss,"//","/");
}

int intRand(const int & min, const int & max)
{
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}


std::string time_in_HH_MM_SS_MMM()
{
    using namespace std::chrono;

    // get current time
    auto now = system_clock::now();

    // get number of milliseconds for the current second
    // (remainder after division into seconds)
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // convert to std::time_t in order to convert to std::tm (broken time)
    auto timer = system_clock::to_time_t(now);

    // convert to broken time
    std::tm bt = *std::localtime(&timer);

    std::ostringstream oss;

    oss << std::put_time(&bt, "%d-%m-%Y-%H-%M-%S"); // HH:MM:SS
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}






std::string stream_data_header::random_string( size_t length )
{
    std::chrono::time_point<std::chrono::system_clock> start;
    start = std::chrono::system_clock::now();
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d_%m_%Y_%H_%M_%S");

    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[intRand(0,max_index )];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;//oss.str();
}

void stream_data_header::write_into_file(std::string root_name,std::string ext, bool rand_ext)
{
    std::string curname;

    if(rand_ext)
        curname = root_name  + "_" + random_string(10)  + ext;
    else
        curname = root_name   + ext;
    set_file_name(curname);
}



void stream_data_header::set_file_name(std::string fname)
{
    filename = fname;
    regularize_sspath(filename);
    type = "f";
}


void stream_data_header::finalize()
{
    if(is_file())
    {
        if(fis != NULL)
            fis->close();
        if(fos != NULL)
        {
            fos->flush();
            fos->close();
            delete fos;
            // int res = system(std::string("chmod 777 " + filename).c_str());
            // if (res!=0) std::cerr << "chmod 777 " << filename << " failed" << std::endl;
        }
    }
    // else if(is_hdfs())
    // {
    //     if(ifile != NULL)
    //     {
    //         delete ifile;
    //     }
    //     if(ofile != NULL)
    //     {
    //         if(log != NULL) log->step("[write]hdfs_create_buffer");
    //         std::string str =  ofile->str();
    //         const char* buffer_out = str.c_str();
    //         write_buffer_hdfs(filename,buffer_out,log);
    //         if(log != NULL) log->step("[write]hdfs_finalize");
    //         delete ofile;
    //     }
    // }
    else
    {
        if(ifile != NULL)
        {
            delete ifile;
        }
    }
}
bool stream_data_header::is_stream()
{
    return (type.compare("s") == 0);
}

bool stream_data_header::is_serialized()
{
    return (type.compare("z") == 0);
}

void stream_data_header::serialize(bool bb)
{
    if(bb)
        type = "z";
}

bool stream_data_header::is_file()
{
    return (type.compare("f") == 0);
}

bool stream_data_header::is_hdfs()
{
    return (type.compare("h") == 0);
}





std::istream & stream_data_header::parse_header(std::istream & ist, bool is_binary)
{
    if(log != NULL) log->step("[read]parse_header");

    ist >> lab;
    int size_c;
    ist >> size_c;
    int idx;
    for(int i = 0; i < size_c; i++)
    {
        ist >> idx;
        lidx.push_back(idx);
    }
    // std::cerr << "lab:" <<  lab << " " << lidx[0] << std::endl;

    if(lab.empty() || lidx.size() == 0)
    {
        std::cerr << "[ERROR] error during header parsing" << std::endl;
        std::exit (EXIT_FAILURE);
    }
    ist >> type;
    //std::cerr << "type =:" << type << std::endl;
    if(is_file())
    {
        ist >> filename;
        // int res = system(std::string("touch " + filename).c_str());
        // if (res!=0) std::cerr << "touch " << filename << " failed" << std::endl;
        fis = new std::ifstream();
        int acc_op = 0;
        bool is_open = false;
        while (acc_op++ < 100)
        {
            if(is_binary)
            {
                fis->open(filename,std::ios::binary);
            }
            else
            {
                fis->open(filename);
            }
            if (fis->is_open())
            {
                is_open = true;
                break;
            }
            else
            {
                std::cerr << "[ERROR] read header ERROR during opening: " << filename << " [" << acc_op << "]" << std::endl;
                std::cerr << filename << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            }
        }
        if(!is_open)
        {
            std::exit(EXIT_FAILURE);
        }
    }
    else if (is_hdfs())
    {
        // ist >> filename;
        // if(log != NULL) log->step("[read]hdfs_create_buffer");
        // char * buffer_in = new  char[MAXBUFLEN+1];
        // int nbr = read_buffer_hdfs(filename,buffer_in,log);
        // ifile = new std::stringstream();
        // if(log != NULL) log->step("[read]hdfs_buffer_stringstream");
        // (*ifile) << buffer_in;
        // delete []buffer_in;
    }
    else if (is_stream())
    {
        if(log != NULL) log->step("[read]cin2sstream");
        std::string input;
        std::getline(std::cin, input);
        // initialize string stream
        ifile = new std::stringstream(input);
    }
    else
    {
        std::cout << " ";
        if(ifile != NULL)
        {
            delete ifile;
        }
    }

    return ist;
}
std::ostream & stream_data_header::write_header(std::ostream & ost, bool is_binary)
{
    //init_file_name(OUTPUT_DIR);
    if(lab.empty() || lidx.size() == 0)
    {
        std::cerr << "[ERROR] error during header writing" << std::endl;
        std::exit (EXIT_FAILURE);
    }

    ost << lab << " "  << lidx.size() << " ";
    for(auto ll : lidx)
        ost << ll << " ";
    ost <<  type << " ";

    if(is_file())
    {
        ost << filename << " ";
        boost::filesystem::path path(filename);
        fos = new std::ofstream();
        int acc_op = 0;
        bool is_open = false;

        while (acc_op++ < 20)
        {
            if(is_binary)
            {
                fos->open(filename,std::ios::binary);
            }
            else
            {
                fos->open(filename, std::ios::out);
            }
            if(fos->is_open())
            {
                is_open = true;
                break;
            }
            else
            {

                std::cerr << "[ERROR] ERROR durring writing header: " << filename << " " << std::endl;
                std::cerr << filename << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
        if(!is_open)
        {
            std::exit(EXIT_FAILURE);
        }
    }
    else if (is_hdfs())
    {
        // Ply writer
        ost << filename << " ";
        ofile = new std::ostringstream();

    }
    return ost;
}
}

#endif



