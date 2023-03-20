#include "register_types.h"

#include "core/object/class_db.h"
#include "dungeon_generator.h"

void initialize_dungeon_generator_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<DungeonGenerator>();
}

void uninitialize_dungeon_generator_module(ModuleInitializationLevel p_level) {}
