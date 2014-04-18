/**
 * @file metapath_measures.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_METAPATH_MEASURES_H_
#define META_METAPATH_MEASURES_H_

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
    using metapath_t = std::vector<std::string>;

    /**
     * @param g
     * @param metapath
     */
    metapath_measures(Graph& g, const metapath_t& metapath);

    /**
     * Performs the PathCount measure function on all pairs of nodes.
     * PathCount is simply the number of path instances between two objects
     * following a given metapath, denoted as \f$PC_R\f$, where \f$R\f$ is the
     * relation defined by the metapath.
     * @return all possible PC measures on the graph
     * @see Co-Author Relationship Prediction in Heterogeneous Bibliographic
     * Networks, Sun et. al. 2011.
     */
    measure_result path_count();

    /**
     * Performs the NormalizedPathCount measure function on all pairs of nodes.
     * @return all possible NPC measures on the graph
     * \f$ NPC_R(a_i, a_j) = \frac{PC_R(a_i, a_j) + PC_{R^{-1}}(a_j,
     * a_i)}{PC_R(a_i,\cdot) + PC_R(\cdot, a_j)} \f$
     * @see Co-Author Relationship Prediction in Heterogeneous Bibliographic
     * Networks, Sun et. al. 2011.
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

  private:
    /**
     * @param orig_id
     * @param id
     * @param result
     * @param depth
     */
    void bfs_match(node_id orig_id, node_id id, measure_result& result,
                   uint64_t depth);

    /**
     * @param id
     * @param result
     */
    uint64_t meta_degree(node_id id, const measure_result& result);

    /// The graph to operate on.
    Graph& g_;

    /// The metapath to use.
    metapath_t metapath_;
};
}
}
}

#include "graph/algorithm/metapath_measures.tcc"
#endif
