#pragma once

#include "jw_util/baseexception.h"

namespace config {

class IllegalConfigException : public jw_util::BaseException {
public:
    IllegalConfigException(const std::string &msg)
        : BaseException(msg)
    {}
};

}
