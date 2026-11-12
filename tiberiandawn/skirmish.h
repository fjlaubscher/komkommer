#ifndef TD_SKIRMISH_H
#define TD_SKIRMISH_H

// Include only from standalone game translation units.
class FileClass;
int Skirmish_Dialog();
void Assign_Skirmish_Houses();
void Save_Skirmish_Recording(FileClass& file);
void Load_Skirmish_Recording(FileClass& file);

#endif
