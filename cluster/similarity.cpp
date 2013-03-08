/**
 * @file similarity.cpp
 */

template <class Key, class Value>
double Similarity::cosine_similarity(const unordered_map<Key, Value> & a,
                                     const unordered_map<Key, Value> & b)
{
    using namespace internal;

    unordered_set<Key> space = get_space(a, b);
    double numerator = 0.0;
    for(auto & key: space)
        numerator += safe_at(a, key) * safe_at(b, key);

    double denominator = magnitude(a) * magnitude(b);
    return numerator / denominator;
}

template <class Key, class Value>
double Similarity::jaccard_similarity(const unordered_map<Key, Value> & a,
                                      const unordered_map<Key, Value> & b)
{
    double in_both = 0.0;
    for(auto & p: a)
    {
        if(b.find(p.first) != b.end())
            ++in_both;
    }

    return in_both / internal::get_space(a, b).size();
}

template <class Key, class Value>
Value Similarity::internal::safe_at(const unordered_map<Key, Value> & map, const Key & key)
{
    auto it = map.find(key);
    if(it == map.end())
        return Key();
    return it->second;
}

template <class Key, class Value>
std::unordered_set<Key> Similarity::internal::get_space(const unordered_map<Key, Value> & a,
                                                        const unordered_map<Key, Value> & b)
{
    unordered_set<Key> space;
    for(auto & p: a)
        space.insert(p.first);
    for(auto & p: b)
        space.insert(p.first);
    return space;
}

template <class Key, class Value>
double Similarity::internal::magnitude(const unordered_map<Key, Value> & map)
{
    double sum = 0.0;
    for(auto & p: map)
        sum += p.second * p.second;
    return sqrt(sum);
}
