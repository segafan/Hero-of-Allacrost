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
	class GameAudio;
	class MusicDescriptor;
	class SoundDescriptor;
}

// Video declarations, see video.h/cpp
namespace hoa_video {
	class GameVideo;
	class ImageDescriptor;
}

// Data declarations, see data.h/cpp
namespace hoa_data {
	class GameData;
}

// Engine declarations, see engine.h/cpp
namespace hoa_engine {
	class GameMode;
	class GameModeManager;
	class GameSettings;
	class KeyState;
	class JoystickState;
	class GameInput;
}

// Global declarations, see global.h/cpp
// namespace hoa_engine {
// 
// }

// Boot mode declarations, see boot.h/cpp
namespace hoa_boot {
	class BootMode;
}

// Map mode declarations, see map.h/cpp
namespace hoa_map {
	class MapMode;
	class ObjectLayer;
	class MapTile;
	class TileFrame;
}

// Battle mode declarations, see battle.h/cpp
namespace hoa_battle {
	class BattleMode;
}

// Menu mode declarations, see menu.h/cpp
namespace hoa_menu {
	class MenuMode;
}

// Pause mode declarations, see pause.h/cpp
namespace hoa_pause {
	class PauseMode;
}

// Scene mode declarations, see scene.h/cpp
namespace hoa_scene {
	class SceneMode;
}

// Quit mode declarations, see quit.h/cpp
namespace hoa_quit {
	class QuitMode;
}
