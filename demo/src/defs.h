///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    defs.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for forward declarations of classes and debug variables.
***
*** The primary purpose of this file is to forward declare classes and variables
*** in such a way that we avoid recursive inclusion problems. It defines every
*** class (that is not in a private_* namespace) across the entire source tree.
*** If you add a new class or namespace, don't forget to add its declaration to
*** this file!
***
*** \note Pretty much every header file in the source tree will need to include
*** this file, with a few exceptions (utils.h is one).
***
*** \note This file should not be included in any source files.
***
*** \note The commenting for all namespaces, variables, and classes declared in
*** this file can be found in the respective header files for where these
*** structures reside in.
*** **************************************************************************/

#ifndef __DEFS_HEADER__
#define __DEFS_HEADER__

////////////////////////////////////////////////////////////////////////////////
// Game Engine Declarations
////////////////////////////////////////////////////////////////////////////////

// Audio declarations, see audio.h/cpp
namespace hoa_audio {
	extern bool AUDIO_DEBUG;
	class GameAudio;

	class MusicDescriptor;
	class SoundDescriptor;

	namespace private_audio {
		class MusicData;
		class SoundData;
	}
}

// Video declarations, see video.h/cpp
namespace hoa_video {
	extern bool VIDEO_DEBUG;
	class GameVideo;

	class Color;
	class CoordSys;
	class ScreenRect;

	class FixedImageNode;
	class VariableImageNode;

	class ImageDescriptor;
	class StillImage;
	class AnimatedImage;

	class FontGlyph;
	class FontProperties;

	class Option;
	class OptionElement;
	class OptionCellBounds;
	class MenuWindow;
	class TextBox;
	class OptionBox;

	class Interpolator;

	class ParticleEffect;
	class ParticleEffectDef;
	class ParticleEmitter;
	class EffectParameters;

	namespace private_video {
		class Context;

		class TexSheet;
		class TexMemMgr;
		class FixedTexMemMgr;
		class VariableTexMemMgr;

		class ImageLoadInfo;
		class Image;
		class ImageElement;
		class MultiImage;
		class AnimationFrame;

		class GUIElement;
		class GUIControl;
		class MenuSkin;
		class GUI;

		class ParticleManager;
		class ParticleSystem;
		class ParticleSystemDef;
		class Particle;
		class ParticleVertex;
		class ParticleTexCoord;
		class ParticleKeyframe;

		class ScreenFader;
		class ShakeForce;
	}
}

// Script declarations, see script.h/cpp
namespace hoa_script {
	extern bool SCRIPT_DEBUG;
	class GameScript;

	class ScriptDescriptor;
}

// Mode manager declarations, see mode_manager.h/cpp
namespace hoa_mode_manager {
	extern bool MODE_MANAGER_DEBUG;
	class GameModeManager;

	class GameMode;
}

// Input declarations, see input.h/cpp
namespace hoa_input {
	extern bool INPUT_DEBUG;
	class GameInput;
}

// Settings declarations, see settings.h/cpp
namespace hoa_system {
	extern bool SYSTEM_DEBUG;
	class GameSystem;
}

// Global declarations, see src/global/
namespace hoa_global {
	extern bool GLOBAL_DEBUG;
	class GameGlobal;

	class GlobalObject;
	class GlobalItem;
	class GlobalWeapon;
	class GlobalArmor;
	class GlobalShard;
	class GlobalKeyItem;

	class GlobalStatusEffect;
	class GlobalElementalEffect;
	class GlobalSkill;

	class GlobalTarget;
	class GlobalAttackPoint;
	class GlobalActor;
	class GlobalCharacter;
	class GlobalEnemy;
	class GlobalParty;
}

////////////////////////////////////////////////////////////////////////////////
// Game Mode Declarations
////////////////////////////////////////////////////////////////////////////////

// Battle mode declarations, see battle.h/cpp
namespace hoa_battle {
	extern bool BATTLE_DEBUG;
	class BattleMode;

	namespace private_battle {
		class ActorEffect;
		class IBattleActor;
		class BattleActor;
		class BattleCharacterActor;
		class BattleEnemyActor;
		class ScriptEvent;
	}
}

// Boot mode declarations, see boot.h/cpp
namespace hoa_boot {
	extern bool BOOT_DEBUG;
	class BootMode;
	class BootMenu;
	class CreditsScreen;
}

// Map mode declarations, see map.h/cpp
namespace hoa_map {
	extern bool MAP_DEBUG;
	class MapMode;

	namespace private_map {
		class MapTile;
		class MapFrame;
		class PathNode;

		class ZoneSection;
		class MapZone;
		class MonsterZone;

		class MapObject;
		class PhysicalObject;
		class VirtualSprite;
		class MapSprite;
		class MonsterSprite;

		class MapDialogue;

		class SpriteAction;
		class ActionPathMove;
		class ActionAnimate;
		class ActionScriptFunction;


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

// Quit mode declarations, see quit.h/cpp
namespace hoa_quit {
	extern bool QUIT_DEBUG;
	class QuitMode;
}

// Scene mode declarations, see scene.h/cpp
namespace hoa_scene {
	extern bool SCENE_DEBUG;
	class SceneMode;
}

// Shop mode declarations, see shop./cpp
namespace hoa_shop {
	extern bool SHOP_DEBUG;
	class ShopMode;
}

////////////////////////////////////////////////////////////////////////////////
// Other Declarations
////////////////////////////////////////////////////////////////////////////////

// Utils declarations, see utils.h/cpp
namespace hoa_utils {
	extern bool UTILS_DEBUG;
	class ustring;
}

#endif /* #ifndef __DEFS_HEADER__ */
