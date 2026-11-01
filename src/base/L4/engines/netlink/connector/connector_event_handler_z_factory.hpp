// 文件名中的'z'字母，并没有什么特殊意义
// 只是为了在IDE中查看文件时，能让工厂类排在最后
// 避免工厂类夹在了多个子类中间，影响阅读源码时的体验
// (当然，也可以用'a'字母让其排在最前，但我觉得"z factory"更好听一些)

#ifndef netlink_connector_event_handler_z_factory_hpp
#define netlink_connector_event_handler_z_factory_hpp

#include "engines/netlink/connector/connector_event_handler.hpp"

class ConnectorEventHandlerFactory
{
public:
    static ConnectorEventHandlerPtr create(EventType);
};

#endif // netlink_connector_event_handler_z_factory_hpp
