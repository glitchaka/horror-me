#pragma once
#include <string>

namespace horror::security {

// Kept separate from gameplay and rendering: authorization runs at startup.
struct InstallationDecision final {
    int code{};
    std::wstring explanation{};
};

InstallationDecision verifyCurrentInstallation();
bool isInstallationProbe(const wchar_t* arguments);

} // namespace horror::security
