/*
    Copyright 2019 SHENZHEN ZHILAI SCI AND TECH CO., LTD,
    15th floor, C3 building, Nanshan ipark,Xueyuan road 1001,
	Nanshan District, Shenzhen 518052, China.
    All rights are reserved. Reproduction in whole or in part is prohibited
    without the prior written consent of the copyright owner.

    COMPANY CONFIDENTIAL

Filename  :  lis2dh.c


Rev     Date    Author  Comments
-------------------------------------------------------------------------------
001  2021.3.27   zjs
002
003
004
-------------------------------------------------------------------------------

@Begin
@module
        Module name
        Module description (Driving Bluetooth module E95)
@end
*/

#include <float.h>

double sqrt(double x)
{
    float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f375a86 - (i >> 1);
    x = *(float*)&i;
    x = x*(1.5f - xhalf*x*x);
    x = x*(1.5f - xhalf*x*x);
    x = x*(1.5f - xhalf*x*x);
    return 1 / x;
}

/*	Declare certain constants volatile to force the compiler to access them
    when we reference them.  This in turn forces arithmetic operations on them
    to be performed at run time (or as if at run time).  We use such operations
    to generate exceptions such as underflow or inexact.
*/
static volatile const double Tiny = 0x1p-1022;


#if defined __STDC__ && 199901L <= __STDC_VERSION__ && !defined __GNUC__
    /* GCC does not currently support FENV_ACCESS.  Maybe someday.*/
    #pragma STDC FENV_ACCESS ON
#endif


double pow(double num,double n)
 {
      double value =1;
      int i =1;
      if(n ==0)
      {
         value =1;
     }
     else
     {
         while(i++<= n)
         {
             value *= num;
         }
     }
     return value;
 }


/* Return arctangent(x) given that 2 < x, with the same properties as atan.*/
static double Tail(double x)
{
    {
        static const double HalfPi = 0x3.243f6a8885a308d313198a2e037ap-1;

        /* For large x, generate inexact and return pi/2.*/
        if (0x1p53 <= x)
            return HalfPi + Tiny;
        else/*if (isnan(x))*/
            return x - x;
    }

    static const double p03 = -0x1.5555555554A51p-2;
    static const double p05 = +0x1.999999989EBCAp-3;
    static const double p07 = -0x1.249248E1422E3p-3;
    static const double p09 = +0x1.C71C5EDFED480p-4;
    static const double p11 = -0x1.745B7F2D72663p-4;
    static const double p13 = +0x1.3AFD7A0E6EB75p-4;
    static const double p15 = -0x1.104146B1A1AE8p-4;
    static const double p17 = +0x1.D78252FA69C1Cp-5;
    static const double p19 = -0x1.81D33E401836Dp-5;
    static const double p21 = +0x1.007733E06CEB3p-5;
    static const double p23 = -0x1.83DAFDA7BD3FDp-7;

    static const double p000 = +0x1.921FB54442D18p0;
    static const double p001 = +0x1.1A62633145C07p-54;

    double y = 1/x;

    /* Square y.*/
    double y2 = y * y;

    return p001 - ((((((((((((
        + p23) * y2
        + p21) * y2
        + p19) * y2
        + p17) * y2
        + p15) * y2
        + p13) * y2
        + p11) * y2
        + p09) * y2
        + p07) * y2
        + p05) * y2
        + p03) * y2 * y + y) + p000;
}


/*	Return arctangent(x) given that 0x1p-27 < |x| <= 1/2, with the same
    properties as atan.
*/
static double atani0(double x)
{
    static const double p03 = -0x1.555555555551Bp-2;
    static const double p05 = +0x1.99999999918D8p-3;
    static const double p07 = -0x1.2492492179CA3p-3;
    static const double p09 = +0x1.C71C7096C2725p-4;
    static const double p11 = -0x1.745CF51795B21p-4;
    static const double p13 = +0x1.3B113F18AC049p-4;
    static const double p15 = -0x1.10F31279EC05Dp-4;
    static const double p17 = +0x1.DFE7B9674AE37p-5;
    static const double p19 = -0x1.A38CF590469ECp-5;
    static const double p21 = +0x1.56CDB5D887934p-5;
    static const double p23 = -0x1.C0EB85F543412p-6;
    static const double p25 = +0x1.4A9F5C4724056p-7;

    /* Square x.*/
    double x2 = x * x;

    return ((((((((((((
        + p25) * x2
        + p23) * x2
        + p21) * x2
        + p19) * x2
        + p17) * x2
        + p15) * x2
        + p13) * x2
        + p11) * x2
        + p09) * x2
        + p07) * x2
        + p05) * x2
        + p03) * x2 * x + x;
}


