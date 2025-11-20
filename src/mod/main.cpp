#include "common.h"
#include "d/d_font_manager.h"
#include "d/d_sc_game.h"
#include "d/d_sc_title.h"
#include "d/snd/d_snd_3d_actor.h"
#include "d/snd/d_snd_actor.h"
#include "d/snd/d_snd_area_sound_effect_mgr.h"
#include "d/snd/d_snd_bgm_mgr.h"
#include "d/snd/d_snd_control_player_mgr.h"
#include "d/snd/d_snd_distant_sound_actor_pool.h"
#include "d/snd/d_snd_fi_vocal_mgr.h"
#include "d/snd/d_snd_mgr.h"
#include "d/snd/d_snd_player_mgr.h"
#include "d/snd/d_snd_sound.h"
#include "m/m_color.h"
#include "m/m_mtx.h"
#include "nw4r/math/math_types.h"
#include "nw4r/snd/snd_SoundArchivePlayer.h"
#include "nw4r/snd/snd_SoundHandle.h"
#include "nw4r/snd/snd_SoundPlayer.h"
#include "nw4r/ut/ut_Font.h"
#include "nw4r/ut/ut_TextWriterBase.h"
#include "sized_string.h"

void writeText(nw4r::ut::TextWriterBase<wchar_t> &writer, const wchar_t *text) {
    u32 old_cursor_pos_x = writer.GetCursorX();
    u32 old_cursor_pos_y = writer.GetCursorY();

    // Black background
    writer.SetTextColor(mColor(0, 0, 0, 0xFF), mColor(0, 0, 0, 0xFF));
    writer.UpdateVertexColor();

    writer.Print(text, wcslen(text));

    writer.SetCursor(old_cursor_pos_x, old_cursor_pos_y);

    // White foreground
    writer.SetTextColor(mColor(0xFF, 0xFF, 0xFF, 0xFF), mColor(0xFF, 0xFF, 0xFF, 0xFF));
    writer.UpdateVertexColor();

    writer.Print(text, wcslen(text));
}

void dumpPlayingSounds(
    nw4r::ut::TextWriterBase<wchar_t> &writer, nw4r::snd::SoundArchivePlayer *archivePlayer,
    nw4r::snd::SoundArchive *archive
) {
    for (u32 i = 0; i < archivePlayer->GetSoundPlayerCount(); i++) {
        nw4r::snd::SoundPlayer &player = archivePlayer->GetSoundPlayer(i);
        typedef nw4r::snd::detail::BasicSound::SoundPlayerPlayLinkList SoundList;

        SoundList &list = player.GetSoundList();
        for (SoundList::Iterator it = list.GetBeginIter(); it != list.GetEndIter(); ++it) {
            u32 soundId = it->GetId();
            const char *label = archive->GetSoundLabelString(soundId);
            SizedWString<128> str;
            str.sprintf(L"%d: %s\n", i, label);
            writeText(writer, str);
        }
    }
}

struct ForeachSoundPrinter {
    nw4r::ut::TextWriterBase<wchar_t> *mpWriter;
    ForeachSoundPrinter(nw4r::ut::TextWriterBase<wchar_t> *writer) : mpWriter(writer) {}
    void operator()(nw4r::snd::SoundHandle &handle) {
        const char *label = dSndMgr_c::GetInstance()->getArchive()->GetSoundLabelString(handle.GetId());
        SizedWString<128> str;
        str.sprintf(L"%s\n", label);
        writeText(*mpWriter, str);
    }
};

extern "C" void *lbl_80575DD8;
static u32 u1 = 0x39;
static u32 u2 = 0;
static s32 timer = 100;

extern "C" void fn_803998B0(void *fiMgr, s16 v1, s16 v2);

