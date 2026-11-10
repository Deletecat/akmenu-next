/*
    main.cpp
    Copyright (C) 2008-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <nds.h>
#include <maxmod7.h>
#include <string.h>
#include <fifotool.h>
#include <memtool.h>
#include <picoLoader7.h>

void VblankHandler(void) {
    inputGetAndSend();
}

#define PM_NDSLITE_ADR (4)
#define PM_NDSLITE_ISLITE BIT(6)

static u32 getSystem(void) {
    static u32 dsGen = 0;
    if (!dsGen) {
        dsGen = 1;                                                             // default ds phat
        if (readPowerManagement(PM_NDSLITE_ADR) & PM_NDSLITE_ISLITE) dsGen++;  // dslite and dsi
        if (readPowerManagement(PM_NDSLITE_ADR) == readPowerManagement(5)) dsGen++;  // dslite
    }
    return dsGen;
}

typedef void (*pico_loader_7_func_t)(void);

static volatile bool maxmodInstalled = false;

static void resetDSPico() {
    zeroMemory((void*)0x40000B0, 0x30);

    REG_IME = IME_DISABLE;
    REG_IE = 0;
    REG_IF = ~0;

    auto header7 = (pload_header7_t*)0x06000000;
    // header7->dldiDriver = (void*)0x037F8000;
    ((pico_loader_7_func_t)header7->entryPoint)();
}

static u8 check3DS(void) {
    if (!isDSiMode()) {
        return 0;
    }

    u8 byteBak = i2cReadRegister(0x4A, 0x71);
    i2cWriteRegister(0x4A, 0x71, 0xD2);
    u8 byteNew = i2cReadRegister(0x4A, 0x71);
    u8 is3DS = (byteNew != 0xD2) ? 1 : 0;
    i2cWriteRegister(0x4A, 0x71, byteBak);

    return is3DS;
}

static void menuValue32Handler(u32 value, void* data) {
    switch (value) {
        case MENU_MSG_GBA: {
            u32 ii = 0;
            if (PersonalData->gbaScreen)
                ii = (1 * PM_BACKLIGHT_BOTTOM) | PM_SOUND_AMP;
            else
                ii = (1 * PM_BACKLIGHT_TOP) | PM_SOUND_AMP;
            writePowerManagement(PM_CONTROL_REG, ii);
            swiChangeSoundBias(0, 0x400);
            swiSwitchToGBAMode();
        } break;
        case MENU_MSG_ARM7_REBOOT_PICOLOADER:
            resetDSPico();
            break;
        case MENU_MSG_SYSTEM:
            fifoSendValue32(FIFO_USER_02, getSystem());
            break;
        case MENU_MSG_IS_3DS:
            fifoSendValue32(FIFO_USER_01, check3DS());
            break;
        case MENU_MSG_MAXMOD_INSTALL:
            if (!maxmodInstalled) {
                mmInstall(FIFO_MAXMOD);
                maxmodInstalled = true;
            }
            fifoSendValue32(FIFO_USER_01, 1);
            break;
        default:
            break;
    }
}

int main() {
    // read User Settings from firmware
    readUserSettings();

    irqInit();
    fifoInit();
    touchInit();

    // Start the RTC tracking IRQ
    initClockIRQTimer(LIBNDS_DEFAULT_TIMER_RTC);

    fifoSetValue32Handler(FIFO_USER_01, menuValue32Handler, 0);

    installSystemFIFO();

    irqSet(IRQ_VBLANK, VblankHandler);
    irqEnable(IRQ_VBLANK);

    if (isDSiMode() && REG_SNDEXTCNT != 0) {
        i2cWriteRegister(0x4A, 0x12, 0x00);	// Press power-button for auto-reset
        i2cWriteRegister(0x4A, 0x70, 0x01);	// Bootflag = Warmboot/SkipHealthSafety
    }
       
    while (true) {
        if (*(u32*)(0x2FFFD0C) == 0x454D4D43) {
            SDMMC_getCid(SDMMC_DEV_CARD, (u32*)0x2FFD7BC);    // Get eMMC CID
            *(u32*)(0x2FFFD0C) = 0;
        }
        swiWaitForVBlank();
    }
}

