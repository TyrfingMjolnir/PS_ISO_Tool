# -----------------------------------------------------------------
# PS ISO Tool MSYS/MINGW Makefile (uses GCC compiler)(CaptainCPS-X, 2013)
# -----------------------------------------------------------------
TARGET		:= 	bin/psiso_tool.exe

CC 			:= 	g++
CXXFLAGS 	:= 	-O1 -Wl,-subsystem,console -Wall -W
LDFLAGS 	:= 	-static-libgcc -static-libstdc++
LIBS		:=	-lkernel32 -lshell32 -luser32
INCLUDES	:= 	-Isource

SRCS		:= 	source/psiso_tool.cpp \
				source/psiso_tool_main.cpp

OBJS		:=	$(SRCS:.cpp=.o)

vpath %.cpp source
vpath %.obj source

.DEFAULT_GOAL := all

.PHONY : cleanup
cleanup :
	@rm -fr $(OBJS)
	@rm -fr $(TARGET)

all: $(TARGET)
	
$(TARGET): $(OBJS)
	@echo "Linking object files ..."
	@$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.cpp
	@echo "Compiling $(<F) $(@F) ..."
	@echo.
	@$(CC) $(CXXFLAGS) $(INCLUDES) -o $@ -c $<


