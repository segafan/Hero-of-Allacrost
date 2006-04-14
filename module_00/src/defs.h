///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    defs.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 17th, 2005
 * \brief   Header file for forward declarations of classes and debug variables.
 *
 * The primary purpose of this file is to forward declare classes and variables
 * in such a way that we avoid recursive inclusion problems. It defines every
 * class (that is not in a private_* namespace) across the entire source tree.
 * If you add a new class or namespace, don't forget to add its declaration to
 * this file!
 *
 * \note Pretty much every header file in the source tree will need to include
 * this file, with a few exceptions (utils.h is one).
 *
 * \note This file should not be included in any source files.
 *
 * \note The commenting for all namespaces, variables, and classes declared in
 * this file can be found in the respective header files for where these
 * structures reside in.
 *****************************************************************************/

#ifndef __DEFS_HEADER__
#define __DEFS_HEADER__


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
	class StillImage;
	class AnimatedImage;
}

// Data declarations, see data.h/cpp
namespace hoa_data {
	extern bool DATA_DEBUG;
	class GameData;
	class ReadDataDescriptor;
	class WriteDataDescriptor;
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
	class GlobalObject;
	class GlobalItem;
	class GlobalWeapon;
	class GlobalArmor;
	class GlobalSkill;
	class GlobalAttackPoint;
	class GlobalEnemy;
	class GlobalCharacter;
	class GameGlobal;
}

// Boot mode declarations, see boot.h/cpp
namespace hoa_boot {
	extern bool BOOT_DEBUG;
	class BootMode;
}

// Map mode declarations, see map.h/cpp
namespace hoa_map {
	extern bool MAP_DEBUG;
	class MapMode;
	namespace private_map {
		class MapFrame;
		class MapTile;
		class MapObject;
		class MapSprite;
		class MapDialogue;
		class TileCheck;
		class TileNode;
		class SpriteDialogue;
		class SpriteAction;
		class ActionPathMove;
		class ActionFrameDisplay;
		class ActionRandomMove;
		class ActionScriptFunction;
	}
}

// Battle mode declarations, see battle.h/cpp
namespace hoa_battle {
	extern bool BATTLE_DEBUG;
        class BattleMode;
        struct BattleStatTypes;
        class ActorEffect;
        
        namespace private_battle {
                class Actor;
                class BattleUI;
                class PlayerActor;
                class EnemyActor;
                class Action;
                class ScriptEvent;
        }
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

#endif
