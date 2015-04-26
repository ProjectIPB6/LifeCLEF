/**
 * @file chanvese.c
 * @brief Chan-Vese active contours without edges image segmentation
 * @author Pascal Getreuer <getreuer@gmail.com>
 *
 * This file implements Chan-Vese active contours without edges two-phase
 * image segmentation.
 *
 *
 * Copyright (c) 2007-2012, Pascal Getreuer
 * All rights reserved.
 *
 * This program is free software: you can use, modify and/or
 * redistribute it under the terms of the simplified BSD License. You
 * should have received a copy of this license along this program. If
 * not, see <http://www.opensource.org/licenses/bsd-license.html>.
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "chanvese.h"

#define Malloc(s)    malloc(s)
#define Free(p)      free(p)

#define DIVIDE_EPS       ((num)1e-16)

#ifndef M_PI
/** @brief The constant pi */
#define M_PI        3.14159265358979323846264338327950288
#endif


/** @brief Options handling for ChanVese */
struct chanvesestruct
{
    num Tol;
    int MaxIter;
    num Mu;
    num Nu;
    num Lambda1;
    num Lambda2;
    num dt;
    int (*PlotFun)(int, int, num, const num*, const num*, const num*,
        int, int, int, void*);
    void *PlotParam;
};

#ifdef __GNUC__
int ChanVeseSimplePlot(int State, int Iter, num Delta,
    const num *c1, const num *c2,
    __attribute__((unused)) const num *Phi,
    __attribute__((unused)) int Width,
    __attribute__((unused)) int Height,
    int NumChannels,
    __attribute__((unused)) void *Param);
#else
int ChanVeseSimplePlot(int State, int Iter, num Delta,
    const num *c1, const num *c2,
    const num *Phi,
    int Width,
    int Height,
    int NumChannels,
    void *Param);
#endif

/** @brief Default options struct */
static struct chanvesestruct DefaultChanVeseOpt =
        {(num)1e-3, 500, (num)0.25, 0, 1, 1, (num)0.5,
        ChanVeseSimplePlot, NULL};
        

/**
 * @brief Chan-Vese two-phase image segmentation
 * @param Phi pointer to array to hold the resulting segmentation
 * @param f the input image
 * @param Width, Height, NumChannels the size of f
 * @param Tol convergence tolerance
 * @param MaxIter maximum number of iterations
 * @param Mu length penalty
 * @param Nu area penalty (positive penalizes area inside the curve)
 * @param Lambda1 fit penalty inside the curve
 * @param Lambda2 fit penalty outside the curve
 * @param dt timestep
 * @param PlotFun function for outputting intermediate results
 *
 * This function performs Chan-Vese active contours two-phase image
 * segmentation by minimizing the functional
 * \f[ \begin{aligned}\operatorname*{arg\,min}_{c_1,c_2,C}\;& \mu
 * \operatorname{Length}(C) + \nu\operatorname{Area}(\mathit{inside}(C)) \\
 * &+ \lambda_1 \int_{\mathit{inside}(C)}|f(x)-c_1|^2 \, dx + \lambda_2
 * \int_{\mathit{outside}(C)} |f(x) - c_2|^2 \, dx, \end{aligned} \f]
 * where the minimization is over all set boundaries C and scalars c1 and c2.
 * The boundary C is implicitly represented by level set function Phi.
 *
 * The input f can be a grayscale image or an image with any number of
 * channels, i.e., three channels for a color image, or possibly many more in a
 * hyperspectral image.  If f is a multichannel image, the segmentation is done
 *  using the Chan, Sandberg, Vese vector extension of the Chan-Vese model,
 * \f[ \begin{aligned}\operatorname*{arg\,min}_{c_1,c_2,C}\;& \mu
 * \operatorname{Length}(C)+\nu\operatorname{Area}(\mathit{inside}(C)) \\ &+
 * \lambda_1 \int_{\mathit{inside}(C)}\|f(x)-c_1\|^2 \,dx+\lambda_2\int_{
 * \mathit{outside}(C)}\|f(x)-c_2\|^2\,dx,\end{aligned} \f]
 * where \f$ \|\cdot\| \f$ denotes the Euclidean norm.
 *
 * The data for f should be stored as a contiguous block of data of
 * Width*Height*NumChannels elements, where the elements are ordered so that
 *   f[x + Width*(y + Height*k)] = kth component of the pixel at (x,y)
 *
 * The array Phi is a contiguous array of size Width by Height with the same
 * order as f.  Phi is a level set function of the segmentation, meaning the
 * segmentation is indicated by its sign:
 *    Phi[x + Width*y] >= 0 means (x,y) is inside the segmentation curve,
 *    Phi[x + Width*y] <  0 means (x,y) is outside.
 * Before calling this routine, Phi should be initialized either by calling
 * InitPhi or by setting it to a level set function of an initial guess of the
 * segmentation.  After this routine, the final segmentation is obtained from
 * the sign of Phi.
 *
 * The routine runs at most MaxIter number of iterations and stops when the
 * change between successive iterations is less than Tol.  Set Tol=0 to force
 * the routine to run exactly MaxIter iterations.
 */
