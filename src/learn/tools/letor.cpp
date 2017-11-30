#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "meta/learn/loss/all.h"
#include "meta/learn/loss/hinge.h"
#include "meta/learn/loss/loss_function.h"
#include "meta/learn/loss/loss_function_factory.h"
#include "meta/learn/sgd.h"

using namespace meta;

int main(int argc, char* argv[])
{

    std::cerr << "Hello LETOR!" << std::endl;

    std::unique_ptr<learn::loss::loss_function> loss = learn::loss::make_loss_function(learn::loss::hinge::id.to_string());
    learn::sgd_model *model = new learn::sgd_model(46);
    std::cerr << "sgd model default settings: " << model->default_learning_rate << std::endl;
    std::cerr << model->default_l2_regularizer << std::endl;
    std::cerr << model->default_l1_regularizer << std::endl;

    std::cerr << "Bye LETOR!" << std::endl;

    return 0;
}
