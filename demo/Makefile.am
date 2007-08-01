EXTRA_DIST = MANUAL configure dat doc img mus snd
AM_CPPFLAGS = -DDATADIR=\"$(datadir)\"
ACLOCAL_AMFLAGS = -I m4
INCLUDES = \
	-I/usr/include/SDL \
	-I/usr/include/AL \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/engine \
	-I$(top_srcdir)/src/engine/audio \
	-I$(top_srcdir)/src/engine/video \
	-I$(top_srcdir)/src/engine/script \
	-I$(top_srcdir)/src/global \
	-I$(top_srcdir)/src/modes \
	-I$(top_srcdir)/src/modes/battle \
	-I$(top_srcdir)/src/modes/boot \
	-I$(top_srcdir)/src/modes/map \
	-I$(top_srcdir)/src/modes/menu \
	-I$(top_srcdir)/src/modes/shop

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
	$(AUDIO_DIR)/audio_descriptor.cpp \
	$(AUDIO_DIR)/audio_descriptor.h \
	$(AUDIO_DIR)/audio_effects.cpp \
	$(AUDIO_DIR)/audio_effects.h \
	$(AUDIO_DIR)/audio_input.cpp \
	$(AUDIO_DIR)/audio_input.h \
	$(AUDIO_DIR)/audio_stream.cpp \
	$(AUDIO_DIR)/audio_stream.h

SCRIPT_DIR = src/engine/script
script_SOURCES = \
	$(SCRIPT_DIR)/script.cpp \
	$(SCRIPT_DIR)/script.h \
	$(SCRIPT_DIR)/script_read.cpp \
	$(SCRIPT_DIR)/script_read.h \
	$(SCRIPT_DIR)/script_write.cpp \
	$(SCRIPT_DIR)/script_write.h \
	$(SCRIPT_DIR)/script_modify.cpp \
	$(SCRIPT_DIR)/script_modify.h

ENGINE_DIR = src/engine
engine_SOURCES = \
	$(ENGINE_DIR)/system.cpp \
	$(ENGINE_DIR)/system.h \
	$(ENGINE_DIR)/input.cpp \
	$(ENGINE_DIR)/input.h \
	$(ENGINE_DIR)/mode_manager.cpp \
	$(ENGINE_DIR)/mode_manager.h \
	$(ENGINE_DIR)/socket.cpp \
	$(ENGINE_DIR)/socket.h \
	$(audio_SOURCES) \
	$(video_SOURCES) \
	$(script_SOURCES)

GLOBAL_DIR = src/global
global_SOURCES = \
	$(GLOBAL_DIR)/global.cpp \
	$(GLOBAL_DIR)/global.h \
	$(GLOBAL_DIR)/global_actors.cpp \
	$(GLOBAL_DIR)/global_actors.h \
	$(GLOBAL_DIR)/global_objects.cpp \
	$(GLOBAL_DIR)/global_objects.h \
	$(GLOBAL_DIR)/global_skills.cpp \
	$(GLOBAL_DIR)/global_skills.h