int ChanVese(num *Phi, const num *f,
    int Width, int Height, int NumChannels, const chanveseopt *Opt)
{
    int (*PlotFun)(int, int, num, const num*, const num*, const num*,
        int, int, int, void*);
    const long NumPixels = ((long)Width) * ((long)Height);
    const long NumEl = NumPixels * NumChannels;
    const num *fPtr, *fPtr2;
    double PhiDiffNorm, PhiDiff;
    num *PhiPtr, *c1 = 0, *c2 = 0;
    num c1Scalar, c2Scalar, Mu, Nu, Lambda1, Lambda2, dt;
    num PhiLast, Delta, PhiX, PhiY, IDivU, IDivD, IDivL, IDivR;
    num Temp1, Temp2, Dist1, Dist2, PhiTol;
    int Iter, i, j, Channel, MaxIter, Success = 2;
    int iu, id, il, ir;
    
    if(!Phi || !f || Width <= 0 || Height <= 0 || NumChannels <= 0)
        return 0;
    
    if(!Opt)
        Opt = &DefaultChanVeseOpt;
    
    Mu = Opt->Mu;
    Nu = Opt->Nu;
    Lambda1 = Opt->Lambda1;
    Lambda2 = Opt->Lambda2;
    dt = Opt->dt;
    MaxIter = Opt->MaxIter;
    PlotFun = Opt->PlotFun;
    PhiTol = Opt->Tol;
    PhiDiffNorm = (PhiTol > 0) ? PhiTol*1000 : 1000;
    
    if(NumChannels > 1)
    {
        if(!(c1 = Malloc(sizeof(num)*NumChannels))
            || !(c2 = Malloc(sizeof(num)*NumChannels)))
            return 0;
    }
    else
    {
        c1 = &c1Scalar;
        c2 = &c2Scalar;
    }
    
    RegionAverages(c1, c2, Phi, f, Width, Height, NumChannels);
    
    if(PlotFun)
        if(!PlotFun(0, 0, PhiDiffNorm, c1, c2, Phi,
                Width, Height, NumChannels, Opt->PlotParam))
            goto Done;
    
    for(Iter = 1; Iter <= MaxIter; Iter++)
    {
        PhiPtr = Phi;
        fPtr = f;
        PhiDiffNorm = 0;
        
        for(j = 0; j < Height; j++)
        {
            iu = (j == 0) ? 0 : -Width;
            id = (j == Height - 1) ? 0 : Width;
            
            for(i = 0; i < Width; i++, PhiPtr++, fPtr++)
            {
                il = (i == 0) ? 0 : -1;
                ir = (i == Width - 1) ? 0 : 1;
                
                Delta = dt/(M_PI*(1 + PhiPtr[0]*PhiPtr[0]));
                PhiX = PhiPtr[ir] - PhiPtr[0];
                PhiY = (PhiPtr[id] - PhiPtr[iu])/2;
                IDivR = (num)(1/sqrt(DIVIDE_EPS + PhiX*PhiX + PhiY*PhiY));
                PhiX = PhiPtr[0] - PhiPtr[il];
                IDivL = (num)(1/sqrt(DIVIDE_EPS + PhiX*PhiX + PhiY*PhiY));
                PhiX = (PhiPtr[ir] - PhiPtr[il])/2;
                PhiY =  PhiPtr[id] - PhiPtr[0];
                IDivD = (num)(1/sqrt(DIVIDE_EPS + PhiX*PhiX + PhiY*PhiY));
                PhiY = PhiPtr[0] - PhiPtr[iu];
                IDivU = (num)(1/sqrt(DIVIDE_EPS + PhiX*PhiX + PhiY*PhiY));
                
                if(NumChannels == 1)
                {
                    Dist1 = fPtr[0] - c1Scalar;
                    Dist2 = fPtr[0] - c2Scalar;
                    Dist1 *= Dist1;
                    Dist2 *= Dist2;
                }
                else
                {
                    Dist1 = Dist2 = 0.0;
                    
                    for(Channel = 0, fPtr2 = fPtr;
                        Channel < NumChannels; Channel++, fPtr2 += NumPixels)
                    {
                        Temp1 = fPtr2[0] - c1[Channel];
                        Temp2 = fPtr2[0] - c2[Channel];
                        Dist1 += Temp1*Temp1;
                        Dist2 += Temp2*Temp2;
                    }
                }
                
                /* Semi-implicit update of phi at the current point */
                PhiLast = PhiPtr[0];
                PhiPtr[0] = (PhiPtr[0] + Delta*(
                        Mu*(PhiPtr[ir]*IDivR + PhiPtr[il]*IDivL
                            + PhiPtr[id]*IDivD + PhiPtr[iu]*IDivU)
                        - Nu - Lambda1*Dist1 + Lambda2*Dist2) ) /
                    (1 + Delta*Mu*(IDivR + IDivL + IDivD + IDivU));
                PhiDiff = (PhiPtr[0] - PhiLast);
                PhiDiffNorm += PhiDiff * PhiDiff;
            }
        }
        
        PhiDiffNorm = sqrt(PhiDiffNorm/NumEl);
        RegionAverages(c1, c2, Phi, f, Width, Height, NumChannels);
        
        if(Iter >= 2 && PhiDiffNorm <= PhiTol)
            break;
        
        if(PlotFun)
            if(!PlotFun(0, Iter, PhiDiffNorm, c1, c2, Phi,
                    Width, Height, NumChannels, Opt->PlotParam))
                goto Done;
    }

    Success = (Iter <= MaxIter) ? 1:2;

    if(PlotFun)
        PlotFun(Success, (Iter <= MaxIter) ? Iter:MaxIter,
            PhiDiffNorm, c1, c2, Phi,
            Width, Height, NumChannels, Opt->PlotParam);
    
Done:
    if(NumChannels > 1)
    {
        Free(c2);
        Free(c1);
    }
    
    return Success;
}


