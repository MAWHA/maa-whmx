#include "FileLocksmithLibInterop.h"
#include "FileLocksmith.h"

namespace FileLocksmith {

std::vector<ProcessResult> find_processes_recursive(const std::wstring& path) {
    const auto                 result = ::find_processes_recursive({path});
    std::vector<ProcessResult> resp;
    for (auto item : result) {
        resp.push_back(ProcessResult{
            .name  = item.name,
            .pid   = item.pid,
            .user  = item.user,
            .files = item.files,
        });
    }
    return resp;
}

std::wstring pid_to_full_path(uint32_t pid) {
    return ::pid_to_full_path(pid);
}

} // namespace FileLocksmith
