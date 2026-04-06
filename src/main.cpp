#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <nc/api/Game.h>
#include <nc/api/Config.h>
#include <nc/Gloss.h>
#include <nc/api/Logger.h>

auto logger_eu = nc::Logger::getOrCreate("EnchantUnbound");

Config& getConfig() {
    static Config cfg("EnchantUnbound", true);
    cfg.open("config.json");
    return cfg;
}

static int g_CompatibilityIDOffset{};

// For Enchant::isCompatibleWith TridentChannelingEnchant::isCompatibleWith TridentRiptideEnchant::isCompatibleWith CrossbowEnchant::isCompatibleWith
bool Enchant_isCompatibleWith(void* a1, uint8_t ID) {
	Config& cfg = getConfig();
    bool freedom = cfg.get<bool>("enchantment.freedom", false);
    if(freedom) return true;
    int CompatibilityID = *(int*)((uintptr_t)a1 + g_CompatibilityIDOffset);
    //logger_eu->i("CompatibilityID=%d 1stAnvilSlot=%u", CompatibilityID, ID);
    if (CompatibilityID == 2 && (ID == 16 || ID == 18)) {
        if (ID == 16) logger_eu->i("Blocked Silk Touch + Fortune!");
        if (ID == 18) logger_eu->i("Blocked Fortune + Silk Touch!");
        return false;
    }
    if ((CompatibilityID == 0 || CompatibilityID == 6) && (ID == 30 || ID == 31 || ID == 32)) {
        if (CompatibilityID == 0 && ID == 30) logger_eu->i("Blocked Riptide + Channeling!");
        if (CompatibilityID == 6 && ID == 32) logger_eu->i("Blocked Channeling + Riptide!");
        if (CompatibilityID == 6 && ID == 30) logger_eu->i("Blocked Riptide + Loyalty!");
        if (CompatibilityID == 6 && ID == 31) logger_eu->i("Blocked Loyalty + Riptide!");
        return false;
    }
    return true;
}

void** FindVtable(const char* cls) {
    static uintptr_t rodata{}, drr{};
    static size_t rodataSize{}, drrSize{};
    if (!rodata) {
        rodata = GlossGetLibSection("libminecraftpe.so", ".rodata", &rodataSize);
        drr = GlossGetLibSection("libminecraftpe.so", ".data.rel.ro", &drrSize);
    }
    char* s = (char*)memmem((void*)rodata, rodataSize, cls, strlen(cls) + 1);
    if (!s) return nullptr;
    uintptr_t zts = (uintptr_t)s, zti{};
    for (size_t i = 0; i < drrSize; i += sizeof(uintptr_t))
        if (*(uintptr_t*)(drr + i) == zts) {
            zti = drr + i - sizeof(uintptr_t);
            break;
        }
    if (!zti) return nullptr;
    for (size_t i = 0; i < drrSize; i += sizeof(uintptr_t))
        if (*(uintptr_t*)(drr + i) == zti)
            return (void**)(drr + i + sizeof(uintptr_t));
    return nullptr;
}

static int (*origin_ec_min)(void*, int) = nullptr;
static int (*origin_ec_max)(void*, int) = nullptr;
static int (*originMax)(void*) = nullptr;

int enchantCost_min(void* self, int level) {
	int originalVal = origin_ec_min(self, level);
	logger_eu->i("Level Min Enchant = {}", level);
	logger_eu->i("Cost Min Enchant = {}", originalVal);
	return originalVal;
}

int enchantCost_max(void* self, int level) {
	int originalVal = origin_ec_max(self, level);
	logger_eu->i("Level Max Enchant = {}", level);
	logger_eu->i("Cost Max Enchant = {}", originalVal);
	return originalVal;
}

int enchantMax(void* self) {
	int originalVal = originMax(self);
	logger_eu->i("Max Enchant = {}", originalVal);
	return originalVal;
}

bool vHook(const char* MCPE_LIB, const char* cls, int slot, void* hookFn, void** orig) {
	return nc::hook::vtable(MCPE_LIB, cls, slot, orig, hookFn);
}

