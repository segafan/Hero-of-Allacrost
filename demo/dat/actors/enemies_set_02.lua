------------------------------------------------------------------------------[[
-- Filename: enemies_set02.lua
--
-- Description: This file contains the definitions of multiple foes that the
-- player encounters in battle. This file contains those enemies who have ids
-- from 101-200.
------------------------------------------------------------------------------]]

-- All enemy definitions are stored in this table
enemies = {}



enemies[101] = {
	name = "Daemarbora",
	filename = "daemarbora",
	sprite_filename = "img/sprites/battle/enemies/daemarbora.png",
	sprite_width = 128,
	sprite_height = 128,

	base_stats = {
		hit_points = 80,
		skill_points = 20,
		experience_points = 25,
		strength = 12,
		vigor = 20,
		fortitude = 31,
		protection = 13,
		agility = 8,
		evade = 1.0
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

	rewards = {
		item_dropped = 10002, -- Iron sword
		chance_to_drop = 5.0,
		money = 55
	},

	attack_points = {
		[1] = {
			name = "Trunk",
			x_position = 58,
			y_position = 36,
			fortitude_bonus = 10,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Branches",
			x_position = 64,
			y_position = 80,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 1.0
		}
	},

	skills = { 1 }
}


enemies[102] = {
	name = "Aerocephal",
	filename = "aerocephal",
	sprite_filename = "img/sprites/battle/enemies/aerocephal.png",
	sprite_width = 192,
	sprite_height = 192,

	
	base_stats = {
		hit_points = 50,
		skill_points = 10,
		experience_points = 20,
		strength = 10,
		vigor = 0,
		fortitude = 7,
		protection = 4,
		agility = 20,
		evade = 10.0
	},
	
	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 10.0,
		vigor = 0.0,
		fortitude = 3.0,
		protection = 4.0,
		agility = 10.0,
		evade = 2.5
	},

	rewards = {
		item_dropped = 10001, -- Karlate sword
		chance_to_drop = 10.0,
		money = 60
	},

	attack_points = {
		[1] = {
			name = "Forehead",
			x_position = 90,
			y_position = 127,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Orifice",
			x_position = 95,
			y_position = 77,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
	},

	skills = { 1 }
}


enemies[103] = {
	name = "Arcana Drake",
	filename = "arcana_drake",
	sprite_filename = "img/sprites/battle/enemies/arcana_drake.png",
	sprite_width = 192,
	sprite_height = 256,

	
	base_stats = {
		hit_points = 55,
		skill_points = 10,
		experience_points = 20,
		strength = 15,
		vigor = 0,
		fortitude = 8,
		protection = 5,
		agility = 10,
		evade = 2.0
	},
	
	growth_stats = {
		hit_points = 15.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 15.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 3.0,
		agility = 6.0,
		evade = 2.0
	},

	rewards = {
		item_dropped = 20002, --Cobalt Helm
		chance_to_drop = 5.0,
		money = 80
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
			name = "Body",
			x_position = 95,
			y_position = 111,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Tail",
			x_position = 22,
			y_position = 146,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		}
	},

	skills = { 1 }
}


enemies[104] = {
	name = "Nagaruda",
	filename = "nagaruda",
	sprite_filename = "img/sprites/battle/enemies/nagaruda.png",
	sprite_width = 192,
	sprite_height = 256,

	base_stats = {
		hit_points = 10,
		skill_points = 10,
		experience_points = 30,
		strength = 10,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 12,
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
		agility = 8.0,
		evade = 2.0
	},

	rewards = {
		item_dropped = 40001, -- Karlate shield
		chance_to_drop = 15.0,
		money = 70
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 70,
			y_position = 165,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Abdomen",
			x_position = 60,
			y_position = 115,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Tail",
			x_position = 70,
			y_position = 65,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		}
	},

	skills = { 1 }
}


enemies[105] = {
	name = "Deceleon",
	filename = "deceleon",
	sprite_filename = "img/sprites/battle/enemies/deceleon.png",
	sprite_width = 256,
	sprite_height = 256,

	base_stats = {
		hit_points = 80,
		skill_points = 10,
		experience_points = 25,
		strength = 15,
		vigor = 0,
		fortitude = 20,
		protection = 4,
		agility = 4,
		evade = 1.0
	},
	
	growth_stats = {
		hit_points = 10.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 7.0,
		vigor = 0.0,
		fortitude = 5.0,
		protection = 4.0,
		agility = 4.0,
		evade = 1.5
	},

	rewards = {
		item_dropped = 10001, -- Karlate sword
		chance_to_drop = 5.0,
		money = 85
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 152,
			y_position = 226,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Chest",
			x_position = 150,
			y_position = 190,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Arm",
			x_position = 200,
			y_position = 155,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[4] = {
			name = "Legs",
			x_position = 150,
			y_position = 105,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		}
	},

	skills = { 1, 2 }
}


enemies[106] = {
	name = "Aurum Drakueli",
	filename = "aurum-drakueli",
	sprite_filename = "img/sprites/battle/enemies/aurum-drakueli.png",
	sprite_width = 320,
	sprite_height = 256,

	
	base_stats = {
		hit_points = 100,
		skill_points = 10,
		experience_points = 40,
		strength = 22,
		vigor = 0,
		fortitude = 8,
		protection = 4,
		agility = 18,
		evade = 2.0
	},
	
	growth_stats = {
		hit_points = 15.0,
		skill_points = 10.0,
		experience_points = 5.0,
		strength = 9.0,
		vigor = 0.0,
		fortitude = 8.0,
		protection = 4.0,
		agility = 7.5,
		evade = 2.0
	},

	rewards = {
		item_dropped = 30001, -- Karlate breastplate
		chance_to_drop = 15.0,
		money = 100
	},

	attack_points = {
		[1] = {
			name = "Head",
			x_position = 156,
			y_position = 222,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[2] = {
			name = "Chest",
			x_position = 199,
			y_position = 155,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		},
		[3] = {
			name = "Arm",
			x_position = 242,
			y_position = 143,
			fortitude_bonus = 0,
			protection_bonus = 0,
			evade_bonus = 0.0
		}
	},

	skills = { 1, 2 }

}
