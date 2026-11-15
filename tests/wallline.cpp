#include "common/wallline.h"

#include <cstdio>

// The wall-line walk is shared by Tiberian Dawn and Red Alert. It fills the
// cells of a run between two points. These checks cover the math that the
// two engines rely on: the length of a run, that it starts and ends where it
// was asked to, and that every cell of it touches the cell before it, so the
// wall art connects the run into one wall.

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
    check(WallLine_Count(4, 7, 4, 7) == 1, "a run onto the same cell is one cell");
    check(WallLine_Walk(4, 7, 4, 7, points, WALL_LINE_MAX_SEGMENTS) == 1, "the same cell walk is one cell");
    check(points[0].X == 4 && points[0].Y == 7, "the same cell walk stays on that cell");

    // A horizontal run.
    check(WallLine_Count(2, 5, 7, 5) == 6, "a horizontal run is its length plus one");
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
    check(WallLine_Count(3, 1, 3, 6) == 6, "a vertical run is its length plus one");
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
    check(WallLine_Count(0, 0, 5, 5) == 11, "a diagonal run is the Manhattan distance plus one");
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
        check(WallLine_Count(from_x, from_y, to_x, to_y) == expected, "the count agrees with the walk");

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

    // The walk back is the walk forward, reversed.
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

    // Asking for the count alone must not write anything.
    int count = WallLine_Walk(2, 2, 9, 9, 0, 0);
    check(count == 15, "the count is returned without a buffer");

    // The path search routes a run around whatever stands in the way.
    WallLinePoint path[64];

    auto open = [](int, int) { return (true); };
    count = WallLine_Path(32, 32, 2, 2, 6, 6, open, path, 64);
    check(count == 9, "the path over open ground is the shortest one");
    check(path[0].X == 2 && path[0].Y == 2, "the path starts where it was asked to");
    check(path[count - 1].X == 6 && path[count - 1].Y == 6, "the path ends where it was asked to");
    bool path_connected = true;
    for (int index = 1; index < count; index++) {
        if (Step_Distance(path[index - 1].X, path[index - 1].Y, path[index].X, path[index].Y) != 1) {
            path_connected = false;
        }
    }
    check(path_connected, "every cell of the path touches the cell before it");

    // A wall across the straight line is routed around.
    auto wall_in_the_way = [](int x, int y) { return (!(x == 4 && y >= 2 && y <= 6)); };
    count = WallLine_Path(32, 32, 2, 2, 6, 6, wall_in_the_way, path, 64);
    check(count > 9, "the path around a wall is longer than the straight line");
    bool path_around = true;
    for (int index = 0; index < count; index++) {
        if (path[index].X == 4 && path[index].Y >= 2 && path[index].Y <= 6) {
            path_around = false;
        }
    }
    check(path_around, "the path goes around the wall");
    check(path[0].X == 2 && path[0].Y == 2, "the detour starts where it was asked to");
    check(path[count - 1].X == 6 && path[count - 1].Y == 6, "the detour ends where it was asked to");

    // A goal that cannot be reached has no path.
    auto boxed_in = [](int x, int y) { return (x < 4 || x > 6 || y < 4 || y > 6); };
    check(WallLine_Path(32, 32, 2, 2, 6, 6, boxed_in, path, 64) == 0, "a goal that is walled off has no path");
    check(WallLine_Path(32, 32, 2, 2, 6, 6, [](int x, int y) { return (false); }, path, 64) == 0,
          "a goal that cannot be built on has no path");

    return failures != 0;
}
