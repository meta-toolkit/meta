//
// Created by Mihika Dave on 12/18/17.
//

#include "../learntorank/letor.h"

using namespace meta;
using namespace learn;
using namespace learntorank;

/**
     *
     * @param data_dir
     * @param num_features
     * @param hasModel
     * @param model_file
     */
void train_spd(const string &data_dir, int num_features, int hasModel, const string &model_file) {
    sgd_model *model = nullptr;
    let letor_model;
    //start timer
    auto start = chrono::steady_clock::now();
    int continue_training;
    model = new sgd_model(num_features);
    if (hasModel) {
        ifstream in{model_file};
        model = new sgd_model(in);
        cout << "Do you want to continue training the loaded sgd model? 1(yes)/0(no)" << endl;
        cin >> continue_training;
    }
    if (!hasModel || continue_training) {
        cout << "start training sgd!" << endl;

        letor_model.train(data_dir, num_features, model);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double> training_time = end - start;
    cout << "Training time in seconds: " << training_time.count() << endl;

    letor_model.validate(data_dir, num_features, let::SPD, nullptr, model);

    letor_model.test(data_dir, num_features, let::SPD, nullptr, model);

    ofstream out{"letor_sgd_train.model"};
    model->save(out);

    delete model;
}

/**
 *
 * @param data_dir
 * @param num_features
 * @param hasModel
 * @param model_file
 */
void train_libsvm(const string &data_dir, int num_features, int hasModel, const string &model_file) {
    auto start = chrono::steady_clock::now();
    svm_wrapper *wrapper = nullptr;
    let letor_model;
    if (hasModel) {
        ifstream in{model_file};
        wrapper = new svm_wrapper(in);
    } else {
        cout << "Please specify full path to libsvm modules" << endl;
        string svm_path;
        cin >> svm_path;
        svm_path += "/";
        cout << "Starting to train svm!" << endl;
        wrapper = letor_model.train_svm(data_dir, num_features, svm_path);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double> training_time = end - start;
    cout << "Training time in seconds: " << training_time.count() << endl;

    letor_model.validate(data_dir, num_features, let::LIBSVM, wrapper, nullptr);

    letor_model.test(data_dir, num_features, let::LIBSVM, wrapper, nullptr);

    ofstream out{"letor_svm_train.model"};
    wrapper->save(out);

    delete wrapper;
}

int main(int argc, char *argv[]) {
    std::cerr << "Hello! This is Learning To Rank LETOR!" << std::endl;
    if (argc != 3) {
        std::cerr << "Please specify full path for training directory and the number of features" << std::endl;
        std::cerr << "Usage: ./letor [-data_dir] [-num_features]" << std::endl;
    }

    string data_dir;
    data_dir = argv[1];
    int num_features;
    stringstream ss(argv[2]);
    ss >> num_features;

    int hasModel;
    string model_file;
    cout << "Do you want to load trained model from file? 1(yes)/0(no)" << endl;
    cin >> hasModel;
    if (hasModel) {
        cout << "Please specify full path to your model file" << endl;
        cin >> model_file;
        cout << "Full path to your model is: " << model_file << endl;
    }

    int selected_method;
    cout << "Please select classification method to use: 0(libsvm), 1(spd)" << endl;
    cin >> selected_method;
    switch (selected_method) {
        case 0:
            cout << "libsvm will be used for training and testing" << endl;
            break;
        case 1:
            cout << "spd will be used for training and testing" << endl;
            break;
        default:
            break;
    }

    if (selected_method == 0) {
        train_libsvm(data_dir, num_features, hasModel, model_file);
    } else {
        train_spd(data_dir, num_features, hasModel, model_file);
    }
    std::cerr << "Exiting LETOR!" << std::endl;
    return 0;
}

