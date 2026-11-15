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

/* $Header: /CounterStrike/SUPER.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SUPER.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 07/28/95                                                     *
 *                                                                                             *
 *                  Last Update : October 11, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SuperClass::AI -- Process the super weapon AI.                                            *
 *   SuperClass::Anim_Stage -- Fetches the animation stage for this super weapon.              *
 *   SuperClass::Discharged -- Handles discharged action for special super weapon.             *
 *   SuperClass::Enable -- Enable this super special weapon.                                   *
 *   SuperClass::Forced_Charge -- Force the super weapon to full charge state.                 *
 *   SuperClass::Impatient_Click -- Called when player clicks on unfinished super weapon.      *
 *   SuperClass::Recharge -- Starts the special super weapon recharging.                       *
 *   SuperClass::Remove -- Removes super weapon availability.                                  *
 *   SuperClass::SuperClass -- Constructor for special super weapon objects.                   *
 *   SuperClass::Suspend -- Suspend the charging of the super weapon.                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"

/***********************************************************************************************
 * SuperClass::SuperClass -- Constructor for special super weapon objects.                     *
 *                                                                                             *
 *    This is the constructor for the super weapons.                                           *
 *                                                                                             *
 * INPUT:   recharge    -- The recharge delay time (in game frames).                           *
 *                                                                                             *
 *          charging    -- Voice to announce that the weapon is charging.                      *
 *                                                                                             *
 *          ready       -- Voice to announce that the weapon is fully charged.                 *
 *                                                                                             *
 *          impatient   -- Voice to announce current charging state.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
SuperClass::SuperClass(int recharge, bool powered, VoxType charging, VoxType ready, VoxType impatient, VoxType suspend)
    : IsPowered(powered)
    , IsPresent(false)
    , IsOneTime(false)
    , IsReady(false)
    , Control(0)
    , OldStage(-1)
    , VoxRecharge(ready)
    , VoxCharging(charging)
    , VoxImpatient(impatient)
    , VoxSuspend(suspend)
    , RechargeTime(recharge)
{
}

/***********************************************************************************************
 * SuperClass::Suspend -- Suspend the charging of the super weapon.                            *
 *                                                                                             *
 *    This will temporarily put on hold the charging of the special weapon. This might be the  *
 *    result of insufficient power.                                                            *
 *                                                                                             *
 * INPUT:   on -- Should the weapon charging be suspended? Else, it will unsuspend.            *
 *                                                                                             *
 * OUTPUT:  Was the weapon suspend state changed?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Suspend(bool on)
{
    if (IsPresent && !IsReady && !IsOneTime && on == Control.Is_Active()) {
        if (!on) {
            Control.Start();
        } else {
            Control.Stop();
        }
        //		if (on != IsSuspended) {
        //			if (on) {
        //				SuspendTime = Control;
        //			} else {
        //				Control = SuspendTime;
        //			}
        //			IsSuspended = on;
        return (true);
        //		}
    }
    return (false);
}

/***********************************************************************************************
 * SuperClass::Enable -- Enable this super special weapon.                                     *
 *                                                                                             *
 *    This routine is called when the special weapon needs to be activated. This is used for   *
 *    both the normal super weapons and the special one-time super weapons (from crates).      *
 *                                                                                             *
 * INPUT:   onetime  -- Is this a special one time super weapon?                               *
 *                                                                                             *
 *          player   -- Is this weapon for the player? If true, then there might be a voice    *
 *                      announcement of this weapon's availability.                            *
 *                                                                                             *
 *          quiet    -- Request that the weapon start in suspended state (quiet mode).         *
 *                                                                                             *
 * OUTPUT:  Was the special super weapon enabled? Failure might indicate that the weapon was   *
 *          already available.                                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Enable(bool onetime, bool player, bool quiet)
{
    if (!IsPresent) {
        IsPresent = true;
        IsOneTime = onetime;
        bool retval = Recharge(player && !quiet);
        if (quiet)
            Suspend(true);
        return (retval);
    }
    return (false);
}

/***********************************************************************************************
 * SuperClass::Remove -- Removes super weapon availability.                                    *
 *                                                                                             *
 *    Call this routine when the super weapon should be removed because of some outside        *
 *    force. For one time special super weapons, they can never be removed except as the       *
 *    result of discharging them.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Was the special weapon removed and disabled?                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Remove(bool forced)
{
    if (IsPresent && (!IsOneTime || forced)) {
        IsReady = false;
        IsPresent = false;
        return (true);
    }
    return (false);
}

/***********************************************************************************************
 * SuperClass::Recharge -- Starts the special super weapon recharging.                         *
 *                                                                                             *
 *    This routine is called when the special weapon is allowed to recharge. Suspension will   *
 *    be disabled and the animation process will begin.                                        *
 *                                                                                             *
 * INPUT:   player   -- Is this for a player owned super weapon? If so, then a voice           *
 *                      announcement might be in order.                                        *
 *                                                                                             *
 * OUTPUT:  Was the super weapon begun charging up?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Recharge(bool player)
{
    if (IsPresent && !IsReady) {
        //		IsSuspended = false;
        OldStage = -1;
        Control.Start();
        Control = RechargeTime;

#ifdef CHEAT_KEYS
        if (Special.IsSpeedBuild) {
            Control = 1;
        }
#endif

        if (player) {
            Speak(VoxCharging);
        }
        return (true);
    }
    return (false);
}

/***********************************************************************************************
 * Superclass::Discharged -- Handles discharged action for special super weapon.               *
 *                                                                                             *
 *    This routine should be called when the special super weapon has been discharged. The     *
 *    weapon will either begin charging anew or will be removed entirely -- depends on the     *
 *    one time flag for the weapon.                                                            *
 *                                                                                             *
 * INPUT:   player   -- Is this special weapon for the player? If so, then there might be a    *
 *                      voice announcement.                                                    *
 *                                                                                             *
 * OUTPUT:  Should the sidebar be reprocessed because the special weapon has been eliminated?  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::Discharged(bool player)
{
    if (Control.Is_Active() && IsPresent && IsReady) {
        IsReady = false;
        if (IsOneTime) {
            IsOneTime = false;
            return (Remove());
        } else {
            Recharge(player);
        }
    }
    return (false);
}

/***********************************************************************************************
 * SuperClass::AI -- Process the super weapon AI.                                              *
 *                                                                                             *
 *    This routine will process the charge up AI for this super weapon object. If the weapon   *
 *    has advanced far enough to change any sidebar graphic that might represent it, then      *
 *    "true" will be returned. Use this return value to intelligently update the sidebar.      *
 *                                                                                             *
 * INPUT:   player   -- Is this for the player? If it is and the weapon is now fully charged,  *
 *                      then this fully charged state will be announced to the player.         *
 *                                                                                             *
 * OUTPUT:  Was the weapon's state changed such that a sidebar graphic update will be          *
 *          necessary?                                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SuperClass::AI(bool player)
{
    if (IsPresent && !IsReady) {
        if (!Control.Is_Active()) {
            if (OldStage != -1) {
                OldStage = -1;
                return (true);
            }
        } else {
            if (Control == 0) {
                IsReady = true;
                if (player) {
                    Speak(VoxRecharge);
                }
                return (true);
            } else {
                if (Anim_Stage() != OldStage) {
                    OldStage = Anim_Stage();
                    return (true);
                }
            }
        }
    }
    return (false);
}

/***********************************************************************************************
 * SuperClass::Anim_Stage -- Fetches the animation stage for this super weapon.                *
 *                                                                                             *
 *    This will return the current animation stage for this super weapon. The value will be    *
 *    between zero (uncharged) to ANIMATION_STAGES (fully charged). Use this value to render   *
 *    the appropriate graphic on the sidebar.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current animation stage for this special super weapon powerup.    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *   10/11/1996 JLB : Doesn't show complete until really complete.                             *
 *=============================================================================================*/
