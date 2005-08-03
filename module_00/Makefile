CC = g++
LD = g++

INCDIRS = /usr/include/SDL src/ src/video
OPTFLAGS = DNDEBUG O2 march=athlon-xp
WRNFLAGS = Wall W ansi pedantic
CFLAGS  = $(INCDIRS:%=I%)
CFLAGS := $(CFLAGS:%=-%)

LDDIRS = 
LDLIBS = m
GLLIBS = SDL GL GLU IL ILUT SDL_mixer SDL_ttf lua lualib
LDFLAGS = $(LDDIRS:%=-L%) $(LDLIBS:%=-l%) $(GLLIBS:%=-l%)

SRC = $(wildcard src/*.cpp src/video/*.cpp)
OBJ = $(SRC:%.cpp=%.o)

DEPDIR = .depend
DEP = $(SRC:%.cpp=$(DEPDIR)/%.dep)

TARGET = allacrost

all: $(TARGET)
$(TARGET) : $(OBJ)
	$(LD) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o : %.cpp
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@

.PHONY : clean
clean :
	rm -f $(TARGET) core $(OBJ)
	rm -rf $(DEPDIR)

.PHONY : depend
depend : $(DEP)
$(DEPDIR)/%.dep : %.cpp
	@$(SHELL) -c '[ -d $(DEPDIR) ] || mkdir $(DEPDIR)'
	@$(SHELL) -c '[ -d $(DEPDIR)/src ] || mkdir $(DEPDIR)/src'
	@$(SHELL) -c '[ -d $(DEPDIR)/src/video ] || mkdir $(DEPDIR)/src/video'
	@$(SHELL) -ec 'echo -n "Building dependencies for $< - "; \
		$(CC) -M $(CFLAGS) $< > $@; echo ok.'

-include $(DEP)
