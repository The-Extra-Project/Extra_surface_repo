
#include "wasure_maths.hpp"

double score_pdf(double a,double scale)
{
    return exp(-(a/scale)*(a/scale));
    //return exp(-(fabs(a)/scale));
    //return m_pdf(fabs((a/scale)*(a/scale)),1);
}

void regularize2(double & a)
{
    if(a > 1) a = 1;
    if(a < 0) a = 0;
}

void regularize(double & a, double & b, double & c)
{
    regularize2(a);
    regularize2(b);
    c = 1-b-a;
    if(c < 0)
    {
        c = 0;
    }
    double norm = a+b+c;
    a = a/norm;
    b = b/norm;
    c = c/norm;
}








