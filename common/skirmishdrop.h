// Instantiate with each game's own DropListClass, only in standalone code.
#ifndef COMMON_SKIRMISH_DROP_H
#define COMMON_SKIRMISH_DROP_H

#include "control.h"

template<class DropList>
class SkirmishDropListClass : public DropList
{
public:
    using DropList::DropList;

    void Peer_To_Peer(unsigned flags, KeyNumType& key, ControlClass& whom) override
    {
        bool was_dropped = this->IsDropped;
        DropList::Peer_To_Peer(flags, key, whom);
        if (this->IsDropped) {
            this->List.Width = this->Width + this->DropButton.Width;
            if (!was_dropped) {
                this->List.Set_Focus();
            }
        }
    }

protected:
    int Action(unsigned flags, KeyNumType& key) override
    {
        if (flags & this->LEFTPRESS) {
            if (this->IsDropped) {
                this->Collapse();
            } else {
                this->Expand();
                this->List.Width = this->Width + this->DropButton.Width;
                this->List.Set_Focus();
            }
            key = KN_NONE;
            return true;
        }
        return DropList::Action(flags, key);
    }
};
#endif
