/**
 * @file pairwise_letor_main.cpp
 * @author Mihika Dave, Anthony Huang, Rachneet Kaur
 * @date 12/18/17
 */

#include "meta/learn/learntorank/pairwise_letor.h"

using namespace meta;
using namespace learn;
using namespace learntorank;

/**
 * Train the pairwise ranker using spd
 * @param data_dir The path to directory containing train.txt
 * @param num_features The number of features
 * @param hasModel If the pairwise model is built from model file
 * @param model_file The path to model file
 */
void train_spd(const string &data_dir, int num_features, int hasModel,
               const string &model_file) {
    //start timer
    auto start = chrono::steady_clock::now();
    int continue_training;
    if (hasModel) {
        cout <<
             "Do you want to continue training the loaded sgd model? 1(yes)/0(no)" << endl;
        cin >> continue_training;
    }
    pairwise_letor letor_model(num_features, pairwise_letor::SPD,
                               hasModel, model_file);
    if (!hasModel || continue_training) {
        cout << "start training sgd!" << endl;

        letor_model.train(data_dir);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double> training_time = end - start;
    cout << "Training time in seconds: " << training_time.count() << endl;

    letor_model.validate(data_dir);

    letor_model.test(data_dir);

    cout << "trained sgd model has been saved to letor_sgd_train.model" << endl;

}

/**
 * Train the pairwise ranker using libsvm
 * @param data_dir The path to directory containing train.txt
 * @param num_features The number of features
 * @param hasModel If the pairwise model is built from model file
 * @param model_file The path to model file
 */
void train_libsvm(const string &data_dir, int num_features,
                  int hasModel, const string &model_file) {
    auto start = chrono::steady_clock::now();
    pairwise_letor letor_model(num_features, pairwise_letor::LIBSVM,
                               hasModel, model_file);
    if (!hasModel) {
        cout << "Please specify path to libsvm modules" << endl;
        string svm_path;
        cin >> svm_path;
        svm_path += "/";
        cout << "Starting to train svm!" << endl;
        letor_model.train_svm(data_dir, svm_path);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double> training_time = end - start;
    cout << "Training time in seconds: " << training_time.count() << endl;

    letor_model.validate(data_dir);

    letor_model.test(data_dir);

    cout << "trained svm model has been saved to letor_svm_train.model" << endl;

}

int main(int argc, char *argv[]) {
    cout << "Hello! This is Learning To Rank LETOR!" << std::endl;
    if (argc != 3) {
        std::cerr <<
                  "Please specify path for training directory and the number of features"
                  << std::endl;
        std::cerr << "Usage: ./letor_main [-data_dir] [-num_features]" << std::endl;
        return 1;
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
        cout << "Please specify path to your model file" << endl;
        cin >> model_file;
        cout << "Path to your model file is: " << model_file << endl;
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

    cout << "Exiting Learning To Rank!" << std::endl;
    return 0;
}

