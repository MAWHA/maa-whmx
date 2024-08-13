#pragma once

#include <vector>
#include <string>
#include <stdint.h>

namespace FileLocksmith {

struct ProcessResult {
    std::wstring              name;
    uint32_t                  pid;
    std::wstring              user;
    std::vector<std::wstring> files;
};

std::vector<ProcessResult> find_processes_recursive(const std::wstring& path);
std::wstring               pid_to_full_path(uint32_t pid);

} // namespace FileLocksmith
