-- Boot mode configuration parameters and data files to load

-- -------------------------------------------------- --
-- ----------------- VIDEO STUFF -------------------- --
-- -------------------------------------------------- --
background_image = "img/backdrops/boot_screen00.jpg";
background_image_width = 1024;
background_image_height = 768;

logo_background = "img/logos/main_logo_background.png";
logo_background_width = 666;
logo_background_height = 239;

logo_sword = "img/logos/main_logo_sword.png";
logo_sword_width = 130;
logo_sword_height = 282;

logo_text = "img/logos/main_logo_text.png";
logo_text_width = 666;
logo_text_height = 239;

coord_sys_x_left = 0;
coord_sys_x_right = 1024;
coord_sys_y_bottom = 0;
coord_sys_y_top = 768;
coord_sys_nl = 1;
-- -------------------------------------------------- --



-- -------------------------------------------------- --
-- ----------------- AUDIO STUFF -------------------- --
-- -------------------------------------------------- --
music_files = { "mus/Allacrost_Opening_Theme.ogg",
                "mus/Opening_Effect.ogg" };

sound_files = { "snd/confirm.wav",
                "snd/cancel.wav",
                "snd/obtain.wav",
                "snd/bump.wav",
                "snd/volume_test.wav" };

-- -------------------------------------------------- --

function BootBattleTest()
	print("BootBattleTest");

	GlobalManager:AddCharacter(1);
	GlobalManager:AddCharacter(2);
	GlobalManager:AddCharacter(4);
	GlobalManager:AddCharacter(8);
	GlobalManager:AddToInventory(1, 5);

	local claudius = GlobalManager:GetCharacter(1);
	claudius:AddSkill(10001); -- Karlate guard, showcasing a status effect
	
	local battle = hoa_battle.BattleMode();
	battle:AddEnemy(1);
	battle:AddEnemy(1);
	battle:AddEnemy(1);
	battle:AddEnemy(1);
	battle:AddEnemy(1);

	ModeManager:Push(battle);
end



function BootMenuTest()
	print("BootMenuTest");
end



function BootShopTest()
	print("BootShopTest");

	GlobalManager:AddCharacter(1);
	GlobalManager:AddCharacter(2);
	GlobalManager:AddCharacter(4);
	GlobalManager:AddCharacter(8);
	GlobalManager:AddDrunes(1842);
	GlobalManager:AddToInventory(1, 5);
	GlobalManager:AddToInventory(30501, 2);
	GlobalManager:AddToInventory(2, 3);
	GlobalManager:AddToInventory(3, 1);
	GlobalManager:AddToInventory(3002, 1);
	GlobalManager:AddToInventory(10001, 1);
	GlobalManager:AddToInventory(10502, 1);

	local shop = hoa_shop.ShopMode();
	shop:AddObject(1, 3);
	shop:AddObject(2, 5);
	shop:AddObject(10501, 2);
	shop:AddObject(10504, 4);
	shop:AddObject(3, 12);
	shop:AddObject(3001, 1);
	shop:AddObject(30001, 2);
	shop:AddObject(30002, 3);
	shop:AddObject(20001, 10);
	shop:AddObject(20002, 11);
	shop:AddObject(20501, 2);
	shop:AddObject(20502, 1);

	ModeManager:Push(shop);
end