int SuperClass::Anim_Stage(void) const
{
    if (IsPresent) {
        if (IsReady) {
            return (ANIMATION_STAGES);
        }
        //		int time = Control;
        //		if (IsSuspended) {
        //			time = SuspendTime;
        //		}

        int stage = ANIMATION_STAGES * fixed(RechargeTime - Control.Value(), RechargeTime);
        stage = min(stage, ANIMATION_STAGES - 1);
        return (stage);
    }
    return (0);
}

/***********************************************************************************************
 * SuperClass::Impatient_Click -- Called when player clicks on unfinished super weapon.        *
 *                                                                                             *
 *    This routine is called when the player clicks on the super weapon icon on the sidebar    *
 *    when the super weapon is not ready yet. This results in a voice message feedback to the  *
 *    player.                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SuperClass::Impatient_Click(void) const
{
    if (!Control.Is_Active()) {
        Speak(VoxSuspend);
    } else {
        Speak(VoxImpatient);
    }
}

/***********************************************************************************************
 * SuperClass::Forced_Charge -- Force the super weapon to full charge state.                   *
 *                                                                                             *
 *    This routine will force the special weapon to full charge state. Call it when the weapon *
 *    needs to be instantly charged. The airstrike (when it first becomes available) is a      *
 *    good example.                                                                            *
 *                                                                                             *
 * INPUT:   player   -- Is this for the player? If true, then the full charge state will be    *
 *                      announced.                                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SuperClass::Forced_Charge(bool player)
{
    if (IsPresent) {
        IsReady = true;
        Control.Start();
        Control = 0;
        //		IsSuspended = false;
        if (player) {
            Speak(VoxRecharge);
        }
    }
}

#ifndef REMASTER_BUILD

/*
**	Support power panel.
*/
SupportPowerPanelClass::ButtonClass SupportPowerPanelClass::Button;
SupportPowerPanelClass SupportPanel;

