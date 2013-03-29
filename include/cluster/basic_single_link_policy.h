/**
 * @file basic_single_link_policy.h
 */

#ifndef _SINGLE_LINK_POLICY_H_
#define _SINGLE_LINK_POLICY_H_

#include <iostream>
#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

#include "cluster/point.h"
#include "parallel/thread_pool.h"

namespace meta {
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
            // current_roots is a list of unique_ptr to treenodes
            std::cout << "merge_clusters()" << std::endl;

            using treeptr = typename RootList::value_type;
            using iterator = typename RootList::iterator;

            parallel::thread_pool pool;

            std::vector<std::future<std::tuple<size_t, iterator, iterator>>> futures;
            for( auto it = current_roots.begin(); 
                    it != current_roots.end(); 
                    ++it ) {
                futures.push_back(
                        pool.submit_task( [&,it]() {
                                auto result = std::make_tuple( std::numeric_limits<size_t>::max(), it, it );
                                auto it2 = it;
                                ++it2;
                                for( ; it2 != current_roots.end(); ++it2 ) {
                                    double distance = single_link_distance( *it, *it2 );
                                    if( distance < std::get<0>( result ) ) {
                                        std::get<0>( result ) = distance;
                                        std::get<1>( result ) = it;
                                        std::get<2>( result ) = it2;
                                    }
                                }
                                return result;
                            }
                        )
                    );
            }
            
            auto best_merge = std::make_tuple( std::numeric_limits<size_t>::max(),
                                               current_roots.begin(), 
                                               current_roots.begin() );
            for( auto & fut : futures ) {
                auto tuple = fut.get();
                if( std::get<0>( tuple ) < std::get<0>( best_merge ) )
                    best_merge = tuple;
            }

            std::cout << std::endl << "finished merge_clusters()" << std::endl;

            treeptr left = std::move( *std::get<1>( best_merge ) );
            treeptr right = std::move( *std::get<2>( best_merge ) );

            current_roots.erase( std::get<1>( best_merge ) );
            current_roots.erase( std::get<2>( best_merge ) );

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
}

#endif
