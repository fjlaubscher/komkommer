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

/* $Header: /CounterStrike/FINDPATH.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FINDPATH.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : May 25, 1995   [PWG]                                         *
 *                                                                                             *
 * The path is produced with an A* search over the cell grid. Each cell's                     *
 * cost and passability come from Passable_Cell, which also applies the                       *
 * caller's movement threshold and (for roundabout teams) threat avoidance.                   *
 * The search is bounded by the map itself and, when the destination is                       *
 * unreachable, still yields the closest reachable cell as a best-effort path.                *
 * The resulting command list is smoothed with Optimize_Moves.                                *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Find_Path -- Find a path from point a to point b.                                         *
 *   Find_Path_AStar -- A* search that fills a PathType command list.                          *
 *   Optimize_Moves -- Optimize the move list.                                                 *
 *   Safety_Point -- Find a safe cell near a destination.                                      *
 *   Passable_Cell -- Determines if a cell can be entered and its cost.                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"

/*
**	This is the marker to signify the end of the path list.
*/
#define END FACING_NONE

/*
**	Modify this macro so that given two cell values, it will return
**	a value between 0 and 7, with 0 being North and moving
**	clockwise (just like map degrees).
*/
#define CELL_FACING(a, b) Dir_Facing(::Direction((a), (b)))

/*
**	Maximum path commands that may be buffered. The A* search rebuilds a
**	reverse-order command list into a scratch buffer of this size.
*/
#define MAX_MLIST_SIZE   300
#define THREAT_THRESHOLD 5

/*
**	X/Y cell offsets for each of the eight facings (FACING_N .. FACING_NW).
**	These mirror the AdjacentCell table in const.cpp.
*/
static int const AStarDX[FACING_COUNT] = {0, 1, 1, 1, 0, -1, -1, -1};
static int const AStarDY[FACING_COUNT] = {-1, -1, 0, 1, 1, 1, 0, -1};

/*-------------------------------------------------------------------------*/
inline FacingType Opposite(FacingType face)
{
    return ((FacingType)(face ^ 4));
}

inline static FacingType Next_Direction(FacingType facing, FacingType dir)
{
    facing = facing + dir;
    return (facing);
}

/*=========================================================================*/
/* A* scratch state. All module private and sized to the map, so no         */
/* allocation occurs while searching.                                      */
/*=========================================================================*/
static int AStarG[MAP_CELL_TOTAL];             // Best known cost-to-reach.
static signed char AStarParent[MAP_CELL_TOTAL]; // FacingType entered from.
static unsigned char AStarState[MAP_CELL_TOTAL]; // 0 unseen, 1 open, 2 closed.
static CELL AStarHeap[MAP_CELL_TOTAL];         // Binary min-heap of open cells.
static short AStarPos[MAP_CELL_TOTAL];         // Heap index of each open cell, or -1.
static int AStarHeapSize;
static int AStarDestX; // Destination cell X (heuristic target).
static int AStarDestY; // Destination cell Y (heuristic target).

static CELL DestLocation;

static inline int AStar_Heuristic(CELL cell)
{
    int dx = Cell_X(cell) - AStarDestX;
    int dy = Cell_Y(cell) - AStarDestY;

    if (dx < 0)
        dx = -dx;
    if (dy < 0)
        dy = -dy;
    return (dx > dy) ? dx : dy;
}

static inline bool AStar_Less(CELL a, CELL b)
{
    int fa = AStarG[a] + AStar_Heuristic(a);
    int fb = AStarG[b] + AStar_Heuristic(b);

    if (fa != fb)
        return (fa < fb);
    return (AStarG[a] > AStarG[b]); // Prefer higher g (smaller h) on ties.
}

static void AStar_SiftUp(int i)
{
    CELL cell = AStarHeap[i];

    while (i > 0) {
        int parent = (i - 1) >> 1;

        if (!AStar_Less(cell, AStarHeap[parent]))
            break;
        AStarHeap[i] = AStarHeap[parent];
        AStarPos[AStarHeap[parent]] = (short)i;
        i = parent;
    }
    AStarHeap[i] = cell;
    AStarPos[cell] = (short)i;
}