SupportPowerPanelClass::SupportPowerPanelClass(void)
    : SlotCount(0)
    , SlotWidth(0)
    , SlotHeight(0)
    , Rows(1)
    , Columns(1)
    , PanelX(0)
    , PanelY(0)
    , IsToRedraw(true)
{
    for (int index = 0; index < SPC_COUNT; index++) {
        Slots[index].Type = SPC_NONE;
        Slots[index].Power = NULL;
        Slots[index].Stage = -1;
    }
}

/***********************************************************************************************
 * SupportPowerPanelClass::One_Time -- Sets up the size of the panel slots.                      *
 *                                                                                             *
 *    The slots are the same size as the objects in the production sidebar, so the support      *
 *    power art is drawn at the same scale as it always was.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
void SupportPowerPanelClass::One_Time(void)
{
    SlotWidth = SidebarClass::StripClass::OBJECT_WIDTH * RESFACTOR;
    SlotHeight = SidebarClass::StripClass::OBJECT_HEIGHT * RESFACTOR;
    Layout();
}

/***********************************************************************************************
 * SupportPowerPanelClass::Init_Clear -- Clears the panel to a known state.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
void SupportPowerPanelClass::Init_Clear(void)
{
    SlotCount = 0;

    for (int index = 0; index < SPC_COUNT; index++) {
        Slots[index].Type = SPC_NONE;
        Slots[index].Power = NULL;
        Slots[index].Stage = -1;
    }

    Button.X = 0;
    Button.Y = 0;
    Button.Width = 0;
    Button.Height = 0;
    IsToRedraw = true;
}

/***********************************************************************************************
 * SupportPowerPanelClass::Layout -- Works out where the powers sit on the panel.                *
 *                                                                                             *
 *    The panel is a column on the left edge of the screen. If there are more powers than fit   *
 *    in one column, the panel grows to the right instead of off the bottom of the screen.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
void SupportPowerPanelClass::Layout(void)
{
    PanelX = 0;
    PanelY = Map.TacPixelY;

    int available = SeenBuff.Get_Height() - PanelY;
    Rows = (SlotHeight > 0) ? (available / SlotHeight) : 1;
    if (Rows < 1) {
        Rows = 1;
    }

    Columns = (SlotCount > 0) ? ((SlotCount + Rows - 1) / Rows) : 1;

    Button.X = PanelX;
    Button.Y = PanelY;
    Button.Width = Columns * SlotWidth;
    Button.Height = ((SlotCount < Rows) ? SlotCount : Rows) * SlotHeight;

    if (SlotCount == 0) {
        Button.Width = 0;
        Button.Height = 0;
    }
}

/***********************************************************************************************
 * SupportPowerPanelClass::AI -- Keeps the panel in step with the powers the player has.         *
 *                                                                                             *
 *    A power shows up in the panel as soon as it is available and disappears again when it     *
 *    is taken away. The charge of every power is watched as well, because the clock overlay    *
 *    that shows it has to be redrawn as the power recharges.                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
void SupportPowerPanelClass::AI(void)
{
    if (Debug_Map || PlayerPtr == NULL) {
        SlotCount = 0;
        Button.Width = 0;
        Button.Height = 0;
        return;
    }

    SuperClass* powers[SPC_COUNT];
    SpecialWeaponType types[SPC_COUNT];
    int count = 0;

    for (int type = SPC_FIRST; type < SPC_COUNT; type++) {
        if (PlayerPtr->SuperWeapon[type].Is_Present()) {
            powers[count] = &PlayerPtr->SuperWeapon[type];
            types[count] = SpecialWeaponType(type);
            count++;
        }
    }

    bool changed = (count != SlotCount);
    for (int index = 0; index < count && !changed; index++) {
        changed = (Slots[index].Type != types[index]) || (Slots[index].Power != powers[index]);
    }

    if (changed) {
        int oldcount = SlotCount;
        for (int index = 0; index < count; index++) {
            Slots[index].Type = types[index];
            Slots[index].Power = powers[index];
            Slots[index].Stage = -1;
        }
        SlotCount = count;
        IsToRedraw = true;

        if (count != oldcount) {
            /*
            **	The panel changed size, so the map has to be redrawn to clear
            **	the area the panel used to cover.
            */
            Map.Flag_To_Redraw(true);
        }
    }

    Layout();

    for (int index = 0; index < SlotCount; index++) {
        int stage = Slots[index].Power->Anim_Stage();
        if (stage != Slots[index].Stage) {
            Slots[index].Stage = stage;
            IsToRedraw = true;
        }
    }

    if (IsToRedraw) {
        IsToRedraw = false;
        Map.Flag_To_Redraw(false);
    }
}

