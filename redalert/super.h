//
// Copyright 2020 Electronic Arts Inc.
//
// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection

/* $Header: /CounterStrike/SUPER.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SUPER.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 07/28/95                                                     *
 *                                                                                             *
 *                  Last Update : July 28, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SUPER_H
#define SUPER_H

#include "ftimer.h"

#ifndef REMASTER_BUILD
// The support power panel is a gadget, and this header can be reached before gadget.h.
#include "gadget.h"
#endif

struct NoInitClass;

class SuperClass
{
public:
    SuperClass(NoInitClass const& x)
        : Control(x){};
    SuperClass(void)
        : Control(NoInitClass()){};
    SuperClass(int recharge,
               bool powered,
               VoxType charging = VOX_NONE,
               VoxType ready = VOX_NONE,
               VoxType impatient = VOX_NONE,
               VoxType suspend = VOX_NONE);

    bool Suspend(bool on);
    bool Enable(bool onetime = false, bool player = false, bool quiet = false);
    void Forced_Charge(bool player = false);
    bool AI(bool player = false);
    bool Remove(bool forced = false);
    void Impatient_Click(void) const;
    int Anim_Stage(void) const;
    bool Discharged(bool player);
    bool Is_Ready(void) const
    {
        return (IsReady);
    }
    bool Is_Present(void) const
    {
        return (IsPresent);
    }
    bool Is_One_Time(void) const
    {
        return (IsOneTime && IsPresent);
    }
    bool Is_Powered(void) const
    {
        return (IsPowered);
    }

    // Needed access to recharge times for tooltips - 2019/08/14 Jason Scott
    int Get_Recharge_Time() const
    {
        return (RechargeTime);
    };

private:
    bool Recharge(bool player = false);

    unsigned IsPowered : 1;
    unsigned IsPresent : 1;
    unsigned IsOneTime : 1;
    unsigned IsReady : 1;

    CDTimerClass<FrameTimerClass> Control;
    int OldStage;

    VoxType VoxRecharge;
    VoxType VoxCharging;
    VoxType VoxImpatient;
    VoxType VoxSuspend;
    int RechargeTime;

    // Needed to make ANIMATION_STAGES public so the animation frame numbers could be turned into progress
    // percentages - 2019/08/07 Jason Scott
public:
    enum
    {
        ANIMATION_STAGES = 54
    };
};

#ifndef REMASTER_BUILD

/*
**	Support powers are a UI area of their own. They are shown in a panel on the
**	left edge of the screen that is always visible, instead of scrolling away
**	in the production sidebar.
*/
class SupportPowerPanelClass
{
public:
    SupportPowerPanelClass(void);

    void One_Time(void);
    void Init_Clear(void);
    void AI(void);
    void Draw_It(void);
    void Flag_To_Redraw(void);

    /*
    **	The whole panel is a single gadget. It works out which power was
    **	clicked from the mouse position.
    */
    class ButtonClass : public GadgetClass
    {
    public:
        ButtonClass(void)
            : GadgetClass(0, 0, 0, 0, LEFTPRESS | LEFTRELEASE | LEFTHELD | LEFTUP | RIGHTPRESS)
        {
        }

    protected:
        virtual int Action(unsigned flags, KeyNumType& key);
    };

    static ButtonClass Button;

private:
    int Slot_At(int x, int y) const;
    void Layout(void);

    struct SlotClass
    {
        SpecialWeaponType Type;
        SuperClass* Power;
        int Stage;
    };

    SlotClass Slots[SPC_COUNT];
    int SlotCount;
    int SlotWidth;
    int SlotHeight;
    int Rows;
    int Columns;
    int PanelX;
    int PanelY;
    unsigned IsToRedraw : 1;
};

extern SupportPowerPanelClass SupportPanel;

#endif

#endif
