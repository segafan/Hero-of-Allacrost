EXTRA_DIST = configure dat fnt img mus snd
ACLOCAL_AMFLAGS = -I m4
INCLUDES = -I/usr/include/SDL -I$(top_srcdir)/src/video -I$(top_srcdir)/src

bin_PROGRAMS = allacrost

#if COND_EDITOR
MAYBE_EDITOR = src/map_editor
bin_PROGRAMS += editor
#endif

#if COND_EDITOR
AM_CXXFLAGS := $(CXXFLAGS) $(QT_CXXFLAGS)
BUILT_SOURCES = $(editor_MOC)
CLEANFILES = $(BUILT_SOURCES)
$(MAYBE_EDITOR)%.moc.cpp: $(MAYBE_EDITOR)%.h
	$(MOC) -o $@ $<
editor_MOC = $(MAYBE_EDITOR)/editor.moc.cpp $(MAYBE_EDITOR)/grid.moc.cpp
editor_SOURCES = $(MAYBE_EDITOR)/editor_main.cpp $(MAYBE_EDITOR)/editor.cpp \
	$(MAYBE_EDITOR)/editor.h $(MAYBE_EDITOR)/grid.cpp src/video/video.h \
	src/video/video.cpp src/video/tex_mgmt.cpp \
	src/video/draw.cpp src/video/fade.cpp src/video/interpolator.cpp \
	src/video/menu_window.cpp src/video/option.cpp src/video/shake.cpp \
	src/video/text.cpp src/video/textbox.cpp src/utils.cpp src/video/gui.h \
	src/video/gui.cpp src/data_bind.h \
	$(MAYBE_EDITOR)/grid.h $(MAYBE_EDITOR)/tile.cpp src/utils.h src/defs.h \
	$(MAYBE_EDITOR)/tile.h $(MAYBE_EDITOR)/tileset.cpp src/data.h src/data.cpp \
	$(MAYBE_EDITOR)/tileset.h $(editor_MOC)
editor_LDADD = $(QT_GUILINK) $(QT_LDADD) -lqt
#endif
#SUBDIRS = $(MAYBE_EDITOR) src

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
	src/scene.h src/utils.h src/video/gui.h src/video/video.h
