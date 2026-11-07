#include "horror/InstallationGuard.hpp"

#include <windows.h>
#include <cstddef>
#include <cwchar>
#include <string>
#include <vector>

#ifndef HORROR_SEAL_PUBLIC_KEY_HEX
#error "Protected builds require a pinned Ed25519 authority public key."
#endif

extern "C" int horror_disk_seal_verify(
    const wchar_t* executable, std::size_t length,
    const char* publicKeyHex, std::size_t keyLength
);

namespace horror::security {
namespace {
constexpr char kAuthorityPublicKey[] = HORROR_SEAL_PUBLIC_KEY_HEX;
static_assert(sizeof(kAuthorityPublicKey) == 65,
              "Ed25519 public key must be exactly 64 hexadecimal characters");
}

bool isInstallationProbe(const wchar_t* arguments) {
    return arguments != nullptr &&
           std::wcscmp(arguments, L"--disk-seal-check") == 0;
}

InstallationDecision verifyCurrentInstallation() {
    std::vector<wchar_t> buffer(32768, L'\0');
    const DWORD length = GetModuleFileNameW(
        nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        return {30, L"No fue posible identificar el ejecutable de HorrorMe."};
    }

    // This function lives in statically linked Rust code inside the running
    // game process. The game itself must have the embedded Installation ID.
    const int result = horror_disk_seal_verify(
        buffer.data(), length,
        kAuthorityPublicKey, sizeof(kAuthorityPublicKey) - 1);

    if (result == 0) return {0, L"Instalación autorizada."};
    if (result == 21)
        return {result, L"Esta copia no tiene un Installation ID autorizado."};
    if (result == 22)
        return {result, L"El sello, licencia o pendrive no coinciden con la instalación."};
    return {result, L"No se pudo completar la validación de seguridad."};
}
} // namespace horror::security
