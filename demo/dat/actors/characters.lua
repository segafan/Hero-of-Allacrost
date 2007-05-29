------------------------------------------------------------------------------[[
-- Filename: characters.lua
--
-- Description: This file contains the definitions of all characters that exist
-- in Hero of Allacrost. When a new character is added to the party, this file
-- is accessed and the character is created using the data loaded from this file.
------------------------------------------------------------------------------]]

-- All character definitions are stored in this table
characters = {}



characters[hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS] = {
	name = "Claudius",
	filename = "claudius",

	base_stats = {
		max_hit_points = 120,
		max_skill_points = 15,
		strength = 12,
		vigor = 4,
		fortitude = 14,
		protection = 6,
		agility = 35,
		evade = 4.0
	},

	growth_stats = {
		hit_points = 10.0,
		skill_points = 5.0,
		strength = 10.0,
		vigor = 1.0,
		fortitude = 8.0,
		protection = 1.0,
		agility = 10.0,
		evade = 2.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 31,
			y_position = 54,
			fortitude_bonus = 0.0,
			protection_bonus = 20.0,
			evade_bonus = 25.0
		},
		[2] = {
			name = "Torso",
			x_position = 37,
			y_position = 34,
			fortitude_bonus = 40,
			protection_bonus = 10,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Arms",
			x_position = 31,
			y_position = 54,
			fortitude_bonus = 10,
			protection_bonus = 0,
			evade_bonus = 10.0
		},
		[4] = {
			name = "Legs",
			x_position = 37,
			y_position = 34,
			fortitude_bonus = 20,
			protection_bonus = 20,
			evade_bonus = 5.0
		}
	},

	initial_weapon = 10001,
	initial_head_armor = 20001,
	initial_torso_armor = 30001,
	initial_arm_armor = 40001,
	initial_leg_armor = 50001,

	initial_skills = { 1, 2 }
}


