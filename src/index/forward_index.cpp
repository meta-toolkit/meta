/**
 * @file forward_index.cpp
 * @author Sean Massung
 */

#include "index/chunk_handler.h"
#include "index/disk_index_impl.h"
#include "index/forward_index.h"
#include "index/inverted_index.h"
#include "index/postings_data.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "index/vocabulary_map.h"
#include "io/libsvm_parser.h"
#include "parallel/thread_pool.h"
#include "util/disk_vector.h"
#include "util/mapping.h"
#include "util/pimpl.tcc"
#include "util/shim.h"

namespace meta
{
namespace index
{

class forward_index::impl
{
  public:
    impl(forward_index* idx);

    /**
     * This function loads a disk index from its filesystem
     * representation.
     */
    void load_index();

    /**
     * This function initializes the forward index.
     * @param config_file The configuration file used to create the index
     */
    void create_index(const std::string& config_file);

    /**
      * Initializes this index's metadata structures.
      */
    void init_metadata();

    /**
     * @param config the configuration settings for this index
     */
    void create_libsvm_postings(const cpptoml::toml_group& config);

    /**
     */
    void create_libsvm_metadata();

    /**
     * @param config the configuration settings for this index
     */
    void uninvert(const cpptoml::toml_group& config);

    /**
     * @param config the configuration settings for this index
     */
    void create_uninverted_metadata(const cpptoml::toml_group& config);

    /**
     * @param config the configuration settings for this index
     * @return whether this index will be based off of a single
     * libsvm-formatted corpus file
     */
    bool is_libsvm_format(const cpptoml::toml_group& config) const;

    /**
     * Calculates which documents start at which bytes in the postings file.
     */
    void set_doc_byte_locations();

    /**
     * Converts postings.index into a libsvm formatted file
     */
    void compressed_postings_to_libsvm();

    /** the total number of unique terms if _term_id_mapping is unused */
    uint64_t _total_unique_terms;

    /** doc_id -> postings file byte location */
    util::optional<util::disk_vector<uint64_t>> _doc_byte_locations;