/** @brief Default initialization for Phi */
void ChanVeseInitPhi(num *Phi, int Width, int Height)
{
    int i, j;
    
    for(j = 0; j < Height; j++)
        for(i = 0; i < Width; i++)
            *(Phi++) = (num)(sin(i*M_PI/5.0)*sin(j*M_PI/5.0));
}


/** @brief Compute averages inside and outside of the segmentation contour */
void RegionAverages(num *c1, num *c2, const num *Phi, const num *f,
    int Width, int Height, int NumChannels)
{
    const long NumPixels = ((long)Width) * ((long)Height);
    num Sum1 = 0, Sum2 = 0;
    long n;
    long Count1 = 0, Count2 = 0;
    int Channel;
    
    for(Channel = 0; Channel < NumChannels; Channel++, f += NumPixels)
    {
        for(n = 0; n < NumPixels; n++)
            if(Phi[n] >= 0)
            {
                Count1++;
                Sum1 += f[n];
            }
            else
            {
                Count2++;
                Sum2 += f[n];
            }
        
        c1[Channel] = (Count1) ? (Sum1/Count1) : 0;
        c2[Channel] = (Count2) ? (Sum2/Count2) : 0;
    }
}



/* If GNU C language extensions are available, apply the "unused" attribute
   to avoid warnings.  TvRestoreSimplePlot is a plotting callback function
   for TvRestore, so the unused arguments are indeed required. */
#ifdef __GNUC__
int ChanVeseSimplePlot(int State, int Iter, num Delta,
    const num *c1, const num *c2,
    __attribute__((unused)) const num *Phi,
    __attribute__((unused)) int Width,
    __attribute__((unused)) int Height,
    int NumChannels,
    __attribute__((unused)) void *Param)
#else
int ChanVeseSimplePlot(int State, int Iter, num Delta,
    const num *c1, const num *c2,
    const num *Phi,
    int Width,
    int Height,
    int NumChannels,
    void *Param)
#endif
{
    switch(State)
    {
    case 0: /* ChanVese is running */
        /* We print to stderr so that messages are displayed on the console
           immediately, during the ChanVese computation.  If we use stdout,
           messages might be buffered and not displayed until after ChanVese
           completes, which would defeat the point of having this real-time
           plot callback. */
        if(NumChannels == 1)
            fprintf(stderr, "   Iteration %4d     Delta %7.4f     c1 = %6.4f     c2 = %6.4f\r",
                Iter, Delta, *c1, *c2);
        else
            fprintf(stderr, "   Iteration %4d     Delta %7.4f\r", Iter, Delta);
        break;
    case 1: /* Converged successfully */
        fprintf(stderr, "Converged in %d iterations.                                            \n",
            Iter);
        break;
    case 2: /* Maximum iterations exceeded */
        fprintf(stderr, "Maximum number of iterations exceeded.                                 \n");
        break;
    }
    return 1;
}


