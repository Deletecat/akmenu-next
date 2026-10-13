# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023

BLOCKSDS	?= /opt/blocksds/core
BLOCKSDSEXT	?= /opt/blocksds/external

# User config
# ===========

NAME		:= akmenu-next

GAME_TITLE      := AKMenu-Next
GAME_SUBTITLE   := github.com/coderkei
GAME_AUTHOR     := coderkei
GAME_ICON	:= icon.png

# Source code paths
# -----------------

# List of folders to combine into the root of NitroFS:
NITROFSDIR	:=

# Tools
# -----

MKDIR		:= mkdir
RM		:= rm -rf
CP		:= cp -rf
MV		:= mv -f

# Verbose flag
# ------------

ifeq ($(VERBOSE),1)
V		:=
else
V		:= @
endif

# Directories
# -----------

ARM9DIR		:= arm9
ARM7DIR		:= arm7

# Build artfacts
# --------------

ROM		:= $(NAME).nds
ROM_DSI		:= $(NAME).dsi
ROM_CIA		:= $(NAME).cia
BOOTLOADER	:= $(CURDIR)/data/load.bin

# Targets
# -------

.PHONY: all clean arm9 arm7 organize_files make_cia

all: $(ROM) $(ROM_DSI)
	$(V)$(MAKE) organize_files

clean:
	@echo "  CLEAN"
	$(V)$(MAKE) -f Makefile.arm9 clean --no-print-directory
	$(V)$(MAKE) -f Makefile.arm7 clean --no-print-directory
	$(V)$(MAKE) -C nds-bootloader clean --no-print-directory
	$(V)$(RM) $(ROM) $(ROM_DSI) data build package/*.nds package/*.dsi package/*.cia package/_nds/akmenunext/language

$(BOOTLOADER):
	$(MKDIR) -p data
	$(V)$(MAKE) -C nds-bootloader LOADBIN=$@

arm9: $(BOOTLOADER)
	$(V)+$(MAKE) -f Makefile.arm9 --no-print-directory

arm7:
	$(V)+$(MAKE) -f Makefile.arm7 --no-print-directory

ifneq ($(strip $(NITROFSDIR)),)
# Additional arguments for ndstool
NDSTOOL_ARGS	:= -d $(NITROFSDIR)

# Make the NDS ROM depend on the filesystem only if it is needed
$(ROM): $(NITROFSDIR)
endif

# Combine the title strings
ifeq ($(strip $(GAME_SUBTITLE)),)
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_AUTHOR)
else
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_SUBTITLE);$(GAME_AUTHOR)
endif

$(ROM): arm9 arm7
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7.elf -9 build/arm9.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_ARGS)

$(ROM_DSI): arm9 arm7
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7.elf -9 build/arm9.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		-g NEXT 01 "AKMENU" -z 80040407 -u 00030004 -a 00000138 -p 0001 \
		$(NDSTOOL_ARGS)

ifeq ($(OS),Windows_NT)
    MAKE_CIA = $(CURDIR)/tools/make_cia.exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        MAKE_CIA = $(CURDIR)/tools/make_cia
    endif
endif

organize_files:
	$(MV) $(ROM) $(ROM_DSI) package/
	$(CP) package/$(ROM_DSI) package/title/00030004/4e455854/content/00000000.app
	$(CP) package/$(ROM) package/boot.nds
	$(CP) package/$(ROM_DSI) package/boot.dsi
	$(V)$(MAKE) make_cia
	$(V)$(RM) package/$(ROM) package/$(ROM_DSI)
	$(CP) language package/_nds/akmenunext/

make_cia:
	$(MAKE_CIA) --srl=$(CURDIR)/package/$(ROM_DSI) -o $(CURDIR)/package/$(ROM_CIA)
