#include "stage_data.h"

#include "stage0_data.h"
#include "stage1_data.h"
#include "stage2_data.h"
#include "stage3_data.h"
#include "stage4_data.h"
#include "stage5_data.h"
#include "stage6_data.h"
#include "stage7_data.h"
#include "stage8_data.h"

// Registry of converted stages. STAGE_DEFINED_COUNT / stage_defs live in
// stage_data.h; the stage select maps its slots to these entries.
// __attribute__((used)): LTO+--gc-sections dropped unreferenced stage
// defs when every call site had a constant stage index.
__attribute__((used)) const StageDef *const stage_defs[STAGE_DEFINED_COUNT] = {
    &stage0_def,
    &stage1_def,
    &stage2_def,
    &stage3_def,
    &stage4_def,
    &stage5_def,
    &stage6_def,
    &stage7_def,
    &stage8_def,
};
const StageDef *stage_def;

void stage_set_data(u8 stage)
{
    if (stage >= STAGE_DEFINED_COUNT) stage = 0;
    stage_def = stage_defs[stage];
}
