#########################################################################
# Open Watcom 1.6 and above
# Makefile for mshell.exe
#
# Michael Greene <greenemk@cox.net>
# November 2006
#
########################################

# uncomment for debug version
#DEBUG = 1

# Machine type see ow docs
MACHINE= -6r

#Optimization Fastest possible -otexan
OPT = -otexan

CC = wcl386
LD = wlink
RC = rc

SDIR = ..\src
BDIR = .\build

# The import lib created from ECS pmspl.dll with wlib
# allows undocumnted SPL32QMINITIALIZE
LIBS = $(SDIR)\ipmspl.lib

INCLUDE = $(SDIR);$(%watcom)\h;$(%watcom)\h\os2

!ifdef DEBUG
CFLAGS  = -i=$(INCLUDE) -za99 -w4 -od -DDEBUG $(MACHINE) -bm -bt=OS2 -mf
LDFLAGS = os2v2_pm d all op map,symf
!else
CFLAGS  = -i=$(INCLUDE) -za99 -d0 -w4 -zq $(OPT) $(MACHINE) -bm -bt=OS2 -mf
LDFLAGS = os2v2_pm op el
!endif

SRCS = $(SDIR)\create.c $(SDIR)\dlg.c $(SDIR)\menu.c $(SDIR)\mshell.c $(SDIR)\object.c

OBJS = create.obj dlg.obj menu.obj mshell.obj object.obj

all: mshell.exe

mshell.exe:
  cd build
  $(CC) -c $(CFLAGS) $(SRCS)
  $(LD) NAME mshell SYS $(LDFLAGS) LIB $(LIBS) FILE {$(OBJS)}
  $(RC) -n -x2 $(SDIR)\mshell.rc mshell.exe
  -@copy mshell.exe ..

clean : .SYMBOLIC
  -@rm .\mshell.exe
  -@rm .\build\*.obj
  -@rm .\build\*.exe
  -@rm .\build\*.err
  -@rm .\build\*.lst
  -@rm .\build\*.map
  -@rm .\src\*.res

cleanrel : .SYMBOLIC
  -@rm .\build\*.obj
  -@rm .\build\*.exe
  -@rm .\build\*.err
  -@rm .\build\*.lst
  -@rm .\build\*.map
  -@rm .\src\*.res

