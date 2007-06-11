------------------------------------------------------------------------------[[
-- Filename: enemies_set01.lua
--
-- Description: This file contains the definitions of multiple foes that the
-- player encounters in battle. This file contains those enemies who have ids
-- from 1-100.
------------------------------------------------------------------------------]]

-- All enemy definitions are stored in this table
enemies = {}



enemies[1] = {
	name = "Green Slime",
	filename = "green_slime",
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
	
	--Add in a stealable item when we get that far
	rewards = {
		item_dropped = 1, --Healing potion
		chance_to_drop = 10.0,
		money = 10
	},
	
	growth_stats = {
		hit_points = 10.0,
		skill_points = 1.0,
		experience_points = 5.0,
		strength = 8.0,
		vigor = 0.0,
		fortitude = 12.0,
		protection = 4.0,
		agility = 5.0,
		evade = 2.0
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 31,
			y_position = 54,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Body",
			x_position = 37,
			y_position = 34,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		}
	},

	skills = { 102 }
}


enemies[2] = {
	name = "Spider",
	filename = "spider",
	sprite_filename = "img/sprites/battle/enemies/spider.png",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 15,
		skill_points = 10,
		experience_points = 6,
		strength = 12,
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
		strength = 8.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0
	},

	rewards = {
		item_dropped = 1, -- Healing potion
		chance_to_drop = 10.0,
		money = 12
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 17,
			y_position = 33,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Abdomen",
			x_position = 48,
			y_position = 57,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		}
	},

	skills = { 101 }
}


enemies[3] = {
	name = "Snake",
	filename = "snake",
	sprite_filename = "img/sprites/battle/enemies/snake.png",
	sprite_width = 128,
	sprite_height = 64,

	base_stats = {
		hit_points = 15,
		skill_points = 10,
		experience_points = 7,
		strength = 14,
		vigor = 0,
		fortitude = 9,
		protection = 4,
		agility = 10,
		evade = 2.0
	},
	
	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 14.0,
		vigor = 0.0,
		fortitude = 9.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0
	},

	rewards = {
		item_dropped = 1, -- Healing potion
		chance_to_drop = 10.0,
		money = 14
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 24,
			y_position = 60,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Body",
			x_position = 58,
			y_position = 25,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Tail",
			x_position = 78,
			y_position = 38,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		}
	},

	skills = { 100 }
}


enemies[4] = {
	name = "Skeleton",
	filename = "skeleton",
	sprite_filename = "img/sprites/battle/enemies/skeleton.png",
	sprite_width = 64,
	sprite_height = 128,

	
	base_stats = {
		hit_points = 20,
		skill_points = 10,
		experience_points = 5,
		strength = 15,
		vigor = 0,
		fortitude = 12,
		protection = 4,
		agility = 10,
		evade = 2.0
	},
	
	growth_stats = {
		hit_points = 13.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 15.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0
	},

	rewards = {
		item_dropped = 1, -- Healing potion
		chance_to_drop = 15.0,
		money = 18
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 9,
			y_position = 108,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Chest",
			x_position = 20,
			y_position = 82,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Legs",
			x_position = 6,
			y_position = 56,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		}
	},

	skills = { 103 }
}


enemies[5] = {
	name = "Scorpion",
	filename = "scorpion",
	sprite_filename = "img/sprites/battle/enemies/scorpion.png",
	sprite_width = 64,
	sprite_height = 64,

	
	base_stats = {
		hit_points = 12,
		skill_points = 10,
		experience_points = 8,
		strength = 12,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 10,
		evade = 2.0
	},

	growth_stats = {
		hit_points = 11.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 9.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.0
	},

	rewards = {
		item_dropped = 1, -- Healing potion
		chance_to_drop = 15.0,
		money = 12
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 26,
			y_position = 23,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Chest",
			x_position = 39,
			y_position = 26,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Legs",
			x_position = 48,
			y_position = 14,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		}
	},

	skills = { 101 }
}
