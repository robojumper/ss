#ifndef D_LYT_METER_H
#define D_LYT_METER_H
// TODO sort includes for vtable order
// clang-format off
#include <d/lyt/d2d.h>
#include <d/lyt/meter/d_lyt_meter.h>
#include <d/lyt/meter/d_lyt_meter_remocon_bg.h>
#include <d/lyt/meter/d_lyt_meter_rupy.h>
#include <d/lyt/meter/d_lyt_meter_ganbari_gauge.h>
#include <d/lyt/meter/d_lyt_meter_item_select.h>
#include <d/lyt/meter/d_lyt_meter_minus_btn.h>
// clang-format on

class dLytMeter1Button_c : dLytMeterBase {
private:
    STATE_FUNC_DECLARE(dLytMeter1Button_c, Wait);
    STATE_FUNC_DECLARE(dLytMeter1Button_c, ToUse);
    STATE_FUNC_DECLARE(dLytMeter1Button_c, ToUnuse);
    STATE_FUNC_DECLARE(dLytMeter1Button_c, Unuse);

    UI_STATE_MGR_DECLARE(dLytMeter1Button_c);
};

class dLytMeter2Button_c : dLytMeterBase {
private:
    STATE_FUNC_DECLARE(dLytMeter2Button_c, Wait);
    STATE_FUNC_DECLARE(dLytMeter2Button_c, ToUse);
    STATE_FUNC_DECLARE(dLytMeter2Button_c, ToUnuse);
    STATE_FUNC_DECLARE(dLytMeter2Button_c, Unuse);

    UI_STATE_MGR_DECLARE(dLytMeter2Button_c);
};

class dLytMeterParts_c {
public:
    dLytMeterParts_c() : mStateMgr(*this, sStateID::null) {}

private:
    STATE_FUNC_DECLARE(dLytMeterParts_c, Invisible);
    STATE_FUNC_DECLARE(dLytMeterParts_c, In);
    STATE_FUNC_DECLARE(dLytMeterParts_c, Visible);
    STATE_FUNC_DECLARE(dLytMeterParts_c, Out);

    UI_STATE_MGR_DECLARE(dLytMeterParts_c);
    u8 field_0x3C[0x10];
};

class dLytMeter_c {
public:
    dLytMeter_c();
    virtual ~dLytMeter_c() {}

    d2d::ResAccIf_c mResAcc;
    d2d::LytBase_c mLyt;
    d2d::AnmGroup_c mAnmGroups[34];
    u8 field_0x00C84[0x00C8C - 0x00C84];
    dLytMeterGanbariGauge_c mGanbariGauge;
    dLytMeterRupy_c mRupy;
    dLytMeterItemSelect_c mItemSelect;
    dLytMeterRemoconBg_c mRemoCon;
    dLytMeterMinusBtn_c mMinusBtn;
    LytMeterGroup mMeters;

    u8 field_0x00[0x11E64];

    /* 0x13000 */ LytMeterListNode mNodes[16];
    /* 0x13200 */ u8 field_0x13200[0x13204 - 0x13200];
    /* 0x13204 */ dLytMeterParts_c mParts[16];

    /* 0x136C4 */ mVec3_c mPosArray1[2];
    /* 0x136DC */ mVec3_c mPosArray2[7];
    /* 0x13730 */ mVec3_c mPos1;
    /* 0x1373C */ mVec3_c mPos2;
    /* 0x13748 */ u8 field_0x13748[0x137B4 - 0x13748];
    /* 0x137B4 */ mVec3_c mPos3;
};

#endif
