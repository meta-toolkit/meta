/**
 * @file xzstream.cpp
 * @author Chase Geigle
 *
 * Based heavily upon the examples in the xz repo.
 * @see
 * http://git.tukaani.org/?p=xz.git;a=blob;f=doc/examples/01_compress_easy.c
 * @see
 * http://git.tukaani.org/?p=xz.git;a=blob;f=doc/examples/02_decompress.c
 */

#include "meta/io/xzstream.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace io
{

namespace
{

void throw_if_error(lzma_ret code, std::string msg)
{
    switch (code)
    {
        case LZMA_OK:
        case LZMA_STREAM_END:
            return;
        case LZMA_MEM_ERROR:
            throw xz_exception{msg + ": Memory allocation failed", code};
        case LZMA_FORMAT_ERROR:
            throw xz_exception{msg + ": Input not in .xz format", code};
        case LZMA_OPTIONS_ERROR:
            throw xz_exception{msg + ": Unsupported compression options", code};
        case LZMA_DATA_ERROR:
            throw xz_exception{msg + ": Compressed file is corrupt", code};
        case LZMA_BUF_ERROR:
            throw xz_exception{
                msg + ": Compressed file is truncated or corrupt", code};
        case LZMA_UNSUPPORTED_CHECK:
            throw xz_exception{
                msg + ": Specified integrity check is not supported", code};
        default:
            throw xz_exception{msg + ": Unknown error", code};
    }
}
}

xzstreambuf::xzstreambuf(const char* filename, const char* openmode,
                         std::size_t buffer_size)
    : in_buffer_(buffer_size),
      out_buffer_(buffer_size),
      file_{std::fopen(filename, openmode)},
      bytes_read_{0}
{
    stream_ = LZMA_STREAM_INIT;
    action_ = LZMA_RUN;
    stream_.next_in = nullptr;
    stream_.avail_in = 0;

    if (file_ == nullptr)
    {
        stream_.next_out = nullptr;
        stream_.avail_out = 0;
        reading_ = true;
        return;
    }

    util::string_view mode{openmode};
    if (mode == "wb")
    {
        reading_ = false;
        setp(&in_buffer_.front(), &in_buffer_.back());
        throw_if_error(lzma_easy_encoder(&stream_, 6, LZMA_CHECK_CRC64),
                       "Failed to initialize encoder");
    }
    else if (mode == "rb")
    {
        auto end = &out_buffer_.back() + 1;
        setg(end, end, end);
        reading_ = true;

        throw_if_error(lzma_stream_decoder(
                           &stream_, std::numeric_limits<uint64_t>::max(), 0),
                       "Failed to initialize decoder");
    }
    else
    {
        throw std::runtime_error{"Unrecognized open mode"};
    }

    stream_.next_out = reinterpret_cast<uint8_t*>(&out_buffer_[0]);
    stream_.avail_out = out_buffer_.size();
}

xzstreambuf::~xzstreambuf()
{
    if (!reading_)
    {
        action_ = LZMA_FINISH;
        sync();
    }

    if (file_)
        fclose(file_);

    lzma_end(&stream_);
}

auto xzstreambuf::underflow() -> int_type
{
    if (gptr() && (gptr() < egptr()))
        return traits_type::to_int_type(*gptr());

    // keep decompressing until we fill the output buffer, reading input
    // from the internal file as needed
    lzma_ret ret;
    do
    {
        if (stream_.avail_in == 0 && !std::feof(file_))
        {
            stream_.next_in = reinterpret_cast<uint8_t*>(&in_buffer_[0]);
            stream_.avail_in = std::fread(&in_buffer_[0], sizeof(uint8_t),
                                          in_buffer_.size(), file_);
            bytes_read_ += stream_.avail_in;

            if (std::ferror(file_))
            {
                setg(&out_buffer_[0], &out_buffer_[0], &out_buffer_[0]);
                return traits_type::eof();
            }

            if (std::feof(file_))
            {
                action_ = LZMA_FINISH;
            }
        }

        ret = lzma_code(&stream_, action_);

        throw_if_error(ret, "Decoder error");
    } while (stream_.avail_out != 0 && ret != LZMA_STREAM_END);

    // on LZMA_STREAM_END, we might not have filled the entire buffer, so
    // compute the actual number of bytes we have in the get buffer
    auto bytes = out_buffer_.size() - stream_.avail_out;
    if (bytes > 0)
    {
        setg(&out_buffer_[0], &out_buffer_[0], &out_buffer_[0] + bytes);
        stream_.next_out = reinterpret_cast<uint8_t*>(&out_buffer_[0]);
        stream_.avail_out = out_buffer_.size();

        return traits_type::to_int_type(*gptr());
    }

    // if we get here, we must have exhausted both the input file and the
    // input buffer, so finally report EOF
    setg(&out_buffer_[0], &out_buffer_[0], &out_buffer_[0]);
    return traits_type::eof();
}

auto xzstreambuf::overflow(int_type ch) -> int_type
{
    if (ch != traits_type::eof())
    {
        *pptr() = traits_type::to_char_type(ch);
        pbump(1);
        if (sync() == 0)
            return ch;
    }

    return traits_type::eof();
}

int xzstreambuf::sync()
{
    auto bytes = pptr() - pbase();
    stream_.next_in = reinterpret_cast<uint8_t*>(pbase());
    stream_.avail_in = static_cast<std::size_t>(bytes);

    // Two cases:
    // 1. We are still compressing the file, in which case we should pump
    //    the loop until all of the available input bytes are consumed; or
    //
    // 2. We are done receiving input (action_ == LZMA_FINISH), in which
    //    case we should pump the loop until we get the LZMA_STREAM_END
    //    return code indicating that all input has been processed (note
    //    that processed != read, hence this second case).
    lzma_ret ret;
    do
    {
        ret = lzma_code(&stream_, action_);

        if (stream_.avail_out == 0 || ret == LZMA_STREAM_END)
        {
            auto size = out_buffer_.size() - stream_.avail_out;

            if (std::fwrite(&out_buffer_[0], sizeof(uint8_t), size, file_)
                != size)
                return -1;

            stream_.next_out = reinterpret_cast<uint8_t*>(&out_buffer_[0]);
            stream_.avail_out = out_buffer_.size();
        }

        throw_if_error(ret, "Encoder error");

    } while (stream_.avail_in > 0
             || (action_ == LZMA_FINISH && ret != LZMA_STREAM_END));

    if (bytes > 0)
        pbump(-static_cast<int>(bytes));

    return 0;
}

bool xzstreambuf::is_open() const
{
    return file_ != nullptr && !::ferror(file_);
}

uint64_t xzstreambuf::bytes_read() const
{
    return bytes_read_;
}
}
}
