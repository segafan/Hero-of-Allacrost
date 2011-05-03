------------------------------------------------------------------------------[[
-- Filename: enemies_set01.lua
--
-- Description: This file contains the definitions of multiple foes that the
-- player encounters in battle. This file contains those enemies who have ids
-- from 1-100.
--
-- At the top of each enemy entry, list the set of general traits that the enemy
-- is to possess. For example, "weak physical attack, high agility". This is done
-- so that if others modify the enemy's properties later in balancing efforts, they
-- know the general properties that the enemy needs to continue to represent. Below
-- is an example of a header you can use.
--
-- Traits -----------------------------------------------------------------------
-- HSP: high HP, low SP
-- ATK: low phys, no meta
-- DEF: low phys, med meta
-- SPD: med agi, low eva
-- XPD: med XP, vlow drunes
--------------------------------------------------------------------------------
--
-- The three letter acronym categories mean the following
-- HSP = HP/SP
-- ATK = Attack Ratings (physical and metaphysical)
-- DEF = Defense Ratings (physical and metaphysical)
-- SPD = Speed Ratings (agility and evade)
-- XPD = Experience points and drunes rewarded
--
-- To stay consistent, use the following degree indicators for each stat:
-- {zero, vlow, low, med, high, vhigh}
------------------------------------------------------------------------------]]

-- All enemy definitions are stored in this table
-- check to see if the enemies table has already been created by another script
if (_G.enemies == nil) then
   enemies = {}
end


