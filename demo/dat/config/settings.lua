--[[ 
-- This file contains all the user settings preferences. Things like screen resolution, 
--  audio volume, language, key mappings, etc. See src/engine.h for details.
--]]

video_defaults = {screen_resx = 1024, screen_resy = 768, full_screen = true};
video_settings = {screen_resx = 1024, screen_resy = 768, full_screen = true};

audio_defaults = {music_vol = 1.0, sound_vol = 1.0};
audio_settings = {music_vol = 1.0, sound_vol = 1.0};

-- Default Language: English = 0
language_defaults = 0;
language_settings = 0;


--[[ DEFAULT KEY SETTINGS --
-- 
-- up           = SDLK_UP:    273
-- down         = SDLK_DOWN:  274
-- left         = SDLK_LEFT:  276
-- right        = SDLK_RIGHT: 275
-- confirm      = SDLK_f:     102
-- cancel       = SDLK_d:     100
-- menu         = SDLK_s:     115
-- swap         = SDLK_a:     97
-- left_select  = SDLK_w:     119
-- right_select = SDLK_e:     101
-- pause        = SDLK_SPACE: 32
--------------------------]]
key_defaults = {}
key_defaults.up = 273
key_defaults.down = 274
key_defaults.left = 276
key_defaults.right = 275
key_defaults.confirm = 102
key_defaults.cancel = 100 
key_defaults.menu = 115
key_defaults.swap = 97
key_defaults.left_select = 119
key_defaults.right_select = 101
key_defaults.pause = 32

key_settings = {}
key_settings.up = 273
key_settings.down = 274
key_settings.left = 276
key_settings.right = 275
key_settings.confirm = 102
key_settings.cancel = 100
key_settings.menu = 115
key_settings.swap = 97
key_settings.left_select = 119
key_settings.right_select = 101
key_settings.pause = 32

--[[ JOYSTICK CONFIGURATION NOTES --
-- 
-- To 'deactivate' a certain button,
-- set its value to 255 (the max).
--
---------------------------]]
                
joystick_defaults = {} 
joystick_defaults.index = 0
joystick_defaults.confirm = 0
joystick_defaults.cancel = 1
joystick_defaults.menu = 2
joystick_defaults.swap = 3
joystick_defaults.left_select = 4
joystick_defaults.right_select = 5
joystick_defaults.pause = 6
joystick_defaults.quit = 7

joystick_settings = {} 
joystick_settings.index = 0
joystick_settings.confirm = 0
joystick_settings.cancel = 1
joystick_settings.menu = 2
joystick_settings.swap = 3
joystick_settings.left_select = 4
joystick_settings.right_select = 5
joystick_settings.pause = 6
joystick_settings.quit = 7

-- Sets all the settings back to their default values
function SetDefaults ()
  video_settings = video_defaults;
  audio_settings = audio_defaults;
  language_settings = language_defaults;
  key_settings = key_defaults;
  joystick_settings = joystick_defaults;
end