void HookCostAndMaxVal() {
	vHook("libminecraftpe.so", "7Enchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min); //MinCost
	vHook("libminecraftpe.so", "7Enchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max); //MaxCost
	vHook("libminecraftpe.so", "7Enchant", 6, (void*)enchantMax, (void**)&originMax); //MaxLevel
	
	vHook("libminecraftpe.so", "10BowEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "10BowEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "10BowEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "13BreachEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "13BreachEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "13BreachEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "15CrossbowEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "15CrossbowEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "15CrossbowEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "19CurseBindingEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "19CurseBindingEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "19CurseBindingEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "21CurseVanishingEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "21CurseVanishingEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "21CurseVanishingEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "14DensityEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "14DensityEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "14DensityEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "14DiggingEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "14DiggingEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "14DiggingEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "14FishingEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "14FishingEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "14FishingEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "18FrostWalkerEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "18FrostWalkerEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "18FrostWalkerEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "11LootEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "11LootEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "11LootEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "12LungeEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "12LungeEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "12LungeEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "18MeleeWeaponEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "18MeleeWeaponEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "18MeleeWeaponEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "14MendingEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "14MendingEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "14MendingEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "17ProtectionEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "17ProtectionEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "17ProtectionEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "16SoulSpeedEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "16SoulSpeedEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "16SoulSpeedEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "17SwiftSneakEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "17SwiftSneakEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "17SwiftSneakEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "11SwimEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "11SwimEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "11SwimEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "24TridentChannelingEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "24TridentChannelingEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "24TridentChannelingEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "21TridentImpalerEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "21TridentImpalerEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "21TridentImpalerEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "21TridentLoyaltyEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "21TridentLoyaltyEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "21TridentLoyaltyEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "21TridentRiptideEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "21TridentRiptideEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "21TridentRiptideEnchant", 6, (void*)enchantMax, (void**)&originMax);
    
    vHook("libminecraftpe.so", "16WindBurstEnchant", 3, (void*)enchantCost_min, (void**)&origin_ec_min);
    vHook("libminecraftpe.so", "16WindBurstEnchant", 4, (void*)enchantCost_max, (void**)&origin_ec_max);
    vHook("libminecraftpe.so", "16WindBurstEnchant", 6, (void*)enchantMax, (void**)&originMax);
}

void HookCompatible() {
    size_t drrSize{};
    uintptr_t drr = GlossGetLibSection("libminecraftpe.so", ".data.rel.ro", &drrSize);
    uintptr_t end = drr + drrSize;
    int replaced{};
    auto Redirect = [&](const char* sym, std::initializer_list<int> idx, uintptr_t hook) {
        void** vt = FindVtable(sym);
        if (!vt) {
            logger_eu->i("%s not found", sym);
            return;
        }
        for (int i : idx) {
            uintptr_t func = (uintptr_t)vt[i];
            for (uintptr_t p = drr; p < end; p += sizeof(uintptr_t)) {
                uintptr_t* entry = (uintptr_t*)p;
                if (*entry == func) {
                    Unprotect((uintptr_t)entry, sizeof(uintptr_t));
                    *entry = hook;
                    replaced++;
                }
            }
        }
    };
    if (void** vt = FindVtable("14MendingEnchant")) {
        uintptr_t func = (uintptr_t)vt[2];
        g_CompatibilityIDOffset = ((*(uint32_t*)func >> 10) & 0xFFF) * 4;
        Redirect("14MendingEnchant", {2}, (uintptr_t)Enchant_isCompatibleWith);
    }
    Redirect("24TridentChannelingEnchant", {2}, (uintptr_t)Enchant_isCompatibleWith);
    Redirect("21TridentRiptideEnchant", {2}, (uintptr_t)Enchant_isCompatibleWith);
    Redirect("15CrossbowEnchant", {2}, (uintptr_t)Enchant_isCompatibleWith);
    logger_eu->i("redirected %d vtable references", replaced);
}

__attribute__((constructor))
void init() {
    logger_eu->i("EnchantUnbound Loaded");
    Config& cfg = getConfig();
    bool freedom = cfg.get<bool>("enchantment.freedom", false);
    cfg.set("enchantment.freedom", freedom);
    HookCompatible();
    HookCostAndMaxVal();
}
