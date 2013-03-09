/**
 * @file agglomerative_clustering.h
 */

#ifndef _AGGLOMERATIVE_CLUSTERING_H_
#define _AGGLOMERATIVE_CLUSTERING_H_

#include <vector>
#include <memory>

namespace clustering {
    
template <class Element, class LinkPolicy>
class agglomerative_clustering {
    friend LinkPolicy;
    
    public:
        agglomerative_clustering( const std::vector<Element> & elems ) {
            // create "root nodes" for each of the elements to be clustered
            for( const auto & e : elems )
                current_roots_.push_back( new leafnode{ &e } );
            start_clustering();
        }
    private:

        void start_clustering() {
            while( current_roots_.size() > 1 )
                link_policy_.merge_clusters( current_roots_ );
        }
        
        struct treenode {
            using treeptr = std::unique_ptr<treenode>;
            
            treeptr left_;
            treeptr right_;
            
            treenode( treeptr left, treeptr right ) 
                : left_( std::move( left ) ),
                  right_( std::move( right ) ) { }
            
            virtual Element * get_element() {
                return nullptr;
            }
        };
        
        struct leafnode : public treenode {
            const Element * element_;
            
            virtual Element * get_element() {
                return element_;
            }

            leafnode( const Element * elem ) : element_{ elem },
                treenode::left_{ nullptr }, treenode::right_{ nullptr } { }
        };

        std::vector<std::unique_ptr<treenode>> current_roots_;

        LinkPolicy link_policy_;
};

}
#endif
