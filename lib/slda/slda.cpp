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

#include <time.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <string.h>
#include "slda.h"
#include "utils.h"
#include "assert.h"
#include "opt.h"

using std::string;

slda::slda()
{
    alpha = 1.0;
    num_topics = 0;
    num_classes = 0;
    size_vocab = 0;

    log_prob_w = NULL;
    eta = NULL;
}

slda::~slda()
{
    free_model();
}

/*
 * init the model
 */

void slda::init(double alpha_, int num_topics_,
                const corpus * c)
{
    alpha = alpha_;
    num_topics = num_topics_;
    size_vocab = c->size_vocab;
    num_classes = c->num_classes;

    log_prob_w = new double * [num_topics];
    for (int k = 0; k < num_topics; k++)
    {
        log_prob_w[k] = new double [size_vocab];
        memset(log_prob_w[k], 0, sizeof(double)*size_vocab);
    }
    //no need to train slda if we only have on class
    if (num_classes > 1)
    {
        eta = new double * [num_classes-1];
        for (int i = 0; i < num_classes-1; i ++)
        {
            eta[i] = new double [num_topics];
            memset(eta[i], 0, sizeof(double)*num_topics);
        }
    }
}

/*
 * free the model
 */

void slda::free_model()
{
    if (log_prob_w != NULL)
    {
        for (int k = 0; k < num_topics; k++)
        {
            delete [] log_prob_w[k];
        }
        delete [] log_prob_w;
        log_prob_w = NULL;
    }
    if (eta != NULL)
    {
        for (int i = 0; i < num_classes-1; i ++)
        {
            delete [] eta[i];
        }
        delete [] eta;
        eta = NULL;
    }
}

/*
 * save the model in the binary format
 */

void slda::save_model(const string & filename)
{
    FILE * file = NULL;
    file = fopen(filename.c_str(), "wb");
    fwrite(&alpha, sizeof (double), 1, file);
    fwrite(&num_topics, sizeof (int), 1, file);
    fwrite(&size_vocab, sizeof (int), 1, file);
    fwrite(&num_classes, sizeof (int), 1, file);

    for (int k = 0; k < num_topics; k++)
    {
        fwrite(log_prob_w[k], sizeof(double), size_vocab, file);
    }
    if (num_classes > 1)
    {
        for (int i = 0; i < num_classes-1; i ++)
        {
            fwrite(eta[i], sizeof(double), num_topics, file);
        }
    }

    fflush(file);
    fclose(file);
}

/*
 * load the model in the binary format
 */

void slda::load_model(const string & filename)
{
    FILE * file = NULL;
    file = fopen(filename.c_str(), "rb");
    fread(&alpha, sizeof (double), 1, file);
    fread(&num_topics, sizeof (int), 1, file);
    fread(&size_vocab, sizeof (int), 1, file);
    fread(&num_classes, sizeof (int), 1, file);

    log_prob_w = new double * [num_topics];
    for (int k = 0; k < num_topics; k++)
    {
        log_prob_w[k] = new double [size_vocab];
        fread(log_prob_w[k], sizeof(double), size_vocab, file);
    }
    if (num_classes > 1)
    {
        eta = new double * [num_classes-1];
        for (int i = 0; i < num_classes-1; i ++)
        {
            eta[i] = new double [num_topics];
            fread(eta[i], sizeof(double), num_topics, file);
        }
    }

    fflush(file);
    fclose(file);
}

/*
 * save the model in the text format
 */

void slda::save_model_text(const string & filename)
{
    FILE * file = NULL;
    file = fopen(filename.c_str(), "w");
    fprintf(file, "alpha: %lf\n", alpha);
    fprintf(file, "number of topics: %d\n", num_topics);
    fprintf(file, "size of vocab: %d\n", size_vocab);
    fprintf(file, "number of classes: %d\n", num_classes);

    fprintf(file, "betas: \n"); // in log space
    for (int k = 0; k < num_topics; k++)
    {
        for (int j = 0; j < size_vocab; j ++)
        {
            fprintf(file, "%lf ", log_prob_w[k][j]);
        }
        fprintf(file, "\n");
    }
    if (num_classes > 1)
    {
        fprintf(file, "etas: \n");
        for (int i = 0; i < num_classes-1; i ++)
        {
            for (int j = 0; j < num_topics; j ++)
            {
                fprintf(file, "%lf ", eta[i][j]);
            }
            fprintf(file, "\n");
        }
    }

    fflush(file);
    fclose(file);
}