MODES_DIR = src/modes
modes_SOURCES = \
	$(MODES_DIR)/battle/battle.cpp \
	$(MODES_DIR)/battle/battle.h \
	$(MODES_DIR)/battle/battle_actions.cpp \
	$(MODES_DIR)/battle/battle_actions.h \
	$(MODES_DIR)/battle/battle_actors.cpp \
	$(MODES_DIR)/battle/battle_actors.h \
	$(MODES_DIR)/battle/battle_windows.cpp \
	$(MODES_DIR)/battle/battle_windows.h \
	$(MODES_DIR)/boot/boot.cpp \
	$(MODES_DIR)/boot/boot.h \
	$(MODES_DIR)/boot/boot_menu.cpp \
	$(MODES_DIR)/boot/boot_menu.h \
	$(MODES_DIR)/boot/boot_credits.cpp \
	$(MODES_DIR)/boot/boot_credits.h \
	$(MODES_DIR)/boot/boot_welcome.cpp \
	$(MODES_DIR)/boot/boot_welcome.h \
	$(MODES_DIR)/map/map.cpp \
	$(MODES_DIR)/map/map.h \
	$(MODES_DIR)/map/map_zones.cpp \
	$(MODES_DIR)/map/map_zones.h \
	$(MODES_DIR)/map/map_actions.cpp \
	$(MODES_DIR)/map/map_actions.h \
	$(MODES_DIR)/map/map_dialogue.cpp \
	$(MODES_DIR)/map/map_dialogue.h \
	$(MODES_DIR)/map/map_objects.cpp \
	$(MODES_DIR)/map/map_objects.h \
	$(MODES_DIR)/map/map_sprites.cpp \
	$(MODES_DIR)/map/map_sprites.h \
	$(MODES_DIR)/menu/menu.cpp \
	$(MODES_DIR)/menu/menu.h \
	$(MODES_DIR)/menu/menu_views.cpp \
	$(MODES_DIR)/menu/menu_views.h \
	$(MODES_DIR)/pause.cpp \
	$(MODES_DIR)/pause.h \
	$(MODES_DIR)/quit.cpp \
	$(MODES_DIR)/quit.h \
	$(MODES_DIR)/scene.cpp \
	$(MODES_DIR)/scene.h \
	$(MODES_DIR)/shop/shop.h \
	$(MODES_DIR)/shop/shop.cpp \
	$(MODES_DIR)/shop/shop_windows.h \
	$(MODES_DIR)/shop/shop_windows.cpp
	
if COND_EDITOR
EDITOR_DIR = src/editor
EDITOR_BIN = allacrost-editor
AM_CXXFLAGS := $(CXXFLAGS) $(QT_CXXFLAGS) -DQT_CLEAN_NAMESPACE -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT3_SUPPORT -DQT_SHARED
BUILT_SOURCES = $(allacrost_editor_MOC)
CLEANFILES = $(BUILT_SOURCES)
$(EDITOR_DIR)%.moc.cpp: $(EDITOR_DIR)%.h
	$(MOC) -o $@ $<
allacrost_editor_MOC = $(EDITOR_DIR)/editor.moc.cpp $(EDITOR_DIR)/grid.moc.cpp
allacrost_editor_SOURCES = \
	$(EDITOR_DIR)/editor_main.cpp \
	$(EDITOR_DIR)/editor.cpp \
	$(EDITOR_DIR)/editor.h \
	$(EDITOR_DIR)/grid.cpp \
	$(EDITOR_DIR)/grid.h \
	$(EDITOR_DIR)/tileset.cpp \
	$(EDITOR_DIR)/tileset.h \
	$(video_SOURCES) \
	$(script_SOURCES) \
	$(ENGINE_DIR)/socket.cpp \
	$(ENGINE_DIR)/socket.h \
	src/defs.h \
	src/utils.cpp \
	src/utils.h
nodist_allacrost_editor_SOURCES = $(allacrost_editor_MOC)
allacrost_editor_LDADD = $(QT_GUILINK) $(QT_LDADD)
else
EDITOR_BIN =
endif

bin_PROGRAMS = allacrost $(EDITOR_BIN)

allacrost_SOURCES = \
	src/main.cpp \
	src/main_options.cpp \
	src/main_options.h \
	src/defs.cpp \
	src/defs.h \
	src/utils.cpp \
	src/utils.h \
	$(engine_SOURCES) \
	$(global_SOURCES) \
	$(modes_SOURCES)

dist-hook:
	rm -rf `find $(distdir) -name .svn`

data_dirs = dat img mus snd
find_filter = -name .svn -prune -o -name ".\#*" -o -type d
find_datadirs = ( cd $(top_srcdir) && find $(data_dirs) $(find_filter) -print )
find_data = ( cd $(top_srcdir) && find $(data_dirs) $(find_filter) -o -print )

install-data-local:
	@$(NORMAL_INSTALL)
	test -z "$(datadir)" || $(mkdir_p) "$(DESTDIR)$(datadir)"
	( $(find_datadirs) ) | while read dir; do \
		$(mkdir_p) "$(DESTDIR)$(datadir)/$$dir"; \
	done; \
	( $(find_data) ) | while read file; do \
		echo $(INSTALL_DATA) "$$file" "$(DESTDIR)$(datadir)/$$file"; \
		$(INSTALL_DATA) "$$file" "$(DESTDIR)$(datadir)/$$file"; \
	done 

uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -r "$(DESTDIR)$(datadir)"
