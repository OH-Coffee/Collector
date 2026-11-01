
#ifndef pch_hpp
#define pch_hpp

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <list>
#include <map>
#include <queue>
#include <set>
#include <vector>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>

#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <unistd.h>

// #include <Helios/assert.h>
// #include <Helios/basic.h>
// #include <Helios/errors.h>
// #include <Helios/interface.h>
// #include <Helios/sstring.h>
// #include <Helios/smartPtr.h>
// #include <Helios/types.h>
// #include <Helios/user.h>
// #include <Helios/helper/lock_wrapper.h>

// #include "Cardinal/Message_Mac.h"

#if defined(OS_MACOS)
    #import <libproc.h>
    #import <bsm/libbsm.h>

    #import <Cocoa/Cocoa.h>
    #import <Foundation/Foundation.h>
    #import <NetworkExtension/NetworkExtension.h>
    #import <SystemExtensions/SystemExtensions.h>
#endif


#include "header/errcode.hpp"
#include "header/errcode.hpp"
// #include "public/macro_functions.hpp"


#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif /* pch_hpp */