/*
 * create the data structure for sufficient statistic 
 */

suffstats * slda::new_suffstats(int num_docs)
{
    suffstats * ss = new suffstats;
    ss->num_docs = num_docs;
    ss->word_total_ss = new double [num_topics];
    memset(ss->word_total_ss, 0, sizeof(double)*num_topics);
    ss->word_ss = new double * [num_topics];
    for (int k = 0; k < num_topics; k ++)
    {
        ss->word_ss[k] = new double [size_vocab];
        memset(ss->word_ss[k], 0, sizeof(double)*size_vocab);
    }

    int num_var_entries = num_topics*(num_topics+1)/2;
    ss->z_bar =  new z_stat [num_docs];
    for (int d = 0; d < num_docs; d ++)
    {
        ss->z_bar[d].z_bar_m = new double [num_topics];
        ss->z_bar[d].z_bar_var = new double [num_var_entries];
        memset(ss->z_bar[d].z_bar_m, 0, sizeof(double)*num_topics);
        memset(ss->z_bar[d].z_bar_var, 0, sizeof(double)*num_var_entries);
    }
    ss->labels = new int [num_docs];
    memset(ss->labels, 0, sizeof(int)*(num_docs));
    ss->tot_labels = new int [num_classes];
    memset(ss->tot_labels, 0, sizeof(int)*(num_classes));

    return(ss);
}


/*
 * initialize the sufficient statistics with zeros
 */

void slda::zero_initialize_ss(suffstats * ss)
{
    memset(ss->word_total_ss, 0, sizeof(double)*num_topics);
    for (int k = 0; k < num_topics; k ++)
    {
        memset(ss->word_ss[k], 0, sizeof(double)*size_vocab);
    }

    int num_var_entries = num_topics*(num_topics+1)/2;
    for (int d = 0; d < ss->num_docs; d ++)
    {
        memset(ss->z_bar[d].z_bar_m, 0, sizeof(double)*num_topics);
        memset(ss->z_bar[d].z_bar_var, 0, sizeof(double)*num_var_entries);
    }
    ss->num_docs = 0;
}


/*
 * initialize the sufficient statistics with random numbers 
 */

void slda::random_initialize_ss(suffstats * ss, corpus* c)
{
    int num_docs = ss->num_docs;
    gsl_rng * rng = gsl_rng_alloc(gsl_rng_taus);
    time_t seed;
    time(&seed);
    gsl_rng_set(rng, (long) seed);
    int k, w, d, j, idx;
    for (k = 0; k < num_topics; k++)
    {
        for (w = 0; w < size_vocab; w++)
        {
            ss->word_ss[k][w] = 1.0/size_vocab + 0.1*gsl_rng_uniform(rng);
            ss->word_total_ss[k] += ss->word_ss[k][w];
        }
    }

    for (d = 0; d < num_docs; d ++)
    {
        document * doc = c->docs[d];
        ss->labels[d] = doc->label;
        ss->tot_labels[doc->label] ++;

        double total = 0.0;
        for (k = 0; k < num_topics; k ++)
        {
            ss->z_bar[d].z_bar_m[k] = gsl_rng_uniform(rng);
            total += ss->z_bar[d].z_bar_m[k];
        }
        for (k = 0; k < num_topics; k ++)
        {
            ss->z_bar[d].z_bar_m[k] /= total;
        }
        for (k = 0; k < num_topics; k ++)
        {
            for (j = k; j < num_topics; j ++)
            {
                idx = map_idx(k, j, num_topics);
                if (j == k)
                    ss->z_bar[d].z_bar_var[idx] = ss->z_bar[d].z_bar_m[k] / (double)(doc->total);
                else
                    ss->z_bar[d].z_bar_var[idx] = 0.0;

                ss->z_bar[d].z_bar_var[idx] -=
                    ss->z_bar[d].z_bar_m[k] * ss->z_bar[d].z_bar_m[j] / (double)(doc->total);
            }
        }
    }

    gsl_rng_free(rng);
}

