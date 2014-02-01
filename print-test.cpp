#include <thread>
#include "logging/logger.h"
#include "util/progress.h"
#include "util/range.h"

int main()
{
    using namespace meta;

    logging::set_cerr_logging();
    printing::progress progress{"Reticulating Splines: ", 10000};

    for (const auto& i : util::range(0, 10000))
    {
        progress(i);
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }
}
