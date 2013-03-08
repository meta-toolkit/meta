/**
 * @file hier_cluster.h
 */

#ifndef _HIER_CLUSTER_
#define _HIER_CLUSTER_

#include <vector>

/**
 * Assume Doc implements something like
 * double Doc::cosine_similarity(const Doc & other) const;
 * Sim is a function, usually Doc::cosine_similarity or jaccard_similarity.
 * But we also need a cluster similarity...
 */
template <class Doc, class Sim>
class HierarchicalCluster
{
    public:

        class Cluster
        {

        };

        HierarchicalCluster(const vector<Doc> & docs);

};

#include "heir_cluster.cpp"
#endif