void slda::corpus_initialize_ss(suffstats* ss, corpus* c)
{
    int num_docs = ss->num_docs;
    gsl_rng * rng = gsl_rng_alloc(gsl_rng_taus);
    time_t seed;
    time(&seed);
    gsl_rng_set(rng, (long) seed);
    int k, n, d, j, idx, i, w;

    for (k = 0; k < num_topics; k++)
    {
        for (i = 0; i < NUM_INIT; i++)
        {
            d = (int)(floor(gsl_rng_uniform(rng) * num_docs));
            printf("initialized with document %d\n", d);
            document * doc = c->docs[d];
            for (n = 0; n < doc->length; n++)
                ss->word_ss[k][doc->words[n]] += doc->counts[n];
        }
        for (w = 0; w < size_vocab; w++)
        {
            ss->word_ss[k][w] = 2*ss->word_ss[k][w] + 5 + gsl_rng_uniform(rng);
            ss->word_total_ss[k] = ss->word_total_ss[k] + ss->word_ss[k][w];
        }
    }

    for (d = 0; d < num_docs; d ++)
    {
        document * doc = c->docs[d];
        ss->labels[d] = doc->label;
        ss->tot_labels[doc->label] ++;

        double total = 0.0;
        for (k = 0; k < num_topics; k ++)
        {
            ss->z_bar[d].z_bar_m[k] = gsl_rng_uniform(rng);
            total += ss->z_bar[d].z_bar_m[k];
        }
        for (k = 0; k < num_topics; k ++)
        {
            ss->z_bar[d].z_bar_m[k] /= total;
        }
        for (k = 0; k < num_topics; k ++)
        {
            for (j = k; j < num_topics; j ++)
            {
                idx = map_idx(k, j, num_topics);
                if (j == k)
                    ss->z_bar[d].z_bar_var[idx] = ss->z_bar[d].z_bar_m[k] / (double)(doc->total);
                else
                    ss->z_bar[d].z_bar_var[idx] = 0.0;

                ss->z_bar[d].z_bar_var[idx] -=
                    ss->z_bar[d].z_bar_m[k] * ss->z_bar[d].z_bar_m[j] / (double)(doc->total);
            }
        }
    }
    gsl_rng_free(rng);
}

void slda::load_model_initialize_ss(suffstats* ss, corpus * c)
{
    int num_docs = ss->num_docs;                                                                         
    for (int d = 0; d < num_docs; d ++)       
    {                                                                                                    
       document * doc = c->docs[d];
       ss->labels[d] = doc->label;
       ss->tot_labels[doc->label] ++;
    }     
}

void slda::free_suffstats(suffstats * ss)
{
    delete [] ss->word_total_ss;

    for (int k = 0; k < num_topics; k ++)
    {
        delete [] ss->word_ss[k];
    }
    delete [] ss->word_ss;

    for (int d = 0; d < ss->num_docs; d ++)
    {
        delete [] ss->z_bar[d].z_bar_m;
        delete [] ss->z_bar[d].z_bar_var;
    }
    delete [] ss->z_bar;
    delete [] ss->labels;
    delete [] ss->tot_labels;

    delete ss;
}

