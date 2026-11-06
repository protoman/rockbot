#ifndef PORT_LAYOUT_H
#define PORT_LAYOUT_H

// The port's game data holds the 14 game rows (224p) in file slots 0..13; the
// 15th slot of the PC file struct is unused padding (see
// tools/import_pc_data.py, which converts the PC data once). The editor works
// on exactly the rows the game shows.
#define EDITOR_MAP_H 14

#endif