  private:
    forward_index* idx_;
};

forward_index::forward_index(const cpptoml::toml_group& config)
    : disk_index{config, *config.get_as<std::string>("forward-index")},
      fwd_impl_{this}
{
    /* nothing */
}

forward_index::impl::impl(forward_index* idx) : idx_{idx}
{
  /* nothing */
}

forward_index::forward_index(forward_index&&) = default;
forward_index::~forward_index() = default;
forward_index& forward_index::operator=(forward_index&&) = default;

std::string forward_index::liblinear_data(doc_id d_id) const
{
    if (d_id >= num_docs())
        throw forward_index_exception{"invalid doc_id in search_primary"};

    uint64_t begin = (*fwd_impl_->_doc_byte_locations)[d_id];
    uint64_t length = 0;
    while (impl_->postings()[begin + length] != '\n')
    {
        ++length; // TODO maybe save lengths as well?
        if (begin + length >= impl_->postings().size())
            throw forward_index_exception{"out of bounds!"};
    }

    return std::string{impl_->postings().begin() + begin, length};
}

void forward_index::load_index()
{
    LOG(info) << "Loading index from disk: " << index_name() << ENDLG;

    fwd_impl_->init_metadata();

    impl_->load_doc_id_mapping();
    impl_->load_postings();

    auto config = cpptoml::parse_file(index_name() + "/config.toml");
    if (!fwd_impl_->is_libsvm_format(config))
        impl_->load_term_id_mapping();

    impl_->load_label_id_mapping();

    std::ifstream unique_terms_file{index_name() + "/corpus.uniqueterms"};
    unique_terms_file >> fwd_impl_->_total_unique_terms;
}

void forward_index::create_index(const std::string& config_file)
{
    filesystem::copy_file(config_file, index_name() + "/config.toml");
    auto config = cpptoml::parse_file(index_name() + "/config.toml");

    // if the corpus is a single libsvm formatted file, then we are done;
    // otherwise, we will create an inverted index and the uninvert it
    if (fwd_impl_->is_libsvm_format(config))
    {
        LOG(info) << "Creating index from libsvm data: " << index_name()
                  << ENDLG;

        fwd_impl_->create_libsvm_postings(config);
        fwd_impl_->create_libsvm_metadata();
        impl_->save_label_id_mapping();
    }
    else
    {
        LOG(info) << "Creating index by uninverting: " << index_name() << ENDLG;
        make_index<inverted_index>(config_file);

        fwd_impl_->create_uninverted_metadata(config);
        impl_->load_label_id_mapping();
        fwd_impl_->uninvert(config);
        fwd_impl_->init_metadata();
        impl_->load_postings();
        fwd_impl_->set_doc_byte_locations();
        impl_->load_term_id_mapping();
        fwd_impl_->_total_unique_terms = impl_->total_unique_terms();
    }

    // now that the files are tokenized, we can create the string_list
    impl_->load_doc_id_mapping();

    std::ofstream unique_terms_file{index_name() + "/corpus.uniqueterms"};
    unique_terms_file << fwd_impl_->_total_unique_terms;

    LOG(info) << "Done creating index: " << index_name() << ENDLG;
}

void forward_index::impl::create_libsvm_postings(
    const cpptoml::toml_group& config)
{
    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw forward_index_exception{"prefix missing from configuration file"};

    auto dataset = config.get_as<std::string>("dataset");
    if (!dataset)
        throw forward_index_exception{
            "dataset missing from configuration file"};

    std::string existing_file =
        *prefix + "/" + *dataset + "/" + *dataset + ".dat";

    filesystem::copy_file(existing_file,
                          idx_->index_name() + idx_->impl_->files[POSTINGS]);

    init_metadata();

    // now, assign byte locations for libsvm doc starting points
    idx_->impl_->load_postings();
    set_doc_byte_locations();
}

void forward_index::impl::set_doc_byte_locations()
{
    doc_id d_id{0};
    uint8_t last_byte = '\n';
    for (uint64_t idx = 0; idx < idx_->impl_->postings().size(); ++idx)
    {
        if (last_byte == '\n')
        {
            (*_doc_byte_locations)[d_id] = idx;
            ++d_id;
        }
        last_byte = idx_->impl_->postings()[idx];
    }
}

void forward_index::impl::init_metadata()
{
    uint64_t num_docs = filesystem::num_lines(idx_->index_name() +
                                              idx_->impl_->files[POSTINGS]);
    idx_->impl_->initialize_metadata(num_docs);
    _doc_byte_locations = util::disk_vector<uint64_t>(
        idx_->index_name() + "/lexicon.index", num_docs);
}

void forward_index::impl::create_libsvm_metadata()
{
    _total_unique_terms = 0;

    doc_id d_id{0};
    std::ifstream in{idx_->index_name() + idx_->impl_->files[POSTINGS]};
    std::string line;
    auto docid_writer = idx_->impl_->make_doc_id_writer(idx_->num_docs());
    while (in.good())
    {
        std::getline(in, line);
        if (line.empty())
            break;

        class_label lbl = io::libsvm_parser::label(line);
        idx_->impl_->set_label(d_id, lbl);

        uint64_t num_unique = 0;
        uint64_t length = 0;
        for (const auto& count_pair : io::libsvm_parser::counts(line))
        {
            ++num_unique;
            if (count_pair.first > _total_unique_terms)
                _total_unique_terms = count_pair.first;
            length += static_cast<uint64_t>(count_pair.second); // TODO
        }

        docid_writer.insert(d_id, "[no path]");
        idx_->impl_->set_length(d_id, length);
        idx_->impl_->set_unique_terms(d_id, num_unique);

        ++d_id;
    }

    ++_total_unique_terms; // since we subtracted one from the ids earlier
}

void forward_index::impl::create_uninverted_metadata(
    const cpptoml::toml_group& config)
{
    auto files = {DOC_IDS_MAPPING,  DOC_IDS_MAPPING_INDEX,   DOC_SIZES,
                  DOC_LABELS,       DOC_UNIQUETERMS,         LABEL_IDS_MAPPING,
                  TERM_IDS_MAPPING, TERM_IDS_MAPPING_INVERSE};

    auto inv_name = *config.get_as<std::string>("inverted-index");
    for (const auto& file : files)
        filesystem::copy_file(inv_name + idx_->impl_->files[file],
                              idx_->index_name() + idx_->impl_->files[file]);
}

bool forward_index::impl::is_libsvm_format(const cpptoml::toml_group& config)
    const
{
    auto analyzers = config.get_group_array("analyzers")->array();
    if (analyzers.size() != 1)
        return false;

    auto method = analyzers[0]->get_as<std::string>("method");
    if (!method)
        throw forward_index_exception{"failed to find analyzer method"};

    return *method == "libsvm";
}

uint64_t forward_index::unique_terms() const
{
    return fwd_impl_->_total_unique_terms;
}

auto forward_index::search_primary(doc_id d_id)
    const -> std::shared_ptr<postings_data_type>
{
    auto pdata = std::make_shared<postings_data_type>(d_id);
    auto line = liblinear_data(d_id);
    pdata->set_counts(io::libsvm_parser::counts(line));
    return pdata;
}

void forward_index::impl::uninvert(const cpptoml::toml_group& config)
{
    auto inv_name = *cpptoml::get_as<std::string>(config, "inverted-index");
    io::compressed_file_reader inv_reader{inv_name +
                                              idx_->impl_->files[POSTINGS],
                                          io::default_compression_reader_func};

    term_id t_id{0};
    chunk_handler<forward_index> handler{idx_->index_name()};
    {
        auto producer = handler.make_producer();
        while (inv_reader.has_next())
        {
            inverted_pdata_type pdata{t_id};
            pdata.read_compressed(inv_reader);
            producer(pdata.primary_key(), pdata.counts());
            ++t_id;
        }
    }

    handler.merge_chunks();
    compressed_postings_to_libsvm();
}

void forward_index::impl::compressed_postings_to_libsvm()
{
    idx_->impl_->load_labels();

    auto filename = idx_->index_name() + idx_->impl_->files[POSTINGS];
    filesystem::rename_file(filename, filename + ".tmp");
    std::ofstream output{filename};
    io::compressed_file_reader input{filename + ".tmp",
                                     io::default_compression_reader_func};

    // read from input, write to output, changing doc_id to class_label for the
    // correct libsvm format
    index_pdata_type pdata;
    while (input >> pdata)
    {
        doc_id d_id = pdata.primary_key();
        pdata.set_primary_key(
            static_cast<doc_id>(idx_->impl_->doc_label_id(d_id)));
        pdata.write_libsvm(output);
    }

    filesystem::delete_file(filename + ".tmp");
}
}
}
