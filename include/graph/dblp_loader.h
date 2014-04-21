/**
 * @file dblp_loader.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DBLP_LOADER_H_
#define META_DBLP_LOADER_H_

#include <fstream>
#include "io/parser.h"

namespace meta
{
namespace graph
{
namespace dblp_loader
{
void load(graph::directed_graph<graph::dblp_node>& g, const std::string& prefix)
{
    // load nodes

    io::parser authors{prefix + "authors.txt", "\t\n"};
    while (authors.has_next())
        g.insert(graph::dblp_node{"author", authors.next()});

    node_id venue_offset{g.size() - 1}; // -1 because DBLP data is 1-based

    io::parser venues{prefix + "venues.txt", "\t\n"};
    while (venues.has_next())
        g.insert(graph::dblp_node{"venue", venues.next()});

    node_id paper_offset{g.size() - 1}; // -1 because DBLP data is 1-based

    io::parser papers{prefix + "papers.txt", "\t\n"};
    while (papers.has_next())
    {
        papers.next(); // paper year
        g.insert(graph::dblp_node{"paper", papers.next()});
    }

    node_id term_offset{g.size() - 1};

    io::parser terms{prefix + "terms.txt", "\n"};
    while (terms.has_next())
        g.insert(graph::dblp_node{"term", terms.next()});

    // load edges

    node_id source;
    node_id dest;

    std::ifstream paper_paper{prefix + "paper_paper.txt"};
    while (paper_paper >> source >> dest)
        g.add_edge(source + paper_offset, dest + paper_offset); // directed

    std::ifstream paper_author{prefix + "paper_author.txt"};
    while (paper_author >> source >> dest)
    {
        try
        {
            g.add_edge(source + paper_offset, node_id{dest - 1});
            // simulate undirected
            g.add_edge(node_id{dest - 1}, source + paper_offset);
        }
        catch (graph::directed_graph_exception& e)
        {
            // it seems there is one duplicate edge in the DBLP data
            std::cout << " > Warning: attempted to add duplicate edge: "
                      << source << " " << dest << std::endl;
        }
    }

    std::ifstream paper_venue{prefix + "paper_venue.txt"};
    while (paper_venue >> source >> dest)
    {
        g.add_edge(source + paper_offset, dest + venue_offset);
        // simulate undirected
        g.add_edge(dest + venue_offset, source + paper_offset);
    }

    std::ifstream paper_term{prefix + "paper_term.txt"};
    while (paper_term >> source >> dest)
    {
        g.add_edge(source + paper_offset, dest + term_offset);
        g.add_edge(dest + term_offset, source + paper_offset);
    }
}
}
}
}

#endif
