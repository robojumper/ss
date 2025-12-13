#ifndef D_LANG_H
#define D_LANG_H

#include "rvl/SC.h"

// all of this is made up

enum dLang_e {
    D_LANG_JP = SC_LANG_JP,
    D_LANG_EN = SC_LANG_EN,
    D_LANG_DE = SC_LANG_DE,
    D_LANG_FR = SC_LANG_FR,
    D_LANG_SP = SC_LANG_SP,
    D_LANG_IT = SC_LANG_IT,
    D_LANG_NL = SC_LANG_NL,
    D_LANG_ZH_S = SC_LANG_ZH_S,
    D_LANG_ZH_T = SC_LANG_ZH_T,
    D_LANG_KR = SC_LANG_KR,
};

// Yes there are three...

u8 getCurrentLanguage1();
u8 getCurrentLanguage2();
u8 getCurrentLanguage();

#endif
