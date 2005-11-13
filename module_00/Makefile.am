EXTRA_DIST = configure dat fnt img mus snd
ACLOCAL_AMFLAGS = -I m4
INCLUDES = -I/usr/include/SDL -I$(top_srcdir)/src/video -I$(top_srcdir)/src

if COND_EDITOR
EDITOR_DIR = src/map_editor
EDITOR_BIN = editor
AM_CXXFLAGS := $(CXXFLAGS) $(QT_CXXFLAGS)
BUILT_SOURCES = $(editor_MOC)
CLEANFILES = $(BUILT_SOURCES)
$(EDITOR_DIR)%.moc.cpp: $(EDITOR_DIR)%.h
	$(MOC) -o $@ $<
editor_MOC = $(EDITOR_DIR)/editor.moc.cpp $(EDITOR_DIR)/grid.moc.cpp
editor_SOURCES = $(EDITOR_DIR)/editor_main.cpp $(EDITOR_DIR)/editor.cpp \
	$(EDITOR_DIR)/editor.h $(EDITOR_DIR)/grid.cpp src/video/video.h \
	src/video/video.cpp src/video/tex_mgmt.cpp \
	src/video/draw.cpp src/video/fade.cpp src/video/interpolator.cpp \
	src/video/menu_window.cpp src/video/option.cpp src/video/shake.cpp \
	src/video/text.cpp src/video/textbox.cpp src/utils.cpp src/video/gui.h \
	src/video/gui.cpp src/data_bind.h \
	$(EDITOR_DIR)/grid.h $(EDITOR_DIR)/tile.cpp src/utils.h src/defs.h \
	$(EDITOR_DIR)/tile.h $(EDITOR_DIR)/tileset.cpp src/data.h src/data.cpp \
	$(EDITOR_DIR)/tileset.h $(editor_MOC)
editor_LDADD = $(QT_GUILINK) $(QT_LDADD) -lqt
else
EDITOR_BIN =
endif

bin_PROGRAMS = allacrost $(EDITOR_BIN)

allacrost_SOURCES = src/audio.cpp src/battle.cpp src/boot.cpp src/data.cpp \
	src/engine.cpp src/global.cpp src/main.cpp src/map_dialogue.cpp \
	src/map_objects.cpp src/map.cpp src/menu.cpp src/pause.cpp src/quit.cpp \
	src/scene.cpp src/utils.cpp src/video/draw.cpp src/video/fade.cpp \
	src/video/gui.cpp src/video/interpolator.cpp src/video/menu_window.cpp \
	src/video/option.cpp src/data_bind.h \
	src/video/shake.cpp src/video/tex_mgmt.cpp src/video/text.cpp \
	src/video/textbox.cpp src/video/video.cpp src/audio.h src/battle.h \
	src/boot.h src/data.h src/defs.h src/engine.h src/global.h src/map.h \
	src/map_dialogue.h src/map_objects.h src/menu.h src/pause.h src/quit.h \
	src/scene.h src/utils.h src/video/gui.h src/video/video.h \
	src/video/color.h src/video/context.h src/video/coord_sys.h \
	src/video/effects.h src/video/fade.cpp src/video/image.cpp src/video/image.h \
	src/video/interpolator.h src/video/menu_window.h src/video/option.h \
	src/video/particle.h src/video/particle_effect.cpp src/video/particle_effect.h \
	src/video/particle_emitter.h src/video/particle_keyframe.h \
	src/video/particle_manager.h src/video/particle_manager.cpp \
	src/video/particle_system.cpp src/video/particle_system.h \
	src/video/screen_rect.h src/video/shake.h src/video/tex_mgmt.h \
	src/video/text.h src/video/textbox.h
