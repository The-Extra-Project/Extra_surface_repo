#ifndef TBMRFRECO_H
#define TBMRFRECO_H


#include "tbmrf.hpp"

template<typename DTW,typename D_MAP>
class tbmrf_reco : public tbmrf<DTW,D_MAP>
{
public :

    typedef typename DTW::Cell_const_iterator                 Cell_const_iterator;
    typedef typename DTW::Traits                              Traits;
    typedef typename Traits::Point                            Point;
    typedef typename Traits::Vertex_const_handle              Vertex_const_handle;
    typedef typename Traits::Cell_const_handle                Cell_const_handle;
    typedef typename Traits::Cell_handle                      Cell_handle;
    typedef typename Traits::Vertex_handle                    Vertex_handle;


    tbmrf_reco(int nblabs, DTW * t,D_MAP * dm) : mode(1),pLabsIn(nblabs),pLabsOut(nblabs),pLabsUnk(nblabs),tbmrf<DTW,D_MAP>(nblabs,t,dm)
    {

        for(int i = 0; i < nblabs; i++)
        {
            double prob = ((double)i)/(double(nblabs-1));
            tbmrf<DTW,D_MAP>::labs[i] = i;
            tbmrf<DTW,D_MAP>::pLabs[i] = prob;
            pLabsIn[i] = (prob >= 0.5) ? (prob-0.5)*2 : 0;
            pLabsOut[i] = (prob < 0.5) ? (0.5-prob)*2 : 0;
            pLabsUnk[i] = 1-fabs(prob-0.5)*2;
        }


    }

    void set_mode(int mm)
    {
        mode = mm;
    }

    //std::vector<Point> parse_points(std::string namefile, int D);
    //void dump_points(std::vector<Point> & points, std::string namefile, int D);


    double get_score_linear(Cell_const_iterator fch,int label,D_MAP & data_map)
    {

        double volume = 100;
        //  if(!tri->is_infinite(fch))
        volume = tbmrf<DTW,D_MAP>::get_volume(fch);
        if(fch->is_infinite())
        {
            if(mode == 2 && pLabsOut[label] > 0.5)
                return 10;
            else
                return 0;
            if(mode == 1 && pLabsOut[label] > 0.5)
                return 0;
            else
                return 10;
        }
        double nbe = 1;//((double)fch->data().dat[3]);
        if(nbe < 1) nbe = 1;
        double coef = volume/nbe;
        int cell_id = fch->lid();
        int tile_id = fch->tile()->id();
        double PIn = data_map[tile_id].format_dst[cell_id][0];
        double POut = data_map[tile_id].format_dst[cell_id][1];
        double PUnk = data_map[tile_id].format_dst[cell_id][2];
        double scoreCurr = fabs(pLabsIn[label] - PIn) + fabs(pLabsOut[label] - POut) + fabs(pLabsUnk[label] - PUnk);


	//std::cerr << "get_score_linear " << label << " " << scoreCurr << " " << PIn << " " << POut << " " << PUnk << std::endl;
	
        return coef*scoreCurr;

    }


    int extract_surface(int tid, std::vector<Facet_const_iterator> & lft, D_MAP & w_datas_tri){
    for(auto fit = this->tri->facets_begin();  fit != this->tri->facets_end(); ++fit)
    {
        try
        {
            if(fit->main_id() != tid || fit->is_infinite())
                continue;


            Cell_const_iterator tmp_fch = fit.full_cell();
            int tmp_idx = fit.index_of_covertex();
            Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);



            if(!this->tri->tile_is_loaded(tmp_fch->main_id()) ||
                    !this->tri->tile_is_loaded(tmp_fchn->main_id()))
                continue;


            Cell_const_iterator fch = tmp_fch->main();
            int id_cov = fit.index_of_covertex();
            Cell_const_iterator fchn = tmp_fchn->main();
            Vertex_h_iterator vht;

            int cccid = fch->cell_data().id;
            int cccidn = fchn->cell_data().id;

            int ch1lab = w_datas_tri[fch->tile()->id()].format_labs[cccid];
            int chnlab = w_datas_tri[fchn->tile()->id()].format_labs[cccidn];
            if(
                (ch1lab != chnlab) ||
                (((fch->is_infinite() && !fchn->is_infinite()) ||
                  (!fch->is_infinite() && fchn->is_infinite())) && ch1lab == mode )
            )
                lft.push_back(*fit);

        }
        catch (ddt::DDT_exeption& e)
        {
            std::cerr << "!! WARNING !!!" << std::endl;
            std::cerr << "Exception catched : " << e.what() << std::endl;
            continue;
        }
    }
    return 0;
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


    void hello_reco()
    {
        std::cout << "hello!reco" << std::endl;
    }


    int mode;
    std::vector<double>  pLabsIn;
    std::vector<double>  pLabsOut;
    std::vector<double> pLabsUnk;

};

#endif
