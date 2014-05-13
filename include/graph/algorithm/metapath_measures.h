/**
 * @file metapath_measures.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_METAPATH_MEASURES_H_
#define META_METAPATH_MEASURES_H_

#include <unordered_map>
#include "graph/directed_graph.h"
#include "graph/metapath.h"

namespace meta
{
namespace graph
{
namespace algorithm
{
template <class Graph>
class metapath_measures
{
  public:
    using measure_result = std::unordered_map
        <node_id, std::unordered_map<node_id, double>>;

    /**
     * @param g
     * @param metapath
     */
    metapath_measures(Graph& g, const metapath& mpath);

    /**
     * Performs the PathCount measure function on all pairs of nodes.
     * PathCount is simply the number of path instances between two objects
     * following a given metapath, denoted as \f$PC_R\f$, where \f$R\f$ is the
     * relation defined by the metapath.
     * @param is_weighted Whether or not to take weighted nodes into account
     * @return all possible PC measures on the graph
     * @see Co-Author Relationship Prediction in Heterogeneous Bibliographic
     * Networks, Sun et. al. 2011.
     */
    measure_result path_count(bool is_weighted = false);

    /**
     * Performs the NormalizedPathCount measure function on all pairs of nodes.
     * @return all possible NPC measures on the graph
     * \f$ NPC_R(a_i, a_j) = \frac{PC_R(a_i, a_j) + PC_{R^{-1}}(a_j,
     * a_i)}{PC_R(a_i,a_i) + PC_R(a_j, a_j)} \f$
     * Note that this is NOT the formulation given in the PathPredict paper.
     * This is the corrected version as it appears in:
     * @see Mining Heterogeneous Information Networks: Principles and
     * Methodologies, Yizhou Sun and Jiawei Han. 2012.
     */
    measure_result normalized_path_count();

    /**
     * Performs the RandomWalk measure function on all pairs of nodes.
     * \f$ RW_R(a_i, a_j) = \frac{PC_R(a_i, a_j)}{PC_R(a_i,\cdot)} \f$
     * @return all possible RW measures on the graph
     * @see Co-Author Relationship Prediction in Heterogeneous Bibliographic
     * Networks, Sun et. al. 2011.
     */
    measure_result random_walk();

    /**
     * Performs the SymmetricRandomWalk measure function on all pairs of nodes.
     * \f$ SRW_R(a_i, a_j) = RW_R(a_i, a_j) + RW_{R^{-1}}(a_j, a_i)\f$
     * @return all possible SRW measures on the graph
     * @see Co-Author Relationship Prediction in Heterogeneous Bibliographic
     * Networks, Sun et. al. 2011.
     */
    measure_result symmetric_random_walk();

    /**
     * @param orig_id
     * @param id
     * @param result
     * @param depth
     */
    void bfs_match(node_id orig_id, node_id id, measure_result& result,
                   uint64_t depth, bool is_weighted = false);
  private:
    /**
     * @param id
     * @param result
     */
    uint64_t meta_degree(node_id id, const measure_result& result);

    /// The graph to operate on.
    Graph& g_;

    /// The metapath to use.
    metapath mpath_;

    double cur_weight_;

    std::vector<std::string> cur_path_;

    static bool constexpr print_paths = false;
};
}
}
}

#include "graph/algorithm/metapath_measures.tcc"
#endif
