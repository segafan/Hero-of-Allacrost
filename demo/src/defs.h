///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
*** This file serves two purposes. The first purpose of this file is to forward
*** declare classes and shared variables in order to avoid problems with
*** recursive inclusion. The second purpose of this file is to declare
*** the function that contains all of the Allacrost Lua binding code. This makes 
*** the C++ engine code available for use in Lua scripts.
***
*** \note Pretty much every header file in the source tree will need to include
*** this file, with a few exceptions (utils.h is one). The only source file
*** that should need to include this file is defs.cpp
***
*** \note The commenting for all namespaces, variables, and classes declared in
*** this file can be found in the respective header files for where these
*** structures reside in. There are no doxygen comments for the classes and
*** namespaces found here.
***
*** \note You should not need to use the hoa_defs namespace unless you are
*** making the call to bind the engine to Lua.
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

	class AudioDescriptor;
	class MusicDescriptor;
	class SoundDescriptor;

	namespace private_audio {
		class AudioCacheElement;
	
		class AudioBuffer;
		class AudioSource;
		class AudioStream;

		class AudioInput;
		class WavFile;
		class OggFile;
		class AudioMemory;

		class AudioEffect;
		class FadeInEffect;
		class FadeOutEffect;
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
	class CompositeImage;

	class TextureController;

	class TextSupervisor;
	class FontGlyph;
	class FontProperties;
	class TextImage;

	class GUISupervisor;
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
		class FixedTexSheet;
		class VariableTexSheet;
		class FixedTexNode;
		class VariableTexNode;

		class ImageMemory;

		class BaseTexture;
		class ImageTexture;
		class TextTexture;
		class TextBlock;
		class AnimationFrame;
		class ImageElement;

		class GUIElement;
		class GUIControl;
		class MenuSkin;

		class Option;
		class OptionElement;
		class OptionCellBounds;

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

// Script declarations, see src/engine/script
namespace hoa_script {
	extern bool SCRIPT_DEBUG;
	class GameScript;

	class ScriptDescriptor;
	class ReadScriptDescriptor;
	class WriteScriptDescriptor;
	class ModifyScriptDescriptor;
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
	class Timer;
}

////////////////////////////////////////////////////////////////////////////////
// Global Code Declarations
////////////////////////////////////////////////////////////////////////////////

// Global declarations, see src/global/
namespace hoa_global {
	extern bool GLOBAL_DEBUG;
	class GameGlobal;
	class GlobalEventGroup;

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
	class GlobalCharacterGrowth;
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
		class BattleActor;
		class BattleCharacter;
		class BattleEnemy;

		class BattleAction;
		class SkillAction;
		class ItemAction;
		class ActorEffect;

		class ActionWindow;
		class FinishWindow;
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
		class EnemyZone;

		class MapObject;
		class PhysicalObject;
		class MapTreasure;

		class VirtualSprite;
		class MapSprite;
		class EnemySprite;

		class TreasureMenu;
		class DialogueManager;
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
// Miscellaneous Declarations
////////////////////////////////////////////////////////////////////////////////

// Utils declarations, see utils.h/cpp
namespace hoa_utils {
	extern bool UTILS_DEBUG;
	class ustring;
	class Exception;
	extern float RandomFloat();
}

////////////////////////////////////////////////////////////////////////////////
// Binding Declarations
////////////////////////////////////////////////////////////////////////////////

//! \brief Namespace which contains the single Lua binding function
namespace hoa_defs {

/** \brief Contains the binding code which makes the C++ engine available to Lua
*** This method should <b>only be called once</b>. It must be called after the
*** ScriptEngine is initialized, otherwise the application will crash.
**/
void BindEngineToLua();

} // namespace hoa_defs

#endif // __DEFS_HEADER__
