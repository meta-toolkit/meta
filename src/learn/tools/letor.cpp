/**
 * @file letor.cpp
 * @author Mihika Dave, Anthony Huang, Rachneet Kaur
 */

#include "letor.h"

int main(int argc, char* argv[]) {
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
    cout << "Do you want to load trained model from file? 1(yes)/0(no)" <<endl;
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

/**
 *
 * @param data_dir
 * @param num_features
 * @param hasModel
 * @param model_file
 */
void train_spd(const string &data_dir, int num_features, int hasModel, const string &model_file) {
    sgd_model *model = nullptr;
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
            train(data_dir, num_features, model);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double> training_time = end - start;
    cout << "Training time in seconds: " << training_time.count() << endl;

    validate(data_dir, num_features, SPD, nullptr, model);

    test(data_dir, num_features, SPD, nullptr, model);

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
    if (hasModel) {
            ifstream in{model_file};
            wrapper = new svm_wrapper(in);
    } else {
        cout << "Please specify full path to libsvm modules" << endl;
        string svm_path;
        cin >> svm_path;
        svm_path += "/";
        cout << "Starting to train svm!" << endl;
        wrapper = train_svm(data_dir, num_features, svm_path);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double> training_time = end - start;
    cout << "Training time in seconds: " << training_time.count() << endl;

    validate(data_dir, num_features, LIBSVM, wrapper, nullptr);

    test(data_dir, num_features, LIBSVM, wrapper, nullptr);

    ofstream out{"letor_svm_train.model"};
    wrapper->save(out);

    delete wrapper;
}

/**
 *
 * @param data_dir
 * @param feature_nums
 * @param svm_path
 * @return
 */
svm_wrapper* train_svm(string data_dir, int feature_nums, string svm_path) {
    vector<string> *training_qids = new vector<string>();

    unordered_map<string, unordered_map<int, vector<feature_vector>>> *training_dataset
            = new unordered_map<string, unordered_map<int, vector<feature_vector>>>();

    read_data(TRAINING, data_dir, training_qids, training_dataset, nullptr, nullptr, feature_nums);

    vector<forward_node> *dataset_nodes = new vector<forward_node>();

    build_dataset_nodes(training_dataset, dataset_nodes);

    //multiclass_dataset *mcdata = new multiclass_dataset(dataset_nodes->begin(), dataset_nodes->end(), feature_nums);
    //classifier::dataset_view_type *mcdv = new classifier::dataset_view_type(*mcdata);
    //mcdv->shuffle();
    {
        std::random_shuffle(dataset_nodes->begin(), dataset_nodes->end());
        std::ofstream out{"svm-train"};
        for (const auto& node : *dataset_nodes)
        {
            out << node.label;
            for (const auto& count : node.fv)
                out << ' ' << (count.first + 1) << ':' << count.second;
            out << endl;
        }
    }

    svm_wrapper *wrapper = new svm_wrapper(svm_path);

    //delete mcdv;
    //delete mcdata;
    delete dataset_nodes;
    delete training_dataset;
    delete training_qids;

    return wrapper;
}

/**
 *
 * @param training_dataset
 * @param dataset_nodes
 */
void build_dataset_nodes(unordered_map<string, unordered_map<int, vector<feature_vector>>> *training_dataset,
                          vector<forward_node> *dataset_nodes) {
    for (auto query_iter = training_dataset->begin(); query_iter != training_dataset->end(); query_iter++) {
        auto &query_dataset = query_iter->second;
        vector<int> label_keys;
        for (auto &iter: query_dataset) {
            label_keys.push_back(iter.first);
        }
        for (int i = 0; i < label_keys.size(); i++) {
            for (int j = i + 1; j < label_keys.size(); j++) {
                int temp_label = label_keys[i] > label_keys[j] ? 1 : -1;
                vector<feature_vector> &vec1 = query_dataset[label_keys[i]];
                vector<feature_vector> &vec2 = query_dataset[label_keys[j]];
                for (auto vec1_iter = vec1.begin(); vec1_iter != vec1.end(); vec1_iter++) {
                    for (auto vec2_iter = vec2.begin(); vec2_iter != vec2.end(); vec2_iter++) {
                        dataset_nodes->push_back(forward_node());
                        forward_node &temp_node = dataset_nodes->back();
                        temp_node.label = temp_label;
                        temp_node.fv = *vec1_iter;
                        temp_node.fv -= *vec2_iter;
                    }
                }
            }
        }
    }
}

/**
 *
 * @param data_dir
 * @param feature_nums
 * @param classify_type
 * @param wrapper
 * @param model
 */
void test(string data_dir, int feature_nums, CLASSIFY_TYPE classify_type, svm_wrapper *wrapper, sgd_model *model) {
    vector<string> *testing_qids = new vector<string>();
    unordered_map<string, unordered_map<int, vector<feature_vector>>> *testing_dataset
            = new unordered_map<string, unordered_map<int, vector<feature_vector>>>();
    unordered_map<string, unordered_map<int, vector<string>>> *testing_docids
            = new unordered_map<string, unordered_map<int, vector<string>>>();
    unordered_map<string, unordered_map<string, int>> *relevance_map
            = new unordered_map<string, unordered_map<string, int>>();
    read_data(TESTING, data_dir, testing_qids, testing_dataset, testing_docids, relevance_map, feature_nums);
    cout << "Evaluating on test data" << endl;
    evaluate(testing_qids, testing_dataset, testing_docids, relevance_map, feature_nums, classify_type, wrapper, model);

    delete testing_qids;
    delete testing_dataset;
    delete testing_docids;
    delete relevance_map;
}

/**
 *
 * @param data_dir
 * @param feature_nums
 * @param classify_type
 * @param wrapper
 * @param model
 */
void validate(string data_dir, int feature_nums, CLASSIFY_TYPE classify_type, svm_wrapper *wrapper, sgd_model *model) {
    vector<string> *validation_qids = new vector<string>();
    unordered_map<string, unordered_map<int, vector<feature_vector>>> *validation_dataset
            = new unordered_map<string, unordered_map<int, vector<feature_vector>>>();
    unordered_map<string, unordered_map<int, vector<string>>> *validation_docids
            = new unordered_map<string, unordered_map<int, vector<string>>>();
    unordered_map<string, unordered_map<string, int>> *relevance_map
            = new unordered_map<string, unordered_map<string, int>>();
    read_data(VALIDATION, data_dir, validation_qids, validation_dataset, validation_docids, relevance_map, feature_nums);
    cout << "Evaluation on Validation set" << endl;
    evaluate(validation_qids,
             validation_dataset,
             validation_docids,
             relevance_map,
             feature_nums,
             classify_type,
             wrapper,
             model);

    delete validation_qids;
    delete validation_dataset;
    delete validation_docids;
    delete relevance_map;
}

/**
 *
 * @param qids
 * @param dataset
 * @param docids
 * @param relevance_map
 * @param feature_nums
 * @param classify_type
 * @param wrapper
 * @param model
 */
void evaluate(vector<string> *qids,
              unordered_map<string, unordered_map<int, vector<feature_vector>>> *dataset,
              unordered_map<string, unordered_map<int, vector<string>>> *docids,
              unordered_map<string, unordered_map<string, int>> *relevance_map,
              int feature_nums,
              CLASSIFY_TYPE classify_type,
              svm_wrapper *wrapper,
              sgd_model *model) {
    int query_num = 0;
    double top_precisions[10];
    double mean_ap = 0;
    double top_ndcgs[10];
    vector<double> *temp_precisions = new vector<double>();
    vector<int> *temp_relevances = new vector<int>();
    vector<int> dcg_rankings;
    double temp_ap, total_relevances, temp_ndcg, temp_idcg;
    for (int index = 0; index < 10; index++) {
        top_precisions[index] = 0;
        top_ndcgs[index] = 0;
    }
    vector<std::pair<string, double>> *doc_scores = new vector<std::pair<string, double>>();
    for (auto query_iter = dataset->begin(); query_iter != dataset->end(); query_iter++) {
        auto &query_dataset = query_iter->second;
        auto &query_docids = (*docids)[query_iter->first];

        for (auto label_iter = query_dataset.begin(); label_iter != query_dataset.end(); label_iter++) {
            auto &label_dataset = label_iter->second;
            auto &label_docids = query_docids[label_iter->first];
            for (int doc_idx = 0; doc_idx < label_docids.size(); doc_idx++) {
                feature_vector &fv = label_dataset[doc_idx];
                string docid = label_docids[doc_idx];
                double score = classify_type == LIBSVM ? wrapper->computeScore(fv) : model->predict(fv);
                doc_scores->push_back(std::make_pair(docid, score));
            }
        }
        std::sort(doc_scores->begin(), doc_scores->end(), compare_docscore);
        if (doc_scores->size() >= 10) {
            auto &query_relevances = (*relevance_map)[query_iter->first];
            int temp_relevance = query_relevances[(*doc_scores)[0].first];
            int last_precision = temp_relevance > 0 ? 1 : 0;
            temp_ap = last_precision * last_precision;
            temp_precisions->push_back(last_precision);
            temp_relevances->push_back(temp_relevance);
            for (int score_idx = 1; score_idx < doc_scores->size(); score_idx++) {
                temp_relevance = query_relevances[(*doc_scores)[score_idx].first];
                last_precision += (temp_relevance > 0 ? 1 : 0);
                temp_ap += ((double)last_precision / (score_idx + 1) * (temp_relevance > 0 ? 1 : 0));
                temp_precisions->push_back(last_precision);
                temp_relevances->push_back(temp_relevance);
            }
            total_relevances = last_precision;
            if (total_relevances > 0) { //must check
                for (int index = 0; index < 10; index++) {
                    top_precisions[index] += ((*temp_precisions)[index] / (index + 1));
                }
                mean_ap += (temp_ap / total_relevances);
                for (int index = 0; index < 10; index++) {
                    dcg_rankings.push_back(query_relevances[(*doc_scores)[index].first]);
                }
                std::sort(temp_relevances->begin(), temp_relevances->end(), std::greater<int>());
                for (int index = 0; index < 10; index++) {
                    temp_ndcg = compute_dcg(index + 1, dcg_rankings);
                    temp_idcg = compute_dcg(index + 1, *temp_relevances);
                    top_ndcgs[index] += (temp_ndcg / temp_idcg);
                }
                query_num++;
            }
            temp_precisions->clear();
            temp_relevances->clear();
            dcg_rankings.clear();
        }
        doc_scores->clear();
    }

    for (int index = 0; index < 10; index++) {
        top_precisions[index] /= query_num;
        cout << "Precision at position " << (index + 1) << ": " << top_precisions[index] << endl;
    }
    mean_ap /= query_num;
    cout << "Mean average precision: " << mean_ap << endl;
    for (int index = 0; index < 10; index++) {
        top_ndcgs[index] /= query_num;
        cout << "NDCG at position " << (index + 1) << ": " << top_ndcgs[index] << endl;
    }

    delete doc_scores;
    delete temp_precisions;
    delete temp_relevances;
}

/**
 *
 * @param limit
 * @param rankings
 * @return
 */
double compute_dcg(int limit, vector<int> &rankings) {
    double dcg = 0, dg;

    dg = pow(2, rankings[0]) - 1;
    dcg += dg;
    for (int index = 1; index < limit; index++) {
        dg = pow(2, rankings[index]) - 1;
        dg /= log2(index + 1);
        dcg += dg;
    }
    return dcg;
}

/**
 * Train the pairwise ranker model
 * @param data_dir
 * @param feature_nums
 * @param model
 */
void train(string data_dir, int feature_nums, sgd_model *model) {
    vector<string> *training_qids = new vector<string>();
    unordered_map<string, unordered_map<int, vector<feature_vector>>> *training_dataset
            = new unordered_map<string, unordered_map<int, vector<feature_vector>>>();
    read_data(TRAINING, data_dir, training_qids, training_dataset, nullptr, nullptr, feature_nums);
    int n_iter = 100000;

    std::unique_ptr<learn::loss::loss_function> loss
            = learn::loss::make_loss_function(learn::loss::hinge::id.to_string());

    for (int i = 0; i < n_iter; i++) {
        int random_seed = i;
        std::pair<tupl, tupl> data_pair = getRandomPair(training_qids, training_dataset, random_seed);
        feature_vector a, b;
        int y_a, y_b;
        string qid;
        std::tie(a, y_a, qid) = data_pair.first;
        std::tie(b, y_b, qid) = data_pair.second;
        feature_vector x = a;
        x -= b;
        double expected_label = y_a - y_b;
        double los = model->train_one(x, expected_label, *loss);
    }
    loss.reset();
    delete training_qids;
    delete training_dataset;
}

/**
 * Return a random pair of tuple for training the svm classifier
 * Tuple is of type (feature_vec, label, qid)
 * @param training_qids
 * @param training_dataset
 * @param random_seed
 * @return
 */
std::pair<tupl, tupl> getRandomPair(vector<string> *training_qids,
                                    unordered_map<string, unordered_map<int, vector<feature_vector>>> *training_dataset,
                                    int random_seed) {

    std::default_random_engine generator(random_seed);
    int rel_levels = 0;
    string qid;
    do {
        //select q uniformly at random from Q
        int max_q = training_qids->size();
        std::uniform_int_distribution<int> qid_distribution(0, max_q-1);
        int q_index = qid_distribution(generator);
        qid = (*training_qids)[q_index];
        auto &qid_vec = (*training_dataset)[qid];
        rel_levels = qid_vec.size();

    } while(rel_levels <= 1);
    unordered_map<int, vector<feature_vector>> &qid_vec = (*training_dataset)[qid];;

    //select ya uniformly at random from Y [q]
    int max_ya = qid_vec.size();
    std::uniform_int_distribution<int> ya_distribution(0, max_ya -1);
    int ya_index = ya_distribution(generator);

    int count = 0;
    int ya = ya_index;
    for (auto iter = qid_vec.begin(); iter != qid_vec.end() && count <= ya; iter++, count++) {
        ya_index = iter->first;
    }

    //select (a, ya, q) uniformly at random from P[q][ya]
    int max_a = qid_vec[ya_index].size();
    std::uniform_int_distribution<int> a_distribution(0, max_a-1);
    int a_index = a_distribution(generator);
    feature_vector &a = qid_vec[ya_index][a_index];
    tupl d1  = std::make_tuple(a, ya_index, qid);

    //select yb uniformly at random from Y [q] − ya.
    int max_yb = max_ya -1;
    std::uniform_int_distribution<int> yb_distribution(0, max_yb-1);
    int yb_index = yb_distribution(generator);
    count = 0;
    int yb = yb_index;
    for (auto iter = qid_vec.begin(); iter != qid_vec.end() && count <= yb; iter++) {
        yb_index = iter->first;
        if (yb_index == ya_index) {
            continue;
        }
        count++;
    }

    //select (b, yb, q) uniformly at random from P[q][yb]
    int max_b = qid_vec[yb_index].size();
    std::uniform_int_distribution<int> b_distribution(0, max_b-1);
    int b_index = b_distribution(generator);
    feature_vector &b = qid_vec[yb_index][b_index];
    tupl d2  = std::make_tuple(b, yb_index, qid);

    return std::make_pair(d1,d2);
}

/**
 * Read data from dataset and store it as nested hash-tables
 * @param data_type
 * @param data_dir
 * @param qids
 * @param dataset
 * @param docids
 * @param relevance_map
 * @param feature_nums
 */
void read_data(DATA_TYPE data_type,
               string data_dir,
               vector<string> *qids,
               unordered_map<string, unordered_map<int, vector<feature_vector>>> *dataset,
               unordered_map<string, unordered_map<int, vector<string>>> *docids,
               unordered_map<string, unordered_map<string, int>> *relevance_map,
               int feature_nums) {
    auto start = chrono::high_resolution_clock::now();

    string data_file = data_dir;
    switch (data_type) {
        case TRAINING:
            data_file += "/train.txt";
            break;
        case VALIDATION:
            data_file += "/vali.txt";
            break;
        case TESTING:
            data_file += "/test.txt";
            break;
    }
    std::ifstream infile(data_file);
    string line;
    int feature_idx;
    unordered_map<string, int> *qid_docids = new unordered_map<string, int>();
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        int label, feature_id;
        string qid;
        double feature_val;
        string tmp_str, docid;
        iss >> label >> tmp_str;
        stringstream ss(tmp_str.substr(tmp_str.find(':') + 1, tmp_str.find(' ')));
        ss >> qid;

        if (dataset->find(qid) == dataset->end()) {
            qids->push_back(qid);
            (*dataset)[qid] = unordered_map<int, vector<feature_vector>>();
        }
        unordered_map<int, vector<feature_vector>> &query_dataset = (*dataset)[qid];

        if (query_dataset.find(label) == query_dataset.end()) {
            query_dataset[label] =  vector<feature_vector>();
        }
        vector<feature_vector> &label_dataset = query_dataset[label];

        label_dataset.push_back(feature_vector(0));
        feature_vector &features = label_dataset.back();
        for (feature_idx = 0; feature_idx < feature_nums; feature_idx++) {
            iss >> tmp_str;
            stringstream ssid(tmp_str.substr(0, tmp_str.find(':')));
            ssid >> feature_id;
            stringstream ssval(tmp_str.substr(tmp_str.find(':') + 1));
            ssval >> feature_val;
            features[term_id{feature_id - 1}] = feature_val;
        }

        if (data_type != TRAINING) {
            if (docids->find(qid) == docids->end()) {
                (*docids)[qid] = unordered_map<int, vector<string> >();
                (*qid_docids)[qid] = 0;
            }
            unordered_map<int, vector<string> > &query_docids = (*docids)[qid];

            if (query_docids.find(label) == query_docids.end()) {
                query_docids[label] = vector<string>();
            }
            vector<string> &label_docids = query_docids[label];

            docid = qid + std::to_string((*qid_docids)[qid]++);
            label_docids.push_back(docid);
            if (relevance_map->find(qid) == relevance_map->end()) {
                (*relevance_map)[qid] = unordered_map<string, int>();
            }
            unordered_map<string, int> &doc_relevance = (*relevance_map)[qid];
            doc_relevance[docid] = label;
        }
    }
    delete qid_docids;

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_time = end - start;
    cout << "Time spent in read_data in seconds: " << elapsed_time.count() << endl;
}