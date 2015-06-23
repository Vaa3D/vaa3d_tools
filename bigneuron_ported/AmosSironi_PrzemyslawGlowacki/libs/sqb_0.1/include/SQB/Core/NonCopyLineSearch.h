#ifndef _SQB_NONCOPYLINESEARCH_H
#define _SQB_NONCOPYLINESEARCH_H

#define myQDebug(...) printf (__VA_ARGS__)
#define myQFatal(...) do{ printf (__VA_ARGS__); exit(1); } while(0)

// This file is part of SQBlib, a gradient boosting/boosted trees implementation.
//
// Copyright (C) 2012 Carlos Becker, http://sites.google.com/site/carlosbecker
//
// SQBlib is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// SQBlib is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with SQBlib.  If not, see <http://www.gnu.org/licenses/>.

#include <lbfgs.h>

namespace SQB
{
    template<class ArrayType, class ParamArrayType>
    class   NonCopyLineSearch
    {
    public:
        // WIll COPY THE DATA!! Not optimal!
        const ArrayType mPrevScores, mNewScores, mY;

        LossType    mLossType;
        double  mLossScale; // this will be set by the first iteration of evaluate()

        static lbfgsfloatval_t evaluate(
            void *instance,
            const lbfgsfloatval_t *x,
            lbfgsfloatval_t *g,
            const int n,
            const lbfgsfloatval_t step
            )
        {
            NonCopyLineSearch *LS = (NonCopyLineSearch *) instance;

            lbfgsfloatval_t fx = 0;

            lbfgsfloatval_t alpha = x[0];

            if ( LS->mLossType == ExpLoss )
            {
                fx = (- LS->mY * (LS->mPrevScores + alpha * LS->mNewScores)).exp().sum();

                if (LS->mLossScale == 0.0)  // initialize for the 1st time
                {
                    LS->mLossScale = 1.0 / fx;
                    std::cout << "Scaling: " << LS->mLossScale << std::endl;
                    fx = 1.0;
                } else
                    fx = fx * LS->mLossScale;



                g[0] = LS->mLossScale * ( - LS->mY * LS->mNewScores * (- LS->mY * (alpha * LS->mNewScores + LS->mPrevScores) ).exp() ).sum();
            }
            else if ( LS->mLossType == LogLoss )
            {
                fx = (1 + std::numeric_limits<double>::epsilon() + (- 2 * LS->mY * (LS->mPrevScores + alpha * LS->mNewScores)).exp()).log().sum();

                if (LS->mLossScale == 0.0)  // initialize for the 1st time
                {
                    LS->mLossScale = 1.0 / fx;
                    std::cout << "Scaling: " << LS->mLossScale << std::endl;
                    fx = 1.0;
                } else
                    fx = fx * LS->mLossScale;


                g[0] = - 2 * LS->mLossScale * ( ( - 2 * LS->mY * ( alpha * LS->mNewScores + LS->mPrevScores )).exp() * LS->mNewScores * LS->mY
                          / ( 1 + (- 2 * LS->mY * ( alpha * LS->mNewScores + LS->mPrevScores )).exp() ) ).sum();



                //g[0] = LS->mLossScale * (- ( 2 * LS->mNewScores * LS->mY ) / ).sum();
                //g[0] = ( - LS->mY * LS->mNewScores * (- LS->mY * (alpha * LS->mNewScores + LS->mPrevScores) ).exp() ).sum();
            }
            else if ( LS->mLossType == SquaredLoss )
            {

                fx = ( LS->mPrevScores + alpha * LS->mNewScores - LS->mY ).square().sum();

                if (LS->mLossScale == 0.0)  // initialize for the 1st time
                {
                    LS->mLossScale = 1.0 / fx;
                    std::cout << "Scaling: " << LS->mLossScale << std::endl;
                    fx = 1.0;
                }
                else
                    fx = fx * LS->mLossScale;

                g[0] = 2 * LS->mLossScale * ( LS->mNewScores * ( alpha * LS->mNewScores + LS->mPrevScores - LS->mY ) ).sum();

            }
            else
                mymyQFatal("LineSearch: FATAL: Wrong loss type!");

            return fx;
        }

        static int progress(
            void *instance,
            const lbfgsfloatval_t *x,
            const lbfgsfloatval_t *g,
            const lbfgsfloatval_t fx,
            const lbfgsfloatval_t xnorm,
            const lbfgsfloatval_t gnorm,
            const lbfgsfloatval_t step,
            int n,
            int k,
            int ls
            )
        {
            /*printf("Iteration %d:\n", k);
            printf("  fx = %f, x[0] = %f\n", fx, x[0]);
            printf("  xnorm = %f, gnorm = %f, step = %f\n", xnorm, gnorm, step);
            printf("\n");*/
            return 0;
        }


