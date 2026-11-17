// set (nn::settings::ISettingsServer): the system's language codes and region. The game builds its
// text language from these (switchbrew, Settings services): it caches the code list with
// GetAvailableLanguageCodes2, then turns a nn::settings::Language enum into a LanguageCode with
// MakeLanguageCode. Both were stubbed to 0 before — an empty cache and an invalid code (u64 0) —
// which made the game fall back to its built-in default text (Japanese) whatever the selected
// language was.
#include <algorithm>
#include <cstring>
#include <iterator>

#include "ipc.h"
#include "platform.h"
#include "svc_am.h"

namespace nx {

namespace {

// The Switch's LanguageCode list, in nn::settings::Language enum order (switchbrew). The pre-4.0.0
// GetAvailableLanguageCodes only ever exposed the entries through LatinAmericanSpanish.
constexpr const char* kLanguageCodes[] = {
    "ja", "en-US", "fr", "de", "it", "es", "zh-CN", "ko", "nl", "pt", "ru", "zh-TW",
    "en-GB", "fr-CA", "es-419", "zh-Hans", "zh-Hant",
};
constexpr int kLegacyVisibleCodes = 15;

// nn::settings::Language enum value -> index into kLanguageCodes.
constexpr int kLanguageEnumToCode[] = {
    0,  // 0 Japanese
    1,  // 1 AmericanEnglish
    2,  // 2 French
    3,  // 3 German
    4,  // 4 Italian
    5,  // 5 Spanish
    6,  // 6 Chinese (zh-CN)
    7,  // 7 Korean
    8,  // 8 Dutch
    9,  // 9 Portuguese
    10, // 10 Russian
    11, // 11 Taiwanese (zh-TW)
    12, // 12 BritishEnglish
    13, // 13 CanadianFrench
    14, // 14 LatinAmericanSpanish
    15, // [4.0.0+] 15 SimplifiedChinese (zh-Hans)
    16, // [4.0.0+] 16 TraditionalChinese (zh-Hant)
    9,  // [10.1.0+] 17 BrazilianPortuguese: the Switch lists "pt" only
};

uint64_t code_u64(const char* code) {
    uint64_t v = 0;
    std::memcpy(&v, code, std::strlen(code));
    return v;
}

// Writes as many codes as fit, returns how many were written (the commands' s32 result).
int write_code_array(Buffer buf, int count) {
    if (!buf.addr) return 0;
    const int n = std::min(count, int(buf.size / sizeof(uint64_t)));
    for (int i = 0; i < n; i++) st<uint64_t>(buf.addr + i * sizeof(uint64_t), code_u64(kLanguageCodes[i]));
    return n;
}

// Debug logging of the language calls, deduplicated so repeated polls don't flood the log.
void log_call(const char* what, int arg, const char* result) {
    static const char* logged_what = nullptr;
    static int logged_arg = -1;
    static const char* logged_result = nullptr;
    if (logged_what != what || logged_arg != arg || logged_result != result) {
        log("set: %s(%d) -> %s", what, arg, result);
        logged_what = what;
        logged_arg = arg;
        logged_result = result;
    }
}

void settings(Service& s) {
    s.cmds[0] = [](IpcCtx& c) {  // GetLanguageCode: the current system language
        log_call("GetLanguageCode", 0, game_language_name());
        c.push<uint64_t>(game_language_code());
        return 0u;
    };
    s.cmds[1] = [](IpcCtx& c) {  // GetAvailableLanguageCodes (a type-0xA buffer)
        const int n = write_code_array(c.in_buf(0), kLegacyVisibleCodes);
        log_call("GetAvailableLanguageCodes", n, "codes");
        c.push<int32_t>(n);
        return 0u;
    };
    s.cmds[2] = [](IpcCtx& c) {  // [4.0.0+] MakeLanguageCode: a nn::settings::Language enum value
        const int language = c.arg<int32_t>(0);
        const int idx = language >= 0 && language < int(std::size(kLanguageEnumToCode))
                            ? kLanguageEnumToCode[language]
                            : 0;
        log_call("MakeLanguageCode", language, kLanguageCodes[idx]);
        c.push<uint64_t>(code_u64(kLanguageCodes[idx]));
        return 0u;
    };
    s.cmds[3] = [](IpcCtx& c) {  // GetAvailableLanguageCodeCount
        log_call("GetAvailableLanguageCodeCount", 0, "15");
        c.push<int32_t>(kLegacyVisibleCodes);
        return 0u;
    };
    s.cmds[4] = [](IpcCtx& c) {  // GetRegionCode: 1 is the Americas
        c.push<int32_t>(1);
        return 0u;
    };
    s.cmds[5] = [](IpcCtx& c) {  // [4.0.0+] GetAvailableLanguageCodes2 (a type-0x6 buffer)
        const int n = write_code_array(c.out_buf(0), int(std::size(kLanguageCodes)));
        log_call("GetAvailableLanguageCodes2", n, "codes");
        c.push<int32_t>(n);
        return 0u;
    };
    s.cmds[6] = [](IpcCtx& c) {  // [4.0.0+] GetAvailableLanguageCodeCount2
        log_call("GetAvailableLanguageCodeCount2", 0, "17");
        c.push<int32_t>(int(std::size(kLanguageCodes)));
        return 0u;
    };
}

}  // namespace

ServiceRegistrar r_set("set", [] {
    auto s = std::make_shared<Service>("set");
    settings(*s);
    return s;
});

}  // namespace nx