/*	Return arctangent(x) given that 1/2 < x <= 3/4, with the same properties as
	atan.
*/
static double atani1(double x)
{
    static const double p00 = +0x1.1E00BABDEFED0p-1;
    static const double p01 = +0x1.702E05C0B8155p-1;
    static const double p02 = -0x1.4AF2B78215A1Bp-2;
    static const double p03 = +0x1.5D0B7E9E69054p-6;
    static const double p04 = +0x1.A1247CA5D9475p-4;
    static const double p05 = -0x1.519E110F61B54p-4;
    static const double p06 = +0x1.A759263F377F2p-7;
    static const double p07 = +0x1.094966BE2B531p-5;
    static const double p08 = -0x1.09BC0AB7F914Cp-5;
    static const double p09 = +0x1.FF3B7C531AA4Ap-8;
    static const double p10 = +0x1.950E69DCDD967p-7;
    static const double p11 = -0x1.D88D31ABC3AE5p-7;
    static const double p12 = +0x1.10F3E20F6A2E2p-8;

    double y = x - 0x1.4000000000027p-1;

    return ((((((((((((
        + p12) * y
        + p11) * y
        + p10) * y
        + p09) * y
        + p08) * y
        + p07) * y
        + p06) * y
        + p05) * y
        + p04) * y
        + p03) * y
        + p02) * y
        + p01) * y
        + p00;
}


/*	Return arctangent(x) given that 3/4 < x <= 1, with the same properties as
    atan.
*/
static double atani2(double x)
{
    static const double p00 = +0x1.700A7C580EA7Ep-01;
    static const double p01 = +0x1.21FB781196AC3p-01;
    static const double p02 = -0x1.1F6A8499714A2p-02;
    static const double p03 = +0x1.41B15E5E8DCD0p-04;
    static const double p04 = +0x1.59BC93F81895Ap-06;
    static const double p05 = -0x1.63B543EFFA4EFp-05;
    static const double p06 = +0x1.C90E92AC8D86Cp-06;
    static const double p07 = -0x1.91F7E2A7A338Fp-08;
    static const double p08 = -0x1.AC1645739E676p-08;
    static const double p09 = +0x1.152311B180E6Cp-07;
    static const double p10 = -0x1.265EF51B17DB7p-08;
    static const double p11 = +0x1.CA7CDE5DE9BD7p-14;

    double y = x - 0x1.c0000000f4213p-1;

    return (((((((((((
        + p11) * y
        + p10) * y
        + p09) * y
        + p08) * y
        + p07) * y
        + p06) * y
        + p05) * y
        + p04) * y
        + p03) * y
        + p02) * y
        + p01) * y
        + p00;
}


/*	Return arctangent(x) given that 1 < x <= 4/3, with the same properties as
	atan.
*/
static double atani3(double x)
{
    static const double p00 = +0x1.B96E5A78C5C40p-01;
    static const double p01 = +0x1.B1B1B1B1B1B3Dp-02;
    static const double p02 = -0x1.AC97826D58470p-03;
    static const double p03 = +0x1.3FD2B9F586A67p-04;
    static const double p04 = -0x1.BC317394714B7p-07;
    static const double p05 = -0x1.2B01FC60CC37Ap-07;
    static const double p06 = +0x1.73A9328786665p-07;
    static const double p07 = -0x1.C0B993A09CE31p-08;
    static const double p08 = +0x1.2FCDACDD6E5B5p-09;
    static const double p09 = +0x1.CBD49DA316282p-13;
    static const double p10 = -0x1.0120E602F6336p-10;
    static const double p11 = +0x1.A89224FF69018p-11;
    static const double p12 = -0x1.883D8959134B3p-12;

    double y = x - 0x1.2aaaaaaaaaa96p0;

    return ((((((((((((
        + p12) * y
        + p11) * y
        + p10) * y
        + p09) * y
        + p08) * y
        + p07) * y
        + p06) * y
        + p05) * y
        + p04) * y
        + p03) * y
        + p02) * y
        + p01) * y
        + p00;
}


/*	Return arctangent(x) given that 4/3 < x <= 5/3, with the same properties as
	atan.
*/
static double atani4(double x)
{
    static const double p00 = +0x1.F730BD281F69Dp-01;
    static const double p01 = +0x1.3B13B13B13B0Cp-02;
    static const double p02 = -0x1.22D719C06115Ep-03;
    static const double p03 = +0x1.C963C83985742p-05;
    static const double p04 = -0x1.135A0938EC462p-06;
    static const double p05 = +0x1.13A254D6E5B7Cp-09;
    static const double p06 = +0x1.DFAA5E77B7375p-10;
    static const double p07 = -0x1.F4AC1342182D2p-10;
    static const double p08 = +0x1.25BAD4D85CBE1p-10;
    static const double p09 = -0x1.E4EEF429EB680p-12;
    static const double p10 = +0x1.B4E30D1BA3819p-14;
    static const double p11 = +0x1.0280537F097F3p-15;

    double y = x - 0x1.8000000000003p0;

    return (((((((((((
        + p11) * y
        + p10) * y
        + p09) * y
        + p08) * y
        + p07) * y
        + p06) * y
        + p05) * y
        + p04) * y
        + p03) * y
        + p02) * y
        + p01) * y
        + p00;
}


