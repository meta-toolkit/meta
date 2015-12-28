/**
 * @file transformer.cpp
 * @author Chase Geigle
 */

#include <unicode/translit.h>

#include "detail.h"
#include "meta/utf/transformer.h"
#include "meta/util/pimpl.tcc"

namespace meta
{
namespace utf
{

/**
 * Implementation class for the transformer.
 */
class transformer::impl
{
  public:
    /**
     * Constructs a new impl.
     * @param id The ICU id for the internal Transliterator to be created
     */
    impl(const std::string& id)
    {
        icu_handle::get();
        auto icu_id = icu::UnicodeString::fromUTF8(id);
        auto status = U_ZERO_ERROR;
        translit_.reset(icu::Transliterator::createInstance(
            icu_id, UTRANS_FORWARD, status));
        if (!translit_ || !U_SUCCESS(status))
            throw std::runtime_error{"failed to create transformer"};
    }

    /**
     * Copy constructs an impl
     * @param other The impl to copy
     */
    impl(const impl& other) : translit_{other.translit_->clone()}
    {
        // nothing
    }

    /// Defaulted move constructor
    impl(impl&&) = default;

    /**
     * Converts a string using the internal Transliterator.
     * @param str The string to convert
     * @return the converted string, encoded in utf8
     */
    std::string convert(const std::string& str)
    {
        auto icu_str = icu::UnicodeString::fromUTF8(str);
        translit_->transliterate(icu_str);
        return icu_to_u8str(icu_str);
    }

  private:
    /// A pointer to the internal Transliterator
    std::unique_ptr<icu::Transliterator> translit_;
};

transformer::transformer(const std::string& id) : impl_{id}
{
    icu_handle::get();
}

transformer::transformer(const transformer& other) : impl_{*other.impl_}
{
    // nothing
}

transformer::transformer(transformer&&) = default;

transformer::~transformer() = default;

std::string transformer::operator()(const std::string& str)
{
    return impl_->convert(str);
}

std::string transform(const std::string& str, const std::string& id)
{
    icu_handle::get();
    transformer trans{id};
    return trans(str);
}
}
}
