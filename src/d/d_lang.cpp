#include "d/d_lang.h"

#include "rvl/SC/scapi.h"

u8 getCurrentLanguage1() {
    return getCurrentLanguage2();
}

u8 getCurrentLanguage2() {
    u8 lang = getCurrentLanguage();

#if BUILD_REGION_PAL
    if (lang == SC_LANG_NL) {
        lang = SC_LANG_EN;
    }
#endif

    return lang;
}

u8 getCurrentLanguage() {
#if BUILD_REGION_JP
    return D_LANG_JP;
#endif

    u8 lang = SCGetLanguage();
#if BUILD_REGION_NTSC
    if (lang != SC_LANG_FR && lang != SC_LANG_SP) {
        lang = SC_LANG_EN;
    }
#elif BUILD_REGION_PAL
    if (lang != SC_LANG_FR && lang != SC_LANG_SP && lang != SC_LANG_IT && lang != SC_LANG_NL && lang != SC_LANG_DE) {
        lang = SC_LANG_EN;
    }
#endif
    return lang;
}
