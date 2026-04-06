#pragma once

#include <cstdint>
#include <nc/api/Macro.h>

#ifdef __cplusplus
namespace nc::hook {
#endif
CAPI bool vtable(const char* MCPE_LIB, const char* cls, int slot, void** outOrig, void* hookFn);
#ifdef __cplusplus
} // namespace nc::hook
#endif

#ifdef __cplusplus
namespace nc::patch {
#endif
CAPI bool PatchMemory(void* addr, const void* data, size_t size);
CAPI bool PatchBytes(uintptr_t addr, const uint8_t* data, size_t size);
#ifdef __cplusplus
} // namespace nc::patch
#endif