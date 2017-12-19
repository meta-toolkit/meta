/**
 * @file pairwise_letor.cpp
 * @author Mihika Dave, Anthony Huang, Rachneet Kaur
 * @date 12/18/17
 */

#include <cassert>

#include "meta/learn/learntorank/pairwise_letor.h"

namespace meta
{
namespace learn
{
namespace learntorank
{

pairwise_letor::pairwise_letor(size_t num_features, CLASSIFY_TYPE classify_type,
                               bool hasModel, string model_file)
        : num_features_{num_features}, classify_type_{classify_type}
{
    if (hasModel) {
        ifstream in{model_file};
        if (classify_type == pairwise_letor::SPD) {
            model_ = make_unique<sgd_model>(in);
        } else {
            string wrapper_id = io::packed::read<std::string>(in);
            assert(svm_wrapper::id.compare(wrapper_id) == 0);
            wrapper_ = make_unique<svm_wrapper>(in);
        }
    } else {
        model_ = make_unique<sgd_model>(num_features);
    }
}

pairwise_letor::~pairwise_letor() {
    if (classify_type_ == pairwise_letor::SPD) {
        ofstream out{"letor_sgd_train.model"};
        model_->save(out);
    } else {
        ofstream out{"letor_svm_train.model"};
        wrapper_->save(out);
    }
}

void pairwise_letor::train(string data_dir) {
    auto training_qids = make_unique<vector<string>>();
    auto training_dataset =
                make_unique<unordered_map<string,unordered_map<int,vector<feature_vector>>>>();
    auto docids =
                make_unique<unordered_map<string, unordered_map<int, vector<string>>>>();
    auto relevance_map =
                make_unique<unordered_map<string, unordered_map<string, int>>>();
    read_data(TRAINING,data_dir,*training_qids,*training_dataset,
              *docids,*relevance_map);
    auto n_iter = 100000;

    auto loss = loss::make_loss_function(loss::hinge::id.to_string());

    for (size_t i = 0; i < n_iter; ++i) {
        auto random_seed = i;
        auto data_pair
                = getRandomPair(*training_qids, *training_dataset, random_seed);
        feature_vector a, b;
        int y_a, y_b;
        string qid;
        tie(a, y_a, qid) = data_pair.first;
        tie(b, y_b, qid) = data_pair.second;
        auto x = a;
        x -= b;
        auto expected_label = y_a - y_b;
        auto los = model_->train_one(x, expected_label, *loss);
    }
    loss.reset();
}

void pairwise_letor::read_data(DATA_TYPE data_type,
                    string data_dir,
                    vector<string>& qids,
                    unordered_map<string, unordered_map<int, vector<feature_vector>>>& dataset,
                    unordered_map<string, unordered_map<int, vector<string>>>& docids,
                    unordered_map<string, unordered_map<string, int>>& relevance_map) {
    auto start = chrono::high_resolution_clock::now();

    auto data_file = data_dir;
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
    auto qid_docids = unordered_map<string, int>();
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        int label, feature_id;
        string qid;
        double feature_val;
        string tmp_str, docid;
        iss >> label >> tmp_str;
        stringstream ss(tmp_str.substr(tmp_str.find(':') + 1, tmp_str.find(' ')));
        ss >> qid;

        if (dataset.find(qid) == dataset.end()) {
            qids.push_back(qid);
            dataset[qid] = unordered_map<int, vector<feature_vector>>();
        }
        auto& query_dataset = dataset[qid];

        if (query_dataset.find(label) == query_dataset.end()) {
            query_dataset[label] = vector<feature_vector>();
        }
        auto& label_dataset = query_dataset[label];

        label_dataset.push_back(feature_vector(0));
        auto& features = label_dataset.back();
        for (auto feature_idx = 0; feature_idx < num_features_; feature_idx++) {
            iss >> tmp_str;
            stringstream ssid(tmp_str.substr(0, tmp_str.find(':')));
            ssid >> feature_id;
            stringstream ssval(tmp_str.substr(tmp_str.find(':') + 1));
            ssval >> feature_val;
            features[term_id{feature_id - 1}] = feature_val;
        }

        if (data_type != TRAINING) {
            if (docids.find(qid) == docids.end()) {
                docids[qid] = unordered_map<int, vector<string> >();
                qid_docids[qid] = 0;
            }
            auto& query_docids = docids[qid];

            if (query_docids.find(label) == query_docids.end()) {
                query_docids[label] = vector<string>();
            }
            auto& label_docids = query_docids[label];

            docid = qid + to_string(qid_docids[qid]++);
            label_docids.push_back(docid);
            if (relevance_map.find(qid) == relevance_map.end()) {
                relevance_map[qid] = unordered_map<string, int>();
            }
            auto& doc_relevance = relevance_map[qid];
            doc_relevance[docid] = label;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_time = end - start;
    cout << "Time spent in read_data in seconds: "
            << elapsed_time.count() << endl;
}

void pairwise_letor::build_dataset_nodes(
        unordered_map<string,unordered_map<int,vector<feature_vector>>>& train_dataset,
        vector<forward_node>& dataset_nodes) {
    for (auto& query_iter : train_dataset) {
        auto& query_dataset = query_iter.second;
        vector<int> label_keys;
        for (auto& iter : query_dataset) {
            label_keys.push_back(iter.first);
        }
        for (int i = 0; i < label_keys.size(); i++) {
            for (int j = i + 1; j < label_keys.size(); j++) {
                auto temp_label = label_keys[i] > label_keys[j] ? 1 : -1;
                auto& vec1 = query_dataset[label_keys[i]];
                auto& vec2 = query_dataset[label_keys[j]];
                for (auto& vec1_iter : vec1) {
                    for (auto& vec2_iter : vec2) {
                        dataset_nodes.push_back(forward_node());
                        auto& temp_node = dataset_nodes.back();
                        temp_node.label = temp_label;
                        temp_node.fv = vec1_iter;
                        temp_node.fv -= vec2_iter;
                    }
                }
            }
        }
    }
}

pair<pairwise_letor::tupl,pairwise_letor::tupl> pairwise_letor::getRandomPair(
        vector<string>& training_qids,
        unordered_map<string,unordered_map<int,vector<feature_vector>>>& train_dataset,
        int random_seed) {

    default_random_engine generator(random_seed);
    auto rel_levels = 0;
    string qid;
    do {
        //select q uniformly at random from Q
        auto max_q = training_qids.size();
        uniform_int_distribution<int> qid_distribution(0, max_q - 1);
        auto q_index = qid_distribution(generator);
        qid = training_qids[q_index];
        auto& qid_vec = train_dataset[qid];
        rel_levels = qid_vec.size();

    } while (rel_levels <= 1);
    auto& qid_vec = train_dataset[qid];

    //select ya uniformly at random from Y [q]
    auto max_ya = qid_vec.size();
    uniform_int_distribution<int> ya_distribution(0, max_ya - 1);
    auto ya_index = ya_distribution(generator);

    auto count = 0;
    auto ya = ya_index;
    for (auto& iter : qid_vec) {
        if (count > ya) {
            break;
        }
        ya_index = iter.first;
        count++;
    }

    //select (a, ya, q) uniformly at random from P[q][ya]
    auto max_a = qid_vec[ya_index].size();
    uniform_int_distribution<int> a_distribution(0, max_a - 1);
    auto a_index = a_distribution(generator);
    auto& a = qid_vec[ya_index][a_index];
    auto d1 = make_tuple(a, ya_index, qid);

    //select yb uniformly at random from Y [q] − ya.
    auto max_yb = max_ya - 1;
    uniform_int_distribution<int> yb_distribution(0, max_yb - 1);
    auto yb_index = yb_distribution(generator);
    count = 0;
    auto yb = yb_index;
    for (auto& iter : qid_vec) {
        if (count > yb) {
            break;
        }
        yb_index = iter.first;
        if (yb_index == ya_index) {
            continue;
        }
        count++;
    }

    //select (b, yb, q) uniformly at random from P[q][yb]
    auto max_b = qid_vec[yb_index].size();
    uniform_int_distribution<int> b_distribution(0, max_b - 1);
    auto b_index = b_distribution(generator);
    auto& b = qid_vec[yb_index][b_index];
    auto d2 = make_tuple(b, yb_index, qid);

    return make_pair(d1, d2);
}

void pairwise_letor::train_svm(string data_dir, string svm_path) {
    auto training_qids = make_unique<vector<string>>();

    auto training_dataset =
            make_unique<unordered_map<string,unordered_map<int,vector<feature_vector>>>>();

    auto docids =
            make_unique<unordered_map<string, unordered_map<int, vector<string>>>>();
    auto relevance_map =
            make_unique<unordered_map<string, unordered_map<string, int>>>();

    read_data(TRAINING,data_dir,*training_qids,*training_dataset,
              *docids,*relevance_map);

    auto dataset_nodes = make_unique<vector<forward_node>>();

    build_dataset_nodes(*training_dataset, *dataset_nodes);

    //multiclass_dataset *mcdata = new multiclass_dataset(dataset_nodes->begin(), dataset_nodes->end(), feature_nums);
    //classifier::dataset_view_type *mcdv = new classifier::dataset_view_type(*mcdata);
    //mcdv->shuffle();
    {
        random_shuffle(dataset_nodes->begin(), dataset_nodes->end());
        ofstream out{"svm-train"};
        for (const auto &node : *dataset_nodes) {
            out << node.label;
            for (const auto &count : node.fv)
                out << ' ' << (count.first + 1) << ':' << count.second;
            out << endl;
        }
    }

    wrapper_ = make_unique<svm_wrapper>(svm_path);

}

void pairwise_letor::validate(string data_dir) {
    auto validation_qids = make_unique<vector<string>>();
    auto validation_dataset =
            make_unique<unordered_map<string,unordered_map<int,vector<feature_vector>>>>();
    auto validation_docids =
            make_unique<unordered_map<string, unordered_map<int, vector<string>>>>();
    auto relevance_map
            = make_unique<unordered_map<string, unordered_map<string, int>>>();
    read_data(VALIDATION,data_dir,*validation_qids,*validation_dataset,
              *validation_docids,*relevance_map);
    cout << "Evaluation on Validation set" << endl;
    evaluate(*validation_qids, *validation_dataset, *validation_docids,
             *relevance_map);

}

void pairwise_letor::test(string data_dir) {
    auto testing_qids = make_unique<vector<string>>();
    auto testing_dataset =
            make_unique<unordered_map<string,unordered_map<int,vector<feature_vector>>>>();
    auto testing_docids =
            make_unique<unordered_map<string, unordered_map<int, vector<string>>>>();
    auto relevance_map
            = make_unique<unordered_map<string, unordered_map<string, int>>>();
    read_data(TESTING, data_dir, *testing_qids, *testing_dataset,
              *testing_docids, *relevance_map);
    cout << "Evaluating on test data" << endl;
    evaluate(*testing_qids, *testing_dataset, *testing_docids,
             *relevance_map);

}

void pairwise_letor::evaluate(vector<string>& qids,
              unordered_map<string, unordered_map<int, vector<feature_vector>>>& dataset,
              unordered_map<string, unordered_map<int, vector<string>>>& docids,
              unordered_map<string, unordered_map<string, int>>& relevance_map) {
    auto query_num = 0;
    double top_precisions[10];
    double mean_ap = 0;
    double top_ndcgs[10];
    vector<double> temp_precisions;
    vector<int> temp_relevances;
    vector<int> dcg_rankings;
    double temp_ap, total_relevances, temp_ndcg, temp_idcg;
    for (auto index = 0; index < 10; index++) {
        top_precisions[index] = 0;
        top_ndcgs[index] = 0;
    }
    vector<pair<string, double>> doc_scores;
    for (auto& query_iter : dataset) {
        auto& query_dataset = query_iter.second;
        auto& query_docids = docids[query_iter.first];

        for (auto& label_iter : query_dataset) {
            auto& label_dataset = label_iter.second;
            auto& label_docids = query_docids[label_iter.first];
            for (size_t doc_idx = 0; doc_idx < label_docids.size(); doc_idx++) {
                auto& fv = label_dataset[doc_idx];
                auto docid = label_docids[doc_idx];
                auto score = classify_type_ == LIBSVM ?
                             wrapper_->computeScore(fv) : model_->predict(fv);
                doc_scores.push_back(make_pair(docid, score));
            }
        }
        sort(doc_scores.begin(), doc_scores.end(), compare_docscore);
        if (doc_scores.size() >= 10) {
            auto& query_relevances = relevance_map[query_iter.first];
            auto temp_relevance = query_relevances[doc_scores[0].first];
            auto last_precision = temp_relevance > 0 ? 1 : 0;
            temp_ap = last_precision * last_precision;
            temp_precisions.push_back(last_precision);
            temp_relevances.push_back(temp_relevance);
            for (auto score_idx = 1; score_idx < doc_scores.size(); score_idx++) {
                temp_relevance = query_relevances[doc_scores[score_idx].first];
                last_precision += (temp_relevance > 0 ? 1 : 0);
                temp_ap +=
                    ((double) last_precision / (score_idx + 1) * (temp_relevance > 0 ? 1 : 0));
                temp_precisions.push_back(last_precision);
                temp_relevances.push_back(temp_relevance);
            }
            total_relevances = last_precision;
            if (total_relevances > 0) { //must check
                for (auto index = 0; index < 10; index++) {
                    top_precisions[index] += (temp_precisions[index] / (index + 1));
                }
                mean_ap += (temp_ap / total_relevances);
                for (auto index = 0; index < 10; index++) {
                    dcg_rankings.push_back(query_relevances[doc_scores[index].first]);
                }
                sort(temp_relevances.begin(), temp_relevances.end(), std::greater<int>());
                for (auto index = 0; index < 10; index++) {
                    temp_ndcg = compute_dcg(index + 1, dcg_rankings);
                    temp_idcg = compute_dcg(index + 1, temp_relevances);
                    top_ndcgs[index] += (temp_ndcg / temp_idcg);
                }
                query_num++;
            }
            temp_precisions.clear();
            temp_relevances.clear();
            dcg_rankings.clear();
        }
        doc_scores.clear();
    }

    cout << endl << "Precision on Test Data: " << endl;
    for (auto index = 0; index < 10; index++) {
        top_precisions[index] /= query_num;
        cout << "Precision at position " << (index + 1)
            << ": " << top_precisions[index] << endl;
    }
    mean_ap /= query_num;

    cout << endl << "Mean Average Precision on Test Data: " << endl;
    cout << "MAP: " << mean_ap << endl;

    cout << endl << "NDCG on Test Data: " << endl;
    for (auto index = 0; index < 10; index++) {
        top_ndcgs[index] /= query_num;
        cout << "NDCG at position " << (index + 1) << ": "
            << top_ndcgs[index] << endl;
    }

}

double pairwise_letor::compute_dcg(int limit, vector<int> &rankings) {
    double dcg = 0, dg;

    dg = pow(2, rankings[0]) - 1;
    dcg += dg;
    for (auto index = 1; index < limit; index++) {
        dg = pow(2, rankings[index]) - 1;
        dg /= log2(index + 1);
        dcg += dg;
    }
    return dcg;
}
}
}
}