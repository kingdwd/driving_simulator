/*
FORCESNLPsolver : A fast customized optimization solver.

Copyright (C) 2013-2016 EMBOTECH GMBH [info@embotech.com]. All rights reserved.


This software is intended for simulation and testing purposes only. 
Use of this software for any commercial purpose is prohibited.

This program is distributed in the hope that it will be useful.
EMBOTECH makes NO WARRANTIES with respect to the use of the software 
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE. 

EMBOTECH shall not have any liability for any damage arising from the use
of the software.

This Agreement shall exclusively be governed by and interpreted in 
accordance with the laws of Switzerland, excluding its principles
of conflict of laws. The Courts of Zurich-City shall have exclusive 
jurisdiction in case of any dispute.

*/

#include <stdio.h>

#ifndef __FORCESNLPsolver_H__
#define __FORCESNLPsolver_H__

/* DATA TYPE ------------------------------------------------------------*/
typedef double FORCESNLPsolver_FLOAT;

typedef double FORCESNLPsolverINTERFACE_FLOAT;

/* SOLVER SETTINGS ------------------------------------------------------*/
/* print level */
#ifndef FORCESNLPsolver_SET_PRINTLEVEL
#define FORCESNLPsolver_SET_PRINTLEVEL    (1)
#endif

/* timing */
#ifndef FORCESNLPsolver_SET_TIMING
#define FORCESNLPsolver_SET_TIMING    (1)
#endif

/* Numeric Warnings */
/* #define PRINTNUMERICALWARNINGS */

/* maximum number of iterations  */
#define FORCESNLPsolver_SET_MAXIT			(250)	

/* scaling factor of line search (FTB rule) */
#define FORCESNLPsolver_SET_FLS_SCALE		(FORCESNLPsolver_FLOAT)(0.99)      

/* maximum number of supported elements in the filter */
#define FORCESNLPsolver_MAX_FILTER_SIZE	(250) 

/* maximum number of supported elements in the filter */
#define FORCESNLPsolver_MAX_SOC_IT			(4) 

/* desired relative duality gap */
#define FORCESNLPsolver_SET_ACC_RDGAP		(FORCESNLPsolver_FLOAT)(0.0001)

/* desired maximum residual on equality constraints */
#define FORCESNLPsolver_SET_ACC_RESEQ		(FORCESNLPsolver_FLOAT)(1E-06)

/* desired maximum residual on inequality constraints */
#define FORCESNLPsolver_SET_ACC_RESINEQ	(FORCESNLPsolver_FLOAT)(1E-06)

/* desired maximum violation of complementarity */
#define FORCESNLPsolver_SET_ACC_KKTCOMPL	(FORCESNLPsolver_FLOAT)(1E-06)


/* RETURN CODES----------------------------------------------------------*/
/* solver has converged within desired accuracy */
#define FORCESNLPsolver_OPTIMAL      (1)

/* maximum number of iterations has been reached */
#define FORCESNLPsolver_MAXITREACHED (0)

/* NaN encountered in function evaluations */
#define FORCESNLPsolver_BADFUNCEVAL  (-6)

/* no progress in method possible */
#define FORCESNLPsolver_NOPROGRESS   (-7)



/* PARAMETERS -----------------------------------------------------------*/
/* fill this with data before calling the solver! */
typedef struct FORCESNLPsolver_params
{
    /* vector of size 450 */
    FORCESNLPsolver_FLOAT x0[450];

    /* vector of size 6 */
    FORCESNLPsolver_FLOAT xinit[6];

    /* vector of size 75000 */
    FORCESNLPsolver_FLOAT all_parameters[75000];

} FORCESNLPsolver_params;


