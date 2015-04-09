/**
 * @file forward_index.cpp
 * @author Sean Massung
 */

#include "cpptoml.h"
#include "index/chunk_handler.h"
#include "index/disk_index_impl.h"
#include "index/forward_index.h"
#include "index/inverted_index.h"
#include "index/postings_file.h"
#include "index/postings_file_writer.h"
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

/**
 * Implementation of a forward_index.
 */
class forward_index::impl
{
  public:
    /**
     * Constructs an implementation based on a forward_index.
     */
    impl(forward_index* idx);

    /**
     * @param config the configuration settings for this index
     */
    void create_libsvm_postings(const cpptoml::table& config);

    /**
     * @param inv_idx The inverted index to uninvert
     */
    void uninvert(const inverted_index& inv_idx);

    /**
     * @param name The name of the inverted index to copy data from
     */
    void create_uninverted_metadata(const std::string& name);

    /**
     * @param config the configuration settings for this index
     * @return whether this index will be based off of a single
     * libsvm-formatted corpus file
     */
    bool is_libsvm_format(const cpptoml::table& config) const;

    /**
     * Compresses the postings file created by uninverting.
     * @param filename The file to compress
     * @param num_docs The number of documents in that file
     */
    void compress(const std::string& filename, uint64_t num_docs);

    /**
     * Loads the postings file.
     * @param filename The path to the postings file to load
     */
    void load_postings();

    /// the total number of unique terms if term_id_mapping_ is unused
    uint64_t total_unique_terms_;

    /// the postings file
    util::optional<postings_file<forward_index::primary_key_type,
                                 forward_index::secondary_key_type>> postings_;

  private:
    /// Pointer to the forward_index this is an implementation of
    forward_index* idx_;
};

forward_index::forward_index(const cpptoml::table& config)
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

bool forward_index::valid() const
{
    if (!filesystem::file_exists(index_name() + "/corpus.uniqueterms"))
    {
        LOG(info) << "Existing forward index detected as invalid; recreating"
                  << ENDLG;
        return false;
    }
    for (auto& f : impl_->files)
    {
        // this is not required if generated directly from libsvm data
        if (f == impl_->files[TERM_IDS_MAPPING]
            || f == impl_->files[TERM_IDS_MAPPING_INVERSE])
            continue;

        if (!filesystem::file_exists(index_name() + "/" + std::string{f}))
        {
            LOG(info)
                << "Existing forward index detected as invalid; recreating"
                << ENDLG;
            return false;
        }
    }
    return true;
}

std::string forward_index::liblinear_data(doc_id d_id) const
{
    if (d_id >= num_docs())
        throw forward_index_exception{"invalid doc_id in search_primary"};

    auto pdata = search_primary(d_id);
    std::stringstream out;

    out << lbl_id(d_id);
    for (const auto& count : pdata->counts())
        out << ' ' << (count.first + 1) << ':' << count.second;
    return out.str();
}

void forward_index::load_index()
{
    LOG(info) << "Loading index from disk: " << index_name() << ENDLG;

    impl_->initialize_metadata();
    impl_->load_doc_id_mapping();

    auto config = cpptoml::parse_file(index_name() + "/config.toml");
    if (!fwd_impl_->is_libsvm_format(config))
        impl_->load_term_id_mapping();

    impl_->load_label_id_mapping();
    fwd_impl_->load_postings();

    std::ifstream unique_terms_file{index_name() + "/corpus.uniqueterms"};
    unique_terms_file >> fwd_impl_->total_unique_terms_;
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
        impl_->save_label_id_mapping();
    }
    else
    {
        LOG(info) << "Creating index by uninverting: " << index_name() << ENDLG;
        {
            // Ensure all files are flushed before uninverting
            make_index<inverted_index>(config_file);
        }
        auto inv_idx = make_index<inverted_index>(config_file);

        fwd_impl_->create_uninverted_metadata(inv_idx->index_name());
        fwd_impl_->uninvert(*inv_idx);
        impl_->load_term_id_mapping();
        fwd_impl_->total_unique_terms_ = impl_->total_unique_terms();
    }

    impl_->load_label_id_mapping();
    fwd_impl_->load_postings();
    impl_->load_doc_id_mapping();
    impl_->initialize_metadata();

    {
        std::ofstream unique_terms_file{index_name() + "/corpus.uniqueterms"};
        unique_terms_file << fwd_impl_->total_unique_terms_;
    }

    assert(filesystem::file_exists(index_name() + "/corpus.uniqueterms"));

    LOG(info) << "Done creating index: " << index_name() << ENDLG;
}