        NonCopyLineSearch( const ParamArrayType &prevScores, const ParamArrayType &newScores, const ParamArrayType &Y, LossType loss, std::vector<unsigned> *subSampIdxs = 0 )
            : mPrevScores(prevScores), mNewScores(newScores), mY(Y), mLossType(loss)
        {
            mLossScale = 0.0;

            if ( subSampIdxs != 0 )
            {
                myQFatal("NonCopyLineSearch does not support subsampIdxs");
            }
        }

        double run()
        {
            const unsigned N = 1;
            double toRet = 0;

            lbfgsfloatval_t fx = 0;
            lbfgsfloatval_t *x = lbfgs_malloc(N);

            lbfgs_parameter_t param;

            x[0] = 0.0; //init at zero
            lbfgs_parameter_init(&param);

            int ret = lbfgs(N, x, &fx, evaluate, progress, (void *) this, &param);
            printf("L-BFGS optimization terminated with status code = %d (%s)\n", ret, getErrDescr(ret));

            toRet = x[0];

            lbfgs_free(x);

            return toRet;
        }

        static const char *getErrDescr(int errID)
        {
            switch(errID)
            {
                case LBFGS_ALREADY_MINIMIZED:
                    return "The initial variables already minimize the objective function.";
                case LBFGSERR_UNKNOWNERROR:	return "Unknown error";
                case LBFGSERR_LOGICERROR: return "Logic error.";
                case LBFGSERR_OUTOFMEMORY: return "Insufficient memory.";
                case LBFGSERR_CANCELED: return "The minimization process has been canceled.";
                case LBFGSERR_INVALID_N: return "Invalid number of variables specified.";
                case LBFGSERR_INVALID_N_SSE: return "Invalid number of variables (for SSE) specified.";
                case LBFGSERR_INVALID_X_SSE: return "The array x must be aligned to 16 (for SSE).";
                case LBFGSERR_INVALID_EPSILON: return "Invalid parameter lbfgs_parameter_t::epsilon specified.";
                case LBFGSERR_INVALID_TESTPERIOD: return "Invalid parameter lbfgs_parameter_t::past specified.";

                case LBFGSERR_INVALID_DELTA 	: return "Invalid parameter lbfgs_parameter_t::delta specified.";

                case LBFGSERR_INVALID_LINESEARCH 	: return "Invalid parameter lbfgs_parameter_t::linesearch specified.";

                case LBFGSERR_INVALID_MINSTEP 	: return "Invalid parameter lbfgs_parameter_t::max_step specified.";

                case LBFGSERR_INVALID_MAXSTEP 	: return "Invalid parameter lbfgs_parameter_t::max_step specified.";

                case LBFGSERR_INVALID_FTOL 	: return "Invalid parameter lbfgs_parameter_t::ftol specified.";

                case LBFGSERR_INVALID_WOLFE 	: return "Invalid parameter lbfgs_parameter_t::wolfe specified.";

                case LBFGSERR_INVALID_GTOL 	: return "Invalid parameter lbfgs_parameter_t::gtol specified.";

                case LBFGSERR_INVALID_XTOL 	: return "Invalid parameter lbfgs_parameter_t::xtol specified.";

                case LBFGSERR_INVALID_MAXLINESEARCH 	: return "Invalid parameter lbfgs_parameter_t::max_linesearch specified.";

                case LBFGSERR_INVALID_ORTHANTWISE 	: return "Invalid parameter lbfgs_parameter_t::orthantwise_c specified.";

                case LBFGSERR_INVALID_ORTHANTWISE_START 	: return "Invalid parameter lbfgs_parameter_t::orthantwise_start specified.";

                case LBFGSERR_INVALID_ORTHANTWISE_END 	: return "Invalid parameter lbfgs_parameter_t::orthantwise_end specified.";

                case LBFGSERR_OUTOFINTERVAL 	: return "The line-search step went out of the interval of uncertainty.";

                case LBFGSERR_INCORRECT_TMINMAX 	: return "A logic error occurred; alternatively, the interval of uncertainty became too small.";

                case LBFGSERR_ROUNDING_ERROR 	: return "A rounding error occurred; alternatively, no line-search step satisfies the sufficient decrease and curvature conditions.";

                case LBFGSERR_MINIMUMSTEP 	: return "The line-search step became smaller than lbfgs_parameter_t::min_step.";

                case LBFGSERR_MAXIMUMSTEP 	: return "The line-search step became larger than lbfgs_parameter_t::max_step.";

                case LBFGSERR_MAXIMUMLINESEARCH 	: return "The line-search routine reaches the maximum number of evaluations.";

                case LBFGSERR_MAXIMUMITERATION 	: return "The algorithm routine reaches the maximum number of iterations.";

                case LBFGSERR_WIDTHTOOSMALL 	: return "Relative width of the interval of uncertainty is at most lbfgs_parameter_t::xtol.";

                case LBFGSERR_INVALIDPARAMETERS 	: return "A logic error (negative line-search step) occurred.";

                case LBFGSERR_INCREASEGRADIENT 	: return "The current search direction increases the objective function value.";
            }

            return "UNKNOWN";
        }
    };
}

#endif // _SQB_LINESEARCH_H