/*	Return arctangent(x) given that 5/3 < x <= 2, with the same properties as
	atan.
*/
static double atani5(double x)
{
    static const double p00 = +0x1.124A85750FB5Cp+00;
    static const double p01 = +0x1.D59AE78C11C49p-03;
    static const double p02 = -0x1.8AD3C44F10DC3p-04;
    static const double p03 = +0x1.2B090AAD5F9DCp-05;
    static const double p04 = -0x1.881EC3D15241Fp-07;
    static const double p05 = +0x1.8CB82A74E0699p-09;
    static const double p06 = -0x1.3182219E21362p-12;
    static const double p07 = -0x1.2B9AD13DB35A8p-12;
    static const double p08 = +0x1.10F884EAC0E0Ap-12;
    static const double p09 = -0x1.3045B70E93129p-13;
    static const double p10 = +0x1.00B6A460AC05Dp-14;

    double y = x - 0x1.d555555461337p0;

    return ((((((((((
        + p10) * y
        + p09) * y
        + p08) * y
        + p07) * y
        + p06) * y
        + p05) * y
        + p04) * y
        + p03) * y
        + p02) * y
        + p01) * y
        + p00;
}


double atan(double x)
{
    if (x < 0)
    {
        if (x < -1)
            if (x < -5/3.)
            {
                if (x < -2)
                {
                    return -Tail(-x);
                }
                else
                {
                    return -atani5(-x);
                }
            }
            else
            {
                if (x < -4/3.)
                {
                    return -atani4(-x);
                }
                else
                {
                    return -atani3(-x);
                }
            }
        else
        {
            if (x < -.5)
            {
                if (x < -.75)
                {
                	return -atani2(-x);
                }
                else
                {
                    return -atani1(-x);
                }
            }
            else
            {
                if (x < -0x1.d12ed0af1a27fp-27)
                {
                	return atani0(x);
                }
                else
                {
                    if (x <= -0x1p-1022)
                    {
                        /* Generate inexact and return x.*/
                        return (Tiny + 1) * x;
                    }
                    else
                    {
                        if (x == 0)
                        {
                            return x;
                        }
                        else
                        {
                            /* Generate underflow and return x.*/
                            return x*Tiny + x;
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (x <= +1)
        {
            if (x <= +.5)
                if (x <= +0x1.d12ed0af1a27fp-27)
                {
                    if (x < +0x1p-1022)
                    {
                        if (x == 0)
                        {
                            return x;
                        }
                        else
                        {
                            /* Generate underflow and return x.*/
                            return x*Tiny + x;
                        }
                    }
                    else
                    {
                        /* Generate inexact and return x.*/
                        return (Tiny + 1) * x;
                    }
                }
                else
                {
                    return atani0(x);
                }
            else
            {
                if (x <= +.75)
                {
                    return +atani1(+x);
                }
                else
                {
                    return +atani2(+x);
                }
            }
        }
        else
        {
            if (x <= +5/3.)
            {
                if (x <= +4/3.)
                {
                    return +atani3(+x);
                }
                else
                {
                    return +atani4(+x);
                }
            }
            else
            {
                if (x <= +2)
                {
                    return +atani5(+x);
                }
                else
                {
                    return +Tail(+x);
                }
			}
		}
	}
}

float fast_atan_table[257] =
{
    0.000000e+00, 3.921549e-03, 7.842976e-03, 1.176416e-02,
    1.568499e-02, 1.960533e-02, 2.352507e-02, 2.744409e-02,
    3.136226e-02, 3.527947e-02, 3.919560e-02, 4.311053e-02,
    4.702413e-02, 5.093629e-02, 5.484690e-02, 5.875582e-02,
    6.266295e-02, 6.656816e-02, 7.047134e-02, 7.437238e-02,
    7.827114e-02, 8.216752e-02, 8.606141e-02, 8.995267e-02,
    9.384121e-02, 9.772691e-02, 1.016096e-01, 1.054893e-01,
    1.093658e-01, 1.132390e-01, 1.171087e-01, 1.209750e-01,
    1.248376e-01, 1.286965e-01, 1.325515e-01, 1.364026e-01,
    1.402496e-01, 1.440924e-01, 1.479310e-01, 1.517652e-01,
    1.555948e-01, 1.594199e-01, 1.632403e-01, 1.670559e-01,
    1.708665e-01, 1.746722e-01, 1.784728e-01, 1.822681e-01,
    1.860582e-01, 1.898428e-01, 1.936220e-01, 1.973956e-01,
    2.011634e-01, 2.049255e-01, 2.086818e-01, 2.124320e-01,
    2.161762e-01, 2.199143e-01, 2.236461e-01, 2.273716e-01,
    2.310907e-01, 2.348033e-01, 2.385093e-01, 2.422086e-01,
    2.459012e-01, 2.495869e-01, 2.532658e-01, 2.569376e-01,
    2.606024e-01, 2.642600e-01, 2.679104e-01, 2.715535e-01,
    2.751892e-01, 2.788175e-01, 2.824383e-01, 2.860514e-01,
    2.896569e-01, 2.932547e-01, 2.968447e-01, 3.004268e-01,
    3.040009e-01, 3.075671e-01, 3.111252e-01, 3.146752e-01,
    3.182170e-01, 3.217506e-01, 3.252758e-01, 3.287927e-01,
    3.323012e-01, 3.358012e-01, 3.392926e-01, 3.427755e-01,
    3.462497e-01, 3.497153e-01, 3.531721e-01, 3.566201e-01,
    3.600593e-01, 3.634896e-01, 3.669110e-01, 3.703234e-01,
    3.737268e-01, 3.771211e-01, 3.805064e-01, 3.838825e-01,
    3.872494e-01, 3.906070e-01, 3.939555e-01, 3.972946e-01,
    4.006244e-01, 4.039448e-01, 4.072558e-01, 4.105574e-01,
    4.138496e-01, 4.171322e-01, 4.204054e-01, 4.236689e-01,
    4.269229e-01, 4.301673e-01, 4.334021e-01, 4.366272e-01,
    4.398426e-01, 4.430483e-01, 4.462443e-01, 4.494306e-01,
    4.526070e-01, 4.557738e-01, 4.589307e-01, 4.620778e-01,
    4.652150e-01, 4.683424e-01, 4.714600e-01, 4.745676e-01,
    4.776654e-01, 4.807532e-01, 4.838312e-01, 4.868992e-01,
    4.899573e-01, 4.930055e-01, 4.960437e-01, 4.990719e-01,
    5.020902e-01, 5.050985e-01, 5.080968e-01, 5.110852e-01,
    5.140636e-01, 5.170320e-01, 5.199904e-01, 5.229388e-01,
    5.258772e-01, 5.288056e-01, 5.317241e-01, 5.346325e-01,
    5.375310e-01, 5.404195e-01, 5.432980e-01, 5.461666e-01,
    5.490251e-01, 5.518738e-01, 5.547124e-01, 5.575411e-01,
    5.603599e-01, 5.631687e-01, 5.659676e-01, 5.687566e-01,
    5.715357e-01, 5.743048e-01, 5.770641e-01, 5.798135e-01,
    5.825531e-01, 5.852828e-01, 5.880026e-01, 5.907126e-01,
    5.934128e-01, 5.961032e-01, 5.987839e-01, 6.014547e-01,
    6.041158e-01, 6.067672e-01, 6.094088e-01, 6.120407e-01,
    6.146630e-01, 6.172755e-01, 6.198784e-01, 6.224717e-01,
    6.250554e-01, 6.276294e-01, 6.301939e-01, 6.327488e-01,
    6.352942e-01, 6.378301e-01, 6.403565e-01, 6.428734e-01,
    6.453808e-01, 6.478788e-01, 6.503674e-01, 6.528466e-01,
    6.553165e-01, 6.577770e-01, 6.602282e-01, 6.626701e-01,
    6.651027e-01, 6.675261e-01, 6.699402e-01, 6.723452e-01,
    6.747409e-01, 6.771276e-01, 6.795051e-01, 6.818735e-01,
    6.842328e-01, 6.865831e-01, 6.889244e-01, 6.912567e-01,
    6.935800e-01, 6.958943e-01, 6.981998e-01, 7.004964e-01,
    7.027841e-01, 7.050630e-01, 7.073330e-01, 7.095943e-01,
    7.118469e-01, 7.140907e-01, 7.163258e-01, 7.185523e-01,
    7.207701e-01, 7.229794e-01, 7.251800e-01, 7.273721e-01,
    7.295557e-01, 7.317307e-01, 7.338974e-01, 7.360555e-01,
    7.382053e-01, 7.403467e-01, 7.424797e-01, 7.446045e-01,
    7.467209e-01, 7.488291e-01, 7.509291e-01, 7.530208e-01,
    7.551044e-01, 7.571798e-01, 7.592472e-01, 7.613064e-01,
    7.633576e-01, 7.654008e-01, 7.674360e-01, 7.694633e-01,
    7.714826e-01, 7.734940e-01, 7.754975e-01, 7.774932e-01,
    7.794811e-01, 7.814612e-01, 7.834335e-01, 7.853983e-01,
    7.853983e-01
};
