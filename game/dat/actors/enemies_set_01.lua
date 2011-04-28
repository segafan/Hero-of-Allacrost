------------------------------------------------------------------------------[[
-- Filename: enemies_set01.lua
--
-- Description: This file contains the definitions of multiple foes that the
-- player encounters in battle. This file contains those enemies who have ids
-- from 1-100.
------------------------------------------------------------------------------]]

-- All enemy definitions are stored in this table
-- check to see if the enemies table has already been created by another script
if (_G.enemies == nil) then
   enemies = {}
end


enemies[1] = {
	name = hoa_system.Translate("Green Slime"),
	filename = "green_slime",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 22,
		skill_points = 10,
		experience_points = 5,
		strength = 15,
		vigor = 0,
		fortitude = 10,
		protection = 4,
		agility = 12,
		evade = 2.0,
		drunes = 10
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = 31,
			y_position = 54,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Body"),
			x_position = 37,
			y_position = 34,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		1001
	},

	drop_objects = {

	}
}


enemies[2] = {
	name = hoa_system.Translate("Spider"),
	filename = "spider",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 25,
		skill_points = 10,
		experience_points = 6,
		strength = 15,
		vigor = 0,
		fortitude = 11,
		protection = 4,
		agility = 18,
		evade = 2.0,
		drunes = 12
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = 17,
			y_position = 33,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Abdomen"),
			x_position = 48,
			y_position = 57,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		1002
	},

	drop_objects = {
		{ 1, 0.10 } -- Minor Healing Potion
	}
}


enemies[3] = {
	name = hoa_system.Translate("Snake"),
	filename = "snake",
	sprite_width = 128,
	sprite_height = 64,

	base_stats = {
		hit_points = 28,
		skill_points = 10,
		experience_points = 7,
		strength = 14,
		vigor = 0,
		fortitude = 9,
		protection = 4,
		agility = 15,
		evade = 2.0,
		drunes = 14
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = 24,
			y_position = 60,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Body"),
			x_position = 58,
			y_position = 25,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = hoa_system.Translate("Tail"),
			x_position = 78,
			y_position = 38,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		1003
	},

	drop_objects = {
		{ 1, 0.10 } -- Minor Healing Potion
	}
}


enemies[4] = {
	name = hoa_system.Translate("Skeleton"),
	filename = "skeleton",
	sprite_width = 64,
	sprite_height = 128,
	
	base_stats = {
		hit_points = 24,
		skill_points = 10,
		experience_points = 5,
		strength = 15,
		vigor = 0,
		fortitude = 14,
		protection = 4,
		agility = 13,
		evade = 2.0,
		drunes = 18
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = 9,
			y_position = 108,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Chest"),
			x_position = 20,
			y_position = 82,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = hoa_system.Translate("Legs"),
			x_position = 6,
			y_position = 56,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		1004
	},

	drop_objects = {
		{ 1, 0.15 } -- Minor Healing Potion
	}
}


enemies[5] = {
	name = hoa_system.Translate("Scorpion"),
	filename = "scorpion",
	sprite_width = 64,
	sprite_height = 64,
	
	base_stats = {
		hit_points = 22,
		skill_points = 10,
		experience_points = 8,
		strength = 18,
		vigor = 0,
		fortitude = 12,
		protection = 4,
		agility = 14,
		evade = 2.0,
		drunes = 12
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = 26,
			y_position = 23,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Chest"),
			x_position = 39,
			y_position = 26,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = hoa_system.Translate("Legs"),
			x_position = 48,
			y_position = 14,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		}
	},

	skills = {
		1002
	},

	drop_objects = {
		{ 1, 0.15 } -- Minor Healing Potion
	}
}
