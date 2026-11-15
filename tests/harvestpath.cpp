#include "common/harvestpath.h"

#include <cstdio>

// The harvester ore selection is shared by Tiberian Dawn and Red Alert. The
// walk that measures the travel distance to a cell stays in each game, because
// the two engines have their own passability rules. The rule that turns the
// list of ore cells into the one a harvester drives to is here, and these
// checks cover it: the nearest ore wins, ore that is a few steps farther
// along the path may be richer, and ore that is only close on the map but
// far to drive to loses.

namespace
{
    // Builds a list of ore cells from a table of cost and amount pairs.
    int Make_Cells(HarvestOre* cells, int const* costs, int const* amounts, int count)
    {
        for (int index = 0; index < count; index++) {
            cells[index].Cell = index;
            cells[index].Cost = costs[index];
            cells[index].Amount = amounts[index];
        }
        return (count);
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

    HarvestOre cells[HARVEST_ORE_CANDIDATES];

    // An empty list has nothing to pick.
    check(HarvestOre_Pick(cells, 0) == -1, "no ore means nothing to pick");

    // A single cell is picked.
    int const one_cost[1] = {4};
    int const one_amount[1] = {2};
    Make_Cells(cells, one_cost, one_amount, 1);
    check(HarvestOre_Pick(cells, 1) == 0, "the only ore cell is picked");

    // The nearest ore wins when the ore is the same.
    int const same_costs[3] = {5, 2, 9};
    int const same_amounts[3] = {3, 3, 3};
    Make_Cells(cells, same_costs, same_amounts, 3);
    check(HarvestOre_Pick(cells, 3) == 1, "the nearest ore wins when the amount is the same");

    // Richer ore a few steps farther along the path wins.
    int const rich_costs[3] = {2, 5, 30};
    int const rich_amounts[3] = {1, 4, 4};
    Make_Cells(cells, rich_costs, rich_amounts, 3);
    check(HarvestOre_Pick(cells, 3) == 1, "richer ore a few steps farther along the path wins");

    // Richer ore that is too far along the path loses.
    int const far_costs[3] = {2, 6, 30};
    int const far_amounts[3] = {1, 4, 4};
    Make_Cells(cells, far_costs, far_amounts, 3);
    check(HarvestOre_Pick(cells, 3) == 0, "richer ore beyond the tolerance loses");

    // The tolerance is the price of the richest ore next to the closest one.
    int const edge_costs[2] = {4, 4 + HARVEST_ORE_TOLERANCE};
    int const edge_amounts[2] = {1, 4};
    Make_Cells(cells, edge_costs, edge_amounts, 2);
    check(HarvestOre_Pick(cells, 2) == 1, "richer ore at the edge of the tolerance wins");
    int const over_costs[2] = {4, 4 + HARVEST_ORE_TOLERANCE + 1};
    int const over_amounts[2] = {1, 4};
    Make_Cells(cells, over_costs, over_amounts, 2);
    check(HarvestOre_Pick(cells, 2) == 0, "richer ore past the tolerance loses");

    // A tie on the amount goes to the cell that is closer.
    int const tie_costs[2] = {3, 5};
    int const tie_amounts[2] = {4, 4};
    Make_Cells(cells, tie_costs, tie_amounts, 2);
    check(HarvestOre_Pick(cells, 2) == 0, "a tie on the amount goes to the closer cell");

    // The order of the list does not change the answer.
    int const order_costs[4] = {30, 5, 2, 6};
    int const order_amounts[4] = {4, 4, 1, 4};
    Make_Cells(cells, order_costs, order_amounts, 4);
    int const pick = HarvestOre_Pick(cells, 4);
    check(pick == 1, "the richest ore within the tolerance of the nearest wins");

    // The same cells in another order pick the same ore.
    int const shuffled_costs[4] = {6, 2, 30, 5};
    int const shuffled_amounts[4] = {4, 1, 4, 4};
    HarvestOre shuffled[4];
    Make_Cells(shuffled, shuffled_costs, shuffled_amounts, 4);
    int const other = HarvestOre_Pick(shuffled, 4);
    bool const same_cost = (other >= 0) && (shuffled[other].Cost == cells[pick].Cost);
    bool const same_amount = (other >= 0) && (shuffled[other].Amount == cells[pick].Amount);
    check(same_cost && same_amount, "the order of the list does not change the answer");

    // Ore that the harvester cannot reach at all is not in the list, so a
    // cell that is far on the map but close to drive to still wins.
    int const reach_costs[2] = {2, 40};
    int const reach_amounts[2] = {1, 1};
    Make_Cells(cells, reach_costs, reach_amounts, 2);
    check(HarvestOre_Pick(cells, 2) == 0, "the cell that is closest to drive to wins");

    if (failures == 0) {
        std::printf("harvest path: all checks passed\n");
    }
    return (failures == 0) ? 0 : 1;
}
