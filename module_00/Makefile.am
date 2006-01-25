EXTRA_DIST = configure dat fnt img mus snd
ACLOCAL_AMFLAGS = -I m4
INCLUDES = -I/usr/include/SDL -I$(top_srcdir)/src/video -I$(top_srcdir)/src

VIDEO_DIR = src/video
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
	
if COND_EDITOR
EDITOR_DIR = src/map_editor
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
	src/data.cpp \
	src/data.h \
	src/defs.h \
	src/utils.cpp \
	src/utils.h
editor_LDADD = $(QT_GUILINK) $(QT_LDADD) -lqt
else
EDITOR_BIN =
endif

bin_PROGRAMS = allacrost $(EDITOR_BIN)

allacrost_SOURCES = \
	src/audio.cpp \
	src/audio.h \
	src/audio_music.cpp \
	src/audio_music.h \
	src/audio_sound.cpp \
	src/audio_sound.h \
	src/battle.cpp \
	src/battle.h \
	src/battle_actions.cpp \
	src/battle_actions.h \
	src/boot.cpp \
	src/boot/h \
	src/data.cpp \
	src/data.h \
	src/data_bind.h \
	src/defs.h \
	src/engine.cpp \
	src/engine.h \
	src/global.cpp \
	src/global.h \
	src/main.cpp \
	src/map.cpp \
	src/map.h \
	src/map_actions.cpp \
	src/map_actions.h \
	src/map_dialogue.cpp \
	src/map_dialogue.h \
	src/map_objects.cpp \
	src/map_objects.h \
	src/menu.cpp \
	src/menu.h \
	src/menu_views.cpp \
	src/menu_views.h \
	src/pause.cpp \
	src/pause.h \
	src/quit.cpp \
	src/quit.h \
	src/scene.cpp \
	src/scene.h \
	src/utils.cpp \
	src/utils.h \
	$(video_SOURCES)
