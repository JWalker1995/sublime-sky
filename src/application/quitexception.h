#pragma once

#include "jw_util/baseexception.h"

namespace application {

class QuitException : public jw_util::BaseException {
public:
    QuitException()
        : BaseException("Quitting normally")
    {}
};

}
