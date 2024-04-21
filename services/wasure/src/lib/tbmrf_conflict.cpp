#include "tbmrf_conflict.hpp"


// double tbmrf_conflict::get_score_linear(Cell_const_iterator fch,int label){
//   double volume = 100;
//   if(!tri->is_infinite(fch))
//     volume = get_volume(fch);


//   double nbe = ((double)fch->data().acc);
//   if(nbe < 1) nbe = 1;
//   double coef = volume/nbe;

//   double PIn = (fch->data().vpo)/nbe;
//   double POut = (fch->data().vpe)/nbe;
//   double PUnk = (fch->data().vpu)/nbe;
//   double scoreCurr = fabs(pLabsIn[label] - PIn) + fabs(pLabsOut[label] - POut) + fabs(pLabsUnk[label] - PUnk);

//   return coef*scoreCurr;

// }



double get_score_linear(Cell_const_iterator fch,int label)
{

    double volume = 100;
    //  if(!tri->is_infinite(fch))
    volume = get_volume(fch);


    double nbe = ((double)fch->data().dat[3]);
    if(nbe < 1) nbe = 1;
    double coef = volume/nbe;
    double PIn = (fch->data().dat[1])/nbe;
    double POut = (fch->data().dat[0])/nbe;
    double PUnk = (fch->data().dat[2])/nbe;

    double scoreCurr;
    double conflict = (PIn*POut)*0.2;
    if(label == 0)
        scoreCurr = (conflict < 1) ? conflict : 1;
    else
        scoreCurr = (1-conflict > 0) ? (1-conflict) : 0;

    return coef*scoreCurr;

}




void get_obs_cut_pursuit(Cell_const_iterator fch,double & obs, double & weight)
{
    double eps = 0.001;
    double volume = 100;
    //  if(!tri->is_infinite(fch))
    volume = get_volume(fch);


    double nbe = ((double)fch->data().dat[3]);
    if(nbe < 1) nbe = 1;
    double coef = volume/nbe;
    double PIn = (fch->data().dat[1])/nbe;
    double POut = (fch->data().dat[0])/nbe;
    double PUnk = (fch->data().dat[2])/nbe;
    if(PIn < eps && POut < eps)
    {
        PIn = POut = eps;
        PUnk = 1-2*eps;
    }


    obs =  (PIn)/(POut+PIn);
    weight = volume*(1-PUnk);

    if(obs != obs || weight != weight)
    {
        std::cout << "/!\\ warning NAN!!!! /!\\" << std::endl;
        obs = 0.5;
        weight = 0.01;
    }

}


