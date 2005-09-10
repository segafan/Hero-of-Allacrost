EXTRA_DIST = configure dat fnt img mus snd
ACLOCAL_AMFLAGS = -I m4

if COND_EDITOR
    MAYBE_EDITOR = src/map_editor
endif
#SUBDIRS = src $(MAYBE_EDITOR)

INCLUDES = -I/usr/include/SDL -I$(top_srcdir)/src/video -I$(top_srcdir)/src

bin_PROGRAMS = allacrost

allacrost_SOURCES = src/audio.cpp src/battle.cpp src/boot.cpp src/data.cpp src/engine.cpp src/global.cpp src/main.cpp src/map.cpp src/map_objects.cpp src/menu.cpp src/pause.cpp src/quit.cpp src/scene.cpp src/utils.cpp src/video/directory.cpp src/video/draw.cpp src/video/fade.cpp src/video/gui.cpp src/video/interpolator.cpp src/video/shake.cpp src/video/tex_mgmt.cpp src/video/text.cpp src/video/video.cpp src/audio.h src/battle.h src/boot.h src/data.h src/defs.h src/engine.h src/global.h src/map.h src/map_objects.h src/menu.h src/pause.h src/quit.h src/scene.h src/utils.h src/video/gui.h src/video/video.h