extern "C" void fn_802EE0B0() {
    nw4r::ut::TextWriterBase<wchar_t> writer;

    if (dScTitle_c::sInstance == nullptr && dScGame_c::GetInstance() == nullptr) {
        return;
    }

    const nw4r::ut::Font *fnt = dFontMng_c::getFont(0);
    writer.SetFont(*fnt);
    writer.SetFontSize(10.0f);
    writer.SetupGX();

    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

    mMtx_c posMtx;
    posMtx.transS(0.0f, 0.0f, 0.0f);

    nw4r::math::MTX44 proj;
    C_MTXOrtho(proj, 0.0f, 480.0f, 0.0f, 640.0f, 0.0f, 10.0f);
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    GXLoadPosMtxImm(posMtx, 0);
    GXSetCurrentMtx(0);

    /*
    writeText(writer, L"lbl_80575DD8:\n");
    if (lbl_80575DD8 != nullptr) {
        ForeachSoundPrinter p(&writer);
        for (int i = 0; i < 0x40; i++) {
            dSndSound_c *snd = (dSndSound_c *)(((char *)lbl_80575DD8) + 0x34 + (0x128 * i) + 0x18);
            if (snd->GetId() != -1) {
                const char *label = dSndMgr_c::GetInstance()->getArchive()->GetSoundLabelString(snd->GetId());
                SizedWString<128> str;
                str.sprintf(L"%s\n", label);
                writeText(writer, str);
            }
        }

        writeText(writer, L"--------\n");

        for (int i = 0; i < 0x40; i++) {
            dSndSound_c *snd = (dSndSound_c *)(((char *)lbl_80575DD8) + 0x4A34 + (0x134 * i) + 0x18);
            if (snd->GetId() != -1) {
                const char *label = dSndMgr_c::GetInstance()->getArchive()->GetSoundLabelString(snd->GetId());
                SizedWString<128> str;
                str.sprintf(L"%s\n", label);
                writeText(writer, str);
            }
        }
    }
    */

    /*
    writeText(writer, L"FANFARE_SOUND_MGR:\n");
    if (dSndBgmMgr_c::GetInstance() != nullptr) {
        ForeachSoundPrinter p(&writer);
        dSndActor_c *ac = (dSndActor_c *)(((char *)dSndBgmMgr_c::GetInstance()) + 0x2A0);
        ac->ForEachSound(p, false);
    }

    writeText(writer, L"dSndAreaSoundEffectMgr_c:\n");
    if (dSndAreaSoundEffectMgr_c::GetInstance() != nullptr) {
        ForeachSoundPrinter p(&writer);
        dSndActor_c *ac = (dSndActor_c *)(((char *)dSndAreaSoundEffectMgr_c::GetInstance()) + 0x334);
        ac->ForEachSound(p, false);
    }

    writeText(writer, L"dSndDistantSoundActorPool_c:\n");
    if (dSndDistantSoundActorPool_c::GetInstance() != nullptr) {
        ForeachSoundPrinter p(&writer);
        for (int i = 0; i < 0x40; i++) {
            dSnd3DActor_c *ac = (dSnd3DActor_c *)(((char *)dSndDistantSoundActorPool_c::GetInstance()) + 0x10 + (0x108 * i));
            // writeText(writer, L"--------\n");
            ac->ForEachSound(p, false);
        }
    }
    */

    /*
    writeText(writer, L"EGG:\n");

    if (dSndMgr_c::GetInstance() != nullptr) {
        dumpPlayingSounds(writer, dSndMgr_c::getPlayer(), dSndMgr_c::GetInstance()->getArchive());
    }

    writeText(writer, L"Other mgr:\n");

    if (dSndPlayerMgr_c::GetInstance() != nullptr) {
        dumpPlayingSounds(
            writer, &dSndPlayerMgr_c::GetInstance()->GetPlayer(), &dSndPlayerMgr_c::GetInstance()->GetArchive()
        );
    }
    */

    if (dScGame_c::GetInstance() == nullptr) {
        return;
    }

    if (timer > 0) {
        timer--;
        return;
    }
    timer = 30;


    SizedWString<128> str;
    str.sprintf(L"%d, %d", u1, u2);
    writeText(writer, str);
    fn_803998B0(dSndFiVocalMgr_c::GetInstance(), u1, u2);
    u2++;

    if (u2 == 9) {
        u2 = 0;
        u1++;
    }

    if (u1 == 0x39 + 0x14) {
        u1 = 0x39;
    }
}
