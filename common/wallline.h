#ifndef WALLLINE_H
#define WALLLINE_H

// Shared wall-line walk for Tiberian Dawn and Red Alert.
//
// Include this from standalone game code only. The Remastered host owns its
// own placement UI and never lays a wall run.
//
// The walk is a straight line between two cells. A horizontal or vertical run
// is contiguous. A diagonal run is a staircase: the walk steps sideways and
// then forward, so every cell is orthogonally adjacent to the cell before it.
// The wall art only joins north, east, south and west neighbours, so a
// staircase renders as one continuous wall where a plain diagonal would be a
// row of separate posts. A staircase is also longer than the diagonal it
// replaces: every cell of it is a wall the player pays for.
//
// The walk works on plain cell coordinates. The two engines size their maps
// differently, so the game converts to and from its own CELL numbering.

enum
{
    // The widest map either engine can load is 128 cells on a side, so the
    // longest possible run is that far across and that far down, plus one.
    WALL_LINE_MAX_SEGMENTS = 257,

    // How far, in cells, a wall run may stretch from an already standing wall
    // to the cell being placed. Within this distance the run joins the two
    // together; beyond it a wall is laid down alone.
    WALL_LINE_STRETCH = 5
};

// One cell of a run.
struct WallLinePoint
{
    int X;
    int Y;
};

// The absolute value of a number.
inline int WallLine_Abs(int value)
{
    return ((value < 0) ? -value : value);
}

// Writes the cells of the run into 'out' and returns how many there are.
// 'out' may be null, or too small, to ask for the count only.
//
// The run always covers the same set of cells whichever end it is laid from:
// walking it backwards gives the same cells in the reverse order. That is
// what makes a run safe to plan from either side, and it is what the player
// expects when the wall they join onto is on the other side of the diagonal.
//
// How that is achieved:
//
//   - The line between the two ends is walked in one canonical direction,
//     which is decided by the geometry of the two ends and not by which end
//     the caller happens to start from. A run that is asked for the other way
//     round is walked in that canonical direction as well and then turned
//     around.
//   - Where the line runs diagonally, the corner cell is filled in so that
//     the run stays connected. Which of the two corner cells is used is
//     decided by the shape of the line and not by the direction of travel: a
//     line that runs more sideways bulges to the south, and a line that runs
//     more upright bulges to the east. That keeps the staircase on the same
//     side of the line whichever end it is laid from.
inline int WallLine_Walk(int from_x, int from_y, int to_x, int to_y, WallLinePoint* out, int max_points)
{
    int dx = to_x - from_x;
    int dy = to_y - from_y;
    int span_x = WallLine_Abs(dx);
    int span_y = WallLine_Abs(dy);

    /*
    **	Walk the line in the canonical direction, so that both ends of a run
    **	produce the same line.
    */
    bool reverse = false;
    int start_x = from_x;
    int start_y = from_y;
    int end_x = to_x;
    int end_y = to_y;

    if (dx < 0 || (dx == 0 && dy < 0)) {
        start_x = to_x;
        start_y = to_y;
        end_x = from_x;
        end_y = from_y;
        reverse = true;
    }

    int step_x = (end_x >= start_x) ? 1 : -1;
    int step_y = (end_y >= start_y) ? 1 : -1;
    bool more_sideways = (span_x >= span_y);

    /*
    **	Step along the line. Every step that runs diagonally leaves a corner
    **	behind, and that corner is filled in so that the run stays connected.
    */
    int total = span_x + span_y + 1;
    int count = 0;
    int err = span_x - span_y;
    int x = start_x;
    int y = start_y;

    while (true) {
        if (out && count < max_points) {
            int cell = reverse ? (total - 1 - count) : count;
            out[cell].X = x;
            out[cell].Y = y;
        }
        count++;

        if (x == end_x && y == end_y) {
            break;
        }

        int e2 = 2 * err;
        bool step_in_x = (e2 > -span_y);
        bool step_in_y = (e2 < span_x);
        if (step_in_x) {
            err -= span_y;
        }
        if (step_in_y) {
            err += span_x;
        }

        if (step_in_x && step_in_y) {
            /*
            **	The line runs diagonally here. The corner cell is filled in so
            **	that the run stays connected. It is the corner that lies on the
            **	south side of a sideways line, or the east side of an upright
            **	one, whichever end the run is laid from.
            **
            **	The line itself keeps stepping along its own path, so the corner
            **	is written out but is not where the line steps to.
            */
            int corner_x = x + step_x;
            int corner_y = y;
            int other_x = x;
            int other_y = y + step_y;

            if (more_sideways) {
                // The line runs more sideways, so it bulges to the south.
                if (corner_y < other_y) {
                    corner_x = other_x;
                    corner_y = other_y;
                }
            } else {
                // The line runs more upright, so it bulges to the east.
                if (corner_x < other_x) {
                    corner_x = other_x;
                    corner_y = other_y;
                }
            }

            if (out && count < max_points) {
                int cell = reverse ? (total - 1 - count) : count;
                out[cell].X = corner_x;
                out[cell].Y = corner_y;
            }
            count++;

            x += step_x;
            y += step_y;
        } else if (step_in_x) {
            x += step_x;
        } else if (step_in_y) {
            y += step_y;
        }
    }

    return (count);
}

#endif
