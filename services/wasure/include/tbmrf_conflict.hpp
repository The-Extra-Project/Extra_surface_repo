#ifndef TBMRFCONFLICT_H
#define TBMRFCONFLICT_H


#include "tbmrf.hpp"

template<typename DTW,typename D_MAP>
class tbmrf_conflict : public tbmrf<DTW,D_MAP>
{
public :

    typedef typename DTW::Cell_const_iterator                 Cell_const_iterator;
    typedef typename DTW::Traits                              Traits;
    typedef typename Traits::Point                            Point;
    typedef typename Traits::Vertex_const_handle              Vertex_const_handle;
    typedef typename Traits::Cell_const_handle                Cell_const_handle;
    typedef typename Traits::Cell_handle                      Cell_handle;
    typedef typename Traits::Vertex_handle                    Vertex_handle;


    tbmrf_conflict(int nblabs, DTW * t,D_MAP * dm) : pLabsIn(nblabs),pLabsOut(nblabs),pLabsUnk(nblabs),tbmrf<DTW,D_MAP>(nblabs,t,dm)
    {

        for(int i = 0; i < nblabs; i++)
        {
            double prob = ((double)i)/(double(nblabs-1));
            tbmrf<DTW,D_MAP>::labs[i] = i;
            tbmrf<DTW,D_MAP>::pLabs[i] = prob;
            pLabsIn[i] = prob*0.5;
            pLabsOut[i] = prob*0.5;
            pLabsUnk[i] = fabs(1-pLabsIn[i]-pLabsOut[i]);
        }


    }

    //std::vector<Point> parse_points(std::string namefile, int D);
    //void dump_points(std::vector<Point> & points, std::string namefile, int D);


    double get_score_linear(Cell_const_iterator fch,int label,D_MAP & data_map)
    {

        double volume = 100;
        //  if(!tri->is_infinite(fch))
        volume = tbmrf<DTW,D_MAP>::get_volume(fch);


        double nbe = 1;//((double)fch->data().dat[3]);
        if(nbe < 1) nbe = 1;
        double coef = volume/nbe;
        int cell_id = fch->cell_data().id;
        int tile_id = fch->tile()->id();
        double PIn = data_map[tile_id].format_dst[cell_id][0];
        double POut = data_map[tile_id].format_dst[cell_id][1];
        double PUnk = data_map[tile_id].format_dst[cell_id][2];

        double scoreCurr;
        double conflict = (PIn*POut)*10;
        if(label == 0)
            scoreCurr = (conflict < 1) ? conflict : 1;
        else
            scoreCurr = (1-conflict > 0) ? (1-conflict) : 0;
        return coef*scoreCurr;

    }




    void get_obs_cut_pursuit(Cell_const_iterator fch,double & obs, double & weight)
    {
        // double eps = 0.001;
        // double volume = 100;
        // //  if(!tri->is_infinite(fch))
        //   volume = get_volume(fch);


        // double nbe = ((double)fch->data().dat[3]);
        // if(nbe < 1) nbe = 1;
        // double coef = volume/nbe;
        // double PIn = (fch->data().dat[1])/nbe;
        // double POut = (fch->data().dat[0])/nbe;
        // double PUnk = (fch->data().dat[2])/nbe;
        // if(PIn < eps && POut < eps){
        //   PIn = POut = eps;
        //   PUnk = 1-2*eps;
        // }


        // obs =  (PIn)/(POut+PIn);
        // weight = volume*(1-PUnk);

        // if(obs != obs || weight != weight){
        //   std::cout << "/!\\ warning NAN!!!! /!\\" << std::endl;
        //   obs = 0.5;
        //   weight = 0.01;
        // }
        return;

    }


    void hello_conflict()
    {
        std::cout << "hello!conflict" << std::endl;
    }



    std::vector<double>  pLabsIn;
    std::vector<double>  pLabsOut;
    std::vector<double> pLabsUnk;

};

#endif
