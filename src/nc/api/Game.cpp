#include <nc/api/Game.h>
//#include <nc/api/Logger.h>
#include <nc/Gloss.h>

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <sys/mman.h>

//auto ncLog = nc::Logger::getOrCreate("NC Game");
namespace nc::hook {
    bool vtable(const char* MCPE_LIB, const char* cls, int slot, void** outOrig, void* hookFn) {
        size_t rodataSize = 0;
        uintptr_t rodata = GlossGetLibSection(MCPE_LIB, ".rodata", &rodataSize);
        if (!rodata || !rodataSize) { /*ncLog->error("vhook: no .rodata for {}", cls); */return false; }
    
        auto scan = [](uintptr_t base, size_t sz, const void* pat, size_t plen) -> uintptr_t {
            auto* m = (const uint8_t*)base; auto* p = (const uint8_t*)pat;
            for (size_t i = 0; i+plen <= sz; ++i)
                if (memcmp(m+i, p, plen) == 0) return base+i;
            return 0;
        };
    
        uintptr_t zts = scan(rodata, rodataSize, cls, strlen(cls)+1);
        if (!zts) { /*ncLog->error("vhook: ZTS not found for {}", cls); */return false; }
    
        size_t drrSize = 0;
        uintptr_t drr = GlossGetLibSection(MCPE_LIB, ".data.rel.ro", &drrSize);
        if (!drr || !drrSize) { /*ncLog->error("vhook: no .data.rel.ro for {}", cls); */return false; }
    
        uintptr_t zti = 0;
        for (size_t i = 0; i+sizeof(uintptr_t) <= drrSize; i += sizeof(uintptr_t)) {
            if (*reinterpret_cast<uintptr_t*>(drr+i) == zts) { zti = drr+i-sizeof(uintptr_t); break; }
        }
        if (!zti) { /*ncLog->error("vhook: ZTI not found for {}", cls); */return false; }
    
        uintptr_t vtbl = 0;
        for (size_t i = 0; i+sizeof(uintptr_t) <= drrSize; i += sizeof(uintptr_t)) {
            if (*reinterpret_cast<uintptr_t*>(drr+i) == zti) { vtbl = drr+i+sizeof(uintptr_t); break; }
        }
        if (!vtbl) { /*ncLog->error("vhook: VTable not found for {}", cls); */return false; }
    
        void** vt = reinterpret_cast<void**>(vtbl);
        *outOrig = vt[slot];
        Unprotect(vtbl + slot*sizeof(void*), sizeof(void*));
        vt[slot] = hookFn;
        __builtin___clear_cache((char*)(vtbl+slot*sizeof(void*)), (char*)(vtbl+(slot+1)*sizeof(void*)));
        //ncLog->info("hooked {} slot[{}]", cls, slot);
        return true;
    }
}

namespace nc::patch {
    bool PatchMemory(void* addr, const void* data, size_t size) {
        uintptr_t page_start = (uintptr_t)addr & ~(4095UL);
        size_t page_size =
            ((uintptr_t)addr + size - page_start + 4095) & ~(4095UL);
    
        if (mprotect((void*)page_start, page_size,
                     PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
            return false;
        }
    
        memcpy(addr, data, size);
        __builtin___clear_cache((char*)addr, (char*)addr + size);
    
        mprotect((void*)page_start, page_size,
                 PROT_READ | PROT_EXEC);
    
        return true;
    }
    
    bool PatchBytes(uintptr_t addr, const uint8_t* data, size_t size) {
        return PatchMemory(reinterpret_cast<void*>(addr), data, size);
    }
}