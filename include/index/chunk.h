/**
 * @file chunk.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _CHUNK_H_
#define _CHUNK_H_

#include <string>
#include "meta.h"

namespace meta {
namespace index {

/**
 * Represents a portion of the inverted_index's postings file. It is an
 * intermediate file mapping term_ids to term document information. The chunks
 * are sorted to enable efficient merging.
 */
class chunk
{
    public:
        /**
         * @param path The path to this chunk file on disk
         * @param size The size in bytes of this chunk file
         */
        chunk(const std::string & path, uint32_t size);

        /**
         * @param other The other chunk to compare with this one
         * @return whether this chunk is less than (has a smaller size than)
         * the parameter
         */
        bool operator<(const chunk & other) const;

        /**
         * @return the size of this postings file chunk in bytes
         */
        uint32_t size() const;

    private:

        std::string _path;
        uint32_t _size;
};

}
}

#endif