void slda::v_em(corpus * c, const settings * setting,
                const string & start, const string & directory)
{
    char filename[100];
    int max_length = c->max_corpus_length();
    double **var_gamma, **phi;
    double likelihood, likelihood_old = 0, converged = 1;
    int n, i;

    // allocate variational parameters
    var_gamma = new double * [c->docs.size()];
    for (size_t d = 0; d < c->docs.size(); d++)
        var_gamma[d] = new double [num_topics];

    phi = new double * [max_length];
    for (n = 0; n < max_length; n++)
        phi[n] = new double [num_topics];

    printf("initializing ...\n");
    suffstats * ss = new_suffstats(c->docs.size());
    if (start == "seeded")
    {
        corpus_initialize_ss(ss, c);
        mle(ss, 0, setting);
    }
    else if (start == "random")
    {
        random_initialize_ss(ss, c);
        mle(ss, 0, setting);
    }
    else
    {
        load_model(start);
        load_model_initialize_ss(ss, c);
    }

    FILE * likelihood_file = NULL;
    sprintf(filename, "%s/likelihood.dat", directory.c_str());
    likelihood_file = fopen(filename, "w");

    int ETA_UPDATE = 0;

    i = 0;
    while (((converged < 0) || (converged > setting->em_converged) || (i <= LDA_INIT_MAX+2)) && (i <= setting->em_max_iter))
    {
        printf("**** em iteration %d ****\n", ++i);
        likelihood = 0;
        zero_initialize_ss(ss);
        if (i > LDA_INIT_MAX) ETA_UPDATE = 1;
        // e-step
        printf("**** e-step ****\n");
        for (size_t d = 0; d < c->docs.size(); d++)
        {
            if ((d % 100) == 0) printf("document %lu\n", d);
            likelihood += doc_e_step(c->docs[d], var_gamma[d], phi, ss, ETA_UPDATE, setting);
        }

        printf("likelihood: %10.10f\n", likelihood);
        // m-step
        printf("**** m-step ****\n");
        mle(ss, ETA_UPDATE, setting);

        // check for convergence
        converged = fabs((likelihood_old - likelihood) / (likelihood_old));
        //if (converged < 0) VAR_MAX_ITER = VAR_MAX_ITER * 2;
        likelihood_old = likelihood;

        // output model and likelihood
        fprintf(likelihood_file, "%10.10f\t%5.5e\n", likelihood, converged);
        fflush(likelihood_file);
        if ((i % LAG) == 0)
        {
            sprintf(filename, "%s/%03d.model", directory.c_str(), i);
            save_model(filename);
            sprintf(filename, "%s/%03d.model.text", directory.c_str(), i);
            save_model_text(filename);
            sprintf(filename, "%s/%03d.gamma", directory.c_str(), i);
            save_gamma(filename, var_gamma, c->docs.size());
        }
    }

    // output the final model
    sprintf(filename, "%s/final.model", directory.c_str());
    save_model(filename);
    sprintf(filename, "%s/final.model.text", directory.c_str());
    save_model_text(filename);
    sprintf(filename, "%s/final.gamma", directory.c_str());
    save_gamma(filename, var_gamma, c->docs.size());


    fclose(likelihood_file);
    FILE * w_asgn_file = NULL;
    sprintf(filename, "%s/word-assignments.dat", directory.c_str());
    w_asgn_file = fopen(filename, "w");
    for (size_t d = 0; d < c->docs.size(); d ++)
    {
        //final inference
        if ((d % 100) == 0) printf("final e step document %lu\n", d);
        likelihood += slda_inference(c->docs[d], var_gamma[d], phi, setting);
        write_word_assignment(w_asgn_file, c->docs[d], phi);

    }
    fclose(w_asgn_file);

    free_suffstats(ss);
    for (size_t d = 0; d < c->docs.size(); d++)
        delete [] var_gamma[d];
    delete [] var_gamma;

    for (n = 0; n < max_length; n++)
        delete [] phi[n];
    delete [] phi;
}