static void AStar_SiftDown(int i)
{
    CELL cell = AStarHeap[i];

    for (;;) {
        int left = (i << 1) + 1;
        int right = left + 1;
        int best = i;

        if (left < AStarHeapSize && AStar_Less(AStarHeap[left], AStarHeap[best]))
            best = left;
        if (right < AStarHeapSize && AStar_Less(AStarHeap[right], AStarHeap[best]))
            best = right;
        if (best == i)
            break;
        AStarHeap[i] = AStarHeap[best];
        AStarPos[AStarHeap[best]] = (short)i;
        i = best;
    }
    AStarHeap[i] = cell;
    AStarPos[cell] = (short)i;
}

static void AStar_Push(CELL cell)
{
    AStarHeap[AStarHeapSize] = cell;
    AStarPos[cell] = (short)AStarHeapSize;
    AStarHeapSize++;
    AStar_SiftUp(AStarHeapSize - 1);
}

static CELL AStar_Pop(void)
{
    CELL top = AStarHeap[0];

    AStarHeapSize--;
    if (AStarHeapSize > 0) {
        AStarHeap[0] = AStarHeap[AStarHeapSize];
        AStarPos[AStarHeap[0]] = 0;
        AStar_SiftDown(0);
    }
    AStarPos[top] = -1;
    return (top);
}

static void AStar_DecreaseKey(CELL cell)
{
    AStar_SiftUp(AStarPos[cell]);
}

/***********************************************************************************************
 * Find_Path_AStar -- A* search that fills a PathType command list.                            *
 *                                                                                             *
 * INPUT:   source      -- Cell to path from.                                                  *
 *          dest        -- Cell to path toward.                                                *
 *          final_moves -- Buffer to receive the FacingType command list.                      *
 *          maxlen      -- Size (in FacingType entries) of final_moves.                        *
 *          threshhold  -- MoveType tolerance passed to Passable_Cell.                         *
 *          threat      -- Threat tolerance passed to Passable_Cell (-1 to ignore).            *
 *          path        -- PathType to fill with the resulting path.                           *
 *                                                                                             *
 * OUTPUT:  bool; True if a complete path to dest was found, false if the best                 *
 *          reachable cell is returned instead (a best-effort partial path).                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1991  CY : Created.                                                                 *
 *   11/10/2026  Komkommer : Replaced edge-following with A*.                                  *
 *=============================================================================================*/
bool FootClass::Find_Path_AStar(CELL source,
                                CELL dest,
                                FacingType* final_moves,
                                int maxlen,
                                MoveType threshhold,
                                int threat,
                                PathType* path)
{
    // Reserve room for the END terminator plus the optimizer's sentinel.
    const int max_moves = maxlen - 2;

    AStarDestX = Cell_X(dest);
    AStarDestY = Cell_Y(dest);

    memset(AStarState, 0, sizeof(AStarState));
    memset(AStarPos, 0xFF, sizeof(AStarPos));
    AStarHeapSize = 0;

    AStarG[source] = 0;
    AStarParent[source] = FACING_NONE;
    AStarState[source] = 1;
    AStar_Push(source);

    CELL best_cell = source;
    int best_h = AStar_Heuristic(source);
    bool reached = false;

    /*
    **	Impassable ground beside a diagonal step is a corridor wall (bridge,
    **	cliff gap, diagonal wall). An occupied passable cell still blocks the cut.
    */
    auto Blocks_Corner = [&](CELL side) -> bool {
        if (Passable_Cell(side, FACING_NONE, threat, threshhold)) {
            return false;
        }
        LandType land = Map[side].Land_Type();
        return (land != LAND_WATER && land != LAND_RIVER && land != LAND_ROCK && land != LAND_WALL);
    };

    while (AStarHeapSize > 0) {
        CELL cur = AStar_Pop();
        AStarState[cur] = 2;

        if (cur == dest) {
            reached = true;
            best_cell = dest;
            break;
        }

        int h = AStar_Heuristic(cur);
        if (h < best_h) {
            best_h = h;
            best_cell = cur;
        }

        int x = Cell_X(cur);
        int y = Cell_Y(cur);

        for (int dir = FACING_N; dir < FACING_COUNT; dir++) {
            int nx = x + AStarDX[dir];
            int ny = y + AStarDY[dir];

            if (nx < 0 || nx >= MAP_CELL_W || ny < 0 || ny >= MAP_CELL_H)
                continue;

            /*
            **	Never cut a corner through an occupied cell. Impassable ground
            **	beside the step does not disconnect a one-cell corridor.
            */
            if (dir & 1) {
                if (Blocks_Corner(XY_Cell(x + AStarDX[dir], y)) || Blocks_Corner(XY_Cell(x, y + AStarDY[dir]))) {
                    continue;
                }
            }

            CELL nxt = XY_Cell(nx, ny);
            int step = Passable_Cell(nxt, (FacingType)dir, threat, threshhold);

            if (step <= 0)
                continue;

            int ng = AStarG[cur] + step;
            unsigned char state = AStarState[nxt];

            if (state == 0) {
                AStarG[nxt] = ng;
                AStarParent[nxt] = (FacingType)dir;
                AStarState[nxt] = 1;
                AStar_Push(nxt);
            } else if (state == 1 && ng < AStarG[nxt]) {
                AStarG[nxt] = ng;
                AStarParent[nxt] = (FacingType)dir;
                AStar_DecreaseKey(nxt);
            }
            // state == 2 (closed): the heuristic is consistent, so no improvement.
        }
    }

    /*
    **	Walk the came-from chain back to the source, then reverse it into the
    **	caller's move buffer. If the destination was unreachable this still
    **	produces the best partial path (closest reachable cell).
    */
    FacingType reverse[MAX_MLIST_SIZE + 2];
    CELL cell = best_cell;
    int count = 0;

    while (cell != source && count < max_moves) {
        FacingType dir = (FacingType)AStarParent[cell];

        reverse[count++] = dir;
        cell = Adjacent_Cell(cell, Opposite(dir));
    }

    path->Start = source;
    path->Cost = AStarG[best_cell];
    path->Overlap = NULL;
    path->LastOverlap = -1;
    path->LastFixup = -1;
    path->Command = final_moves;
    path->Length = 0;
    for (int i = count - 1; i >= 0; i--) {
        final_moves[path->Length++] = reverse[i];
    }
    final_moves[path->Length++] = END;

    return (reached);
}