/***********************************************************************************************
 * SupportPowerPanelClass::Paint_Over_Scrolled -- Erases the panel after the map scrolled.     *
 *                                                                                             *
 *    The panel is drawn on top of the tactical map, so the map scroll shifts the panel pixels   *
 *    along with the map. Those pixels have to be painted over, or the panel leaves a trail of   *
 *    copies behind it.                                                                         *
 *                                                                                             *
 * INPUT:   dx,dy -- The distance in pixels that the map was scrolled by.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
void SupportPowerPanelClass::Paint_Over_Scrolled(int dx, int dy)
{
    if (SlotCount == 0 || SlotWidth == 0 || SlotHeight == 0) {
        return;
    }

    int width = Columns * SlotWidth;
    int height = ((SlotCount < Rows) ? SlotCount : Rows) * SlotHeight;

    Paint_Over(PanelX + dx, PanelY + dy, width, height);

    /*
    **	The help text is drawn next to the panel, so it was scrolled as well.
    */
    int mousex = Get_Mouse_X();
    int mousey = Get_Mouse_Y();
    int text = Help_Text_At(mousex, mousey);
    if (text != TXT_NONE) {
        int textwidth = String_Pixel_Width(Text_String(text));
        Paint_Over(PanelX + width + dx, mousey + dy, textwidth + 2, FontHeight + 2);
    }
}

