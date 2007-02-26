------------------------------------------------------------------------------[[
-- Filename: enemies_set01.lua
--
-- Description: This file contains the definitions of multiple foes that the
-- player encounters in battle. This file contains those enemies who have ids
-- from 1-99.
------------------------------------------------------------------------------]]

-- All enemy definitions are stored in this table
enemies = {}



enemies[1] = {
	name = "Green Slime",
	sprite_filename = "img/sprites/battle/enemies/green_slime.png",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 10,
		skill_points = 10,
		experience_points = 5,
		strength = 10,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 10,
		evade = 2.0
	},

	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 10.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 31,
			y_position = 54,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Body",
			x_position = 37,
			y_position = 34,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		}
	},

	skills = {}
}


enemies[2] = {
	name = "Spider",
	img_filename = "img/sprites/battle/enemies/spider.png",
	img_width = 64,
	img_height = 64,

	base_stats = {
		hit_points = 10,
		skill_points = 10,
		experience_points = 5,
		strength = 10,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 10,
		evade = 2.0
	},

	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 10.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 17,
			y_position = 33,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Abdomen",
			x_position = 48,
			y_position = 57,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		}
	},

	skills = {}
}


enemies[3] = {
	name = "Snake",
	img_filename = "img/sprites/battle/enemies/snake.png",
	img_width = 128,
	img_height = 64,

	base_stats = {
		hit_points = 10,
		skill_points = 10,
		experience_points = 5,
		strength = 10,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 10,
		evade = 2.0
	},

	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 10.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 24,
			y_position = 60,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Body",
			x_position = 58,
			y_position = 25,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Tail",
			x_position = 78,
			y_position = 38,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		}
	},

	skills = {}
}


enemies[4] = {
	name = "Skeleton",
	img_filename = "img/sprites/battle/enemies/skeleton.png",
	img_width = 64,
	img_height = 128,

	
	base_stats = {
		hit_points = 10,
		skill_points = 10,
		experience_points = 5,
		strength = 10,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 10,
		evade = 2.0
	},

	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 10.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 9,
			y_position = 108,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Chest",
			x_position = 20,
			y_position = 82,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Legs",
			x_position = 6,
			y_position = 56,
			fortitude_bonus = 0.0,
			protection_bonus = 0.0,
			evade_bonus = 0.0
		}
	},

	skills = {}
}

