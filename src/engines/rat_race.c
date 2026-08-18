#include "engines/rat_race.h"

#ifndef PLATFORM_PC
asm(".include \"include/gba.inc\""); // Temporary
#endif

// For readability.
#define gRatRace ((struct RatRaceEngineData *)gCurrentEngineData)


/* RAT RACE */


#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_08039dfc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_08039e0c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_08039e3c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_08039e68.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a154.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a158.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a164.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a198.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a1d4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a1e4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a1f8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a204.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a230.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a2a8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a350.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a3b8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a3c4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a41c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a434.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a458.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a47c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a490.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a4a4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a4a8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a4f8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a564.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5a4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5bc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5c0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5dc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a5e0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a610.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a640.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a644.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a648.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a64c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a650.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a654.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a678.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a798.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803a8e4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803aa58.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803aa9c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803aba4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803ac98.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803ad50.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803ad60.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803aef4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b034.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b1ac.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b1e8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b230.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b258.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b37c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b924.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803b9fc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803baa0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803baf8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bb2c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bbd8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bc08.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bc40.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bc98.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bd0c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/rat_race/asm_0803bd58.s"
#endif