/* OUTPUTS --------------------------------------------------------------*/
/* the desired variables are put here by the solver */
typedef struct FORCESNLPsolver_output
{
    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x01[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x02[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x03[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x04[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x05[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x06[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x07[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x08[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x09[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x10[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x11[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x12[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x13[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x14[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x15[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x16[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x17[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x18[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x19[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x20[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x21[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x22[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x23[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x24[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x25[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x26[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x27[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x28[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x29[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x30[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x31[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x32[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x33[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x34[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x35[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x36[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x37[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x38[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x39[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x40[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x41[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x42[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x43[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x44[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x45[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x46[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x47[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x48[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x49[9];

    /* vector of size 9 */
    FORCESNLPsolver_FLOAT x50[9];

} FORCESNLPsolver_output;


/* SOLVER INFO ----------------------------------------------------------*/
/* diagnostic data from last interior point step */
typedef struct FORCESNLPsolver_info
{
    /* iteration number */
    int it;

	/* number of iterations needed to optimality (branch-and-bound) */
	int it2opt;
	
    /* inf-norm of equality constraint residuals */
    FORCESNLPsolver_FLOAT res_eq;
	
    /* inf-norm of inequality constraint residuals */
    FORCESNLPsolver_FLOAT res_ineq;

    /* primal objective */
    FORCESNLPsolver_FLOAT pobj;	
	
    /* dual objective */
    FORCESNLPsolver_FLOAT dobj;	

    /* duality gap := pobj - dobj */
    FORCESNLPsolver_FLOAT dgap;		
	
    /* relative duality gap := |dgap / pobj | */
    FORCESNLPsolver_FLOAT rdgap;		

    /* duality measure */
    FORCESNLPsolver_FLOAT mu;

	/* duality measure (after affine step) */
    FORCESNLPsolver_FLOAT mu_aff;
	
    /* centering parameter */
    FORCESNLPsolver_FLOAT sigma;
	
    /* number of backtracking line search steps (affine direction) */
    int lsit_aff;
    
    /* number of backtracking line search steps (combined direction) */
    int lsit_cc;
    
    /* step size (affine direction) */
    FORCESNLPsolver_FLOAT step_aff;
    
    /* step size (combined direction) */
    FORCESNLPsolver_FLOAT step_cc;    

	/* solvertime */
	FORCESNLPsolver_FLOAT solvetime;   

	/* time spent in function evaluations */
	FORCESNLPsolver_FLOAT fevalstime;  

} FORCESNLPsolver_info;








/* SOLVER FUNCTION DEFINITION -------------------------------------------*/
/* examine exitflag before using the result! */
#ifdef __cplusplus
extern "C" {
#endif		
void FORCESNLPsolver_casadi2forces(FORCESNLPsolver_FLOAT *x,        /* primal vars                                         */
                                 FORCESNLPsolver_FLOAT *y,        /* eq. constraint multiplers                           */
                                 FORCESNLPsolver_FLOAT *l,        /* ineq. constraint multipliers                        */
                                 FORCESNLPsolver_FLOAT *p,        /* parameters                                          */
                                 FORCESNLPsolver_FLOAT *f,        /* objective function (scalar)                         */
                                 FORCESNLPsolver_FLOAT *nabla_f,  /* gradient of objective function                      */
                                 FORCESNLPsolver_FLOAT *c,        /* dynamics                                            */
                                 FORCESNLPsolver_FLOAT *nabla_c,  /* Jacobian of the dynamics (column major)             */
                                 FORCESNLPsolver_FLOAT *h,        /* inequality constraints                              */
                                 FORCESNLPsolver_FLOAT *nabla_h,  /* Jacobian of inequality constraints (column major)   */
                                 FORCESNLPsolver_FLOAT *H,        /* Hessian (column major)                              */
                                 int stage                      /* stage number (0 indexed)                            */
                  );
typedef void (*FORCESNLPsolver_ExtFunc)(FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, FORCESNLPsolver_FLOAT*, int);

int FORCESNLPsolver_solve(FORCESNLPsolver_params* params, FORCESNLPsolver_output* output, FORCESNLPsolver_info* info, FILE* fs, FORCESNLPsolver_ExtFunc FORCESNLPsolver_evalExtFunctions);	


#ifdef __cplusplus
}
#endif

#endif
