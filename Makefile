PROJECT_PATH = $(shell pwd)
export OBJSDIR = $(PROJECT_PATH)/objs
INC = -I$(PROJECT_PATH)
export INCDIR = $(INC)
PREFIX  =$(PROJECT_PATH)/resources
DATADIR = $(PREFIX)
DEFINES = -DDATADIR=\"$(DATADIR)/\"
CFLAGS = -g -Wall -O0 -lstdc++ $(DEFINES) -fPIC
CPPFLAGS = -g -Wall -I$(PROJECT_PATH) $(DEFINES) -fPIC
#DIRS = engine sdl_util glad
#FILES = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp) $(wildcard *.c))
#OBJS = $(patsubst %.cpp, %.o, $(FILES))
RTSPDIRS = edit engine engine/client engine/server game gl_util sdl_util glad ui util
CFILES = $(foreach dir, $(RTSPDIRS), $(wildcard $(dir)/*.c))
COBJS = $(patsubst %.c, %.o, $(CFILES))
CPPFILES = $(foreach dir, $(RTSPDIRS), $(wildcard $(dir)/*.cpp))
CPPOBJS = $(patsubst %.cpp, %.o, $(CPPFILES))
LIBS += -lSDL2 -lSDL2_mixer -lSDL2_ttf -lSDL2_image -lGL -lpng -lassimp -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lXmu -lpthread -pthread -lrt -lm -lfreetype -lz -ldl
LIBPATH = -L/usr/local/lib -L/usr/local/SDL2_image/lib -L/usr/local/SDL2_mixer/lib -L/usr/local/SDL2_ttf/lib -L/usr/local/png/lib
#-L/usr/lib/X11

TARGET = sdl_opengl

#all:$(TARGET)

$(TARGET): main.o $(CPPOBJS) $(COBJS)
	#$(MAKE) -C pputil
	#$(MAKE) -C rtsp
	$(CC) main.o $(CPPFLAGS) $(CFLAGS) $(COBJS) $(CPPOBJS) -o $@ $(LIBS) $(LIBPATH)
#$(LIBPATH) $(OBJSDIR)/*.o

$(OBJSDIR):
	mkdir -p $@
#$(OBJS)/%.o:%.cpp
#$(CXX) -c $< -o $@ $(CFLAGS) $(CPPFLAGS) -ItestApp -IshaderUtil -Iutils -Iglad
#%.o:%.cpp
	#$(CC) $(CPPFLAGS) $(CFLAGS) $(OBJS) -c $< -o $@ $(LIBS)

.PHONY:clean
clean :
	-$(RM) $(TARGET) $(OBJSDIR)/*.o $(COBJS) $(CPPOBJS)
