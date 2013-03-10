/**
 * @file single_link_policy.h
 */

#ifndef _SINGLE_LINK_POLICY_H_
#define _SINGLE_LINK_POLICY_H_

#include <iostream>
#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

#include "cluster/point.h"

namespace clustering {
    
/**
 * A simple linking policy for agglomerative clustering utilizing the
 * single-link metric. The single link metric is one that merges cluster
 * \f$X\f$ and cluster \f$Y\f$ which have the minimum distance \f$D_{xy}\f$
 * between them, where \f$D_{xy}\f$ is defined as being the minimum
 * distance between any point in cluster \f$X\f$ and any other point in
 * cluster \f$Y\f$.
 */
template <class Similarity>
class basic_single_link_policy {
    public:
        template <class RootList>
        void merge_clusters( RootList & current_roots ) {
            std::cout << "merge_clusters()" << std::endl;

            using treeptr = typename RootList::value_type;

            // current_roots is a list of unique_ptr to treenodes
            double min_distance = std::numeric_limits<double>::max();
            auto to_merge = std::make_pair( current_roots.begin(), 
                                            current_roots.begin() );
            size_t remaining_loops = current_roots.size();
            for( auto it = current_roots.begin(); 
                    it != current_roots.end(); 
                    ++it ) {
                std::cout << "iterations remaining: " << (remaining_loops--) << "           \r" << std::flush;
                auto it2 = it;
                ++it2;
                for( ; it2 != current_roots.end(); ++it2 ) {
                    double distance = single_link_distance( *it, *it2 );
                    if( distance < min_distance ) {
                        min_distance = distance;
                        to_merge = std::make_pair( it, it2 );
                    }
                }
            }

            std::cout << std::endl << "finished merge_clusters()" << std::endl;

            treeptr left = std::move( *to_merge.first );
            treeptr right = std::move( *to_merge.second );

            current_roots.erase( to_merge.first );
            current_roots.erase( to_merge.second );

            using treenode = typename treeptr::element_type;
            current_roots.emplace_back( 
                new treenode{ std::move( left ), std::move( right ) } );
        }
    private:
        
        template <class TreeNodePtr>
        double single_link_distance( const TreeNodePtr & first, 
                                     const TreeNodePtr & second ) {
            double min_distance = std::numeric_limits<double>::max();
            for( const auto & first_point : first->points() ) {
                for( const auto & second_point : second->points() ) {
                    min_distance = std::min(
                        min_distance,
                        sim_( first_point->vector(), 
                              second_point->vector() ) );
                }
            }
            return min_distance;
        }

        Similarity sim_;
};

}

#endif
