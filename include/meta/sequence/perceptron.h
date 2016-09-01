/**
 * @file perceptron.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_SEQUENCE_PERCEPTRON_H_
#define META_SEQUENCE_PERCEPTRON_H_

#include <random>

#include "meta/classify/models/linear_model.h"
#include "meta/config.h"
#include "meta/sequence/sequence_analyzer.h"

namespace meta
{
namespace sequence
{

/**
 * A greedy averaged perceptron tagger.
 */
class perceptron
{
  public:
    /**
     * Training options required for learning a tagger.
     */
    struct training_options
    {
        /**
         * How many iterations should the training algorithm run?
         */
        uint64_t max_iterations = 5;

        /**
         * The seed for the random number generator used for shuffling
         * examples during training.
         */
        std::random_device::result_type seed = std::random_device{}();
    };

    /**
     * Default constructor.
     */
    perceptron();

    /**
     * Loads a perceptron tagger from a given prefix.
     * @param prefix The folder that contains the tagger model
     */
    perceptron(const std::string& prefix);

    /**
     * Tags a sequence. This sets *both* the label and tag of the
     * sequence's observations.
     * @param seq The sequence to be tagged
     */
    void tag(sequence& seq) const;

    /**
     * Trains the tagger on a set of sequences using the given options. The
     * sequences given for training will be analyzed by the tagger
     * internally, so they do not need to be analyzed ahead of time.
     *
     * @param sequences The training data
     * @param options THe training options
     */
    void train(std::vector<sequence>& sequences,
               const training_options& options);

    /**
     * Saves the model to the folder specified by prefix. Both the tagger
     * and its analyzer are serialized.
     *
     * @param prefix The folder to save the model to
     */
    void save(const std::string& prefix) const;

  private:
    /**
     * The analyzer used for feature generation.
     */
    sequence_analyzer analyzer_;

    /**
     * The model storage.
     */
    classify::linear_model<feature_id, double, label_id> model_;
};
}
}

#endif
