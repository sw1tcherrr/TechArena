#pragma once
#include <iosfwd>
#include <string>
#include <experimental/iterator>

namespace util {

    template<class Container>
    void print(std::ostream& os, Container& c,
                        std::string const& pref = "", std::string const& post = "", std::string const& sep = " ") {
        os << pref;
        std::copy(c.begin(), c.end(), std::experimental::ostream_joiner(os, sep));
        os << post;
    }

} // namespace util
