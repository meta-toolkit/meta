/**
 * @file agglomerative_clustering.h
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef META_AGGLOMERATIVE_CLUSTERING_H_
#define META_AGGLOMERATIVE_CLUSTERING_H_

#include <iostream>
#include <list>
#include <memory>
#include <vector>

#include "cluster/point.h"

namespace meta
{
namespace clustering
{

template <class Element, class LinkPolicy>
class agglomerative_clustering
{
  public:
    agglomerative_clustering(const std::vector<Element>& elems)
    {
        std::cout << "Creating initial clusters..." << std::endl;
        // create "root nodes" for each of the elements to be clustered
        for (const auto& e : elems)
            current_roots_.emplace_back(new leafnode{e});
        std::cout << "Starting clustering..." << std::endl;
        start_clustering();
    }

  private:
    void start_clustering()
    {
        while (current_roots_.size() > 1)
        {
            std::cout << current_roots_.size() << " current clusters..."
                      << std::endl;
            link_policy_.merge_clusters(current_roots_);
        }
    }

    struct treenode
    {
        using treeptr = std::unique_ptr<treenode>;

        // I really wish there was an easier way of getting the point
        // type for a given element non-intrusively. This works, but it
        // quite a bit of voodoo...
        using Point = decltype(make_point(*(static_cast<Element*>(nullptr))));

        using elem_vector = std::vector<const Element*>;
        using point_vector = std::vector<const Point*>;

        treeptr left_;
        treeptr right_;

        treenode(treeptr left, treeptr right)
            : left_(std::move(left)), right_(std::move(right))
        {
            /* nothing */
        }

        virtual const Element* get_element()
        {
            return nullptr;
        }

        elem_vector elements() const
        {
            elem_vector elems;
            find_elements(elems);
            return elems;
        }

        point_vector points() const
        {
            point_vector points;
            find_points(points);
            return points;
        }

      private:
        virtual void find_elements(elem_vector& elems) const
        {
            left_->find_elements(elems);
            right_->find_elements(elems);
        }

        virtual void find_points(point_vector& points) const
        {
            left_->find_points(points);
            right_->find_points(points);
        }
    };

    struct leafnode : public treenode
    {
        using Point = typename treenode::Point;
        using elem_vector = typename treenode::elem_vector;
        using point_vector = typename treenode::point_vector;

        Point point_;

        virtual const Element* get_element()
        {
            return point_.element();
        }

        leafnode(const Element& e)
            : treenode{nullptr, nullptr}, point_{make_point(e)}
        {
            /* nothing */
        }

      private:
        virtual void find_elements(elem_vector& elems) const
        {
            elems.push_back(point_.element());
        }

        virtual void find_points(point_vector& points) const
        {
            points.push_back(&point_);
        }
    };

    std::list<std::unique_ptr<treenode>> current_roots_;

    LinkPolicy link_policy_;
};
}
}

#endif