void slda::mle(suffstats * ss, int eta_update, const settings * setting)
{
    int k, w;

    for (k = 0; k < num_topics; k++)
    {
        for (w = 0; w < size_vocab; w++)
        {
            if (ss->word_ss[k][w] > 0)
                log_prob_w[k][w] = log(ss->word_ss[k][w]) - log(ss->word_total_ss[k]);
            else
                log_prob_w[k][w] = -100.0;
        }
    }
    if (eta_update == 0) return;

    //the label part goes here
    printf("maximizing ...\n");
	double f = 0.0;
	int status;
	int opt_iter;
	int opt_size = (num_classes-1) * num_topics;
	int l;

	opt_parameter param;
	param.ss = ss;
	param.model = this;
	param.PENALTY = setting->penalty;

	const gsl_multimin_fdfminimizer_type * T;
	gsl_multimin_fdfminimizer * s;
	gsl_vector * x;
	gsl_multimin_function_fdf opt_fun;
	opt_fun.f = &softmax_f;
	opt_fun.df = &softmax_df;
	opt_fun.fdf = &softmax_fdf;
	opt_fun.n = opt_size;
	opt_fun.params = (void*)(&param);
	x = gsl_vector_alloc(opt_size);


	for (l = 0; l < num_classes-1; l ++)
	{
		for (k = 0; k < num_topics; k ++)
		{
			gsl_vector_set(x, l*num_topics + k, eta[l][k]);
		}
	}

	T = gsl_multimin_fdfminimizer_vector_bfgs;
	s = gsl_multimin_fdfminimizer_alloc(T, opt_size);
	gsl_multimin_fdfminimizer_set(s, &opt_fun, x, 0.02, 1e-4);

	opt_iter = 0;
	do
	{
		opt_iter ++;
		status = gsl_multimin_fdfminimizer_iterate(s);
		if (status)
			break;
		status = gsl_multimin_test_gradient(s->gradient, 1e-3);
		if (status == GSL_SUCCESS)
			break;
		f = -s->f;
		if ((opt_iter-1) % 10 == 0)
			printf("step: %02d -> f: %f\n", opt_iter-1, f);
	} while (status == GSL_CONTINUE && opt_iter < MSTEP_MAX_ITER);

	for (l = 0; l < num_classes-1; l ++)
	{
		for (k = 0; k < num_topics; k ++)
		{
			eta[l][k] = gsl_vector_get(s->x, l*num_topics + k);
		}
	}

	gsl_multimin_fdfminimizer_free (s);
	gsl_vector_free (x);

	printf("final f: %f\n", f);
}

double slda::doc_e_step(document* doc, double* gamma, double** phi,
                        suffstats * ss, int eta_update, const settings * setting)
{
    double likelihood = 0.0;
    if (eta_update == 1)
        likelihood = slda_inference(doc, gamma, phi, setting);
    else
        likelihood = lda_inference(doc, gamma, phi, setting);

    int d = ss->num_docs;

    int n, k, i, idx;

    // update sufficient statistics

    for (n = 0; n < doc->length; n++)
    {
        for (k = 0; k < num_topics; k++)
        {
            ss->word_ss[k][doc->words[n]] += doc->counts[n]*phi[n][k];
            ss->word_total_ss[k] += doc->counts[n]*phi[n][k];

            //statistics for each document of the supervised part
            ss->z_bar[d].z_bar_m[k] += doc->counts[n] * phi[n][k]; //mean
            for (i = k; i < num_topics; i ++) //variance
            {
                idx = map_idx(k, i, num_topics);
                if (i == k)
                    ss->z_bar[d].z_bar_var[idx] +=
                        doc->counts[n] * doc->counts[n] * phi[n][k]; 

                ss->z_bar[d].z_bar_var[idx] -=
                    doc->counts[n] * doc->counts[n] * phi[n][k] * phi[n][i];
            }
        }
    }
    for (k = 0; k < num_topics; k++)
    {
        ss->z_bar[d].z_bar_m[k] /= (double)(doc->total);
    }
    for (i = 0; i < num_topics*(num_topics+1)/2; i ++)
    {
        ss->z_bar[d].z_bar_var[i] /= (double)(doc->total * doc->total);
    }

    ss->num_docs = ss->num_docs + 1; //because we need it for store statistics for each docs

    return (likelihood);
}