/***********************************************************************************************
 * Find_Path -- Find a path from point a to point b.                                           *
 *                                                                                             *
 * INPUT:      int source x,y, int destination x,y, char *final moves                          *
 *             array to store moves, int maximum moves we may attempt                          *
 *                                                                                             *
 * OUTPUT:     int number of moves it took (IMPOSSIBLE_MOVES if we could                       *
 *             not reach the destination                                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1991  CY : Created.                                                                 *
 *   11/10/2026  Komkommer : Replaced edge-following with A*.                                  *
 *=============================================================================================*/
PathType* FootClass::Find_Path(CELL dest, FacingType* final_moves, int maxlen, MoveType threshhold)
{
    CELL source = Coord_Cell(Coord); // Source expressed as cell.
    static PathType path;            // Main path control.

    /*
    **	If we have been provided an illegal place to store our final moves
    **	then forget it.
    */
    if (!final_moves)
        return (NULL);

    BStart(BENCH_FINDPATH);

    PathCount++;

    DestLocation = dest;

    /*
    **	Roundabout teams avoid high-threat cells. Start with the most
    **	cautious threat level and loosen it until a path is found. Other
    **	objects ignore threat entirely and only run a single search.
    */
    int unit_threat = -1;
    if (Team && Team->Class->IsRoundAbout) {
        unit_threat = (Team) ? Team->Risk : Risk();
    }

    int levels = (unit_threat != -1) ? 4 : 1;
    bool reached = false;

    for (int stage = 0; stage < levels; stage++) {
        int threat;

        if (unit_threat == -1) {
            threat = -1;
        } else {
            switch (stage) {
            case 0:
                threat = 0;
                break;

            case 1:
                threat = unit_threat >> 1;
                break;

            case 2:
                threat = (unit_threat >> 1) + unit_threat;
                break;

            default:
                threat = -1;
                break;
            }
        }

        reached = Find_Path_AStar(source, dest, final_moves, maxlen, threshhold, threat, &path);
        if (reached)
            break;
    }

    /*
    **	Optimize the move list but only necessary if
    **	diagonal moves are allowed.
    */
    Optimize_Moves(&path, threshhold);

    BEnd(BENCH_FINDPATH);

    return (&path);
}