/***********************************************************************************************
 * SupportPowerPanelClass::Paint_Over -- Paints over a piece of the map and flags its cells.   *
 *                                                                                             *
 *    The area is painted black, because that is what a cell that is not on the map looks like.  *
 *    The cells underneath are flagged, so that the map is drawn over the black again.           *
 *                                                                                             *
 * INPUT:   x,y    -- The upper left corner of the area in screen coordinates.                   *
 *                                                                                             *
 *          width  -- The width of the area.                                                    *
 *                                                                                             *
 *          height -- The height of the area.                                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
void SupportPowerPanelClass::Paint_Over(int x, int y, int width, int height)
{
    /*
    **	Only the part of the area that is on the map has to be painted over.
    */
    int right = Map.TacPixelX + Lepton_To_Pixel(Map.TacLeptonWidth) - 1;
    int bottom = Map.TacPixelY + Lepton_To_Pixel(Map.TacLeptonHeight) - 1;

    if (x < Map.TacPixelX) {
        x = Map.TacPixelX;
    }
    if (y < Map.TacPixelY) {
        y = Map.TacPixelY;
    }
    if (x + width - 1 > right) {
        width = right - x + 1;
    }
    if (y + height - 1 > bottom) {
        height = bottom - y + 1;
    }
    if (width < 1 || height < 1) {
        return;
    }

    CELL ul = Map.Click_Cell_Calc(x, y);
    CELL lr = Map.Click_Cell_Calc(x + width - 1, y + height - 1);
    int x1, y1, x2, y2;

    if (ul == -1 || lr == -1) {
        return;
    }
    if (!Map.Coord_To_Pixel(Coord_Whole(Cell_Coord(ul)), x1, y1)
        || !Map.Coord_To_Pixel(Coord_Whole(Cell_Coord(lr)), x2, y2)) {
        return;
    }

    /*
    **	The terrain of a cell is drawn with an icon that is twice the size of a
    **	cell, so the painted over area has to be that big as well. Otherwise
    **	a stripe of black would be left behind.
    */
    HidPage.Fill_Rect(x1, y1, x2 + ICON_PIXEL_W, y2 + ICON_PIXEL_H, BLACK);

    for (int py = y1; py <= y2; py += CELL_PIXEL_H) {
        for (int px = x1; px <= x2; px += CELL_PIXEL_W) {
            CELL cell = Map.Click_Cell_Calc(px, py);
            if (cell != -1) {
                Map[cell].Redraw_Objects(true);
            }
        }
    }
}

/***********************************************************************************************
 * SupportPowerPanelClass::Help_Text_At -- Finds the help text of the power under the mouse.    *
 *                                                                                             *
 * INPUT:   x,y   -- The mouse position in screen coordinates.                                   *
 *                                                                                             *
 * OUTPUT:  The text number of the power under the mouse, or TXT_NONE if there is none.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
int SupportPowerPanelClass::Help_Text_At(int x, int y) const
{
    int slot = Slot_At(x, y);
    if (slot < 0) {
        return (TXT_NONE);
    }
    return (SpecialWeaponHelp[Slots[slot].Type]);
}

/***********************************************************************************************
 * SupportPowerPanelClass::Draw_It -- Renders the support power panel.                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
void SupportPowerPanelClass::Draw_It(void)
{
    if (SlotCount == 0 || Debug_Map) {
        return;
    }

    if (!LogicPage->Lock()) {
        return;
    }

    int width = Columns * SlotWidth;
    int height = ((SlotCount < Rows) ? SlotCount : Rows) * SlotHeight;

    /*
    **	Draw the panel background.
    */
    Draw_Box(PanelX, PanelY, width, height, BOXSTYLE_RAISED, true);

    for (int index = 0; index < SlotCount; index++) {
        int x = PanelX + ((index / Rows) * SlotWidth);
        int y = PanelY + ((index % Rows) * SlotHeight);
        SuperClass* power = Slots[index].Power;
        void const* cameo = Map.Column[0].Get_Special_Cameo(Slots[index].Type);

        if (cameo != NULL) {
            CC_Draw_Shape(cameo, 0, x, y, WINDOW_MAIN, SHAPE_NORMAL);
        }

        if (power->Is_Ready()) {

            /*
            **	The power is charged and waiting to be fired.
            */
            CC_Draw_Shape(ObjectTypeClass::PipShapes,
                          PIP_READY,
                          x + (17 * RESFACTOR),
                          y + (4 * RESFACTOR),
                          WINDOW_MAIN,
                          SHAPE_CENTER);
        } else {

            /*
            **	The clock shows how far the power has recharged.
            */
            CC_Draw_Shape(Map.Column[0].ClockShapes,
                          power->Anim_Stage() + 1,
                          x,
                          y,
                          WINDOW_MAIN,
                          SHAPE_NORMAL | SHAPE_GHOST,
                          NULL,
                          Map.Column[0].ClockTranslucentTable);
        }
    }

    LogicPage->Unlock();
}