double slda::lda_inference(document* doc, double* var_gamma, double** phi, const settings * setting)
{
    int k, n, var_iter;
    double converged = 1, phisum = 0, likelihood = 0, likelihood_old = 0;

    double *oldphi = new double [num_topics];
    double *digamma_gam = new double [num_topics];

    // compute posterior dirichlet
    for (k = 0; k < num_topics; k++)
    {
        var_gamma[k] = alpha + (doc->total/((double) num_topics));
        digamma_gam[k] = digamma(var_gamma[k]);
        for (n = 0; n < doc->length; n++)
            phi[n][k] = 1.0/num_topics;
    }
    var_iter = 0;

    while (converged > setting->var_converged && (var_iter < setting->var_max_iter || setting->var_max_iter == -1))
    {
        var_iter++;
        for (n = 0; n < doc->length; n++)
        {
            phisum = 0;
            for (k = 0; k < num_topics; k++)
            {
                oldphi[k] = phi[n][k];
                phi[n][k] = digamma_gam[k] + log_prob_w[k][doc->words[n]];

                if (k > 0)
                    phisum = log_sum(phisum, phi[n][k]);
                else
                    phisum = phi[n][k]; // note, phi is in log space
            }

            for (k = 0; k < num_topics; k++)
            {
                phi[n][k] = exp(phi[n][k] - phisum);
                var_gamma[k] = var_gamma[k] + doc->counts[n]*(phi[n][k] - oldphi[k]);
                digamma_gam[k] = digamma(var_gamma[k]);
            }
        }

        likelihood = lda_compute_likelihood(doc, phi, var_gamma);
        assert(!isnan(likelihood));
        converged = (likelihood_old - likelihood) / likelihood_old;
        likelihood_old = likelihood;
    }

    delete [] oldphi;
    delete [] digamma_gam;

    return likelihood;
}

double slda::lda_compute_likelihood(document* doc, double** phi, double* var_gamma)
{
    double likelihood = 0, digsum = 0, var_gamma_sum = 0;
    double *dig = new double [num_topics];
    int k, n;
    double alpha_sum = num_topics * alpha;
    for (k = 0; k < num_topics; k++)
    {
        dig[k] = digamma(var_gamma[k]);
        var_gamma_sum += var_gamma[k];
    }
    digsum = digamma(var_gamma_sum);

    likelihood = lgamma(alpha_sum) - lgamma(var_gamma_sum);

    for (k = 0; k < num_topics; k++)
    {
        likelihood += - lgamma(alpha) + (alpha - 1)*(dig[k] - digsum) +
                      lgamma(var_gamma[k]) - (var_gamma[k] - 1)*(dig[k] - digsum);

        for (n = 0; n < doc->length; n++)
        {
            if (phi[n][k] > 0)
            {
                likelihood += doc->counts[n]*(phi[n][k]*((dig[k] - digsum) -
                                              log(phi[n][k]) + log_prob_w[k][doc->words[n]]));
            }
        }
    }

    delete [] dig;
    return likelihood;
}

double slda::slda_compute_likelihood(document* doc, double** phi, double* var_gamma)
{
    double likelihood = 0, digsum = 0, var_gamma_sum = 0, t = 0.0, t1 = 0.0, t2 = 0.0;
    double * dig = new double [num_topics];
    int k, n, l;
    double alpha_sum = num_topics * alpha;
    for (k = 0; k < num_topics; k++)
    {
        dig[k] = digamma(var_gamma[k]);
        var_gamma_sum += var_gamma[k];
    }
    digsum = digamma(var_gamma_sum);

    likelihood = lgamma(alpha_sum) - lgamma(var_gamma_sum);
    t = 0.0;
    for (k = 0; k < num_topics; k++)
    {
        likelihood += -lgamma(alpha) + (alpha - 1)*(dig[k] - digsum) + lgamma(var_gamma[k]) - (var_gamma[k] - 1)*(dig[k] - digsum);

        for (n = 0; n < doc->length; n++)
        {
            if (phi[n][k] > 0)
            {
                likelihood += doc->counts[n]*(phi[n][k]*((dig[k] - digsum) - log(phi[n][k]) + log_prob_w[k][doc->words[n]]));
                if (doc->label < num_classes-1)
                    t += eta[doc->label][k] * doc->counts[n] * phi[n][k];
            }
        }
    }
    likelihood += t / (double)(doc->total); 	//eta_k*\bar{\phi}

    t = 1.0; //the class model->num_classes-1
    for (l = 0; l < num_classes-1; l ++)
    {
        t1 = 1.0; 
        for (n = 0; n < doc->length; n ++)
        {
            t2 = 0.0;
            for (k = 0; k < num_topics; k ++)
            {
                t2 += phi[n][k] * exp(eta[l][k] * doc->counts[n]/(double)(doc->total));
            }
            t1 *= t2; 
        }
        t += t1; 
    }
    likelihood -= log(t); 
    delete [] dig;
    //printf("%lf\n", likelihood);
    return likelihood;
}