/*
 * Functions for options handling
 */

/**
* @brief Create a new chanveseopt options object
*
* This routine creates a new chanveseopt options object and initializes it to
* default values.  It is the caller's responsibility to call ChanVeseFreeOpt
* to free the chanveseopt object when done.
*/
chanveseopt *ChanVeseNewOpt()
{
    chanveseopt *Opt;
        
    if((Opt = (chanveseopt *)Malloc(sizeof(struct chanvesestruct))))
        *Opt = DefaultChanVeseOpt;
    
    return Opt;
}


/** @brief Free chanveseopt options object */
void ChanVeseFreeOpt(chanveseopt *Opt)
{
    if(Opt)
        Free(Opt);
}


/** @brief Specify mu, the edge length penalty */
void ChanVeseSetMu(chanveseopt *Opt, num Mu)
{
    if(Opt)
        Opt->Mu = Mu;
}


/** @brief Specify nu, the area penalty (may be positive or negative) */
void ChanVeseSetNu(chanveseopt *Opt, num Nu)
{
    if(Opt)
        Opt->Nu = Nu;
}


/** @brief Specify lambda1, the fit weight inside the curve */
void ChanVeseSetLambda1(chanveseopt *Opt, num Lambda1)
{
    if(Opt)
        Opt->Lambda1 = Lambda1;
}


/** @brief Specify lambda2, the fit weight outside the curve */
void ChanVeseSetLambda2(chanveseopt *Opt, num Lambda2)
{
    if(Opt)
        Opt->Lambda2 = Lambda2;
}


/** @brief Specify the convergence tolerance */
void ChanVeseSetTol(chanveseopt *Opt, num Tol)
{
    if(Opt)
        Opt->Tol = Tol;
}


/** @brief Specify the timestep */
void ChanVeseSetDt(chanveseopt *Opt, num dt)
{
    if(Opt)
        Opt->dt = dt;
}


/** @brief Specify the maximum number of iterations */
void ChanVeseSetMaxIter(chanveseopt *Opt, int MaxIter)
{
    if(Opt)
        Opt->MaxIter = MaxIter;
}


/**
 * @brief Specify plotting function
 * @param Opt chanveseopt options object
 * @param PlotFun plotting function
 * @param PlotParam void pointer for passing addition parameters
 *
 * Specifying the plotting function gives control over how ChanVese displays
 * information.  Setting PlotFun = NULL disables all normal display (error
 * messages are still displayed).
 *
 * An example PlotFun is
@code
    int ExamplePlotFun(int State, int Iter, num Delta,
        const num *c1, const num *c2,   const num *Phi,
        int Width, int Height, int NumChannels, void *Param)
    {
        switch(State)
        {
        case 0:
            fprintf(stderr, " RUNNING   Iter=%4d, Delta=%7.4f\r", Iter, Delta);
            break;
        case 1:
            fprintf(stderr, " CONVERGED Iter=%4d, Delta=%7.4f\n", Iter, Delta);
            break;
        case 2:
            fprintf(stderr, " Maximum number of iterations exceeded!\n");
            break;
        }
        
        return 1;
    }
@endcode
 * The State argument is either 0, 1, or 2, and indicates ChanVese's status.
 * Iter is the number of Bregman iterations completed, Delta is the change in
 * the solution Delta = ||u^cur - u^prev||_2 / ||f||_2.  Argument u gives a
 * pointer to the current solution, which can be used to plot an animated
 * display of the solution progress.  PlotParam is a void pointer that can be
 * used to pass additional information to PlotFun if needed.
 */
void ChanVeseSetPlotFun(chanveseopt *Opt,
    int (*PlotFun)(int, int, num, const num*, const num*, const num*,
        int, int, int, void*), void *PlotParam)
{
    if(Opt)
    {
        Opt->PlotFun = PlotFun;
        Opt->PlotParam = PlotParam;
    }
}


void ChanVesePrintOpt(const chanveseopt *Opt)
{
    if(!Opt)
        Opt = &DefaultChanVeseOpt;
    
    printf("tol       : %g\n", Opt->Tol);
    printf("max iter  : %d\n", Opt->MaxIter);
    printf("mu        : %g\n", Opt->Mu);
    printf("nu        : %g\n", Opt->Nu);
    printf("lambda1   : %g\n", Opt->Lambda1);
    printf("lambda2   : %g\n", Opt->Lambda2);
    printf("dt        : %g\n", Opt->dt);
}
