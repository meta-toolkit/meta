/**
 * @file okapi_bm25.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 *
 * @author Sean Massung
 */

#ifndef _OKAPI_BM25_H_
#define _OKAPI_BM25_H_

#include "index/ranker/ranker.h"

namespace meta
{
namespace index
{

/**
 * The Okapi BM25 scoring function.
 */
class okapi_bm25 : public ranker
{
  public:
    /**
     * @param k1
     * @param b
     * @param k3
     */
    okapi_bm25(double k1 = 1.2, double b = 0.75, double k3 = 500.0);

    /**
     * @param sd
     */
    double score_one(const score_data& sd) override;

  private:
    const double _k1;
    const double _b;
    const double _k3;
};
}
}

#endif