double slda::slda_inference(document* doc, double* var_gamma, double** phi, const settings * setting)
{
    int k, n, var_iter, l;
    int FP_MAX_ITER = 10;
    int fp_iter = 0;
    double converged = 1, phisum = 0, likelihood = 0, likelihood_old = 0;
    double * oldphi = new double [num_topics];
    double * digamma_gam = new double [num_topics];
    double * sf_params = new double [num_topics];
    double * sf_aux = new double [num_classes-1];
    double sf_val = 0.0;

    // compute posterior dirichlet
    for (k = 0; k < num_topics; k++)
    {
        var_gamma[k] = alpha + (doc->total/((double) num_topics));
        digamma_gam[k] = digamma(var_gamma[k]);
        for (n = 0; n < doc->length; n++)
            phi[n][k] = 1.0/(double)(num_topics);
    }

    double t = 0.0;
    for (l = 0; l < num_classes-1; l ++)
    {
        sf_aux[l] = 1.0; // the quantity for equation 6 of each class
        for (n = 0; n < doc->length; n ++)
        {
            t = 0.0;
            for (k = 0; k < num_topics; k ++)
            {
                t += phi[n][k] * exp(eta[l][k] * doc->counts[n]/(double)(doc->total));
            }
            sf_aux[l] *= t;
        }
    }

    var_iter = 0;

    while ((converged > setting->var_converged) && ((var_iter < setting->var_max_iter) || (setting->var_max_iter == -1)))
    {
        var_iter++;
        for (n = 0; n < doc->length; n++)
        {
            //compute sf_params
            memset(sf_params, 0, sizeof(double)*num_topics); //in log space
            for (l = 0; l < num_classes-1; l ++)
            {
                t = 0.0;
                for (k = 0; k < num_topics; k ++)
                {
                    t += phi[n][k] * exp(eta[l][k] * doc->counts[n]/(double)(doc->total));
                }
                sf_aux[l] /= t; //take out word n

                for (k = 0; k < num_topics; k ++)
                {
                    //h in the paper
                    sf_params[k] += sf_aux[l]*exp(eta[l][k] * doc->counts[n]/(double)(doc->total));
                }
            }
            //
            for (k = 0; k < num_topics; k++)
            {
                oldphi[k] = phi[n][k];
            }
            for (fp_iter = 0; fp_iter < FP_MAX_ITER; fp_iter ++) //fixed point update
            {
                sf_val = 1.0; // the base class, in log space
                for (k = 0; k < num_topics; k++)
                {
                    sf_val += sf_params[k]*phi[n][k];
                }

                phisum = 0;
                for (k = 0; k < num_topics; k++)
                {
                    phi[n][k] = digamma_gam[k] + log_prob_w[k][doc->words[n]];

                    //added softmax parts
                    if (doc->label < num_classes-1)
                        phi[n][k] += eta[doc->label][k]/(double)(doc->total);
                    phi[n][k] -= sf_params[k]/(sf_val*(double)(doc->counts[n]));

                    if (k > 0)
                        phisum = log_sum(phisum, phi[n][k]);
                    else
                        phisum = phi[n][k]; // note, phi is in log space
                }
                for (k = 0; k < num_topics; k++)
                {
                    phi[n][k] = exp(phi[n][k] - phisum); //normalize
                }
            }
            //back to sf_aux value
            for (l = 0; l < num_classes-1; l ++)
            {
                t = 0.0;
                for (k = 0; k < num_topics; k ++)
                {
                    t += phi[n][k] * exp(eta[l][k] * doc->counts[n]/(double)(doc->total));
                }
                sf_aux[l] *= t;
            }
            for (k = 0; k < num_topics; k++)
            {
                var_gamma[k] = var_gamma[k] + doc->counts[n]*(phi[n][k] - oldphi[k]);
                digamma_gam[k] = digamma(var_gamma[k]);
            }
        }

        likelihood = slda_compute_likelihood(doc, phi, var_gamma);
        assert(!isnan(likelihood));
        converged = fabs((likelihood_old - likelihood) / likelihood_old);
        likelihood_old = likelihood;
    }

    delete [] oldphi;
    delete [] digamma_gam;
    delete [] sf_params;
    delete [] sf_aux;
    return likelihood;
}