/***********************************************************************************************
 * SupportPowerPanelClass::Slot_At -- Finds the power that is under the mouse.                  *
 *                                                                                             *
 * INPUT:   x,y   -- The mouse position in screen coordinates.                                   *
 *                                                                                             *
 * OUTPUT:  The index of the power under the mouse, or -1 if there is none.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
int SupportPowerPanelClass::Slot_At(int x, int y) const
{
    if (SlotCount == 0 || SlotWidth == 0 || SlotHeight == 0) {
        return (-1);
    }

    if (x < PanelX || y < PanelY) {
        return (-1);
    }

    int row = (y - PanelY) / SlotHeight;
    int column = (x - PanelX) / SlotWidth;

    if (column < 0 || column >= Columns || row < 0 || row >= Rows) {
        return (-1);
    }

    int slot = (column * Rows) + row;
    if (slot < 0 || slot >= SlotCount) {
        return (-1);
    }
    return (slot);
}

/***********************************************************************************************
 * SupportPowerPanelClass::Flag_To_Redraw -- Marks the panel as needing a redraw.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
void SupportPowerPanelClass::Flag_To_Redraw(void)
{
    IsToRedraw = true;
    Map.Flag_To_Redraw(false);
}

/***********************************************************************************************
 * SupportPowerPanelClass::ButtonClass::Action -- Handles the mouse over the support panel.     *
 *                                                                                             *
 *    A left click fires the power that is under the mouse. A right click leaves the targeting   *
 *    mode again.                                                                              *
 *                                                                                             *
 * INPUT:   flags -- The gadget event flags that triggered the call to this function.            *
 *                                                                                             *
 *          key   -- A reference to the keyboard event (if any).                               *
 *                                                                                             *
 * OUTPUT:  bool; Should processing be aborted on any succeeding buttons in the chain?         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    11/15/2026 : Created.                                                                    *
 *=============================================================================================*/
int SupportPowerPanelClass::ButtonClass::Action(unsigned flags, KeyNumType& key)
{
    int mousex;
    int mousey;

    /*
    **	For a button event the mouse position is held in the queue globals, because
    **	that is where the mouse was when the button was pressed.
    */
    if (flags & (LEFTPRESS | LEFTRELEASE | RIGHTPRESS | RIGHTRELEASE)) {
        mousex = Keyboard->MouseQX;
        mousey = Keyboard->MouseQY;
    } else {
        mousex = Get_Mouse_X();
        mousey = Get_Mouse_Y();
    }

    int slot = SupportPanel.Slot_At(mousex, mousey);
    if (slot < 0) {
        return (false);
    }

    /*
    **	The mouse is over the panel, not over the tactical map.
    */
    Map.Override_Mouse_Shape(MOUSE_NORMAL);

    SuperClass* power = SupportPanel.Slots[slot].Power;
    SpecialWeaponType type = SupportPanel.Slots[slot].Type;

    /*
    **	Show what this power is while the mouse is over it. The text is placed
    **	to the right of the panel, because help text that is drawn off the
    **	left edge of the tactical map can never be painted over again.
    */
    if (flags & LEFTUP) {
        RemapControlType* scheme = GadgetClass::Get_Color_Scheme();
        if (scheme != NULL && (unsigned)type < SPC_COUNT) {
            int textwidth = String_Pixel_Width(Text_String(SpecialWeaponHelp[type]));
            Map.Help_Text(SpecialWeaponHelp[type],
                          SupportPanel.PanelX + (SupportPanel.Columns * SupportPanel.SlotWidth) + textwidth,
                          mousey,
                          scheme->Color,
                          true);
        }
        flags &= ~LEFTUP;
    }

    /*
    **	A right mouse button press cancels the targeting mode.
    */
    if (flags & RIGHTPRESS) {
        Map.IsTargettingMode = SPC_NONE;
    }

    /*
    **	A left mouse button press fires the power, if it is ready.
    */
    if (flags & LEFTPRESS) {
        if ((unsigned)type < SPC_COUNT) {
            if (power->Is_Ready()) {
                if (type != SPC_SONAR_PULSE) {
                    Map.IsTargettingMode = type;
                    Unselect_All();
                    Speak(VOX_SELECT_TARGET);
                } else {
                    OutList.Add(EventClass(EventClass::SPECIAL_PLACE, SPC_SONAR_PULSE, 0));
                }
            } else {
                power->Impatient_Click();
            }
        }
    }

    /*
    **	The click was on the panel, so the tactical map must not act on it.
    */
    return (true);
}

#endif
