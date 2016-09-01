/**
 * @file printing.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_PRINTING_H_
#define META_PRINTING_H_

#include <iostream>
#include <string>

#include "meta/config.h"
#include "meta/logging/logger.h"

namespace meta
{
namespace printing
{

/**
 * @param number The string-encoded number
 * @return the parameter with commas added every thousandths place
 */
inline std::string add_commas(const std::string& number)
{
    std::string ret{""};
    size_t counter = 0;
    for (auto it = number.rbegin(); it != number.rend(); ++it, ++counter)
    {
        if (counter != 0 && counter != number.size() && counter % 3 == 0)
            ret = ',' + ret;
        ret = *it + ret;
    }

    return ret;
}

/**
 * @param str The string to turn green
 * @return the parameter string colored green
 */
inline std::string make_green(std::string str)
{
    return "\033[32m" + str + "\033[0m";
}

/**
 * @param str The string to turn red
 * @return the parameter string colored red
 */
inline std::string make_red(std::string str)
{
    return "\033[31m" + str + "\033[0m";
}

/**
 * @param str The string to turn bold
 * @return the parameter string bolded
 */
inline std::string make_bold(std::string str)
{
    return "\033[1m" + str + "\033[22m";
}

/**
 * Converts a number of bytes into a human-readable number.
 * @param num_bytes The number in bytes to convert
 * @return a human-readable string
 */
inline std::string bytes_to_units(double num_bytes)
{
    std::string units = "bytes";
    for (auto& u : {"KB", "MB", "GB", "TB"})
    {
        if (num_bytes >= 1024)
        {
            num_bytes /= 1024;
            units = u;
        }
    }

    num_bytes = static_cast<double>(static_cast<int>(num_bytes * 100)) / 100;
    return std::to_string(num_bytes) + " " + units;
}

/**
 * @param idx The current progress in the operation
 * @param max The maximum value of idx, when it is done
 * @param freq How often to write output to the terminal
 * @param prefix The text to show before the percentage
 */
inline void show_progress(size_t idx, size_t max, size_t freq,
                          const std::string& prefix = "")
{
    if (idx % freq == 0)
        LOG(progress) << prefix << static_cast<double>(idx) / max * 100
                      << "%    \r" << ENDLG;
}

/**
 * Ends output from a call to show_progess by displaying 100% completion.
 * @param prefix The text to show before the percentage
 */
inline void end_progress(const std::string& prefix)
{
    LOG(progress) << prefix << "100%         \n" << ENDLG;
    LOG(info) << prefix << "100%" << ENDLG;
}
}
}

#endif
