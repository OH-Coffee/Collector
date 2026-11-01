#include "plugins-func/controller/controller.hpp"

ret_code Controller::msg_handler_for_controller_stop(uint64_t, uint8_t *, size_t)
{
    this->exit(); return RC_OK;
}
