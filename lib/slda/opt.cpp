// (C) Copyright 2009, Chong Wang, David Blei and Li Fei-Fei

// written by Chong Wang, chongw@cs.princeton.edu

// This file is part of slda.

// slda is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your
// option) any later version.

// slda is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
#include "opt.h"
#include "slda.h"
#include "utils.h"
/*
 * Here the implementation is slightly different from the equations
 * in the paper, we instead use a second-order taylor expansion to approximate
 * the second line in eqaution (6).
 */

double softmax_f(const gsl_vector * x, void * opt_param)
{
    opt_parameter * gsl_param = (opt_parameter *)opt_param;
    double PENALTY = gsl_param->PENALTY;
    slda * model = gsl_param->model;
    suffstats * ss = gsl_param->ss;

    double f, t, a1 = 0.0, a2 = 0.0;

    int k, d, j, l, idx;

    double f_regularization = 0.0;


    for (l = 0; l < model->num_classes-1; l ++)
    {
        for (k = 0; k < model->num_topics; k ++)
        {
            model->eta[l][k] = gsl_vector_get(x, l*model->num_topics + k);
            f_regularization -= pow(model->eta[l][k], 2) * PENALTY/2.0;
        }
    }
    f = 0.0; //log likelihood
    for (d = 0; d < ss->num_docs; d ++)
    {
        for (k = 0; k < model->num_topics; k ++)
        {
            if (ss->labels[d] < model->num_classes-1)
            {
                f += model->eta[ss->labels[d]][k] * ss->z_bar[d].z_bar_m[k];
            }
        }

        t = 0.0; // in log space,  1+exp()+exp()...
        for (l = 0; l < model->num_classes-1; l ++)
        {
            a1 = 0.0; // \eta_k^T * \bar{\phi}_d
            a2 = 0.0; // 1 + 0.5 * \eta_k^T * Var(z_bar)\eta_k
            for (k = 0; k < model->num_topics; k ++)
            {
                a1 += model->eta[l][k] * ss->z_bar[d].z_bar_m[k];
                for (j = 0; j < model->num_topics; j ++)
                {
                    idx = map_idx(k, j, model->num_topics);
                    a2 += model->eta[l][k] * ss->z_bar[d].z_bar_var[idx] * model->eta[l][j];
                }
            }
            a2 = 1.0 + 0.5 * a2;
            t = log_sum(t, a1 + log(a2));
        }
        f -= t; 
    }

    return -(f + f_regularization);
}
void softmax_df(const gsl_vector * x, void * opt_param, gsl_vector * df)
{

    opt_parameter * gsl_param = (opt_parameter *)opt_param;
    double PENALTY = gsl_param->PENALTY;
    slda * model = gsl_param->model;
    suffstats * ss = gsl_param->ss;
    gsl_vector_set_zero(df);
    gsl_vector * df_tmp = gsl_vector_alloc(df->size);

    double t, a1 = 0.0, a2 = 0.0, g;
    int k, d, j, l, idx;

    double * eta_aux = new double [model->num_topics];

    for (l = 0; l < model->num_classes-1; l ++)
    {
        for (k = 0; k < model->num_topics; k ++)
        {
            idx = l*model->num_topics + k;
            model->eta[l][k] = gsl_vector_get(x, idx); 
            g = -PENALTY * model->eta[l][k];
            gsl_vector_set(df, idx, g);
        }
    }
    for (d = 0; d < ss->num_docs; d ++)
    {
        for (k = 0; k < model->num_topics; k ++)
        {
            l = ss->labels[d];
            if (l < model->num_classes-1)
            {
                idx = l*model->num_topics + k;
                g = gsl_vector_get(df, idx) + ss->z_bar[d].z_bar_m[k];
                gsl_vector_set(df, idx, g);
            }
        }

        t = 0.0; // in log space, 1+exp()+exp()+....
        gsl_vector_memcpy(df_tmp, df);
        gsl_vector_set_zero(df);
        for (l = 0; l < model->num_classes-1; l ++)
        {
            memset(eta_aux, 0, sizeof(double)*model->num_topics);
            a1 = 0.0; // \eta_k^T * \bar{\phi}_d
            a2 = 0.0; // 1 + 0.5*\eta_k^T * Var(z_bar)\eta_k
            for (k = 0; k < model->num_topics; k ++)
            {
                a1 += model->eta[l][k] * ss->z_bar[d].z_bar_m[k];
                for (j = 0; j < model->num_topics; j ++)
                {
                    idx = map_idx(k, j, model->num_topics);
                    a2 += model->eta[l][k] * ss->z_bar[d].z_bar_var[idx] * model->eta[l][j];
                    eta_aux[k] += ss->z_bar[d].z_bar_var[idx] * model->eta[l][j];
                }
            }
            a2 = 1.0 + 0.5 * a2;
            t = log_sum(t, a1 + log(a2));

            for (k = 0; k < model->num_topics; k ++)
            {
                idx = l*model->num_topics + k;
                g =  gsl_vector_get(df, idx) -
                     exp(a1) * (ss->z_bar[d].z_bar_m[k] * a2 + eta_aux[k]);
                gsl_vector_set(df, idx, g);
            }
        }
        gsl_vector_scale(df, exp(-t));
        gsl_vector_add(df, df_tmp);
    }
    gsl_vector_scale(df, -1.0);
    delete [] eta_aux;
    gsl_vector_free(df_tmp);
}
void softmax_fdf(const gsl_vector * x, void * opt_param, double * f, gsl_vector * df)
{
    opt_parameter * gsl_param = (opt_parameter *)opt_param;
    double PENALTY = gsl_param->PENALTY;
    slda * model = gsl_param->model;
    suffstats * ss = gsl_param->ss;
    gsl_vector_set_zero(df);
    gsl_vector * df_tmp = gsl_vector_alloc(df->size);

    double t, a1 = 0.0, a2 = 0.0, g;
    int k, d, j, l, idx;

    double f_regularization = 0.0;

    double* eta_aux = new double [model->num_topics];

    for (l = 0; l < model->num_classes-1; l ++)
    {
        for (k = 0; k < model->num_topics; k ++)
        {
            model->eta[l][k] = gsl_vector_get(x, l*model->num_topics + k);
            f_regularization -= pow(model->eta[l][k], 2) * PENALTY/2.0;
            idx = l*model->num_topics + k;
            g = -PENALTY * model->eta[l][k];
            gsl_vector_set(df, idx, g);
        }
    }
    *f = 0.0; //log likelihood
    for (d = 0; d < ss->num_docs; d ++)
    {
        for (k = 0; k < model->num_topics; k ++)
        {
            l = ss->labels[d];
            if (l < model->num_classes-1)
            {
                *f += model->eta[l][k] * ss->z_bar[d].z_bar_m[k];
                idx = l*model->num_topics + k;
                g = gsl_vector_get(df, idx) + ss->z_bar[d].z_bar_m[k];
                gsl_vector_set(df, idx, g);
            }
        }
        t = 0.0; // in log space,  base class 1+exp()+exp()
        gsl_vector_memcpy(df_tmp, df);
        gsl_vector_set_zero(df);
        for (l = 0; l < model->num_classes-1; l ++)
        {
            memset(eta_aux, 0, sizeof(double)*model->num_topics);
            a1 = 0.0; // \eta_k^T * \bar{\phi}_d
            a2 = 0.0; // 1 + 0.5 * \eta_k^T * Var(z_bar)\eta_k
            for (k = 0; k < model->num_topics; k ++)
            {
                a1 += model->eta[l][k] * ss->z_bar[d].z_bar_m[k];
                for (j = 0; j < model->num_topics; j ++)
                {
                    idx = map_idx(k, j, model->num_topics);
                    a2 += model->eta[l][k] * ss->z_bar[d].z_bar_var[idx] * model->eta[l][j];
                    eta_aux[k] += ss->z_bar[d].z_bar_var[idx] * model->eta[l][j];
                }
            }
            a2 = 1.0 + 0.5 * a2;
            t = log_sum(t, a1 + log(a2));

            for (k = 0; k < model->num_topics; k ++)
            {
                idx = l*model->num_topics + k;
                g =  gsl_vector_get(df, idx) -
                     exp(a1) * (ss->z_bar[d].z_bar_m[k] * a2 + eta_aux[k]);
                gsl_vector_set(df, idx, g);
            }
        }
        gsl_vector_scale(df, exp(-t));
        gsl_vector_add(df, df_tmp);
        *f -= t; 
    }
    gsl_vector_scale(df, -1.0);
    *f = -(*f + f_regularization);
    delete [] eta_aux;
    gsl_vector_free(df_tmp);
}

