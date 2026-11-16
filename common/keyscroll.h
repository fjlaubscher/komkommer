#ifndef KEYSCROLL_H
#define KEYSCROLL_H

// Hold-to-pan for the tactical map, shared by Tiberian Dawn and Red Alert.
//
// The view starts at the slowest step and speeds up while a direction is held,
// stopping at the fastest step the scroll-rate option allows. That is the same
// ramp the screen edge already uses: step 0 is the fastest and step 8 is the
// slowest, and the option's value plus one is the fastest step that may be
// used. Releasing the keys coasts back down. Two opposite keys are not a
// release: the view stops instead of coasting on.
//
// Directions are the DirType values both games already use, so the caller can
// hand one to Scroll_Map. KEYSCROLL_NONE means there is nothing to scroll.

enum
{
    KEYSCROLL_STEPS = 9,
    KEYSCROLL_NONE = -1
};

struct KeyScrollState
{
    int Inertia;
    int Direction;

    KeyScrollState()
        : Inertia(0)
        , Direction(KEYSCROLL_NONE)
    {
    }
};

struct KeyScrollPlan
{
    int Direction;
    int Distance;
    int Inertia;
    bool Accelerate;
    bool Decay;
};

struct KeyScrollHold
{
    bool Left;
    bool Right;
    bool Up;
    bool Down;
};

// x is -1 west, +1 east. y is -1 north, +1 south. Opposite keys cancel.
inline void KeyScroll_Axes(bool left, bool right, bool up, bool down, int& x, int& y, bool& held)
{
    x = (int)right - (int)left;
    y = (int)down - (int)up;
    held = left || right || up || down;
}

inline int KeyScroll_Direction(int x, int y)
{
    static int const facing[3][3] = {
        {7 << 5, 0, 1 << 5},
        {6 << 5, KEYSCROLL_NONE, 2 << 5},
        {5 << 5, 4 << 5, 3 << 5},
    };
    return (facing[y + 1][x + 1]);
}

// steps[0] is the fastest distance and steps[8] the slowest. scroll_rate is
// the existing option: 0 is the fastest cap. delay_elapsed is the one-tick
// gate the edge scroll uses before it changes speed.
inline KeyScrollPlan KeyScroll_Plan(KeyScrollState const& state,
    int x,
    int y,
    bool held,
    int scroll_rate,
    bool delay_elapsed,
    int const* steps,
    int step_count)
{
    KeyScrollPlan plan;
    plan.Direction = KEYSCROLL_NONE;
    plan.Distance = 0;
    plan.Inertia = state.Inertia;
    plan.Accelerate = false;
    plan.Decay = false;

    bool const moving = (x != 0 || y != 0);
    int direction = state.Direction;
    if (moving) {
        direction = KeyScroll_Direction(x, y);
    }

    if (moving || (state.Inertia > 0 && !held)) {
        int rate = 8 - state.Inertia;
        int const cap = scroll_rate + 1;
        if (rate < cap) {
            rate = cap;
        }
        if (rate < 0) {
            rate = 0;
        }
        if (rate > 8) {
            rate = 8;
        }
        // Only a held direction writes the cap back into the ramp. A coast
        // keeps the speed it had and lets the decay below ease it off by one.
        if (moving) {
            plan.Inertia = 8 - rate;
        }
        if (direction != KEYSCROLL_NONE && steps != 0 && step_count > 0) {
            if (rate >= step_count) {
                rate = step_count - 1;
            }
            plan.Direction = direction;
            plan.Distance = steps[rate];
            plan.Accelerate = delay_elapsed && moving;
        }
    }

    if (!moving && delay_elapsed) {
        plan.Decay = true;
    }
    return (plan);
}

inline void KeyScroll_Commit(KeyScrollState& state, KeyScrollPlan const& plan, bool scrolled)
{
    state.Inertia = plan.Inertia;
    if (plan.Direction != KEYSCROLL_NONE) {
        state.Direction = plan.Direction;
    }
    if (scrolled && plan.Accelerate) {
        state.Inertia++;
    }
    if (plan.Decay) {
        state.Inertia--;
        if (state.Inertia < 0) {
            state.Inertia = 0;
        }
    }
}

// A saved scroll hotkey of 0 is the old unbound value. Put_Int wrote that
// default back into existing inis, so 0 means "use the arrow default".
inline unsigned KeyScroll_Default(unsigned loaded, unsigned arrow)
{
    if (loaded == 0) {
        return (arrow);
    }
    return (loaded);
}

inline bool KeyScroll_Match(unsigned input, unsigned key)
{
    if ((key & 0xFF) == 0) {
        return (false);
    }
    return ((input & 0xFF) == (key & 0xFF));
}

// The four directions are whatever KeyScrollLeft/Right/Up/Down are bound to.
template <class Options> inline KeyScrollHold KeyScroll_Holds(Options const& options, bool (*down)(unsigned))
{
    KeyScrollHold hold;
    hold.Left = hold.Right = hold.Up = hold.Down = false;
    if (down == 0) {
        return (hold);
    }

    unsigned const keys[4] = {options.KeyScrollLeft, options.KeyScrollRight, options.KeyScrollUp, options.KeyScrollDown};
    bool* const axis[4] = {&hold.Left, &hold.Right, &hold.Up, &hold.Down};
    for (int index = 0; index < 4; index++) {
        if ((keys[index] & 0xFF) != 0 && down(keys[index])) {
            *axis[index] = true;
        }
    }
    return (hold);
}

template <class Options> inline bool KeyScroll_Swallow(Options const& options, unsigned input)
{
    unsigned const keys[4] = {options.KeyScrollLeft, options.KeyScrollRight, options.KeyScrollUp, options.KeyScrollDown};
    for (int index = 0; index < 4; index++) {
        if (KeyScroll_Match(input, keys[index])) {
            return (true);
        }
    }
    return (false);
}

// blocked means a message is being typed or the map editor has the keys.
// The view then coasts to a stop, and the key is left for that screen.
template <class Options>
inline void KeyScroll_Apply(KeyScrollState& state,
    Options const& options,
    bool (*down)(unsigned),
    bool (*scroll)(int direction, int distance),
    bool blocked,
    int scroll_rate,
    bool delay_elapsed,
    int const* steps,
    int step_count,
    unsigned& input)
{
    KeyScrollHold hold;
    hold.Left = hold.Right = hold.Up = hold.Down = false;
    if (!blocked) {
        hold = KeyScroll_Holds(options, down);
    }

    int x = 0;
    int y = 0;
    bool held = false;
    KeyScroll_Axes(hold.Left, hold.Right, hold.Up, hold.Down, x, y, held);

    KeyScrollPlan plan = KeyScroll_Plan(state, x, y, held, scroll_rate, delay_elapsed, steps, step_count);
    bool scrolled = false;
    if (plan.Distance != 0 && plan.Direction != KEYSCROLL_NONE && scroll != 0) {
        scrolled = scroll(plan.Direction, plan.Distance);
    }
    KeyScroll_Commit(state, plan, scrolled);

    if (!blocked && KeyScroll_Swallow(options, input)) {
        input = 0;
    }
}

#endif
