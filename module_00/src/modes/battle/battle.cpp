///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    battle.cpp
 * \author  Corey Hoffstein, visage@allacrost.org
 * \brief   Source file for battle mode interface.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include <sstream>
#include "battle.h"
#include "audio.h"
#include "video.h"
#include "mode_manager.h"
#include "input.h"
#include "settings.h"
#include "global.h"
#include "data.h"

using namespace std;
using namespace hoa_battle::private_battle;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_settings;
using namespace hoa_global;
using namespace hoa_data;


namespace
  hoa_battle
{

  bool
    BATTLE_DEBUG = false;

  namespace
    private_battle
  {

//Color(1.0f, 1.0f, 0.0f, 0.8f)
    void
    TEMP_Draw_Text (Color c, float x, float y, std::string text)
    {
      VideoManager->
      SetTextColor (c);
      VideoManager->
      Move (x, y);
      VideoManager->
      DrawText(text);
    }
/*

        ACTOR

*/

    Actor::Actor (BattleMode * ABattleMode, uint32 AXLocation,
		  uint32 AYLocation):
    _owner_battle_mode (ABattleMode),
    _x_origin (AXLocation),
    _y_origin (AYLocation),
    _x_location (AXLocation),
    _y_location (AYLocation),
    _max_skill_points (0),
    _current_skill_points (0),
    _is_move_capable (true),
    _is_alive (true),
    _is_queued_to_perform (false),
    _warmup_time (0),
    _cooldown_time (0),
    _defensive_mode_bonus (0),
    _total_strength_modifier (0),
    _total_agility_modifier (0),
    _total_intelligence_modifier (0)
    {
      _TEMP_total_time_damaged = 0;
    }

/*!
        Need to implement this!
*/
    Actor::~
    Actor ()
    {

    }

/*!
        Stuff relating to, you know, death
*/
    void
    Actor::Die ()
    {
      _is_alive = false;
      //remove any scripts we are a part of
      GetOwnerBattleMode ()->RemoveScriptedEventsForActor (this);
    }

    bool
    Actor::IsAlive () const
    {
      return
	_is_alive;
    }

/*!
        Get the mode we are currently fighting in
*/
    BattleMode *
    Actor::GetOwnerBattleMode () const
    {
      return
	_owner_battle_mode;
    }

/*!
        Manage effects that the player is feeling
*/
    void
    Actor::UpdateEffects (uint32 ATimeElapsed)
    {
      for (unsigned int i = 0; i < _effects.size (); i++)
	{
	  _effects[i].Update (ATimeElapsed);
	}
    }

    void
    Actor::PushEffect (const ActorEffect AEffect)
    {
      _effects.push_back (AEffect);
    }

/*!
        Is the player frozen, asleep, et cetera?
*/
    bool
    Actor::IsMoveCapable () const
    {
      return
	_is_move_capable;
    }

    void
    Actor::SetMoveCapable (bool AMoveCapable)
    {
      _is_move_capable = AMoveCapable;
    }

/*!
        If the player is warming up, it really can't do anything
        Sort of a special case
*/
    bool
    Actor::IsWarmingUp () const
    {
      return
	_warmup_time !=
	0;
    }

    void
    Actor::SetWarmupTime (uint32 AWarmupTime)
    {
      _warmup_time = AWarmupTime;
    }

    bool
    Actor::IsQueuedToPerform () const
    {
      return
	_is_queued_to_perform;
    }

    void
    Actor::SetQueuedToPerform (bool AQueuedToPerform)
    {
      _is_queued_to_perform = AQueuedToPerform;
    }

/*!
        Defensive mode boosts defense
*/
    bool
    Actor::IsInDefensiveMode () const
    {
      return
	_defensive_mode_bonus !=
	0;
    }

    void
    Actor::SetDefensiveBonus (uint32 ADefensiveBonus)
    {
      _defensive_mode_bonus = ADefensiveBonus;
    }

/*!
        Empty method.  For scripting purposes only.
        PlayerActor is animated.  Enemy is not.
*/
    void
    Actor::SetAnimation (std::string AAnimation)
    {
    }


    void
    Actor::SetTotalStrengthModifier (uint32 AStrengthModifier)
    {
      _total_strength_modifier = AStrengthModifier;
    }

    uint32
    Actor::GetTotalStrengthModifier ()
    {
      return _total_strength_modifier;
    }

    void
    Actor::SetTotalAgilityModifier (uint32 AAgilityModifier)
    {
      _total_agility_modifier = AAgilityModifier;
    }

    uint32
    Actor::GetTotalAgilityModifier ()
    {
      return _total_agility_modifier;
    }

    void
    Actor::SetTotalIntelligenceModifier (uint32 AIntelligenceModifier)
    {
      _total_intelligence_modifier = AIntelligenceModifier;
    }

    uint32
    Actor::GetTotalIntelligenceModifier ()
    {
      return _total_intelligence_modifier;
    }

    void
    Actor::TEMP_Deal_Damage (uint32 damage)
    {

      _TEMP_damage_dealt = damage;
      _TEMP_total_time_damaged = 1;

      if (_TEMP_damage_dealt >= GetHealth ())
	{
	  SetHealth (0);
	  Die ();
	}
      else
	SetHealth (GetHealth () - _TEMP_damage_dealt);
    }

/*

        BATTLEUI

*/

    BattleUI::BattleUI (BattleMode * const ABattleMode):
    _battle_mode (ABattleMode),
    _currently_selected_player_actor (NULL),
    _necessary_selections (0),
    _current_hover_selection (0),
    _current_map_selection (0),
    _number_menu_items (0),
    _cursor_state (CURSOR_ON_PLAYER_CHARACTERS),
    _sub_menu (NULL),
    _sub_menu_window (NULL)
    {
      _actor_index = _battle_mode->GetIndexOfFirstIdleCharacter ();

      std::vector < hoa_video::StillImage > attack_point_indicator;
      StillImage
	frame;
      frame.SetDimensions (16, 16);
      frame.SetFilename ("img/icons/battle/indicator_1.png");
      attack_point_indicator.push_back (frame);
      frame.SetFilename ("img/icons/battle/indicator_2.png");
      attack_point_indicator.push_back (frame);
      frame.SetFilename ("img/icons/battle/indicator_3.png");
      attack_point_indicator.push_back (frame);
      frame.SetFilename ("img/icons/battle/indicator_4.png");
      attack_point_indicator.push_back (frame);

      VideoManager->BeginImageLoadBatch ();
      for (uint32 i = 0; i < attack_point_indicator.size (); i++)
	{
	  if (!VideoManager->LoadImage (attack_point_indicator[i]))
	    std::cerr << "Failed to load MAPS indicator." << std::endl;	//failed to laod image
	}
      VideoManager->EndImageLoadBatch ();

      for (uint32 i = 0; i < attack_point_indicator.size (); i++)
	{
	  _MAPS_indicator.AddFrame (attack_point_indicator[i], 10);
	}

      _general_menu.SetCellSize (50.0f, 79.0f);
      _general_menu.SetSize (5, 1);
      _general_menu.SetPosition (0.0f, 620.0f);
      _general_menu.SetFont("battle");
      _general_menu.SetAlignment (VIDEO_X_LEFT, VIDEO_Y_CENTER);
      _general_menu.SetOptionAlignment (VIDEO_X_CENTER, VIDEO_Y_CENTER);
      _general_menu.SetSelectMode (VIDEO_SELECT_SINGLE);
      _general_menu.SetHorizontalWrapMode (VIDEO_WRAP_MODE_STRAIGHT);

      vector < hoa_utils::ustring > formatText;
      formatText.push_back(MakeWideString ("<img/icons/battle/icon_attack.png>"));
      formatText.push_back(MakeWideString ("<img/icons/battle/icon_defend.png>"));
      formatText.push_back(MakeWideString ("<img/icons/battle/icon_support.png>"));
      formatText.push_back(MakeWideString ("<img/icons/battle/icon_item.png>"));
      formatText.push_back(MakeWideString ("<img/icons/battle/icon_extra.png>"));

      _general_menu.SetOptions (formatText);
      _general_menu.SetSelection (0);
      _general_menu_cursor_location = 0;
      _general_menu.EnableOption (4, false);

      _general_menu.SetCursorOffset (-15, 0);

      _battle_lose_menu.SetCellSize (128.0f, 50.0f);
      _battle_lose_menu.SetPosition (530.0f, 380.0f);
      _battle_lose_menu.SetSize (1, 1);
      _battle_lose_menu.SetAlignment (VIDEO_X_CENTER, VIDEO_Y_CENTER);
      _battle_lose_menu.SetOptionAlignment (VIDEO_X_CENTER, VIDEO_Y_CENTER);
      _battle_lose_menu.SetSelectMode (VIDEO_SELECT_SINGLE);
      _battle_lose_menu.SetHorizontalWrapMode (VIDEO_WRAP_MODE_STRAIGHT);
      _battle_lose_menu.SetCursorOffset (-35.0f, -4.0f);
      vector < hoa_utils::ustring > loseText;
      loseText.push_back (MakeWideString ("Return to the main menu"));
      _battle_lose_menu.SetOptions (loseText);
      _battle_lose_menu.SetSelection (0);

      _player_selector_image.SetDimensions (109, 78);
      _player_selector_image.
	SetFilename ("img/icons/battle/character_selection.png");
      if (!VideoManager->LoadImage (_player_selector_image))
	{
	  cerr << "Unable to load player selector image." << endl;
	  //kill the owner battle mode some how
	}
    }

    BattleUI::~BattleUI ()
    {
      //_battle_lose_menu.Destroy();
      //_general_menu.Destroy();

      if (_sub_menu)
	{
	  //_sub_menu->Destroy();
	  delete
	    _sub_menu;
	}
      if (_sub_menu_window)
	{
	  _sub_menu_window->Destroy ();
	  delete
	    _sub_menu_window;
	}
    }
/*!
        Get the actor we are currently on
*/
    Actor *const
    BattleUI::GetSelectedActor () const
    {
      return
	_currently_selected_player_actor;
    }

/*!
        We clicked on an actor
*/
    void
    BattleUI::SetPlayerActorSelected (PlayerActor * const AWhichActor)
    {
      _currently_selected_player_actor = AWhichActor;
      _actor_index =
	_battle_mode->IndexLocationOfPlayerCharacter (AWhichActor);
    }

/*!
        No actor is selected...we are now selecting an actor
*/
    void
    BattleUI::DeselectPlayerActor ()
    {
      _currently_selected_player_actor = NULL;
    }

/*!
        Get other people selected
*/
    std::deque < Actor * >BattleUI::GetSelectedArgumentActors ()const
    {
      return
	_currently_selected_argument_actors;
    }

/*!
        The actor we just selected is now an argument
*/
    void
    BattleUI::SetActorAsArgument (Actor * const AActor)
    {
      _currently_selected_argument_actors.push_back (AActor);
    }

/*!
        Sets the number of arguments we should be allowing
*/
    void
    BattleUI::SetNumberNecessarySelections (uint32 ANumSelections)
    {
      _necessary_selections = ANumSelections;
    }

/*!
        Draw the actual windows
*/
    void
    BattleUI::Draw ()
    {
      // Battle is over
      if (_battle_mode->IsBattleOver ())
	{
	  if (_battle_mode->IsVictorious ())	// Draw a victory screen along with the loot. TODO: Maybe do this in a separate function
	    {
	      VideoManager->Move(520.0f, 384.0f);
	      VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	      VideoManager->DisableFog();	// Turn off the fog
	      VideoManager->DrawText("You have won the battle!\n\n\Exp: +50\n\nLoot : 1 HP Potion");
	      VideoManager->EnableFog (Color::orange, 0.3f);	// golden
	      VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	    }
	  else			// show the lose screen
	    {
	      VideoManager->DisableFog();	// Turn off the fog
	      VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	      VideoManager->Move (520.0f, 430.0f);
	      VideoManager->DrawText("You have lost the battle!");
	      _battle_lose_menu.Draw ();
	      VideoManager->EnableFog(Color (0.6f, 0, 0, 1.0f), 0.6f);	// blood-red fog
	      VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	    }
	  return;
	}

      if (_cursor_state >= CURSOR_ON_SUB_MENU && _sub_menu_window)
	{
	  _sub_menu_window->Draw ();
	}
      if (_cursor_state >= CURSOR_ON_SUB_MENU && _sub_menu)
	{
	  _sub_menu->Draw ();
	}

      if (_cursor_state > CURSOR_ON_PLAYER_CHARACTERS)
	{
	  _general_menu.Draw ();
	}

      /*
         draw damage information
       */

      //always draw the currently selected character
      VideoManager->SetDrawFlags (VIDEO_BLEND, 0);
      VideoManager->Move (_currently_selected_player_actor->GetXLocation () -
			  20,
			  _currently_selected_player_actor->GetYLocation () -
			  20);
      _player_selector_image.Draw ();

      //draw over our currently selected enemy
      if (_cursor_state == CURSOR_SELECT_TARGET)
	{
	  EnemyActor *
	    e = _battle_mode->GetEnemyActorAt (_argument_actor_index);
	  VideoManager->Move (e->GetXLocation () - 20,
			      e->GetYLocation () - 20);
	  _player_selector_image.Draw ();
	}
    }

    void
    BattleUI::DrawTopElements ()
    {
      if (_cursor_state == CURSOR_ON_SELECT_MAP)
	{
	  EnemyActor *
	    e = _battle_mode->GetEnemyActorAt (_argument_actor_index);
	  std::vector < GlobalAttackPoint * >global_attack_points =
	    e->GetAttackPoints ();

	  VideoManager->Move (e->GetXLocation () +
			      global_attack_points[_current_map_selection]->
			      GetXPosition (),
			      e->GetYLocation () +
			      global_attack_points[_current_map_selection]->
			      GetYPosition ());
	  VideoManager->SetDrawFlags (VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	  VideoManager->SetDrawFlags (VIDEO_BLEND, 0);
	  VideoManager->DrawImage (_MAPS_indicator);
	  VideoManager->SetDrawFlags (VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	  Color
	  c (0.0f, 0.0f, 1.0f, 1.0f);
	  TEMP_Draw_Text (c, 850, 100,
			  global_attack_points[_current_map_selection]->
			  GetName ());
	}
    }

/*!
        Update the windows
*/
    void
    BattleUI::Update (uint32 AUpdateTime)
    {

      // Check if the battle is over
      if (_battle_mode->IsBattleOver ())
	{
	  // Battle was won :)
	  if (_battle_mode->IsVictorious ())
	    {
	      if (InputManager->ConfirmPress ())
		{
		  _battle_mode->PlayerVictory ();	// Handle victory
		}
	    }
	  else			// Battle was lost :(
	    {
	      _battle_lose_menu.Update (AUpdateTime);	// Update lose menu
	      if (InputManager->ConfirmRelease ())
		{
		  //_battle_lose_menu.HandleConfirmKey(); // This needs to be handled when there's more than 1 option
		  InputManager->EventHandler ();	// Clear input in here because we don't want confirm press in boot mode!
		  _battle_mode->PlayerDefeat ();	// Handle defeat
		}
	    }

	  return;		// No need to handle other menu events any more
	}			// end if battle over

      if (_cursor_state > CURSOR_ON_PLAYER_CHARACTERS)
	{
	  _general_menu.Update (AUpdateTime);
	}

      if (_sub_menu && _cursor_state == CURSOR_ON_SUB_MENU)
	{
	  _sub_menu->Update (AUpdateTime);
	}

      if (_cursor_state == CURSOR_ON_SELECT_MAP)
	{
	  _MAPS_indicator.Update ();
	}

      if (_cursor_state == CURSOR_ON_PLAYER_CHARACTERS)
	{
	  if (_actor_index == -1)
	    {
	      _actor_index = _battle_mode->GetIndexOfFirstIdleCharacter ();
	    }
	  else if (_battle_mode->NumberOfPlayerCharactersAlive () == 1)
	    {
	      _cursor_state = CURSOR_ON_MENU;
	    }
	  else if (InputManager->UpPress () || InputManager->RightPress ())
	    {
	      //select the character "to the top"
	      int
		working_index = _actor_index;
	      while (working_index <
		     _battle_mode->GetNumberOfPlayerCharacters ())
		{
		  if (_battle_mode->
		      GetPlayerCharacterAt ((working_index + 1))->IsAlive ())
		    {
		      _actor_index = working_index + 1;
		      break;
		    }
		  else
		    {
		      ++working_index;
		    }
		}
	    }
	  else if (InputManager->DownPress () || InputManager->LeftPress ())
	    {
	      //select the character "to the bottom"
	      int
		working_index = _actor_index;

	      while (working_index > 0)
		{
		  if (_battle_mode->
		      GetPlayerCharacterAt ((working_index - 1))->IsAlive ())
		    {
		      _actor_index = working_index - 1;
		      break;
		    }
		  else
		    {
		      --working_index;
		    }
		}
	    }
	  else if (InputManager->ConfirmPress ())
	    {
	      //we need to select the current actor...
	      _currently_selected_player_actor =
		_battle_mode->GetPlayerCharacterAt (_actor_index);
	      _cursor_state = CURSOR_ON_MENU;
	    }
	}
      else if (_cursor_state == CURSOR_ON_MENU)
	{
	  if (InputManager->LeftPress ())
	    {
	      if (_general_menu_cursor_location > 0)
		{
		  _general_menu.HandleLeftKey ();
		  _general_menu_cursor_location--;
		}
	    }
	  else if (InputManager->RightPress ())
	    {
	      if (_general_menu_cursor_location < 3)
		{
		  _general_menu.HandleRightKey ();
		  _general_menu_cursor_location++;
		}
	    }
	  else if (InputManager->ConfirmPress ())
	    {
	      //confirm the press
	      _cursor_state = CURSOR_ON_SUB_MENU;
	      PlayerActor *
		p = _battle_mode->GetPlayerCharacterAt (_actor_index);

	      //if we have an old submenu, delete it
	      if (_sub_menu)
		{
		  //_sub_menu->Destroy();
		  delete
		    _sub_menu;
		}
	      if (_sub_menu_window)
		{
		  _sub_menu_window->Destroy ();
		  delete
		    _sub_menu_window;
		}

	      _sub_menu = new OptionBox ();
	      _sub_menu->SetPosition(50.0f, 550.0f);
	      _sub_menu->SetFont("battle");
	      _sub_menu->SetAlignment (VIDEO_X_LEFT, VIDEO_Y_CENTER);
	      _sub_menu->SetOptionAlignment (VIDEO_X_CENTER, VIDEO_Y_CENTER);
	      _sub_menu->SetSelectMode (VIDEO_SELECT_SINGLE);
	      _sub_menu->SetHorizontalWrapMode (VIDEO_WRAP_MODE_STRAIGHT);
	      _sub_menu->SetCellSize (100.0f, 50.0f);
	      _sub_menu->SetCursorOffset (-30, -5);

	      switch (_general_menu_cursor_location)
		{
		case 0:
		  {		//attack
		    std::vector < hoa_global::GlobalSkill * >attack_skills =
		      p->GetAttackSkills ();
		    if (attack_skills.size () > 0)
		      {
			std::vector < ustring > attack_skill_names;
			for (uint32 i = 0; i < attack_skills.size (); ++i)
			  {
			    std::ostringstream sp_usage;
			    sp_usage << attack_skills[i]->GetSPUsage ();
			    std::string skill_string =
			      attack_skills[i]->GetName () +
			      std::string ("     ") + sp_usage.str ();
			    attack_skill_names.
			      push_back (MakeWideString (skill_string));
			  }
			_sub_menu->SetSize (1, attack_skill_names.size ());
			_sub_menu->SetOptions (attack_skill_names);
			_sub_menu->SetSelection (0);

			_sub_menu_window = new MenuWindow ();
			_sub_menu_window->Create (200.0f,
						  20.0f +
						  50.0f *
						  attack_skill_names.size ());
			_sub_menu_window->SetPosition (0.0f, 600.0f);
			_sub_menu_window->Show ();
		      }
		    else
		      {
			_cursor_state = CURSOR_ON_MENU;
		      }
		  }
		  break;
		case 1:
		  {		//defend
		    std::vector < hoa_global::GlobalSkill * >defense_skills =
		      p->GetDefenseSkills ();
		    if (defense_skills.size () > 0)
		      {
			std::vector < ustring > defense_skill_names;
			for (uint32 i = 0; i < defense_skills.size (); ++i)
			  {
			    std::ostringstream sp_usage;
			    sp_usage << defense_skills[i]->GetSPUsage ();
			    std::string skill_string =
			      defense_skills[i]->GetName () +
			      std::string ("     ") + sp_usage.str ();
			    defense_skill_names.
			      push_back (MakeWideString (skill_string));
			  }
			_sub_menu->SetOptions (defense_skill_names);
			_sub_menu->SetSize (1, defense_skill_names.size ());
			_sub_menu->SetSelection (0);

			_sub_menu_window = new MenuWindow ();
			_sub_menu_window->Create (200.0f,
						  20.0f +
						  50.0f *
						  defense_skill_names.
						  size ());
			_sub_menu_window->SetPosition (0.0f, 600.0f);
			_sub_menu_window->Show ();
		      }
		    else
		      {
			_cursor_state = CURSOR_ON_MENU;
		      }
		  }
		  break;
		case 2:
		  {		//support
		    std::vector < hoa_global::GlobalSkill * >support_skills =
		      p->GetSupportSkills ();
		    if (support_skills.size () > 0)
		      {
			std::vector < ustring > support_skill_names;
			for (uint32 i = 0; i < support_skills.size (); ++i)
			  {
			    std::ostringstream sp_usage;
			    sp_usage << support_skills[i]->GetSPUsage ();
			    std::string skill_string =
			      support_skills[i]->GetName () +
			      std::string ("     ") + sp_usage.str ();
			    support_skill_names.
			      push_back (MakeWideString (skill_string));
			  }
			_sub_menu->SetOptions (support_skill_names);
			_sub_menu->SetSize (1, support_skill_names.size ());
			_sub_menu->SetSelection (0);

			_sub_menu_window = new MenuWindow ();
			_sub_menu_window->Create (200.0f,
						  20.0f +
						  50.0f *
						  support_skill_names.
						  size ());
			_sub_menu_window->SetPosition (0.0f, 600.0f);
			_sub_menu_window->Show ();
		      }
		    else
		      {
			_cursor_state = CURSOR_ON_MENU;
		      }
		  }
		  break;
		case 3:
		  {		//item

		    //blatantly stolen from menu_views.cpp
		    vector < GlobalObject * >inv =
		      GlobalManager->GetInventory ();

		    // Set the size of the option box
		    // Calculate the number of rows, this is dividing by 6, and if there is a remainder > 0
		    // add one more row for the remainder.
		    _sub_menu->SetSize (6,
					inv.size () / 6 + ((inv.size () % 6) >
							   0 ? 1 : 0));

		    std::vector < ustring > inv_names;

		    for (uint32 i = 0; i < inv.size (); ++i)
		      {
			// Create the item text
			std::ostringstream os_obj_count;
			os_obj_count << inv[i]->GetCount ();
			string
			  inv_item_str =
			  string ("<") + inv[i]->GetIconPath () +
			  string ("><32>") + inv[i]->GetName () +
			  string ("<R>") + string ("    ") +
			  os_obj_count.str ();
			inv_names.push_back (MakeWideString (inv_item_str));
		      }
		    _sub_menu->SetOptions (inv_names);
		    _sub_menu->SetSize (1, inv_names.size ());
		    _sub_menu->SetSelection (0);

		    _sub_menu_window = new MenuWindow ();
		    _sub_menu_window->Create (200.0f,
					      20.0f +
					      50.0f * inv_names.size ());
		    _sub_menu_window->SetPosition (0.0f, 600.0f);
		    _sub_menu_window->Show ();
		  }
		  break;
		}
	    }
	  else if (InputManager->CancelPress ())
	    {

	      if (_battle_mode->NumberOfPlayerCharactersAlive () > 1)
		{
		  _actor_index =
		    _battle_mode->GetIndexOfFirstIdleCharacter ();
		  _cursor_state = CURSOR_ON_PLAYER_CHARACTERS;
		}
	    }
	}
      else if (_cursor_state == CURSOR_ON_SUB_MENU)
	{
	  if (InputManager->DownPress ())
	    {
	      _sub_menu->HandleDownKey ();
	    }
	  else if (InputManager->UpPress ())
	    {
	      _sub_menu->HandleUpKey ();
	    }
	  else if (InputManager->ConfirmPress ())
	    {
	      //for now, we only select one target
	      SetNumberNecessarySelections (1);
	      _argument_actor_index =
		_battle_mode->GetIndexOfFirstAliveEnemy ();

	      //get the skill

	      //if the skill is for the players, put the
	      //cursor on the player characters

	      //other wise, put the cursor on the enemy
	      //characters

	      //put the skill in the battle script queue
	      //get out of the menu

	      _cursor_state = CURSOR_SELECT_TARGET;
	    }
	  else if (InputManager->CancelPress ())
	    {
	      _cursor_state = CURSOR_ON_MENU;
	    }
	}
      else if (_cursor_state == CURSOR_SELECT_TARGET)
	{
	  if (InputManager->DownPress () || InputManager->LeftPress ())
	    {
	      //select the character "to the top"
	      int
		working_index = _argument_actor_index;
	      while (working_index > 0)
		{
		  if (_battle_mode->GetEnemyActorAt ((working_index - 1))->
		      IsAlive ())
		    {
		      _argument_actor_index = working_index - 1;
		      break;
		    }
		  else
		    {
		      --working_index;
		    }
		}
	    }
	  else if (InputManager->UpPress () || InputManager->RightPress ())
	    {
	      //select the character "to the bottom"
	      int
		working_index = _argument_actor_index;
	      while (working_index <
		     _battle_mode->GetNumberOfEnemyActors () - 1)
		{
		  if (_battle_mode->GetEnemyActorAt ((working_index + 1))->
		      IsAlive ())
		    {
		      _argument_actor_index = working_index + 1;
		      break;
		    }
		  else
		    {
		      ++working_index;
		    }
		}
	    }
	  else if (InputManager->ConfirmPress ())
	    {
	      _cursor_state = CURSOR_ON_SELECT_MAP;
	      _current_map_selection = 0;
	    }

	  else if (InputManager->CancelPress ())
	    {
	      _cursor_state = CURSOR_ON_SUB_MENU;
	    }
	}
      else if (_cursor_state == CURSOR_ON_SELECT_MAP)
	{
	  EnemyActor *
	    e = _battle_mode->GetEnemyActorAt (_argument_actor_index);
	  std::vector < GlobalAttackPoint * >global_attack_points =
	    e->GetAttackPoints ();


	  if (InputManager->ConfirmPress ())
	    {
	      SetActorAsArgument (dynamic_cast <
				  Actor *
				  >(_battle_mode->
				    GetEnemyActorAt (_argument_actor_index)));
	      if (GetSelectedArgumentActors ().size () ==
		  _necessary_selections)
		{
		  _battle_mode->
		    AddScriptEventToQueue (ScriptEvent
					   (_currently_selected_player_actor,
					    GetSelectedArgumentActors (),
					    "sword_swipe"));

		  _currently_selected_player_actor->SetQueuedToPerform (true);

		  _currently_selected_argument_actors.clear ();

		  _actor_index =
		    _battle_mode->GetIndexOfFirstIdleCharacter ();
		  _cursor_state = CURSOR_ON_PLAYER_CHARACTERS;
		}
	      else
		{
		  _cursor_state = CURSOR_SELECT_TARGET;
		}
	    }
	  else if (InputManager->UpPress () || InputManager->RightPress ())
	    {
	      if (_current_map_selection < global_attack_points.size () - 1)
		{
		  _current_map_selection++;
		}
	      else if (_current_map_selection ==
		       global_attack_points.size () - 1)
		{
		  _current_map_selection = 0;
		}
	    }
	  else if (InputManager->DownPress () || InputManager->LeftPress ())
	    {
	      if (_current_map_selection > 0)
		{
		  _current_map_selection--;
		}
	      else if (_current_map_selection == 0)
		{
		  _current_map_selection = global_attack_points.size () - 1;
		}
	    }
	  else if (InputManager->CancelPress ())
	    {
	      _cursor_state = CURSOR_SELECT_TARGET;
	    }
	}
    }

/*! PlayerCharacter


*/


    PlayerActor::PlayerActor (GlobalCharacter * const AWrapped,
			      BattleMode * const ABattleMode, uint32 AXLoc,
			      uint32 AYLoc):
    Actor (ABattleMode, AXLoc, AYLoc),
    _wrapped_character (AWrapped)
    {

      _current_animation = _wrapped_character->GetAnimation ("IDLE");
    }

    PlayerActor::~PlayerActor ()
    {

    }

    void
    PlayerActor::Update (uint32 ATimeElapsed)
    {
      if (IsAlive ())
	{
	  if (GetHealth () <= 0)
	    {
	      Die ();
	    }
	  _current_animation.Update ();	//update the current animation
	}
    }

    void
    PlayerActor::Draw ()
    {

      //draw the blended face

      std::vector < hoa_video::StillImage > head_shots =
	_wrapped_character->GetBattleHeadShots ();

      VideoManager->Move (50, 10);
      VideoManager->SetDrawFlags (VIDEO_BLEND, 0);

      uint32
	health = GetHealth ();
      uint32
	maxhealth = GetMaxHealth ();

      double
	percent = health / (double) maxhealth;

      if (percent == 0)
	{
	  VideoManager->DrawImage (head_shots[5]);
	}
      if (percent < 0.1)
	{
	  VideoManager->DrawImage (head_shots[5]);
	  double
	    alpha = percent * 10;
	  VideoManager->DrawImage (head_shots[4],
				   Color (1.0f, 1.0f, 1.0f, alpha));
	}
      else if (percent < 0.25f)
	{
	  VideoManager->DrawImage (head_shots[4]);
	  //.25 then alpha = 1
	  double
	    alpha = (percent - 0.25f / .1);
	  VideoManager->DrawImage (head_shots[3],
				   Color (1.0f, 1.0f, 1.0f, alpha));
	}
      else if (percent < 0.50f)
	{
	  VideoManager->DrawImage (head_shots[3]);
	  double
	    alpha = (percent - 0.25f / .25);
	  VideoManager->DrawImage (head_shots[2],
				   Color (1.0f, 1.0f, 1.0f, alpha));
	}
      else if (percent < 0.75f)
	{
	  VideoManager->DrawImage (head_shots[2]);
	  double
	    alpha = (percent - 0.25f / .50);
	  VideoManager->DrawImage (head_shots[1],
				   Color (1.0f, 1.0f, 1.0f, alpha));
	}
      else if (percent < 1.0f)
	{
	  VideoManager->DrawImage (head_shots[1]);
	  double
	    alpha = (percent - 0.25f / .75);
	  VideoManager->DrawImage (head_shots[0],
				   Color (1.0f, 1.0f, 1.0f, alpha));
	}
      else
	{
	  VideoManager->DrawImage (head_shots[0]);
	}


      if (IsAlive ())
	{
	  //more temporary crap

	  if (_TEMP_total_time_damaged > 0)
	    {

	      _TEMP_total_time_damaged += SettingsManager->GetUpdateTime ();
	      Color
	      c (1.0f, 0.0f, 0.0f, 1.0f);
	      ostringstream
		damage_amount;
	      damage_amount << _TEMP_damage_dealt;

	      TEMP_Draw_Text (c, GetXLocation () + 100, GetYLocation () + 70,
			      damage_amount.str ());

	      if (_TEMP_total_time_damaged > 3000)
		{
		  _TEMP_total_time_damaged = 0;
		  GetOwnerBattleMode ()->SetPerformingScript (false);
		}
	    }

	  Color
	  c (1.0f, 1.0f, 1.0f, 1.0f);
	  ostringstream
	    health_amount;
	  health_amount << GetHealth () << " / " << GetMaxHealth ();

	  TEMP_Draw_Text (c, 320, 90, health_amount.str ());

	  //move to x,y
	  VideoManager->Move (GetXLocation (), GetYLocation ());
	  //draw the current animation
	  VideoManager->SetDrawFlags (VIDEO_BLEND, 0);
	  _current_animation.Draw ();
	}
      else
	{
	  //draw the "dead" body here
	}
    }

/*!
        \brief Get the skills from GlobalCharacter
*/
    std::vector < GlobalSkill * >PlayerActor::GetAttackSkills ()const
    {
      return
	_wrapped_character->
      GetAttackSkills ();
    }

    std::vector <
    GlobalSkill * >
    PlayerActor::GetDefenseSkills () const
    {
      return
	_wrapped_character->
      GetDefenseSkills ();
    }

    std::vector <
    GlobalSkill * >
    PlayerActor::GetSupportSkills () const
    {
      return
	_wrapped_character->
      GetSupportSkills ();
    }

/*!
        \brief More getters from GlobalCharacter
*/
    const
      std::string
    PlayerActor::GetName () const
    {
      return
	_wrapped_character->
      GetName ();
    }

    const
      std::vector <
    GlobalAttackPoint * >
    PlayerActor::GetAttackPoints () const
    {
      return
	_wrapped_character->
      GetAttackPoints ();
    }

    uint32
    PlayerActor::GetHealth () const
    {
      return
	_wrapped_character->
      GetHP ();
    }

    void
    PlayerActor::SetHealth (uint32 AHealth)
    {
      _wrapped_character->SetHP (AHealth);
    }

    uint32
    PlayerActor::GetMaxHealth () const
    {
      return
	_wrapped_character->
      GetMaxHP ();
    }

    uint32
    PlayerActor::GetSkillPoints () const
    {
      return
	_wrapped_character->
      GetSP ();
    }

    void
    PlayerActor::SetSkillPoints (uint32 ASkillPoints)
    {
      _wrapped_character->SetSP (ASkillPoints);
    }

    uint32
    PlayerActor::GetMaxSkillPoints () const
    {
      return
	_wrapped_character->
      GetMaxSP ();
    }

    uint32
    PlayerActor::GetStrength () const
    {
      return
	_wrapped_character->
      GetStrength ();
    }

    uint32
    PlayerActor::GetIntelligence () const
    {
      return
	_wrapped_character->
      GetIntelligence ();
    }

    uint32
    PlayerActor::GetAgility () const
    {
      return
	_wrapped_character->
      GetAgility ();
    }

    uint32
    PlayerActor::GetMovementSpeed () const
    {
      return
	_wrapped_character->
      GetMovementSpeed ();
    }

    void
    PlayerActor::SetAnimation (std::string ACurrentAnimation)
    {
      _current_animation =
	_wrapped_character->GetAnimation (ACurrentAnimation);
    }


/*! EnemyActor

*/

    EnemyActor::EnemyActor (GlobalEnemy AGlobalEnemy,
			    BattleMode * const ABattleMode, uint32 AXLoc,
			    uint32 AYLoc):
    Actor (ABattleMode, AXLoc, AYLoc),
    _wrapped_enemy (AGlobalEnemy)
    {

    }

    EnemyActor::~EnemyActor ()
    {

    }

    void
    EnemyActor::Update (uint32 ATimeElapsed)
    {
      /*
         static double totalHealthLost = 0;
         if( IsAlive() ) {
         totalHealthLost += ATimeElapsed/300.0f;

         float health = GetHealth() - totalHealthLost;
         if(health - (uint32)health > .5)
         health = (uint32)health + 1;
         else
         health = (uint32)health;

         if(GetHealth() > 0)
         SetHealth((uint32)health);
         else
         Die();

         if(totalHealthLost > .5)
         totalHealthLost = 0;
         }
       */
    }

    void
    EnemyActor::Draw ()
    {
      if (_TEMP_total_time_damaged > 0)
	{

	  _TEMP_total_time_damaged += SettingsManager->GetUpdateTime ();
	  Color
	  c (1.0f, 0.0f, 0.0f, 1.0f);
	  ostringstream
	    damage_amount;
	  damage_amount << _TEMP_damage_dealt;

	  TEMP_Draw_Text (c, GetXLocation () + 100, GetYLocation () + 70,
			  damage_amount.str ());

	  if (_TEMP_total_time_damaged > 3000)
	    {
	      _TEMP_total_time_damaged = 0;
	      GetOwnerBattleMode ()->SetPerformingScript (false);
	    }
	}


      if (IsAlive ())
	{
	  Color
	  c (0.0f, 1.0f, 0.0f, 1.0f);
	  //ostringstream health_amount;
	  //health_amount << "HP: " << GetHealth() << " / " << GetMaxHealth();

	  //TEMP_Draw_Text(c, GetXLocation()-20, GetYLocation()-20, health_amount.str());

	  std::vector < hoa_video::StillImage > animations =
	    _wrapped_enemy.GetAnimation ("IDLE");

	  VideoManager->Move (GetXLocation (), GetYLocation ());
	  VideoManager->SetDrawFlags (VIDEO_BLEND, 0);

	  if (animations.size () == 1)
	    {
	      VideoManager->DrawImage (animations[0]);
	      return;
	    }
	  uint32
	    health = GetHealth ();
	  uint32
	    maxhealth = GetMaxHealth ();

	  double
	    percent = health / (double) maxhealth;

	  if (percent < 0.33)
	    {
	      VideoManager->DrawImage (animations[2]);
	      double
		alpha = 1.0f - (percent * 3);
	      VideoManager->DrawImage (animations[3],
				       Color (1.0f, 1.0f, 1.0f, alpha));
	    }
	  else if (percent <= 0.66)
	    {
	      VideoManager->DrawImage (animations[1]);
	      double
		alpha = 1.0f - ((percent - 0.33f) * 3);
	      VideoManager->DrawImage (animations[2],
				       Color (1.0f, 1.0f, 1.0f, alpha));
	    }
	  else if (percent < 1.0f)
	    {
	      VideoManager->DrawImage (animations[0]);
	      double
		alpha = 1.0f - ((percent - 0.66f) * 3);
	      VideoManager->DrawImage (animations[1],
				       Color (1.0f, 1.0f, 1.0f, alpha));
	    }
	  else
	    {
	      VideoManager->DrawImage (animations[0]);
	    }
	}
    }

/*!
        Has the GlobalEnemy level up to average_level
*/
    void
    EnemyActor::LevelUp (uint32 AAverageLevel)
    {
      _wrapped_enemy.LevelSimulator (AAverageLevel);
    }

/*!
        The AI routine
*/
    void
    EnemyActor::DoAI ()
    {
      static uint32
	next_attack = 0;
      static uint32
	last_attack = 0;

      //make sure the enemy isn't queued to perform.  set the next attack time
      if (next_attack == 0 && !IsQueuedToPerform ())
	{
	  next_attack = rand () % 30000;
	  last_attack = 0;
	}

      last_attack += SettingsManager->GetUpdateTime ();

      if (last_attack > next_attack && !IsQueuedToPerform ())
	{
	  //we can perform another attack

	  std::deque < Actor * >final_targets;
	  std::deque < PlayerActor * >targets =
	    GetOwnerBattleMode ()->ReturnCharacters ();

	  for (uint8 i = 0; i < targets.size (); i++)
	    {
	      final_targets.push_back (dynamic_cast < Actor * >(targets[i]));
	    }

	  //okay, we can perform another attack.  set us up as queued to perform.
	  //cerr << this << " set to attack." << endl;
	  SetQueuedToPerform (true);
	  ScriptEvent
	  s (dynamic_cast < Actor * >(this), final_targets, "sword_swipe");
	  GetOwnerBattleMode ()->AddScriptEventToQueue (s);

	  last_attack = 0;
	  next_attack = 0;
	}
    }

/*!
        GlobalEnemy getters
*/
    const
      std::vector <
    GlobalSkill * >
    EnemyActor::GetSkills () const
    {
      return
	_wrapped_enemy.
      GetSkills ();
    }

    const
      std::string
    EnemyActor::GetName () const
    {
      return
	_wrapped_enemy.
      GetName ();
    }

    const
      std::vector <
    GlobalAttackPoint * >
    EnemyActor::GetAttackPoints () const
    {
      return
	_wrapped_enemy.
      GetAttackPoints ();
    }

    uint32
    EnemyActor::GetHealth () const
    {
      return
	_wrapped_enemy.
      GetHP ();
    }

    void
    EnemyActor::SetHealth (uint32 AHealth)
    {
      _wrapped_enemy.SetHP (AHealth);
    }

    uint32
    EnemyActor::GetMaxHealth () const
    {
      return
	_wrapped_enemy.
      GetMaxHP ();
    }

    uint32
    EnemyActor::GetSkillPoints () const
    {
      return
	_wrapped_enemy.
      GetSP ();
    }

    void
    EnemyActor::SetSkillPoints (uint32 ASkillPoints)
    {
      _wrapped_enemy.SetSP (ASkillPoints);
    }

    uint32
    EnemyActor::GetMaxSkillPoints () const
    {
      return
	_wrapped_enemy.
      GetMaxSP ();
    }

    uint32
    EnemyActor::GetStrength () const
    {
      return
	_wrapped_enemy.
      GetStrength ();
    }

    uint32
    EnemyActor::GetIntelligence () const
    {
      return
	_wrapped_enemy.
      GetIntelligence ();
    }

    uint32
    EnemyActor::GetAgility () const
    {
      return
	_wrapped_enemy.
      GetAgility ();
    }

    uint32
    EnemyActor::GetMovementSpeed () const
    {
      return
	_wrapped_enemy.
      GetMovementSpeed ();
    }

/*!
        Effect

*/

    ActorEffect::ActorEffect (Actor * const AHost, std::string AEffectName,
			      StatusSeverity AHowSevere, uint32 ATTL,
			      bool ACanMove, uint32 AHealthModifier,
			      uint32 ASkillPointModifier,
			      uint32 AStrengthModifier,
			      uint32 AIntelligenceModifier,
			      uint32 AAgilityModifier, uint32 AUpdateLength):
    _host (AHost),
    _effect_name (AEffectName),
    _TTL (ATTL),
    _severeness (AHowSevere),
    _can_move (ACanMove),
    _health_modifier (AHealthModifier),
    _skill_point_modifier (ASkillPointModifier),
    _strength_modifier (ASkillPointModifier),
    _intelligence_modifier (AIntelligenceModifier),
    _agility_modifier (AAgilityModifier),
    _update_length (AUpdateLength),
    _age (0),
    _times_updated (0)
    {
      _last_update = SettingsManager->GetUpdateTime ();
    }

    ActorEffect::~ActorEffect ()
    {

    }

    uint32
    ActorEffect::GetTTL () const
    {
      return
	_TTL;
    }

    void
    ActorEffect::Update (uint32 ATimeElapsed)
    {

    }

    Actor *const
    ActorEffect::GetHost () const
    {
      return
	_host;
    }

    std::string
    ActorEffect::GetEffectName () const
    {
      return
	_effect_name;
    }

    uint32
    ActorEffect::GetUpdateLength () const
    {
      return
	_update_length;
    }

    uint32
    ActorEffect::GetLastUpdate () const
    {
      return
	_last_update;
    }

    bool
    ActorEffect::CanMove () const
    {
      return
	_can_move;
    }

    uint32
    ActorEffect::GetHealthModifier () const
    {
      return
	_health_modifier;
    }

    uint32
    ActorEffect::GetSkillPointModifier () const
    {
      return
	_skill_point_modifier;
    }

    uint32
    ActorEffect::GetStrengthModifier () const
    {
      return
	_strength_modifier;
    }

    uint32
    ActorEffect::GetIntelligenceModifier () const
    {
      return
	_intelligence_modifier;
    }

    uint32
    ActorEffect::GetAgilityModifier () const
    {
      return
	_agility_modifier;
    }

    void
    ActorEffect::SetLastUpdate (uint32 ALastUpdate)
    {
      _last_update = ALastUpdate;
    }

    void
    ActorEffect::UndoEffect () const
    {

    }

/*!
        Scripted Event class wrapper
*/

    ScriptEvent::ScriptEvent (Actor * AHost, std::deque < Actor * >AArguments,
			      std::string AScriptName):
    _script_name (AScriptName),
    _host (AHost),
    _arguments (AArguments)
    {

    }

    ScriptEvent::~ScriptEvent ()
    {

    }

    void
    ScriptEvent::RunScript ()
    {
      //get script from global script repository and run,
      //passing in list of arguments and host actor

      //just do damage to the _arguments
      for (uint8 i = 0; i < _arguments.size (); i++)
	{
	  _arguments[i]->TEMP_Deal_Damage (rand () % 20);
	}
    }

    Actor *
    ScriptEvent::GetHost ()
    {
      return _host;
    }

  }				//end namespace for private battle

/*!
        The actual battle mode
*/

  int
    BattleMode::MAX_PLAYER_CHARACTERS_IN_BATTLE = 4;
  int
    BattleMode::MAX_ENEMY_CHARACTERS_IN_BATTLE = 8;

BattleMode::BattleMode ():
  _user_interface (this), _performing_script (false), _battle_over (false)
  {
    MusicDescriptor
      MD;
    MD.LoadMusic ("Confrontation");
    _battle_music.push_back (MD);

    Reset ();

    _TEMP_LoadTestData ();

    //std::cout << "Well, everything is loaded." << std::endl;

    //from the global party average level, level up all global enemies passed in
  }

  BattleMode::~BattleMode ()
  {
    // Delete the player actors.  Don't want to waste memory
    std::deque < PlayerActor * >::iterator pc_itr = _player_actors.begin ();
    for (; pc_itr != _player_actors.end (); pc_itr++)
      {
	delete *
	  pc_itr;
      }
    _player_actors.clear ();
    _players_characters_in_battle.clear ();

    std::deque < private_battle::EnemyActor * >::iterator it =
      _enemy_actors.begin ();
    for (; it != _enemy_actors.end (); it++)
      delete *
	it;
    _enemy_actors.clear ();

    //get rid of the battle images that we created (TEMP)
    for (uint32 i = 0; i < _battle_images.size (); i++)
      {
	VideoManager->DeleteImage (_battle_images[i]);
      }
  }

//! Resets appropriate class members. Called whenever BootMode is made the active game mode.
  void
  BattleMode::Reset ()
  {
    //VideoManager->SetCoordSys(0.0f, (float)SCREEN_LENGTH, 0.0f, (float)SCREEN_HEIGHT);
    VideoManager->SetCoordSys (0.0f, (float) SCREEN_LENGTH * TILE_SIZE, 0.0f,
    	(float) SCREEN_HEIGHT * TILE_SIZE);
    VideoManager->SetDrawFlags (VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->SetFont("battle");
    _battle_music[0].PlayMusic ();
  }

//! Wrapper function that calls different update functions depending on the battle state.
  void
  BattleMode::Update ()
  {
    uint32
      updateTime = SettingsManager->GetUpdateTime ();

    //check here for end conditions.  How many people are still alive?
    bool
      defeat = true;

    if (_players_characters_in_battle.size () == 0)
      defeat = false;

    for (uint8 i = 0; i < _players_characters_in_battle.size (); i++)
      {
	if (_players_characters_in_battle[i]->IsAlive ())
	  {
	    defeat = false;
	    break;
	  }
      }

    bool
      victory = true;

    if (_enemy_actors.size () == 0)
      victory = false;

    for (uint8 i = 0; i < _enemy_actors.size (); i++)
      {
	if (_enemy_actors[i]->IsAlive ())
	  {
	    victory = false;
	    break;
	  }
      }

    //we won or lost -- either way, the battle is over
    if (victory || defeat)
      {
	_battle_over = true;
	if (victory)
	  _victorious_battle = true;
	else
	  _victorious_battle = false;
      }

    //if the battle is not over, update the characters
    if (!_battle_over)
      {
	for (uint8 i = 0; i < _players_characters_in_battle.size (); i++)
	  {
	    _players_characters_in_battle[i]->Update (updateTime);
	  }

	for (uint8 i = 0; i < _enemy_actors.size (); i++)
	  {
	    _enemy_actors[i]->Update (updateTime);
	    _enemy_actors[i]->DoAI ();
	  }

	//check here if any scripts need to be run or characters need to perform actions
	if (!_IsPerformingScript () && _script_queue.size () > 0)
	  {
	    //cerr << _script_queue.front().GetHost() << "'s script called." << endl;
	    _script_queue.front ().RunScript ();
	    SetPerformingScript (true);
	  }
      }

    //update the user interface...
    _user_interface.Update (updateTime);
  }

//! Wrapper function that calls different draw functions depending on the battle state.
  void
  BattleMode::Draw ()
  {
    VideoManager->SetDrawFlags (VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
    _DrawBackground ();
    _user_interface.Draw ();
    _DrawCharacters ();
    _user_interface.DrawTopElements ();
  }

  void
  BattleMode::_DrawBackground ()
  {
    VideoManager->Move (0, 0);
    VideoManager->SetDrawFlags (VIDEO_NO_BLEND, 0);
    VideoManager->DrawImage (_battle_images[0]);

    //_TEMP
    VideoManager->DrawImage (_battle_images[1]);
  }

  void
  BattleMode::_DrawCharacters ()
  {
    for (unsigned int i = 0; i < _players_characters_in_battle.size (); i++)
      {
	_players_characters_in_battle[i]->Draw ();
      }

    for (unsigned int i = 0; i < _enemy_actors.size (); i++)
      {
	_enemy_actors[i]->Draw ();
      }
  }

//! Shutdown the battle mode
  void
  BattleMode::_ShutDown ()
  {
    if (BATTLE_DEBUG)
      cout << "BATTLE: ShutDown() called!" << endl;
    VideoManager->DisableFog();	// Turn off any remaining fog
    InputManager->EventHandler ();	// Clear input
    ModeManager->Pop ();	// Pop out the BattleMode state
  }

// Is the battle over?
  bool
  BattleMode::IsBattleOver ()
  {
    return _battle_over;
  }

// Was the battle victorious?
  bool
  BattleMode::IsVictorious ()
  {
    return _victorious_battle;
  }

//!Are we performing an action
  bool
  BattleMode::_IsPerformingScript ()
  {
    return _performing_script;
  }

//! Sets T/F whether an action is being performed
  void
  BattleMode::SetPerformingScript (bool AIsPerforming)
  {
    _performing_script = AIsPerforming;

    //a script has just ended.  Set them as false to perform
    //pop the script from the front
    if (_performing_script == false)
      {
	_script_queue.front ().GetHost ()->SetQueuedToPerform (false);
	_script_queue.pop_front ();	//get that script out of here!
      }
  }

  void
  BattleMode::AddScriptEventToQueue (ScriptEvent AEventToAdd)
  {
    _script_queue.push_back (AEventToAdd);
  }

//! Remove all scripted events for an actor
  void
  BattleMode::RemoveScriptedEventsForActor (Actor * AActorToRemove)
  {
    std::list < private_battle::ScriptEvent >::iterator it =
      _script_queue.begin ();

    while (it != _script_queue.end ())
      {
	if ((*it).GetHost () == AActorToRemove)
	  {
	    it = _script_queue.erase (it);	//remove this location
	  }
	else
	  {			//otherwise, increment the iterator
	    it++;
	  }
      }
  }

//! Returns all player actors
  std::deque < PlayerActor * >BattleMode::ReturnCharacters ()const
  {
    return
      _players_characters_in_battle;
  }

  void
  BattleMode::PlayerVictory ()
  {
    //stubbed for now ... go back to map mode?  Tell the GUI, show the player, pop the state
    if (BATTLE_DEBUG)
      cout << "Player has won a battle!" << endl;

    // Give player some loot
    GlobalItem *
      new_item =
      new GlobalItem (GLOBAL_HP_RECOVERY_ITEM, GLOBAL_ALL_CHARACTERS, HP_POTION, 1);
    new_item->SetRecoveryAmount (20);
    GlobalManager->AddItemToInventory (new_item);

    // Give some experience as well
    GlobalCharacter *
      claudius = GlobalManager->GetCharacter (hoa_global::GLOBAL_CLAUDIUS);
    if (claudius != 0)
      {
	claudius->AddXP (50);
      }

    VideoManager->DisableFog();	// Turn off the fog
    _ShutDown ();
  }

  void
  BattleMode::PlayerDefeat ()
  {
    if (BATTLE_DEBUG)
      cout << "Player was defeaten in a battle!" << endl;
    VideoManager->DisableFog();	// Turn off the fog
    ModeManager->Pop ();	// Pop out battle mode
    ModeManager->Pop ();	// Pop out map mode
  }

  void
  BattleMode::_BuildPlayerCharacters ()
  {
    //from global party, get characters
    //put them into PlayerCharacters

    //make sure the list is clean first
    std::deque < PlayerActor * >::iterator pc_itr = _player_actors.begin ();
    for (; pc_itr != _player_actors.end (); pc_itr++)
      {
	delete *
	  pc_itr;
      }
    _player_actors.clear ();
    _players_characters_in_battle.clear ();
  }

  void
  BattleMode::_BuildEnemyActors ()
  {
    //the x and y location of the enemies, based on how many enemies there are in the list
    int
      x = 10;			//dummy variables
    int
      y = 10;

    //make sure the list is clean first
    /*
       std::deque<private_battle::EnemyActor *>::iterator it = _enemy_actors.begin();
       for(; it != _enemy_actors.end(); it++)
       delete *it;
       _enemy_actors.clear();

       std::deque<hoa_global::GlobalEnemy>::iterator it = _global_enemies.begin();
       for(; it != _global_enemies.end(); it++)
       {
       EnemyActor *enemy = new EnemyActor(*it, this, x, y);
       _enemy_actors.push_back(enemy);
       }
     */
  }

  void
  BattleMode::SwapCharacters (private_battle::PlayerActor * AActorToRemove,
			      private_battle::PlayerActor * AActorToAdd)
  {
    //put AActorToAdd at AActorToRemove's location
    std::deque < private_battle::PlayerActor * >::iterator it =
      _players_characters_in_battle.begin ();
    for (; it != _players_characters_in_battle.end (); it++)
      {
	if (*it == AActorToRemove)
	  {
	    _players_characters_in_battle.erase (it);
	    break;
	  }
      }

    //set location and origin to removing characters location
    //and origin
    AActorToAdd->SetXOrigin (AActorToRemove->GetXOrigin ());
    AActorToAdd->SetYOrigin (AActorToRemove->GetYOrigin ());
    AActorToAdd->SetXLocation (AActorToRemove->GetXOrigin ());
    AActorToAdd->SetYLocation (AActorToRemove->GetYOrigin ());

    _players_characters_in_battle.push_back (AActorToAdd);	//add the other character to battle
  }

  uint32
  BattleMode::NumberOfPlayerCharactersAlive ()
  {
    int
      numAlive = 0;

    std::deque < private_battle::PlayerActor * >::iterator it =
      _players_characters_in_battle.begin ();
    for (; it != _players_characters_in_battle.end (); it++)
      {
	if ((*it)->IsAlive ())
	  {
	    numAlive++;
	  }
      }

    return numAlive;
  }

  uint32
  BattleMode::GetNumberOfPlayerCharacters ()
  {
    return _players_characters_in_battle.size ();
  }

  uint32
  BattleMode::GetNumberOfEnemyActors ()
  {
    return _enemy_actors.size ();
  }

  int32
  BattleMode::GetIndexOfFirstAliveEnemy ()
  {
    int32
      index = -1;

    std::deque < private_battle::EnemyActor * >::iterator it =
      _enemy_actors.begin ();
    for (uint32 i = 0; it != _enemy_actors.end (); i++, it++)
      {
	if ((*it)->IsAlive ())
	  {
	    return i;
	  }
      }

    return index;
  }

  int32
  BattleMode::GetIndexOfFirstIdleCharacter ()
  {
    int32
      index = -1;

    std::deque < private_battle::PlayerActor * >::iterator it =
      _players_characters_in_battle.begin ();
    for (uint32 i = 0; it != _players_characters_in_battle.end (); i++, it++)
      {
	if (!(*it)->IsQueuedToPerform () && (*it)->IsAlive ())
	  {
	    index = i;
	    break;
	  }
      }

    return index;
  }

  private_battle::PlayerActor *
    BattleMode::GetPlayerCharacterAt (uint32 AIndex) const
  {
    return
      _players_characters_in_battle[AIndex];
  }

  private_battle::EnemyActor *
  BattleMode::GetEnemyActorAt (uint32 AIndex) const
  {
    return
      _enemy_actors[AIndex];
  }

  int32
  BattleMode::IndexLocationOfPlayerCharacter (private_battle::PlayerActor *
					      const AActor)
  {
    int32
      index = 0;
    std::deque < private_battle::PlayerActor * >::iterator it =
      _players_characters_in_battle.begin ();
    for (; it != _players_characters_in_battle.end (); it++)
      {
	if (*it == AActor)
	  {
	    return index;
	  }
	else
	  index++;
      }
    return -1;
  }

  void
  BattleMode::_TEMP_LoadTestData ()
  {
    StillImage
      backgrd;
    StillImage
      overback;


    backgrd.SetFilename ("img/backdrops/battle/battle_cave.png");
    backgrd.SetDimensions (SCREEN_LENGTH * TILE_SIZE,
			   SCREEN_HEIGHT * TILE_SIZE);
    _battle_images.push_back (backgrd);
    if (!VideoManager->LoadImage (_battle_images[0]))
      {
	cerr << "Failed to load background image." << endl;	//failed to laod image
	_ShutDown ();
      }

    overback.SetFilename ("img/menus/battle_bottom_menu.png");
    overback.SetDimensions (1024, 128);
    _battle_images.push_back (overback);
    if (!VideoManager->LoadImage (_battle_images[1]))
      {
	cerr << "Failed to load background over image." << endl;	//failed to laod image
	_ShutDown ();
      }


    std::vector < hoa_video::StillImage > enemyAnimation;
    StillImage
      anim;
    anim.SetDimensions (64, 64);
    anim.SetFilename ("img/sprites/battle/enemies/spider_d0.png");
    enemyAnimation.push_back (anim);
    anim.SetFilename ("img/sprites/battle/enemies/spider_d1.png");
    enemyAnimation.push_back (anim);
    anim.SetFilename ("img/sprites/battle/enemies/spider_d2.png");
    enemyAnimation.push_back (anim);
    anim.SetFilename ("img/sprites/battle/enemies/spider_d3.png");
    enemyAnimation.push_back (anim);

    VideoManager->BeginImageLoadBatch ();
    for (uint32 i = 0; i < enemyAnimation.size (); i++)
      {
	if (!VideoManager->LoadImage (enemyAnimation[i]))
	  {
	    cerr << "Failed to load spider image." << endl;	//failed to laod image
	    _ShutDown ();
	  }
      }
    VideoManager->EndImageLoadBatch ();

    std::vector < hoa_video::StillImage > enemyAnimation2;
    StillImage
      anim2;
    anim2.SetDimensions (64, 128);
    anim2.SetFilename ("img/sprites/battle/enemies/skeleton_d0.png");
    enemyAnimation2.push_back (anim2);
    anim2.SetFilename ("img/sprites/battle/enemies/skeleton_d1.png");
    enemyAnimation2.push_back (anim2);
    anim2.SetFilename ("img/sprites/battle/enemies/skeleton_d2.png");
    enemyAnimation2.push_back (anim2);
    anim2.SetFilename ("img/sprites/battle/enemies/skeleton_d3.png");
    enemyAnimation2.push_back (anim2);

    VideoManager->BeginImageLoadBatch ();
    for (uint32 i = 0; i < enemyAnimation2.size (); i++)
      {
	if (!VideoManager->LoadImage (enemyAnimation2[i]))
	  {
	    cerr << "Failed to load skeleton image." << endl;	//failed to laod image
	    _ShutDown ();
	  }
      }
    VideoManager->EndImageLoadBatch ();


    std::vector < hoa_video::StillImage > enemyAnimation3;
    StillImage
      anim3;
    anim3.SetDimensions (64, 64);
    anim3.SetFilename ("img/sprites/battle/enemies/greenslime_d0.png");
    enemyAnimation3.push_back (anim3);
    anim3.SetFilename ("img/sprites/battle/enemies/greenslime_d1.png");
    enemyAnimation3.push_back (anim3);
    anim3.SetFilename ("img/sprites/battle/enemies/greenslime_d2.png");
    enemyAnimation3.push_back (anim3);
    anim3.SetFilename ("img/sprites/battle/enemies/greenslime_d3.png");
    enemyAnimation3.push_back (anim3);

    VideoManager->BeginImageLoadBatch ();
    for (uint32 i = 0; i < enemyAnimation3.size (); i++)
      {
	if (!VideoManager->LoadImage (enemyAnimation3[i]))
	  {
	    cerr << "Failed to load green slime image." << endl;	//failed to laod image
	    _ShutDown ();
	  }
      }
    VideoManager->EndImageLoadBatch ();

    std::vector < hoa_video::StillImage > enemyAnimation4;
    StillImage
      anim4;
    anim4.SetDimensions (128, 64);
    anim4.SetFilename ("img/sprites/battle/enemies/snake_d0.png");
    enemyAnimation4.push_back (anim4);
    anim4.SetFilename ("img/sprites/battle/enemies/snake_d1.png");
    enemyAnimation4.push_back (anim4);
    anim4.SetFilename ("img/sprites/battle/enemies/snake_d2.png");
    enemyAnimation4.push_back (anim4);
    anim4.SetFilename ("img/sprites/battle/enemies/snake_d3.png");
    enemyAnimation4.push_back (anim4);

    VideoManager->BeginImageLoadBatch ();
    for (uint32 i = 0; i < enemyAnimation4.size (); i++)
      {
	if (!VideoManager->LoadImage (enemyAnimation4[i]))
	  {
	    cerr << "Failed to load snake image." << endl;	//failed to laod image
	    _ShutDown ();
	  }
      }
    VideoManager->EndImageLoadBatch ();

    GlobalCharacter *
      claud = GlobalManager->GetCharacter (hoa_global::GLOBAL_CLAUDIUS);
    if (claud == NULL)
      {
	std::cerr << "No claudius character?  What?." << std::endl;
	_ShutDown ();
      }
    else
      {
	//cerr << "Creating claudius player character." << endl;
	PlayerActor *
	  claudius = new PlayerActor (claud, this, 250, 200);
	_player_actors.push_back (claudius);
	_players_characters_in_battle.push_back (claudius);
	_user_interface.SetPlayerActorSelected (claudius);
      }

    GlobalEnemy
    e ("spider");
    e.AddAnimation ("IDLE", enemyAnimation);
    //e.AddAttackSkill(new GlobalSkill("sword_swipe"));
    EnemyActor *
      enemy = new EnemyActor (e, this, 600, 130);
    enemy->LevelUp (2);

    GlobalEnemy
    e2 ("skeleton");
    e2.AddAnimation ("IDLE", enemyAnimation2);
    //e2.AddAttackSkill(new GlobalSkill("sword_swipe"));
    EnemyActor *
      enemy2 = new EnemyActor (e2, this, 805, 330);
    enemy2->LevelUp (2);

    GlobalEnemy
    e3 ("slime");
    e3.AddAnimation ("IDLE", enemyAnimation3);
    //e3.AddAttackSkill(new GlobalSkill("sword_swipe"));
    EnemyActor *
      enemy3 = new EnemyActor (e3, this, 805, 170);
    enemy3->LevelUp (2);

    GlobalEnemy
    e4 ("snake");
    e4.AddAnimation ("IDLE", enemyAnimation4);
    //e3.AddAttackSkill(new GlobalSkill("sword_swipe"));
    EnemyActor *
      enemy4 = new EnemyActor (e4, this, 600, 280);
    enemy4->LevelUp (2);

    _enemy_actors.push_back (enemy);
    _enemy_actors.push_back (enemy3);
    _enemy_actors.push_back (enemy4);
    _enemy_actors.push_back (enemy2);

  }

}				// namespace hoa_battle
