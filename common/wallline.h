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

    // The number of cells of the biggest map either engine can load.
    WALL_LINE_MAX_CELLS = 16384
};

// One cell of a run.
struct WallLinePoint
{
    int X;
    int Y;
};

// How many cells a run from (from_x, from_y) to (to_x, to_y) covers. Every
// cell after the first is one orthogonal step, so a run is always the
// Manhattan distance between the two ends plus one.
inline int WallLine_Count(int from_x, int from_y, int to_x, int to_y)
{
    int dx = to_x - from_x;
    int dy = to_y - from_y;
    if (dx < 0) {
        dx = -dx;
    }
    if (dy < 0) {
        dy = -dy;
    }
    return (dx + dy + 1);
}

// Writes the cells of the run into 'out' and returns how many there are.
// 'out' may be null, or too small, to ask for the count only.
inline int WallLine_Walk(int from_x, int from_y, int to_x, int to_y, WallLinePoint* out, int max_points)
{
    int dx = to_x - from_x;
    int dy = to_y - from_y;
    int step_x = (dx < 0) ? -1 : 1;
    int step_y = (dy < 0) ? -1 : 1;
    if (dx < 0) {
        dx = -dx;
    }
    if (dy < 0) {
        dy = -dy;
    }

    // A plain diagonal would leave the wall pieces touching at a corner only.
    // Every diagonal step is split into two orthogonal steps instead, so the
    // run stays connected. The half that is further west comes first, which
    // keeps the staircase on one side of the line and makes the run between
    // two cells the same set of cells whichever end it is laid from.

    int total = dx + dy + 1;
    int count = 0;
    int x = from_x;
    int y = from_y;
    int err = dx - dy;
    int half_x = x;
    int half_y = y;
    bool half_pending = false;

    while (count < total) {
        if (out && count < max_points) {
            out[count].X = x;
            out[count].Y = y;
        }
        count++;
        if (count == total) {
            break;
        }

        if (half_pending) {
            // The second half of a split diagonal step.
            x = half_x;
            y = half_y;
            half_pending = false;
            continue;
        }

        int e2 = 2 * err;
        bool step_in_x = (e2 > -dy);
        bool step_in_y = (e2 < dx);
        if (step_in_x) {
            err -= dy;
        }
        if (step_in_y) {
            err += dx;
        }

        if (step_in_x && step_in_y) {
            if (step_x > 0) {
                // The run travels east, so the southern half comes first.
                y += step_y;
                half_x = x + step_x;
                half_y = y;
            } else {
                // The run travels west, so the western half comes first.
                x += step_x;
                half_x = x;
                half_y = y + step_y;
            }
            half_pending = true;
        } else if (step_in_x) {
            x += step_x;
        } else if (step_in_y) {
            y += step_y;
        }
    }

    return (total);
}

// Finds a path of cells from the start to the goal that avoids the cells the
// predicate rejects. The cells of the path are written into 'out', oldest
// first, and the number of them is returned. Zero is returned when there is
// no path.
//
// The search steps north, east, south and west, and every step costs the
// same, so the path it finds is a shortest one. It is the same path on every
// machine, because the map is the same on every machine.
//
// The predicate is called as passable(x, y) for the cells the search may
// step on. The start is always allowed, because a run starts on the cell it
// was anchored on, even when that cell already holds a wall.
template <class Passable>
int WallLine_Path(int map_width,
                  int map_height,
                  int start_x,
                  int start_y,
                  int goal_x,
                  int goal_y,
                  Passable passable,
                  WallLinePoint* out,
                  int max_points)
{
    // The cells the search has seen, and the direction each of them was
    // reached from. The queue holds the cells that still have to be looked at.
    static int queue[WALL_LINE_MAX_CELLS];
    static unsigned char came[WALL_LINE_MAX_CELLS];

    static const int step_x[4] = {0, 1, 0, -1};
    static const int step_y[4] = {-1, 0, 1, 0};

    if (map_width <= 0 || map_height <= 0 || (map_width * map_height) > WALL_LINE_MAX_CELLS) {
        return (0);
    }
    if (start_x < 0 || start_x >= map_width || start_y < 0 || start_y >= map_height) {
        return (0);
    }
    if (goal_x < 0 || goal_x >= map_width || goal_y < 0 || goal_y >= map_height) {
        return (0);
    }
    if (!passable(goal_x, goal_y)) {
        return (0);
    }

    for (int index = 0; index < (map_width * map_height); index++) {
        came[index] = 0;
    }

    int start = start_y * map_width + start_x;
    int goal = goal_y * map_width + goal_x;

    int head = 0;
    int tail = 0;
    queue[tail++] = start;
    came[start] = 5;

    while (head < tail && came[goal] == 0) {
        int cell = queue[head++];
        int x = cell % map_width;
        int y = cell / map_width;

        for (int direction = 0; direction < 4; direction++) {
            int next_x = x + step_x[direction];
            int next_y = y + step_y[direction];
            if (next_x < 0 || next_x >= map_width || next_y < 0 || next_y >= map_height) {
                continue;
            }

            int next = next_y * map_width + next_x;
            if (came[next] != 0) {
                continue;
            }
            if (!passable(next_x, next_y)) {
                continue;
            }

            /*
            **	Remember the way back to the cell the search came from.
            */
            came[next] = (unsigned char)(((direction + 2) & 3) + 1);
            queue[tail++] = next;
        }
    }

    if (came[goal] == 0) {
        return (0);
    }

    /*
    **	Walk the path back from the goal to the start, then turn it around.
    */
    int count = 0;
    int cell = goal;
    while (cell != start) {
        if (count >= max_points) {
            return (0);
        }
        out[count].X = cell % map_width;
        out[count].Y = cell / map_width;
        count++;

        int direction = (came[cell] - 1) & 3;
        cell += step_x[direction] + (step_y[direction] * map_width);
    }

    if (count >= max_points) {
        return (0);
    }
    out[count].X = start_x;
    out[count].Y = start_y;
    count++;

    for (int index = 0; index < (count / 2); index++) {
        WallLinePoint swap = out[index];
        out[index] = out[count - 1 - index];
        out[count - 1 - index] = swap;
    }

    return (count);
}

#endif
