#include "psp_ram.h"
//#include "/usr/local/pspdev/psp/include/malloc.h"
#include "malloc.h"

psp_ram::psp_ram()
{
}

// RAM functions
u32 psp_ram::ramAvailableLineareMax (void)
{
    u32 size, sizeblock;
    u8 *ram;


    // Init variables
    size = 0;
    sizeblock = RAM_BLOCK;

    // Check loop
    while (sizeblock) {
        // Increment size
        size += sizeblock;

        // Allocate ram
        ram = (u8*)malloc(size);

        // Check allocate
        if (!(ram)) {
            // Restore old size
            size -= sizeblock;

            // Size block / 2
            sizeblock >>= 1;
        } else {
            free(ram);
        }
    }

 return size;
}

u32 psp_ram::ramAvailable (void) {
    u8 **ram, **temp;
    u32 size, count, x;


    // Init variables
    ram = NULL;
    size = 0;
    count = 0;

    // Check loop
    for (;;) {
        // Check size entries
        if (!(count % 10)) {
            // Allocate more entries if needed
            temp = (u8**)realloc(ram,sizeof(u8 *) * (count + 10));
            if (!(temp)) {
                break;
            }

            // Update entries and size (size contains also size of entries)
            ram = temp;
            size += (sizeof(u8 *) * 10);
        }

        // Find max lineare size available
        x = ramAvailableLineareMax();
        if (!(x)) {
            break;
        }

        // Allocate ram
        ram[count] = (u8*)malloc(x);
        if (!(ram[count])) {
            break;
        }

        // Update variables
        size += x;
        count++;
    }

    // Free ram
    if (ram) {
        for (x=0;x<count;x++) free(ram[x]);
        free(ram);
    }

    return size;
}

/*
void execute_memory_test() {
#ifdef PSP
    psp_ram _ram_counter;
#endif

    int stage_number = 1;
    int map_id = 0;
    int npc_id = 0;
    int object_id = 0;

    fio.read_stage(stage_data, stage_number);


#ifdef PSP
    std::cout << "MEMTEST[NPC]::BEFORE='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif
    classnpc* new_npc = new classnpc(stage_number, map_id, stage_data.maps[map_id].map_npcs[npc_id].id_npc, npc_id);
    new_npc->clean_character_graphics_list();
    delete new_npc;
#ifdef PSP
    std::cout << "MEMTEST[NPC]::AFTER='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif

#ifdef PSP
    std::cout << "MEMTEST[OBJECT]::BEFORE='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif
    object* new_obj = new object(object_id, NULL, st_position(0, 0));
    new_obj->remove_graphic();
    delete new_obj;
#ifdef PSP
    std::cout << "MEMTEST[OBJECT]::AFTER='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif


#ifdef PSP
    std::cout << "MEMTEST[MAP]::BEFORE='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif
    classMap* new_map = new classMap();
    new_map->setMapNumber(map_id);
    new_map->setStageNumber(stage_number);
    new_map->loadMap();
    delete new_map;
#ifdef PSP
    std::cout << "MEMTEST[MAP]::AFTER='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif

    std::fflush(stdout);
    timer.delay(200);
    std::fflush(stdout);
    std::cout << "MEMTEST[END]" << std::endl;
    timer.delay(200);
}
*/