void forward_index::impl::create_libsvm_postings(const cpptoml::table& config)
{
    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw forward_index_exception{"prefix missing from configuration file"};

    auto dataset = config.get_as<std::string>("dataset");
    if (!dataset)
        throw forward_index_exception{
            "dataset missing from configuration file"};

    auto libsvm_data = *prefix + "/" + *dataset + "/" + *dataset + ".dat";
    auto filename = idx_->index_name() + idx_->impl_->files[POSTINGS];

    uint64_t num_docs = filesystem::num_lines(libsvm_data);
    idx_->impl_->initialize_metadata(num_docs);

    total_unique_terms_ = 0;
    {
        postings_file_writer out{filename, num_docs};

        printing::progress progress{" > Creating postings from libsvm data: ",
                                    num_docs};
        doc_id d_id{0};
        std::ifstream input{libsvm_data};
        std::string line;
        auto docid_writer = idx_->impl_->make_doc_id_writer(num_docs);
        while (std::getline(input, line))
        {
            progress(d_id);

            auto lbl = io::libsvm_parser::label(line);
            idx_->impl_->set_label(d_id, lbl);

            uint64_t num_unique = 0;
            double length = 0;
            forward_index::postings_data_type pdata{d_id};

            auto counts = io::libsvm_parser::counts(line);
            for (const auto& count : counts)
            {
                ++num_unique;
                if (count.first > total_unique_terms_)
                    total_unique_terms_ = count.first;
                length += count.second;
            }

            pdata.set_counts(counts);
            out.write<double>(pdata);

            docid_writer.insert(d_id, "[no path]");
            idx_->impl_->set_length(d_id, static_cast<uint64_t>(length));
            idx_->impl_->set_unique_terms(d_id, num_unique);

            ++d_id;
        }

        // +1 since we subtracted one from each of the ids in the
        // libsvm_parser::counts() function
        ++total_unique_terms_;
    }

    LOG(info) << "Created compressed postings file ("
              << printing::bytes_to_units(filesystem::file_size(filename))
              << ")" << ENDLG;
}

void forward_index::impl::create_uninverted_metadata(const std::string& name)
{
    auto files = {DOC_IDS_MAPPING, DOC_IDS_MAPPING_INDEX, DOC_SIZES, DOC_LABELS,
                  DOC_UNIQUETERMS, LABEL_IDS_MAPPING, TERM_IDS_MAPPING,
                  TERM_IDS_MAPPING_INVERSE};

    for (const auto& file : files)
        filesystem::copy_file(name + idx_->impl_->files[file],
                              idx_->index_name() + idx_->impl_->files[file]);
}

bool forward_index::impl::is_libsvm_format(const cpptoml::table& config) const
{
    auto analyzers = config.get_table_array("analyzers")->get();
    if (analyzers.size() != 1)
        return false;

    auto method = analyzers[0]->get_as<std::string>("method");
    if (!method)
        throw forward_index_exception{"failed to find analyzer method"};

    return *method == "libsvm";
}

uint64_t forward_index::unique_terms() const
{
    return fwd_impl_->total_unique_terms_;
}

auto forward_index::search_primary(doc_id d_id) const
    -> std::shared_ptr<postings_data_type>
{
    return fwd_impl_->postings_->find<double>(d_id);
}

void forward_index::impl::uninvert(const inverted_index& inv_idx)
{
    io::compressed_file_reader inv_reader{inv_idx.index_name()
                                              + idx_->impl_->files[POSTINGS],
                                          io::default_compression_reader_func};

    term_id t_id{0};
    chunk_handler<forward_index> handler{idx_->index_name()};
    {
        auto producer = handler.make_producer();
        while (inv_reader.has_next())
        {
            inverted_pdata_type pdata{t_id};
            pdata.read_compressed<uint64_t>(inv_reader);
            producer(pdata.primary_key(), pdata.counts());
            ++t_id;
        }
    }

    handler.merge_chunks();
    compress(idx_->index_name() + idx_->impl_->files[POSTINGS],
             inv_idx.num_docs());
}

void forward_index::impl::compress(const std::string& filename,
                                   uint64_t num_docs)
{
    auto ucfilename = filename + ".uncompressed";
    filesystem::rename_file(filename, ucfilename);

    // create a scope to ensure the reader and writer close properly so we
    // can calculate the size of the compressed file and delete the
    // uncompressed version at the end
    {
        postings_file_writer out{filename, num_docs};

        forward_index::postings_data_type pdata;
        auto length = filesystem::file_size(ucfilename) * 8; // number of bits
        io::compressed_file_reader in{ucfilename,
                                      io::default_compression_reader_func};

        printing::progress progress{
            " > Compressing postings: ", length, 500, 8 * 1024 /* 1KB */
        };
        // note: we will be accessing pdata in sorted order
        while (in.has_next())
        {
            in >> pdata;
            progress(in.bit_location());
            out.write<double>(pdata);
        }
    }

    LOG(info) << "Created compressed postings file ("
              << printing::bytes_to_units(filesystem::file_size(filename))
              << ")" << ENDLG;

    filesystem::delete_file(ucfilename);
}

void forward_index::impl::load_postings()
{
    postings_ = {idx_->index_name() + idx_->impl_->files[POSTINGS]};
}
}
}