/***********************************************************************************************
 * Optimize_Moves -- Optimize the move list.                                                   *
 *                                                                                             *
 * INPUT:      char *moves to optimize                                                         *
 *                                                                                             *
 * OUTPUT:     none (list is optimized)                                                        *
 *                                                                                             *
 * WARNINGS:   EMPTY moves are used to hold the place of eliminated                            *
 *             commands. Also, NEVER call this routine with a list that                        *
 *             contains illegal commands. The list MUST be terminated                          *
 *             with a EOL command                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1991  CY : Created.                                                                 *
 *   06/01/1992  JLB : Optimized and commented.                                                *
 *=============================================================================================*/
#define EMPTY (FacingType) - 2
int FootClass::Optimize_Moves(PathType* path, MoveType threshhold)
// int Optimize_Moves(PathType *path, int (*callback)(CELL, FacingType), int threshold)
{
    /*
    **	Facing command pair adjustment table. Compare the facing difference between
    **	the two commands. 0 means no optimization is possible. 3 means backtracking
    **	so eliminate both commands. Any other value adjusts the first command facing.
    */
    static FacingType _trans[FACING_COUNT] = {
        (FacingType)0,
        (FacingType)0,
        (FacingType)1,
        (FacingType)2,
        (FacingType)3,
        (FacingType)-2,
        (FacingType)-1,
        (FacingType)0,
    }; // Smoothing.

    FacingType *cmd1,  // Floating first command pointer.
        *cmd2,         // Floating second command pointer.
        newcmd;        // Calculated new optimized command.
    FacingType newdir; // Tentative new direction for smoothing.
    CELL cell;         // Working cell (as it moves along path).

    /*
    **	Abort if there is any illegal parameter.
    */
    if (!path || !path->Command)
        return (0);

    /*
    **	Optimization loop -- start scanning with the
    **	first pair of commands (if there are at least two
    **	in the command list).
    */
    path->Command[path->Length] = END; // Force end of list.

    if (path->Length == 0)
        return (0);

    cell = path->Start;
    if (path->Length > 1) {
        cmd2 = path->Command + 1;
        while (*cmd2 != END) {

            /*
            **	Set the cmd1 pointer to point to the valid command closest, but
            **	previous to cmd2. Be sure not to go previous to the head of the
            **	command list.
            */
            cmd1 = cmd2 - 1;
            while (*cmd1 == EMPTY && cmd1 != path->Command) {
                cmd1--;
            }

            /*
            **	If there isn't any valid previous command, then bump the
            **	cmd pointers to the next command pair and continue...
            */
            if (*cmd1 == EMPTY) {
                cmd2++;
                continue;
            }

            /*
            **	Fetch precalculated command change value. 0 means leave
            **	command set alone, 3 means backtrack and eliminate two
            **	commands. Any other value is new direction and eliminate
            **	one command.
            */
            newcmd = (FacingType)(*cmd2 - *cmd1);
            if (newcmd < FACING_N)
                newcmd = (FacingType)(newcmd + FACING_COUNT);
            newcmd = _trans[newcmd];

            /*
            **	Check for backtracking. If this occurs, then eliminate the
            **	two commands. This is the easiest optimization.
            */
            if (newcmd == FACING_SE) {
                *cmd1 = EMPTY;
                *cmd2++ = EMPTY;
                continue;
            }

            /*
            **	If an optimization code was found the process it. The command is a facing
            **	offset to more directly travel toward the immediate destination cell.
            */
            if (newcmd) {

                /*
                **	Optimizations differ when dealing with diagonals. Especially when dealing
                **	with diagonals of 90 degrees. In such a case, 90 degree optimizations can
                **	only be optimized if the intervening cell is passable. The distance travelled
                **	is the same, but the path is less circuitous.
                */
                if (*cmd1 & FACING_NE) {

                    /*
                    **	Diagonal optimizations are always only 45
                    **	degree adjustments.
                    */
                    newdir = Next_Direction(*cmd1, (newcmd < FACING_N) ? (FacingType)-1 : (FacingType)1);

                    /*
                    **	Diagonal 90 degree changes can be smoothed, although
                    **	the path isn't any shorter.
                    */
                    if (ABS((int)newcmd) == 1) {
                        if (Passable_Cell(Adjacent_Cell(cell, newdir), newdir, -1, threshhold)) {
                            *cmd2 = newdir;
                            *cmd1 = newdir;
                        }
                        // BOB 16.12.92
                        cell = Adjacent_Cell(cell, *cmd1);
                        cmd2++;
                        continue;
                    }
                } else {
                    newdir = Next_Direction(*cmd1, newcmd);
                }

                /*
                **	Shorten a 90° or 135° bend only when the shortcut cell is
                **	passable. Otherwise the drive rejects the step and drops the path.
                */
                if (!Passable_Cell(Adjacent_Cell(cell, newdir), newdir, -1, threshhold)) {
                    cell = Adjacent_Cell(cell, *cmd1);
                    cmd2++;
                    continue;
                }

                *cmd2 = newdir;
                *cmd1 = EMPTY;

                /*
                **	Backup what it thinks is the current cell.
                */
                while (*cmd1 == EMPTY && cmd1 != path->Command) {
                    cmd1--;
                }
                if (*cmd1 != EMPTY) {
                    cell = Adjacent_Cell(cell, Next_Direction(*cmd1, FACING_S));
                } else {
                    cell = path->Start;
                }
                continue;
            }

            /*
            **	Since we could not make an optimization, we move our
            **	head pointer forward.
            */
            cell = Adjacent_Cell(cell, *cmd1);
            cmd2++;
        }
    }

    /*
    **	Pack the command list to remove any EMPTY command entries.
    */
    cmd1 = path->Command;
    cmd2 = path->Command;
    cell = path->Start;
    path->Cost = 0;
    path->Length = 0;
    while (*cmd2 != END) {
        if (*cmd2 != EMPTY) {
            cell = Adjacent_Cell(cell, *cmd2);
            path->Cost += Passable_Cell(cell, *cmd2, -1, threshhold);
            path->Length++;
            *cmd1++ = *cmd2;
        }
        cmd2++;
    }
    path->Length++;
    *cmd1 = END;
    return (path->Length);
}