void slda::infer_only(corpus * c, const settings * setting, const string & directory)
{
    int i, k, n;
    double **var_gamma, likelihood, **phi;
    double* phi_m;
    char filename[100];
    double base_score, score;
    int label;
    int num_correct = 0;
    int max_length = c->max_corpus_length();


    var_gamma = new double * [c->docs.size()];
    for (size_t i = 0; i < c->docs.size(); i++)
        var_gamma[i] = new double [num_topics];


    phi = new double * [max_length];
    for (n = 0; n < max_length; n++)
        phi[n] = new double [num_topics];

    phi_m = new double [num_topics];

    FILE * likelihood_file = NULL;
    sprintf(filename, "%s/inf-likelihood.dat", directory.c_str());
    likelihood_file = fopen(filename, "w");
    FILE * inf_label_file = NULL;
    sprintf(filename, "%s/inf-labels.dat", directory.c_str());
    inf_label_file = fopen(filename, "w");

    for (size_t d = 0; d < c->docs.size(); d++)
    {
        if ((d % 100) == 0)
            printf("document %lu\n", d);

        document * doc = c->docs[d];
        likelihood = lda_inference(doc, var_gamma[d], phi, setting);

        memset(phi_m, 0, sizeof(double)*num_topics); //zero_initialize
        for (n = 0; n < doc->length; n++)
        {
            for (k = 0; k < num_topics; k ++)
            {
                phi_m[k] += doc->counts[n] * phi[n][k];
            }
        }
        for (k = 0; k < num_topics; k ++)
        {
            phi_m[k] /= (double)(doc->total);
        }

        //do classification
        label = num_classes-1;
        base_score = 0.0;
        for (i = 0; i < num_classes-1; i ++)
        {
            score = 0.0;
            for (k = 0; k < num_topics; k ++)
            {
                score += eta[i][k] * phi_m[k];
            }
            if (score > base_score)
            {
                base_score = score;
                label = i;
            }
        }
        if (label == doc->label)
            num_correct ++;

        fprintf(likelihood_file, "%5.5f\n", likelihood);
        fprintf(inf_label_file, "%d\n", label);
    }

    printf("average accuracy: %.3f\n", (double)num_correct / (double) c->docs.size());

    sprintf(filename, "%s/inf-gamma.dat", directory.c_str());
    save_gamma(filename, var_gamma, c->docs.size());

    for (size_t d = 0; d < c->docs.size(); d++)
        delete [] var_gamma[d];
    delete [] var_gamma;

    for (n = 0; n < max_length; n++)
        delete [] phi[n];
    delete [] phi;

    delete [] phi_m;
}

void slda::save_gamma(char* filename, double** gamma, int num_docs)
{
    int d, k;

    FILE* fileptr = fopen(filename, "w");
    for (d = 0; d < num_docs; d++)
    {
        fprintf(fileptr, "%5.10f", gamma[d][0]);
        for (k = 1; k < num_topics; k++)
            fprintf(fileptr, " %5.10f", gamma[d][k]);
        fprintf(fileptr, "\n");
    }
    fclose(fileptr);
}

void slda::write_word_assignment(FILE* f, document* doc, double** phi)
{
    int n;

    fprintf(f, "%03d", doc->length);
    for (n = 0; n < doc->length; n++)
    {
        fprintf(f, " %04d:%02d", doc->words[n], argmax(phi[n], num_topics));
    }
    fprintf(f, "\n");
    fflush(f);
}
