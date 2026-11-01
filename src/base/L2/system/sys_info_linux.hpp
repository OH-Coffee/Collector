
#ifndef sys_info_linux_hpp
#define sys_info_linux_hpp

#include <functional>
#include <string>
#include <mntent.h>

void sys_mounts(const std::string &i_path, const std::function<bool(const struct mntent *)> &i_cb);

std::string sys_mount_dir_for_cgroup_cpu();

bool sys_is_mount_cgroup_cpu();
bool sys_is_enable_cgroup_cpu();

#endif // sys_info_linux_hpp
