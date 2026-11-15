#include "common/wallline.h"

#include <cstdio>

// The wall-line walk is shared by Tiberian Dawn and Red Alert. It fills the
// cells of a run between two points. These checks cover the math that the
// two engines rely on: the length of a run, that it starts and ends where it
// was asked to, that every cell of it touches the cell before it, so the wall
// art connects the run into one wall, and that the run is the same whichever
// end it is laid from, so a player can join a run onto a wall from either
// side and get the same result.

namespace
{
    int Abs(int value)
    {
        return (value < 0) ? -value : value;
    }

    // The distance between two cells, counted in orthogonal steps.
    int Step_Distance(int x1, int y1, int x2, int y2)
    {
        return (Abs(x2 - x1) + Abs(y2 - y1));
    }
}

int main()
{
    int failures = 0;
    auto check = [&](bool condition, char const* message) {
        if (!condition) {
            std::fprintf(stderr, "%s\n", message);
            failures++;
        }
    };

    WallLinePoint points[WALL_LINE_MAX_SEGMENTS];

    // A run of one cell.
    check(WallLine_Walk(4, 7, 4, 7, points, WALL_LINE_MAX_SEGMENTS) == 1, "a run onto the same cell is one cell");
    check(points[0].X == 4 && points[0].Y == 7, "the same cell walk stays on that cell");

    // A horizontal run.
    check(WallLine_Walk(2, 5, 7, 5, points, WALL_LINE_MAX_SEGMENTS) == 6, "the horizontal walk fills every cell");
    bool horizontal_ok = true;
    for (int index = 0; index < 6; index++) {
        if (points[index].X != 2 + index || points[index].Y != 5) {
            horizontal_ok = false;
        }
    }
    check(horizontal_ok, "a horizontal run walks east one cell at a time");

    // A horizontal run the other way.
    check(WallLine_Walk(7, 5, 2, 5, points, WALL_LINE_MAX_SEGMENTS) == 6, "the reverse horizontal walk fills every cell");
    bool reverse_ok = true;
    for (int index = 0; index < 6; index++) {
        if (points[index].X != 7 - index || points[index].Y != 5) {
            reverse_ok = false;
        }
    }
    check(reverse_ok, "a horizontal run walks west one cell at a time");

    // A vertical run.
    check(WallLine_Walk(3, 1, 3, 6, points, WALL_LINE_MAX_SEGMENTS) == 6, "the vertical walk fills every cell");
    bool vertical_ok = true;
    for (int index = 0; index < 6; index++) {
        if (points[index].X != 3 || points[index].Y != 1 + index) {
            vertical_ok = false;
        }
    }
    check(vertical_ok, "a vertical run walks south one cell at a time");

    // A diagonal run is a staircase. It is longer than the diagonal, because
    // every cell of it is a wall the player pays for, but every cell touches
    // the cell before it so the wall art connects it.
    check(WallLine_Walk(0, 0, 5, 5, points, WALL_LINE_MAX_SEGMENTS) == 11, "the diagonal walk fills every cell");

    // Every run, of any shape, has to hold these properties.
    struct Run
    {
        int FromX, FromY, ToX, ToY;
    };
    Run runs[] = {
        {0, 0, 0, 0},
        {0, 0, 9, 0},
        {0, 0, 0, 9},
        {9, 9, 0, 9},
        {9, 9, 9, 0},
        {0, 0, 5, 5},
        {0, 0, 5, 3},
        {0, 0, 3, 5},
        {5, 5, 0, 0},
        {0, 9, 12, 2},
        {12, 2, 0, 9},
        {4, 4, 17, 23},
        {17, 23, 4, 4},
    };

    for (unsigned run = 0; run < sizeof(runs) / sizeof(runs[0]); run++) {
        int from_x = runs[run].FromX;
        int from_y = runs[run].FromY;
        int to_x = runs[run].ToX;
        int to_y = runs[run].ToY;
        int expected = Step_Distance(from_x, from_y, to_x, to_y) + 1;
        int count = WallLine_Walk(from_x, from_y, to_x, to_y, points, WALL_LINE_MAX_SEGMENTS);

        check(count == expected, "the run is the Manhattan distance plus one");

        if (count < 1 || count > WALL_LINE_MAX_SEGMENTS) {
            continue;
        }

        check(points[0].X == from_x && points[0].Y == from_y, "the run starts where it was asked to");
        check(points[count - 1].X == to_x && points[count - 1].Y == to_y, "the run ends where it was asked to");

        bool connected = true;
        bool unique = true;
        for (int index = 1; index < count; index++) {
            if (Step_Distance(points[index - 1].X, points[index - 1].Y, points[index].X, points[index].Y) != 1) {
                connected = false;
            }
        }
        for (int index = 0; index < count; index++) {
            for (int other = index + 1; other < count; other++) {
                if (points[index].X == points[other].X && points[index].Y == points[other].Y) {
                    unique = false;
                }
            }
        }
        check(connected, "every cell of the run touches the cell before it");
        check(unique, "no cell of the run is used twice");
    }

    // The walk back is the walk forward, reversed. A run that is laid from
    // the other end has to cover the same cells, or a player who joins a run
    // onto a wall from one side gets a different wall than from the other.
    check(WallLine_Walk(0, 0, 7, 4, points, WALL_LINE_MAX_SEGMENTS) == 12, "the shallow diagonal has the expected length");
    WallLinePoint back[WALL_LINE_MAX_SEGMENTS];
    WallLine_Walk(7, 4, 0, 0, back, WALL_LINE_MAX_SEGMENTS);
    bool mirrored = true;
    for (int index = 0; index < 12; index++) {
        if (back[index].X != points[11 - index].X || back[index].Y != points[11 - index].Y) {
            mirrored = false;
        }
    }
    check(mirrored, "walking a run backwards gives the same cells in reverse");

    // The same has to hold for every shape of run, not just the one above.
    // This is a sweep over every offset, because a single case is not enough
    // to catch a run that is only symmetric in some directions.
    bool sweep_ok = true;
    bool sweep_connected = true;
    bool sweep_unique = true;
    for (int from_x = -6; from_x <= 6; from_x++) {
        for (int from_y = -6; from_y <= 6; from_y++) {
            for (int span_x = 0; span_x <= 6; span_x++) {
                for (int span_y = 0; span_y <= 6; span_y++) {
                    int to_x = from_x + span_x;
                    int to_y = from_y + span_y;
                    if (from_x == to_x && from_y == to_y) {
                        continue;
                    }

                    int expected = Step_Distance(from_x, from_y, to_x, to_y) + 1;
                    int forward = WallLine_Walk(from_x, from_y, to_x, to_y, points, WALL_LINE_MAX_SEGMENTS);
                    int backward = WallLine_Walk(to_x, to_y, from_x, from_y, back, WALL_LINE_MAX_SEGMENTS);

                    if (forward != expected || backward != expected) {
                        sweep_ok = false;
                        continue;
                    }
                    if (points[0].X != from_x || points[0].Y != from_y ||
                        points[forward - 1].X != to_x || points[forward - 1].Y != to_y) {
                        sweep_ok = false;
                    }
                    if (back[0].X != to_x || back[0].Y != to_y ||
                        back[backward - 1].X != from_x || back[backward - 1].Y != from_y) {
                        sweep_ok = false;
                    }
                    for (int index = 0; index < forward; index++) {
                        if (back[index].X != points[forward - 1 - index].X ||
                            back[index].Y != points[forward - 1 - index].Y) {
                            sweep_ok = false;
                        }
                    }
                    for (int index = 1; index < forward; index++) {
                        if (Step_Distance(points[index - 1].X, points[index - 1].Y,
                                          points[index].X, points[index].Y) != 1) {
                            sweep_connected = false;
                        }
                    }
                    for (int index = 0; index < forward; index++) {
                        for (int other = index + 1; other < forward; other++) {
                            if (points[index].X == points[other].X && points[index].Y == points[other].Y) {
                                sweep_unique = false;
                            }
                        }
                    }
                }
            }
        }
    }
    check(sweep_ok, "every run is the same set of cells whichever end it is laid from");
    check(sweep_connected, "every cell of every run touches the cell before it");
    check(sweep_unique, "no cell of any run is used twice");

    // Asking for the count alone must not write anything.
    int count = WallLine_Walk(2, 2, 9, 9, 0, 0);
    check(count == 15, "the count is returned without a buffer");

    return failures != 0;
}
