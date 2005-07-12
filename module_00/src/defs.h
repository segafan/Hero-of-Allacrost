/*
 * defs.h
 *  Hero of Allacrost header file for forward declarations of classes
 *  (C) 2005 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */
 
// The purpose of this file is to define forward declarations in such a way that
//  we avoid in recursive inclusion problems. Pretty much every header file
//  in the source tree will need to include this, with a few exceptions (utils.h 
//  is one). It should not be included in source files.
 
// Audio declarations, see audio.h/cpp
namespace hoa_audio {
	extern bool AUDIO_DEBUG;
	class GameAudio;
	class MusicDescriptor;
	class SoundDescriptor;
}

// Video declarations, see video.h/cpp
namespace hoa_video {
	extern bool VIDEO_DEBUG;
	class GameVideo;
	class ImageDescriptor;
}

// Data declarations, see data.h/cpp
namespace hoa_data {
	extern bool DATA_DEBUG;
	class GameData;
}

// Engine declarations, see engine.h/cpp
namespace hoa_engine {
	extern bool ENGINE_DEBUG;
	class GameMode;
	class GameModeManager;
	class GameSettings;
	class KeyState;
	class JoystickState;
	class GameInput;
}

// Global declarations, see global.h/cpp
namespace hoa_global {
	extern bool GLOBAL_DEBUG;
	class GObject;
	class GItem;
	class GSkillBook;
	class GWeapon;
	class GArmor;
	class GSkill;
	class GAttackPoint;
	class GEnemy;
	class GCharacter;
	class GameInstance;
}

// Boot mode declarations, see boot.h/cpp
namespace hoa_boot {
	extern bool BOOT_DEBUG;
	class BootMode;
}

// Map mode declarations, see map.h/cpp
namespace hoa_map {
	extern bool MAP_DEBUG;
	class MapTile;
	class TileFrame;
	class ObjectLayer;
	class MapSprite;
	class SpriteDialogue;
	class MapMode;
}

// Battle mode declarations, see battle.h/cpp
namespace hoa_battle {
	extern bool BATTLE_DEBUG;
	class BattleMode;
}

// Menu mode declarations, see menu.h/cpp
namespace hoa_menu {
	extern bool MENU_DEBUG;
	class MenuMode;
}

// Pause mode declarations, see pause.h/cpp
namespace hoa_pause {
	extern bool PAUSE_DEBUG;
	class PauseMode;
}

// Scene mode declarations, see scene.h/cpp
namespace hoa_scene {
	extern bool SCENE_DEBUG;
	class SceneMode;
}

// Quit mode declarations, see quit.h/cpp
namespace hoa_quit {
	extern bool QUIT_DEBUG;
	class QuitMode;
}

// Utils declarations, see utils.h/cpp
namespace hoa_utils {
	extern bool UTILS_DEBUG;
}