CELL FootClass::Safety_Point(CELL src, CELL dst, int start, int max)
{
    FacingType dir;
    CELL next;
    int lp;

    dir = (FacingType)(CELL_FACING(src, dst) ^ 4) - 1;

    /*
    ** Loop through the different acceptable distances.
    */
    for (int dist = start; dist < max; dist++) {

        /*
        ** Move to the starting location.
        */
        next = dst;

        for (lp = 0; lp < dist; lp++) {
            next = Adjacent_Cell(next, dir);
        }

        if (dir & 1) {
            /*
            ** If our direction is diagonal than we need to check
            ** only one side which is as long as both of the old sides
            ** together.
            */
            for (lp = 0; lp < dist << 1; lp++) {
                next = Adjacent_Cell(next, dir + 3);
                if (!Can_Enter_Cell(next)) {
                    return (next);
                }
            }
        } else {
            /*
            ** If our direction is not diagonal than we need to check two
            ** sides so that we are checking a corner like location.
            */
            for (lp = 0; lp < dist; lp++) {
                next = Adjacent_Cell(next, dir + 2);
                if (!Can_Enter_Cell(next)) {
                    return (next);
                }
            }

            for (lp = 0; lp < dist; lp++) {
                next = Adjacent_Cell(next, dir + 4);
                if (!Can_Enter_Cell(next)) {
                    return (next);
                }
            }
        }
    }
    return (-1);
}

int FootClass::Passable_Cell(CELL cell, FacingType face, int threat, MoveType threshhold)
{
    MoveType move = Can_Enter_Cell(cell, face);

    if (move < MOVE_MOVING_BLOCK && Distance(Cell_Coord(cell)) > 0x0100)
        threshhold = MOVE_MOVING_BLOCK;

    if (move > threshhold)
        return (0);

    if (Session.Type == GAME_NORMAL) {
        if (threat != -1) {
            if (::Distance(Cell_Coord(cell), Cell_Coord(DestLocation)) > (THREAT_THRESHOLD * CELL_LEPTON_W)) {
                //			if (Map.Cell_Distance(cell, DestLocation) > THREAT_THRESHOLD) {
                if (Map.Cell_Threat(cell, Owner()) > threat)
                    return (0);
            }
        }
    }

    static int _value[MOVE_COUNT] = {
        1,  //	MOVE_OK
        1,  //	MOVE_CLOAK
        3,  //	MOVE_MOVING_BLOCK
        8,  //	MOVE_DESTROYABLE
        10, //	MOVE_TEMP
        0   //	MOVE_NO
    };
    return (_value[move]);
}