-- Traits -----------------------------------------------------------------------
-- HSP: low HP, vlow SP
-- ATK: low phys, zero meta
-- DEF: low phys, vlow meta
-- SPD: low agi, low eva
-- XPD: low XP, vlow drunes
--------------------------------------------------------------------------------
enemies[1] = {
	name = hoa_system.Translate("Green Slime"),
	filename = "green_slime",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 55,
		skill_points = 10,
		strength = 15,
		vigor = 0,
		fortitude = 18,
		protection = 10,
		agility = 24,
		evade = 2.0,
		experience_points = 5,
		drunes = 10
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Body"),
			x_position = 5,
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

-- Traits -----------------------------------------------------------------------
-- HSP: low HP, vlow SP
-- ATK: low phys, zero meta
-- DEF: med phys, low meta
-- SPD: low agi, low eva
-- XPD: low XP, low drunes
--------------------------------------------------------------------------------
enemies[2] = {
	name = hoa_system.Translate("Spider"),
	filename = "spider",
	sprite_width = 64,
	sprite_height = 64,

	base_stats = {
		hit_points = 125,
		skill_points = 10,
		strength = 15,
		vigor = 0,
		fortitude = 11,
		protection = 4,
		agility = 18,
		evade = 2.0,
		experience_points = 6,
		drunes = 12
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = -15,
			y_position = 33,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Abdomen"),
			x_position = 16,
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

-- Traits -----------------------------------------------------------------------
-- HSP: med HP, low SP
-- ATK: med phys, zero meta
-- DEF: med phys, low meta
-- SPD: med agi, low eva
-- XPD: low XP, med drunes
--------------------------------------------------------------------------------
enemies[3] = {
	name = hoa_system.Translate("Snake"),
	filename = "snake",
	sprite_width = 128,
	sprite_height = 64,

	base_stats = {
		hit_points = 128,
		skill_points = 10,
		strength = 14,
		vigor = 0,
		fortitude = 9,
		protection = 4,
		agility = 15,
		evade = 2.0,
		experience_points = 7,
		drunes = 14
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = -40,
			y_position = 60,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Body"),
			x_position = -6,
			y_position = 25,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = hoa_system.Translate("Tail"),
			x_position = 14,
			y_position = 38,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0,
			status_effects = { [hoa_global.GameGlobal.GLOBAL_STATUS_STRENGTH_LOWER] = 10.0 }
		}
	},

	skills = {
		1003
	},

	drop_objects = {
		{ 1, 0.10 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: med HP, med SP
-- ATK: high phys, zero meta
-- DEF: high phys, low meta
-- SPD: low agi, low eva
-- XPD: med XP, med drunes
--------------------------------------------------------------------------------
enemies[4] = {
	name = hoa_system.Translate("Skeleton"),
	filename = "skeleton",
	sprite_width = 64,
	sprite_height = 128,
	
	base_stats = {
		hit_points = 124,
		skill_points = 10,
		strength = 15,
		vigor = 0,
		fortitude = 14,
		protection = 4,
		agility = 13,
		evade = 2.0,
		experience_points = 5,
		drunes = 18
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = -23,
			y_position = 108,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Chest"),
			x_position = -12,
			y_position = 82,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = hoa_system.Translate("Legs"),
			x_position = -26,
			y_position = 56,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0,
			status_effects = { [hoa_global.GameGlobal.GLOBAL_STATUS_AGILITY_LOWER] = 20.0 }
		}
	},

	skills = {
		1004
	},

	drop_objects = {
		{ 1, 0.15 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: low HP, low SP
-- ATK: med phys, zero meta
-- DEF: med phys, low meta
-- SPD: high agi, med eva
-- XPD: med XP, low drunes
--------------------------------------------------------------------------------
enemies[5] = {
	name = hoa_system.Translate("Scorpion"),
	filename = "scorpion",
	sprite_width = 64,
	sprite_height = 64,
	
	base_stats = {
		hit_points = 122,
		skill_points = 10,
		strength = 18,
		vigor = 0,
		fortitude = 12,
		protection = 4,
		agility = 14,
		evade = 2.0,
		experience_points = 8,
		drunes = 12
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Head"),
			x_position = -6,
			y_position = 23,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Thorax"),
			x_position = 7,
			y_position = 26,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = hoa_system.Translate("Legs"),
			x_position = 16,
			y_position = 14,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0,
			status_effects = { [hoa_global.GameGlobal.GLOBAL_STATUS_AGILITY_LOWER] = 10.0 }
		}
	},

	skills = {
		1002
	},

	drop_objects = {
		{ 1, 0.15 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: med HP, low SP
-- ATK: low phys, zero meta
-- DEF: low phys, low meta
-- SPD: high agi, med eva
-- XPD: low XP, low drunes
--------------------------------------------------------------------------------
enemies[6] = {
	name = hoa_system.Translate("Bat"),
	filename = "bat",
	sprite_width = 64,
	sprite_height = 128,
	
	base_stats = {
		hit_points = 122,
		skill_points = 10,
		strength = 10,
		vigor = 0,
		fortitude = 10,
		protection = 4,
		agility = 30,
		evade = 18.0,
		experience_points = 8,
		drunes = 12
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Torso"),
			x_position = -6,
			y_position = 23,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Wings"),
			x_position = 7,
			y_position = 26,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
	},

	skills = {
		1002
	},

	drop_objects = {
		{ 1, 0.15 } -- Minor Healing Potion
	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: vhigh HP, med SP
-- ATK: high phys, zero meta
-- DEF: high phys, low meta
-- SPD: med agi, low eva
-- XPD: vhigh XP, high drunes
-- Notes: First boss in prologue module
--------------------------------------------------------------------------------
enemies[91] = {
	name = hoa_system.Translate("Scorpion Goliath"),
	filename = "scorpion_goliath",
	sprite_width = 512,
	sprite_height = 448,
	
	base_stats = {
		hit_points = 122,
		skill_points = 10,
		strength = 10,
		vigor = 0,
		fortitude = 10,
		protection = 4,
		agility = 30,
		evade = 18.0,
		experience_points = 8,
		drunes = 12
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Mesosoma"),
			x_position = -6,
			y_position = 23,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Pincers"),
			x_position = 7,
			y_position = 26,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = hoa_system.Translate("Legs"),
			x_position = 7,
			y_position = 26,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
	},

	skills = {

	},

	drop_objects = {

	}
}

-- Traits -----------------------------------------------------------------------
-- HSP: vhigh HP, med SP
-- ATK: high phys, zero meta
-- DEF: high phys, low meta
-- SPD: med agi, low eva
-- XPD: vhigh XP, high drunes
-- Notes: First boss in prologue module
--------------------------------------------------------------------------------
enemies[91] = {
	name = hoa_system.Translate("Scorpion Goliath"),
	filename = "scorpion_goliath",
	sprite_width = 512,
	sprite_height = 448,
	
	base_stats = {
		hit_points = 122,
		skill_points = 10,
		strength = 10,
		vigor = 0,
		fortitude = 10,
		protection = 4,
		agility = 30,
		evade = 18.0,
		experience_points = 8,
		drunes = 12
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Mesosoma"),
			x_position = -6,
			y_position = 23,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Pincers"),
			x_position = 7,
			y_position = 26,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = hoa_system.Translate("Legs"),
			x_position = 7,
			y_position = 26,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
	},

	skills = {

	},

	drop_objects = {

	}
}


-- Traits -----------------------------------------------------------------------
-- HSP: high HP, med SP
-- ATK: med phys, zero meta
-- DEF: med phys, low meta
-- SPD: med agi, low eva
-- XPD: high XP, high drunes
-- Notes: Second boss in prologue module
--------------------------------------------------------------------------------
enemies[91] = {
	name = hoa_system.Translate("Armored Beast"),
	filename = "armored_beast",
	sprite_width = 256,
	sprite_height = 256,
	
	base_stats = {
		hit_points = 122,
		skill_points = 10,
		strength = 10,
		vigor = 0,
		fortitude = 10,
		protection = 4,
		agility = 30,
		evade = 18.0,
		experience_points = 8,
		drunes = 12
	},

	attack_points = {
		[1] = {
			name = hoa_system.Translate("Mesosoma"),
			x_position = -6,
			y_position = 23,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[2] = {
			name = hoa_system.Translate("Pincers"),
			x_position = 7,
			y_position = 26,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
		[3] = {
			name = hoa_system.Translate("Legs"),
			x_position = 7,
			y_position = 26,
			fortitude_modifier = 0.0,
			protection_modifier = 0.0,
			evade_modifier = 0.0
		},
	},

	skills = {

	},

	drop_objects = {

	}
}

