/**
 * @file evaluate.h
 */

#ifndef _EVALUATE_H_
#define _EVALUATE_H_

int get_num_classes(double* target, struct problem* prob)
{
    int max = 0;
    for(int i = 0; i < prob->l; ++i)
    {
        if(target[i] > max)
            max = (int) target[i];
    }
    return max;
}

void print_matrix(int** matrix, int num_classes)
{
    for(int i = 0; i < num_classes; ++i)
    {
        for(int j = 0; j < num_classes; ++j)
        {
            if(i == j)
                printf("[%5d]", matrix[i][j]);
            else
                printf(" %5d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void get_stats(int** matrix, int num_classes, int class_num, int* t_corr, double* precision, double* recall, double* f1)
{
    *t_corr += matrix[class_num][class_num];

    *precision = 0;
    for(int i = 0; i < num_classes; ++i)
        *precision += matrix[i][class_num];
    if(*precision != 0)
    *precision = matrix[class_num][class_num] / *precision;

    *recall = 0;
    for(int i = 0; i < num_classes; ++i)
        *recall += matrix[class_num][i];
    if(*recall != 0)
        *recall = matrix[class_num][class_num] / *recall;

    if(*precision + *recall == 0)
        *f1 = 0;
    else
        *f1 = (2 * *precision * *recall) / (*precision + *recall);
}

void print_stats(int** matrix, int num_classes)
{
    double t_prec = 0, t_rec = 0, t_f1 = 0;
    int t_corr = 0, total = 0;
    for(int i = 0; i < num_classes; ++i)
    {
        double precision, recall, f1;
        get_stats(matrix, num_classes, i, &t_corr, &precision, &recall, &f1);
        t_prec += precision;
        t_rec += recall;
        t_f1 += f1;
        printf(" %d: f1:%.4f p:%.4f r:%.4f\n", i, f1, precision, recall);
        for(int j = 0; j < num_classes; ++j)
            total += matrix[i][j];
    }
    printf("\n f1:%.4f acc:%.4f p:%.4f r:%.4f\n", t_f1 / num_classes,
        (double) t_corr / total, t_prec / num_classes, t_rec / num_classes);
}

void evaluate(double* target, struct problem* prob)
{
    int num_classes = get_num_classes(target, prob);
    int** matrix = (int**) calloc(num_classes, sizeof(int*));
    for(int i = 0; i < num_classes; ++i)
        matrix[i] = (int*) calloc(num_classes, sizeof(int));

    // gather evaluation data
    // why these are arrays of doubles, I have no idea.
	for(int i = 0; i < prob->l; ++i)
        ++matrix[(int) target[i] - 1][(int) prob->y[i] - 1];

    print_matrix(matrix, num_classes);
    printf("\n");
    print_stats(matrix, num_classes);

    for(int i = 0; i < num_classes; ++i)
        free(matrix[i]);
    free(matrix);
}

#endif
