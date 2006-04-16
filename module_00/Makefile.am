EXTRA_DIST = configure dat fnt img mus snd
ACLOCAL_AMFLAGS = -I m4
INCLUDES = \
	-I/usr/include/SDL \
	-I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/engine \
	-I$(top_srcdir)/src/engine/audio \
	-I$(top_srcdir)/src/engine/video \
	-I$(top_srcdir)/src/modes \
	-I$(top_srcdir)/src/modes/battle \
	-I$(top_srcdir)/src/modes/boot \
	-I$(top_srcdir)/src/modes/map \
	-I$(top_srcdir)/src/modes/menu

VIDEO_DIR = src/engine/video
video_SOURCES = \
	$(VIDEO_DIR)/color.h \
	$(VIDEO_DIR)/context.h \
	$(VIDEO_DIR)/coord_sys.h \
	$(VIDEO_DIR)/draw.cpp \
	$(VIDEO_DIR)/effects.cpp \
	$(VIDEO_DIR)/fade.cpp \
	$(VIDEO_DIR)/fade.h \
	$(VIDEO_DIR)/gui.cpp \
	$(VIDEO_DIR)/gui.h \
	$(VIDEO_DIR)/image.cpp \
	$(VIDEO_DIR)/image.h \
	$(VIDEO_DIR)/interpolator.cpp \
	$(VIDEO_DIR)/interpolator.h \
	$(VIDEO_DIR)/menu_window.cpp \
	$(VIDEO_DIR)/menu_window.h \
	$(VIDEO_DIR)/option.cpp \
	$(VIDEO_DIR)/option.h \
	$(VIDEO_DIR)/particle.h \
	$(VIDEO_DIR)/particle_effect.cpp \
	$(VIDEO_DIR)/particle_effect.h \
	$(VIDEO_DIR)/particle_emitter.h \
	$(VIDEO_DIR)/particle_keyframe.h \
	$(VIDEO_DIR)/particle_manager.cpp \
	$(VIDEO_DIR)/particle_manager.h \
	$(VIDEO_DIR)/particle_system.cpp \
	$(VIDEO_DIR)/particle_system.h \
	$(VIDEO_DIR)/screen_rect.h \
	$(VIDEO_DIR)/shake.cpp \
	$(VIDEO_DIR)/shake.h \
	$(VIDEO_DIR)/tex_mgmt.cpp \
	$(VIDEO_DIR)/tex_mgmt.h \
	$(VIDEO_DIR)/text.cpp \
	$(VIDEO_DIR)/text.h \
	$(VIDEO_DIR)/textbox.cpp \
	$(VIDEO_DIR)/textbox.h \
	$(VIDEO_DIR)/video.cpp \
	$(VIDEO_DIR)/video.h

AUDIO_DIR = src/engine/audio
audio_SOURCES = \
	$(AUDIO_DIR)/audio.cpp \
	$(AUDIO_DIR)/audio.h \
	$(AUDIO_DIR)/audio_sound.cpp \
	$(AUDIO_DIR)/audio_sound.h \
	$(AUDIO_DIR)/audio_music.cpp \
	$(AUDIO_DIR)/audio_music.h 

ENGINE_DIR = src/engine
engine_SOURCES = \
	$(ENGINE_DIR)/settings.cpp \
	$(ENGINE_DIR)/settings.h \
	$(ENGINE_DIR)/input.cpp \
	$(ENGINE_DIR)/input.h \
	$(ENGINE_DIR)/mode_manager.cpp \
	$(ENGINE_DIR)/mode_manager.h \
	$(ENGINE_DIR)/data.cpp \
	$(ENGINE_DIR)/data.h \
	$(ENGINE_DIR)/data_bind.h \
	$(ENGINE_DIR)/global.cpp \
	$(ENGINE_DIR)/global.h \
	$(audio_SOURCES) \
	$(video_SOURCES)

MODES_DIR = src/modes
modes_SOURCES = \
	$(MODES_DIR)/battle/battle.cpp \
	$(MODES_DIR)/battle/battle.h \
	$(MODES_DIR)/boot/boot.cpp \
	$(MODES_DIR)/boot/boot.h \
	$(MODES_DIR)/boot/boot_menu.cpp \
	$(MODES_DIR)/boot/boot_menu.h \
	$(MODES_DIR)/map/map.cpp \
	$(MODES_DIR)/map/map.h \
	$(MODES_DIR)/map/map_actions.cpp \
	$(MODES_DIR)/map/map_actions.h \
	$(MODES_DIR)/map/map_dialogue.cpp \
	$(MODES_DIR)/map/map_dialogue.h \
	$(MODES_DIR)/map/map_objects.cpp \
	$(MODES_DIR)/map/map_objects.h \
	$(MODES_DIR)/menu/menu.cpp \
	$(MODES_DIR)/menu/menu.h \
	$(MODES_DIR)/menu/menu_views.cpp \
	$(MODES_DIR)/menu/menu_views.h \
	$(MODES_DIR)/pause.cpp \
	$(MODES_DIR)/pause.h \
	$(MODES_DIR)/quit.cpp \
	$(MODES_DIR)/quit.h \
	$(MODES_DIR)/scene.cpp \
	$(MODES_DIR)/scene.h
	
if COND_EDITOR
EDITOR_DIR = src/editor
EDITOR_BIN = editor
AM_CXXFLAGS := $(CXXFLAGS) $(QT_CXXFLAGS)
BUILT_SOURCES = $(editor_MOC)
CLEANFILES = $(BUILT_SOURCES)
$(EDITOR_DIR)%.moc.cpp: $(EDITOR_DIR)%.h
	$(MOC) -o $@ $<
editor_MOC = $(EDITOR_DIR)/editor.moc.cpp $(EDITOR_DIR)/grid.moc.cpp
editor_SOURCES = \
	$(EDITOR_DIR)/editor_main.cpp \
	$(EDITOR_DIR)/editor.cpp \
	$(EDITOR_DIR)/editor.h \
	$(EDITOR_DIR)/grid.cpp \
	$(EDITOR_DIR)/grid.h \
	$(EDITOR_DIR)/tile.cpp \
	$(EDITOR_DIR)/tile.h \
	$(EDITOR_DIR)/tileset.cpp \
	$(EDITOR_DIR)/tileset.h \
	$(editor_MOC) \
	$(video_SOURCES) \
	$(ENGINE_DIR)/data.cpp \
	$(ENGINE_DIR)/data.h \
	src/defs.h \
	src/utils.cpp \
	src/utils.h
editor_LDADD = $(QT_GUILINK) $(QT_LDADD) -lqt
else
EDITOR_BIN =
endif

bin_PROGRAMS = allacrost $(EDITOR_BIN)

allacrost_SOURCES = \
	src/main.cpp \
	src/defs.h \
	src/utils.cpp \
	src/utils.h \
	$(engine_SOURCES) \
	$(modes_SOURCES)
