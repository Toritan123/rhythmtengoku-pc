#include "engines/drum_intro.h"
#include "src/scenes/gameplay.h"
#include "src/memory.h"
#include "src/code_08001360.h"
#include "src/task_pool.h"
#include "src/memory_heap.h"
#include "src/code_08007468.h"
#include "src/text_printer.h"
#include "src/code_0800b778.h"
#include "src/lib_0804ca80.h"

#ifndef PLATFORM_PC
asm(".include \"include/gba.inc\""); // Temporary
#endif

// For readability.
#define gDrumIntro ((struct DrumIntroEngineData *)gCurrentEngineData)


/* DRUM INTRO */


#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080239a0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080239bc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080239ec.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023a18.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023bb8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023bcc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023bf4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023c0c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023c44.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023c58.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023c6c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023d44.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023d60.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023d64.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023d68.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023d6c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023d78.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023da0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023da4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023df8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023e4c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023e50.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023edc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023f68.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023f6c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08023ffc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080240a4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024134.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080241c0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_0802424c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_0802428c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080242cc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080242f8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024978.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_0802497c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080249c0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080249f0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024a4c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024ae4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024ba0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024bd0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024be8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024bfc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024c2c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024cb0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024d44.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024d48.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024d4c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024d68.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024d6c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024da4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024ddc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024e0c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024e48.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024ecc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024ed0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024ef4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024f64.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024fb4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024fbc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024fc4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08024ff4.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08025020.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08025038.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080251d0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080251d8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080251e8.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080251ec.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080251f0.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_080251fc.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08025204.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_0802520c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08025214.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08025218.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_0802521c.s"
#endif

#ifndef PLATFORM_PC
#include "asm/engines/drumming_lessons/asm_08025220.s"
#endif
