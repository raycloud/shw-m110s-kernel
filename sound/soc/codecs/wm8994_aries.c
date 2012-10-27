/*
 * wm8994_aries.c  --  WM8994 ALSA Soc Audio driver related Aries
 *
 *  Copyright (C) 2010 Samsung Electronics.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <plat/gpio-cfg.h>
#include <plat/map-base.h>
#include <mach/regs-clock.h>
#include "wm8994_samsung.h"

/* Debug Feature */
#define SUBJECT "wm8994_aries.c"

/* Definitions of tunning volumes for wm8994 */
struct gain_info_t playback_gain_table[PLAYBACK_GAIN_NUM] = {
	{ /* COMMON */
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_LEFT_VOLUME,	/* 610h */
		.mask = WM8994_DAC1L_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_RIGHT_VOLUME,	/* 611h */
		.mask = WM8994_DAC1R_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0
	}, { /* RCV */
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_OUTPUT_MIXER_5,		/* 31h */
		.mask = WM8994_DACL_MIXOUTL_VOL_MASK,
		.gain = 0x0 << WM8994_DACL_MIXOUTL_VOL_SHIFT
	}, {
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_OUTPUT_MIXER_6,		/* 32h */
		.mask = WM8994_DACR_MIXOUTR_VOL_MASK,
		.gain = 0x0 << WM8994_DACR_MIXOUTR_VOL_SHIFT
	}, {
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = PLAYBACK_RCV,
		.reg  = WM8994_HPOUT2_VOLUME,		/* 1Fh */
		.mask = WM8994_HPOUT2_VOL_MASK,
		.gain = 0x0 << WM8994_HPOUT2_VOL_SHIFT
	}, { /* SPK */
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_SPKMIXL_ATTENUATION,	/* 22h */
		.mask = WM8994_SPKMIXL_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_SPKMIXR_ATTENUATION,	/* 23h */
		.mask = WM8994_SPKMIXR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E     /* +5dB */
	}, {
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,
		.gain = 0
	}, {
		.mode = PLAYBACK_SPK,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x05 << WM8994_SPKOUTL_BOOST_SHIFT  /* +7.5dB */
	}, { /* HP */
		.mode = PLAYBACK_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x2F		/* -10dB */
	}, {
		.mode = PLAYBACK_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x2F		/* -10dB */
	}, {
		.mode = PLAYBACK_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, { /* SPK_HP */
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_SPKMIXL_ATTENUATION,	/* 22h */
		.mask = WM8994_SPKMIXL_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_SPKMIXR_ATTENUATION,	/* 23h */
		.mask = WM8994_SPKMIXR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x5 << WM8994_SPKOUTL_BOOST_SHIFT
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x1E
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x1E
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_SPK_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, { /* RING_SPK */
		.mode = PLAYBACK_RING_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = PLAYBACK_RING_SPK,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x5 << WM8994_SPKOUTL_BOOST_SHIFT
	}, { /* RING_HP */
		.mode = PLAYBACK_RING_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x34
	}, {
		.mode = PLAYBACK_RING_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x34
	}, {
		.mode = PLAYBACK_RING_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_RING_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, { /* RING_SPK_HP */
		.mode = PLAYBACK_RING_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = PLAYBACK_RING_SPK_HP,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x5 << WM8994_SPKOUTL_BOOST_SHIFT
	}, {
		.mode = PLAYBACK_RING_SPK_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x1E
	}, {
		.mode = PLAYBACK_RING_SPK_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x1E
	}, { /* HP_NO_MIC */
		.mode = PLAYBACK_HP_NO_MIC,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x2F		/* -10dB */
	}, {
		.mode = PLAYBACK_HP_NO_MIC,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x2F		/* -10dB */
	}, {
		.mode = PLAYBACK_HP_NO_MIC,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_HP_NO_MIC,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, { /* EXTRA_DOCK_SPEAKER */
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_OUTPUT_MIXER_5,		/* 31h */
		.mask = WM8994_DACL_MIXOUTL_VOL_MASK,
		.gain = 0x0 << WM8994_DACL_MIXOUTL_VOL_SHIFT
	}, {
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_OUTPUT_MIXER_6,		/* 32h */
		.mask = WM8994_DACR_MIXOUTR_VOL_MASK,
		.gain = 0x0 << WM8994_DACR_MIXOUTR_VOL_SHIFT
	}, {
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = PLAYBACK_EXTRA_DOCK,
		.reg  = WM8994_LINE_OUTPUTS_VOLUME,	/* 1Eh */
		.mask = WM8994_LINEOUT2_VOL_MASK,
		.gain = 0x0 << WM8994_LINEOUT2_VOL_SHIFT
	},
};

struct gain_info_t voicecall_gain_table[VOICECALL_GAIN_NUM] = {
	{ /* COMMON */
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_LEFT_VOLUME,	/* 610h */
		.mask = WM8994_DAC1L_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_RIGHT_VOLUME,	/* 611h */
		.mask = WM8994_DAC1R_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0	/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0	/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC2_LEFT_VOLUME,	/* 612h */
		.mask = WM8994_DAC2L_VOL_MASK,
		.gain = WM8994_DAC2_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC2_RIGHT_VOLUME,	/* 613h */
		.mask = WM8994_DAC2R_VOL_MASK,
		.gain = WM8994_DAC2_VU | 0xC0		/* 0dB */
	}, { /* RCV */
		.mode = VOICECALL_RCV,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x16		/* +16.5dB */
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x10				/* +30dB */
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_OUTPUT_MIXER_5,		/* 31h */
		.mask = WM8994_DACL_MIXOUTL_VOL_MASK,
		.gain = 0x0 << WM8994_DACL_MIXOUTL_VOL_SHIFT
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_OUTPUT_MIXER_6,		/* 32h */
		.mask = WM8994_DACR_MIXOUTR_VOL_MASK,
		.gain = 0x0 << WM8994_DACR_MIXOUTR_VOL_SHIFT
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x3D
	}, {
		.mode = VOICECALL_RCV,
		.reg  = WM8994_HPOUT2_VOLUME,		/* 1Fh */
		.mask = WM8994_HPOUT2_VOL_MASK,
		.gain = 0x0 << WM8994_HPOUT2_VOL_SHIFT
	}, { /* SPK */
		.mode = VOICECALL_SPK,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x10				/* +30dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x0D		/* 3dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_SPKMIXL_ATTENUATION,	/* 22h */
		.mask = WM8994_SPKMIXL_VOL_MASK,
		.gain = 0x0				/* +0dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_SPKMIXR_ATTENUATION,	/* 23h */
		.mask = WM8994_SPKMIXR_VOL_MASK,
		.gain = 0x0				/* +0dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E		/* +5dB */
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,	/* -57dB */
		.gain = 0x0
	}, {
		.mode = VOICECALL_SPK,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x7 << WM8994_SPKOUTL_BOOST_SHIFT	/* +12dB */
	}, { /* HP */
		.mode = VOICECALL_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x1D
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN1R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}, {
		.mode = VOICECALL_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}, { /* HP_NO_MIC */
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x0D		/* 3dB */
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x10
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}, {
		.mode = VOICECALL_HP_NO_MIC,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x30
	}

};

struct gain_info_t recording_gain_table[RECORDING_GAIN_NUM] = {
	{ /* MAIN */
		.mode = RECORDING_MAIN,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x18		/* +19.5dB */
	}, {
		.mode = RECORDING_MAIN,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x10				/* +30dB */
	}, {
		.mode = RECORDING_MAIN,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0       /* 0dB */
	}, {
		.mode = RECORDING_MAIN,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0       /* 0dB */
	}, { /* HP */
		.mode = RECORDING_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x15
	}, {
		.mode = RECORDING_HP,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN1R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x10
	}, {
		.mode = RECORDING_HP,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, {
		.mode = RECORDING_HP,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, { /* RECOGNITION_MAIN */
		.mode = RECORDING_REC_MAIN,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x1C		/* +25.5dB */
	}, {
		.mode = RECORDING_REC_MAIN,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x0				/* 0dB */
	}, {
		.mode = RECORDING_REC_MAIN,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xD0	/* +6dB */
	}, {
		.mode = RECORDING_REC_MAIN,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xD0	/* +6dB */
	}, { /* RECOGNITION_HP */
		.mode = RECORDING_REC_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x10		/* +7.5dB */
	}, {
		.mode = RECORDING_REC_HP,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN1R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x10   /* +30dB */
	}, {
		.mode = RECORDING_REC_HP,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, {
		.mode = RECORDING_REC_HP,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, { /* CAMCORDER_MAIN */
		.mode = RECORDING_CAM_MAIN,
		.reg  = WM8994_LEFT_LINE_INPUT_1_2_VOLUME,	/* 18h */
		.mask = WM8994_IN1L_VOL_MASK,
		.gain = WM8994_IN1L_VU | 0x18		/* +19.5dB */
	}, {
		.mode = RECORDING_CAM_MAIN,
		.reg  = WM8994_INPUT_MIXER_3,		/* 29h */
		.mask = WM8994_IN1L_MIXINL_VOL_MASK | WM8994_MIXOUTL_MIXINL_VOL_MASK,
		.gain = 0x10  /* 30dB */
	}, {
		.mode = RECORDING_CAM_MAIN,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0	/* +0dB */
	}, {
		.mode = RECORDING_CAM_MAIN,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0	/* +0dB */
	}, { /* CAMCORDER_HP */
		.mode = RECORDING_CAM_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,	/* 1Ah */
		.mask = WM8994_IN1R_VOL_MASK,
		.gain = WM8994_IN1R_VU | 0x15		/* +15dB */
	}, {
		.mode = RECORDING_CAM_HP,
		.reg  = WM8994_INPUT_MIXER_4,		/* 2Ah */
		.mask = WM8994_IN1R_MIXINR_VOL_MASK | WM8994_MIXOUTR_MIXINR_VOL_MASK,
		.gain = 0x10				/* +30dB */
	}, {
		.mode = RECORDING_CAM_HP,
		.reg  = WM8994_AIF1_ADC1_LEFT_VOLUME,	/* 400h */
		.mask = WM8994_AIF1ADC1L_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	}, {
		.mode = RECORDING_CAM_HP,
		.reg  = WM8994_AIF1_ADC1_RIGHT_VOLUME,	/* 401h */
		.mask = WM8994_AIF1ADC1R_VOL_MASK,
		.gain = WM8994_AIF1ADC1_VU | 0xC0
	},
};

struct gain_info_t fmradio_gain_table[FMRADIO_GAIN_NUM] = {
	{ /* COMMON */
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_LEFT_VOLUME,	/* 610h */
		.mask = WM8994_DAC1L_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC1_RIGHT_VOLUME,	/* 611h */
		.mask = WM8994_DAC1R_VOL_MASK,
		.gain = WM8994_DAC1_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0	/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xC0	/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC2_LEFT_VOLUME,	/* 612h */
		.mask = WM8994_DAC2L_VOL_MASK,
		.gain = WM8994_DAC2_VU | 0xC0		/* 0dB */
	}, {
		.mode = COMMON_SET_BIT,
		.reg  = WM8994_DAC2_RIGHT_VOLUME,	/* 613h */
		.mask = WM8994_DAC2R_VOL_MASK,
		.gain = WM8994_DAC2_VU | 0xC0		/* 0dB */
	}, { /* Headset */
		.mode = FMRADIO_HP,
		.reg  = WM8994_LEFT_LINE_INPUT_3_4_VOLUME,	/* 19h */
		.mask = WM8994_IN2L_VOL_MASK,
		.gain = WM8994_IN2L_VU | 0x0B
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x0B
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x3C
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x3C
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xA8
	}, {
		.mode = FMRADIO_HP,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0xA8
	}, { /* Speaker */
		.mode = FMRADIO_SPK,
		.reg  = WM8994_LEFT_LINE_INPUT_3_4_VOLUME,	/* 19h */
		.mask = WM8994_IN2L_VOL_MASK,
		.gain = WM8994_IN2L_VU | 0x0F
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x0F
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPKMIXL_ATTENUATION,	/* 22h */
		.mask = WM8994_SPKMIXL_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPKMIXR_ATTENUATION,	/* 23h */
		.mask = WM8994_SPKMIXR_VOL_MASK,
		.gain = 0x0
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = FMRADIO_SPK,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x6 << WM8994_SPKOUTL_BOOST_SHIFT
	}, { /* SPK_HP */
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_LEFT_LINE_INPUT_3_4_VOLUME,	/* 19h */
		.mask = WM8994_IN2L_VOL_MASK,
		.gain = WM8994_IN2L_VU | 0x0F
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_RIGHT_LINE_INPUT_3_4_VOLUME,	/* 1Bh */
		.mask = WM8994_IN2R_VOL_MASK,
		.gain = WM8994_IN2R_VU | 0x0F
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_LEFT_OPGA_VOLUME,	/* 20h */
		.mask = WM8994_MIXOUTL_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_RIGHT_OPGA_VOLUME,	/* 21h */
		.mask = WM8994_MIXOUTR_VOL_MASK,
		.gain = WM8994_MIXOUT_VU | 0x39
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_LEFT_OUTPUT_VOLUME,	/* 1Ch */
		.mask = WM8994_HPOUT1L_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x3C
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_RIGHT_OUTPUT_VOLUME,	/* 1Dh */
		.mask = WM8994_HPOUT1R_VOL_MASK,
		.gain = WM8994_HPOUT1_VU | 0x3C
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_AIF1_DAC1_LEFT_VOLUME,	/* 402h */
		.mask = WM8994_AIF1DAC1L_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0x70
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_AIF1_DAC1_RIGHT_VOLUME,	/* 403h */
		.mask = WM8994_AIF1DAC1R_VOL_MASK,
		.gain = WM8994_AIF1DAC1_VU | 0x70
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_CLASSD,			/* 25h */
		.mask = WM8994_SPKOUTL_BOOST_MASK,
		.gain = 0x6 << WM8994_SPKOUTL_BOOST_SHIFT
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_LEFT,	/* 26h */
		.mask = WM8994_SPKOUTL_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	}, {
		.mode = FMRADIO_SPK_HP,
		.reg  = WM8994_SPEAKER_VOLUME_RIGHT,	/* 27h */
		.mask = WM8994_SPKOUTR_VOL_MASK,
		.gain = WM8994_SPKOUT_VU | 0x3E
	},

};
static void wait_for_dc_servo(struct snd_soc_codec *codec, unsigned int op)
{
	unsigned int reg;
	int count = 0;
	unsigned int val, start;

	val = op | WM8994_DCS_ENA_CHAN_0 | WM8994_DCS_ENA_CHAN_1;

	/* Trigger the command */
	snd_soc_write(codec, WM8994_DC_SERVO_1, val);

	start = jiffies;
	pr_debug("%s: Waiting for DC servo...\n", __func__);

	do {
		count++;
		msleep(1);
		reg = snd_soc_read(codec, WM8994_DC_SERVO_1);
		pr_debug("%s: DC servo: %x\n", __func__, reg);
	} while (reg & op && count < 400);

	pr_debug("%s: DC servo took %dms\n", __func__,
		jiffies_to_msecs(jiffies - start));

	if (reg & op)
		pr_err("%s: Timed out waiting for DC Servo\n", __func__);
}

/* S5P_SLEEP_CONFIG must be controlled by codec if codec use XUSBTI */
int wm8994_configure_clock(struct snd_soc_codec *codec, int en)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	if (en) {
		clk_enable(wm8994->codec_clk);
		DEBUG_LOG("USBOSC Enabled in Sleep Mode\n");
	} else {
		clk_disable(wm8994->codec_clk);
		DEBUG_LOG("USBOSC disable in Sleep Mode\n");
	}

	return 0;
}

void audio_ctrl_mic_bias_gpio(struct wm8994_platform_data *pdata, int enable)
{
	DEBUG_LOG("enable = [%d]", enable);

	if (!pdata)
		pr_err("%s: failed to turn off micbias pin\n", __func__);
	else {
		if (enable)
			pdata->set_mic_bias(true);
		else
			pdata->set_mic_bias(false);
	}
}

/* Audio Routing routines for the universal board..wm8994 codec */
void wm8994_disable_path(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;
	enum audio_path path = wm8994->cur_path;

	DEBUG_LOG("Path = [%d]", path);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);

	switch (path) {
	case RCV:
		/* Disbale the HPOUT2 */
		val &= ~(WM8994_HPOUT2_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		/* Disable left MIXOUT */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_DAC1L_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		/* Disable right MIXOUT */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_DAC1R_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		/* Disable HPOUT Mixer */
		val = wm8994_read(codec, WM8994_HPOUT2_MIXER);
		val &= ~(WM8994_MIXOUTLVOL_TO_HPOUT2_MASK |
			WM8994_MIXOUTRVOL_TO_HPOUT2_MASK);
		wm8994_write(codec, WM8994_HPOUT2_MIXER, val);

		/* Disable mixout volume control */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
		val &= ~(WM8994_MIXOUTLVOL_ENA_MASK |
			WM8994_MIXOUTRVOL_ENA_MASK |
			WM8994_MIXOUTL_ENA_MASK |
			WM8994_MIXOUTR_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);
		break;

	case SPK:
		/* Disbale the SPKOUTL */
		val &= ~(WM8994_SPKOUTL_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		/* Disable SPKLVOL */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
		val &= ~(WM8994_SPKLVOL_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

		/* Disable SPKOUT mixer */
		val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
		val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK |
			 WM8994_SPKMIXR_TO_SPKOUTL_MASK |
			 WM8994_SPKMIXR_TO_SPKOUTR_MASK);
		wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

		/* Mute Speaker mixer */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);
		break;

	case HP:
	case HP_NO_MIC:
		val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
		val &= ~(0x02C0);
		val |= 0x02C0;
		wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, 0x02C0);

		val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
		val &= ~(0x02C0);
		val |= 0x02C0;
		wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, 0x02C0);

		val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
		val &= ~(0x0022);
		val |= 0x0022;
		wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x0022);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(0x0);
		val |= 0x0;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, 0x0);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(0x0);
		val |= 0x0;
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, 0x0);

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
		val &= ~(0x0300);
		val |= 0x0300;
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, 0x0300);

		/* Disable charge pump */
		val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
		val &= ~(0x1F25);
		val |= 0x1F25;
		wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x1F25);
		break;

	case BT:
		val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
		val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
		val |= (WM8994_AIF1DAC1_MUTE);
		wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);
		break;

	case SPK_HP:
		val &= ~(WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK |
				WM8994_SPKOUTL_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

		/* Disable DAC1L to HPOUT1L path */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_DAC1L_TO_HPOUT1L_MASK |
				WM8994_DAC1L_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		/* Disable DAC1R to HPOUT1R path */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK |
				WM8994_DAC1R_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

		/* Disable Charge Pump */
		val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
		val &= ~WM8994_CP_ENA_MASK;
		val |= WM8994_CP_ENA_DEFAULT;
		wm8994_write(codec, WM8994_CHARGE_PUMP_1, val);

		/* Intermediate HP settings */
		val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
		val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
		      WM8994_HPOUT1R_RMV_SHORT_MASK | WM8994_HPOUT1L_DLY_MASK |
		      WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
		wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

		/* Disable SPKLVOL */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
		val &= ~(WM8994_SPKLVOL_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

		/* Disable SPKOUT mixer */
		val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
		val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK |
			 WM8994_SPKMIXR_TO_SPKOUTL_MASK |
			 WM8994_SPKMIXR_TO_SPKOUTR_MASK);
		wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

		/* Mute Speaker mixer */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);
		break;

	default:
		DEBUG_LOG_ERR("Path[%d] is not invaild!\n", path);
		return;
		break;
	}
}

void wm8994_disable_rec_path(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;
	enum mic_path mic = wm8994->rec_path;

	wm8994->rec_path = MIC_OFF;

	if (!(wm8994->codec_state & CALL_ACTIVE))
		audio_ctrl_mic_bias_gpio(wm8994->pdata, 0);

	switch (mic) {
	case MAIN:
		DEBUG_LOG("Disabling MAIN Mic Path..\n");

		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
		val &= ~(WM8994_IN1L_ENA_MASK | WM8994_MIXINL_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

		/* Mute IN1L PGA, update volume */
		val = wm8994_read(codec,
				WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
		val &= ~(WM8994_IN1L_MUTE_MASK | WM8994_IN1L_VOL_MASK);
		val |= (WM8994_IN1L_VU | WM8994_IN1L_MUTE);
		wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME,
				val);

		/*Mute the PGA */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
		val &= ~(WM8994_IN1L_TO_MIXINL_MASK |
			WM8994_IN1L_MIXINL_VOL_MASK |
			WM8994_MIXOUTL_MIXINL_VOL_MASK);
		wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

		/* Disconnect IN1LN ans IN1LP to the inputs */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
		val &= (WM8994_IN1LN_TO_IN1L_MASK | WM8994_IN1LP_TO_IN1L_MASK);
		wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

		/* Digital Paths */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_ADCL_ENA_MASK | WM8994_AIF1ADC1L_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

		/* Disable timeslots */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1L_TO_AIF1ADC1L);
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);
		break;

	case SUB:
		DEBUG_LOG("Disbaling SUB Mic path..\n");
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
		val &= ~(WM8994_IN1R_ENA_MASK | WM8994_MIXINR_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

		/* Disable volume,unmute Right Line */
		val = wm8994_read(codec,
				WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
		val &= ~WM8994_IN1R_MUTE_MASK;	/* Unmute IN1R */
		val |= (WM8994_IN1R_VU | WM8994_IN1R_MUTE);
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,
			     val);

		/* Mute right pga, set volume */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
		val &= ~(WM8994_IN1R_TO_MIXINR_MASK |
		      WM8994_IN1R_MIXINR_VOL_MASK |
		      WM8994_MIXOUTR_MIXINR_VOL_MASK);
		wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

		/* Disconnect in1rn to inr1 and in1rp to inrp */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
		val &= ~(WM8994_IN1RN_TO_IN1R_MASK | WM8994_IN1RP_TO_IN1R_MASK);
		wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

		/* Digital Paths */
		/* Disable right ADC and time slot */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
		val &= ~(WM8994_ADCR_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

		/* ADC Right mixer routing */
		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_ADC1R_TO_AIF1ADC1R_MASK);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);
		break;

	case BT_REC:
		DEBUG_LOG("Disbaling BT Mic path..\n");
		val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
		val &= ~(WM8994_AIF2DACL_TO_AIF1ADC1L_MASK |
			WM8994_ADC1L_TO_AIF1ADC1L_MASK);
		wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
		val &= ~(WM8994_AIF2DACR_TO_AIF1ADC1R_MASK |
			WM8994_ADC1R_TO_AIF1ADC1R_MASK);
		wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

		val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
		val &= ~(WM8994_AIF2DAC_MUTE_MASK);
		val |= (WM8994_AIF2DAC_MUTE);
		wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);
		break;

	case MIC_OFF:
		DEBUG_LOG("Mic is already OFF!\n");
		break;

	default:
		DEBUG_LOG_ERR("Path[%d] is not invaild!\n", mic);
		break;
	}
}

void wm8994_set_bluetooth_common_setting(struct snd_soc_codec *codec)
{
	u32 val;

	wm8994_write(codec, WM8994_GPIO_1, 0xA101);
	wm8994_write(codec, WM8994_GPIO_2, 0x8100);
	wm8994_write(codec, WM8994_GPIO_3, 0x0100);
	wm8994_write(codec, WM8994_GPIO_4, 0x0100);
	wm8994_write(codec, WM8994_GPIO_5, 0x8100);
	wm8994_write(codec, WM8994_GPIO_6, 0xA101);
	wm8994_write(codec, WM8994_GPIO_7, 0x0100);
	wm8994_write(codec, WM8994_GPIO_8, 0xA101);
	wm8994_write(codec, WM8994_GPIO_9, 0xA101);
	wm8994_write(codec, WM8994_GPIO_10, 0xA101);
	wm8994_write(codec, WM8994_GPIO_11, 0xA101);

	wm8994_write(codec, WM8994_FLL2_CONTROL_2, 0x0700);
	wm8994_write(codec, WM8994_FLL2_CONTROL_3, 0x3126);
	wm8994_write(codec, WM8994_FLL2_CONTROL_4, 0x0100);
	wm8994_write(codec, WM8994_FLL2_CONTROL_5, 0x0C88);
	wm8994_write(codec, WM8994_FLL2_CONTROL_1,
		WM8994_FLL2_FRACN_ENA | WM8994_FLL2_ENA);

	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1);
	if (!(val & WM8994_AIF2CLK_ENA))
		wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0018);

	wm8994_write(codec, WM8994_AIF2_RATE, 0x9 << WM8994_AIF2CLK_RATE_SHIFT);

	/* AIF2 Interface - PCM Stereo mode */
	/* Left Justified, BCLK invert, LRCLK Invert */
	wm8994_write(codec, WM8994_AIF2_CONTROL_1,
		WM8994_AIF2ADCR_SRC | WM8994_AIF2_BCLK_INV | 0x18);

	wm8994_write(codec, WM8994_AIF2_BCLK, 0x70);
	wm8994_write(codec, WM8994_AIF2_CONTROL_2, 0x0000);
	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, WM8994_AIF2_MSTR |
		WM8994_AIF2_CLK_FRC | WM8994_AIF2_LRCLK_FRC);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK |
		WM8994_AIF1DAC1L_ENA_MASK | WM8994_AIF1DAC1R_ENA_MASK |
		WM8994_DAC1L_ENA_MASK | WM8994_DAC1R_ENA_MASK);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
		WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* Clocking */
	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val |= (WM8994_DSP_FS2CLK_ENA | WM8994_SYSCLK_SRC);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	/* AIF1 & AIF2 Output is connected to DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK |
		WM8994_AIF2DACL_TO_DAC1L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC1L | WM8994_AIF2DACL_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK |
		WM8994_AIF2DACR_TO_DAC1R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC1R | WM8994_AIF2DACR_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);
}

void wm8994_record_headset_mic(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("Recording through Headset Mic\n");

	wm8994_write(codec, WM8994_ANTIPOP_2, 0x68);

	/* Enable high pass filter to control bounce on startup */
	val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
	val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
	val |= (WM8994_AIF1ADC1R_HPF);
	wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

	/* Enable mic bias, vmid, bias generator */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
	val &= ~(WM8994_INPUTS_CLAMP_MASK);
	val |= (WM8994_INPUTS_CLAMP);
	wm8994_write(codec, WM8994_INPUT_MIXER_1, val);

	val = (WM8994_MIXINR_ENA | WM8994_IN1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);


	val = (WM8994_IN1RN_TO_IN1R | WM8994_IN1RP_TO_IN1R);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM8994_IN1R_MUTE_MASK);
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
	val &= ~(WM8994_IN1R_TO_MIXINR_MASK);
	val |= (WM8994_IN1R_TO_MIXINR);
	wm8994_write(codec, WM8994_INPUT_MIXER_4 , val);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
	val &= ~(WM8994_INPUTS_CLAMP_MASK);
	wm8994_write(codec, WM8994_INPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME);
	val |= (WM8994_AIF1ADC1_VU);
	wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_VOLUME, val);

	val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
	val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
	val |= (WM8994_AIF1ADC1R_HPF | 0x2000);
	wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

	val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
	val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);
	wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

	wm8994_write(codec, WM8994_GPIO_1, 0xA101);

	/* Mixing left channel output to right channel */
	val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
	val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
	val |= (WM8994_AIF1ADCL_SRC | WM8994_AIF1ADCR_SRC);
	wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

	/* Digital Paths  */
	/* Enable right ADC and time slot */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val &= ~(WM8994_ADCR_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
	val |= (WM8994_AIF1ADC1R_ENA | WM8994_ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);


	/* ADC Right mixer routing */
	val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC1R_TO_AIF1ADC1R_MASK);
	val |= WM8994_ADC1R_TO_AIF1ADC1R;
	wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val &= ~WM8994_MIXINL_TO_SPKMIXL_MASK;
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &= ~WM8994_MIXINL_TO_MIXOUTL_MASK;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~WM8994_MIXINR_TO_MIXOUTR_MASK;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	if (wm8994->input_source == RECOGNITION)
		wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_REC_HP);
	else if (wm8994->input_source == CAMCORDER)
		wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_CAM_HP);
	else
		wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_HP);

}

void wm8994_record_main_mic(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("Recording through Main Mic\n");
	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	/* Main mic volume issue fix: requested H/W */
	wm8994_write(codec, WM8994_ANTIPOP_2, 0x68);

	/* High pass filter to control bounce on enable */
	val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
	val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
	val |= (WM8994_AIF1ADC1L_HPF);
	wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
	val &= ~(WM8994_INPUTS_CLAMP_MASK);
	val |= (WM8994_INPUTS_CLAMP);
	wm8994_write(codec, WM8994_INPUT_MIXER_1, val);

	val = (WM8994_MIXINL_ENA | WM8994_IN1L_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

	val = (WM8994_IN1LP_TO_IN1L | WM8994_IN1LN_TO_IN1L);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);


	val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM8994_IN1L_MUTE_MASK);
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val &= ~(WM8994_IN1L_TO_MIXINL_MASK);
	val |= (WM8994_IN1L_TO_MIXINL);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_1);
	val &= ~(WM8994_INPUTS_CLAMP_MASK);
	wm8994_write(codec, WM8994_INPUT_MIXER_1, val);


	val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_VOLUME);
	val |= (WM8994_AIF1ADC1_VU);
	wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_AIF1_ADC1_FILTERS);
	val &= ~(WM8994_AIF1ADC1L_HPF_MASK | WM8994_AIF1ADC1R_HPF_MASK);
	val |= (WM8994_AIF1ADC1L_HPF | 0x2000);
	wm8994_write(codec, WM8994_AIF1_ADC1_FILTERS, val);

	val = wm8994_read(codec, WM8994_AIF1_MASTER_SLAVE);
	val |= (WM8994_AIF1_MSTR | WM8994_AIF1_CLK_FRC | WM8994_AIF1_LRCLK_FRC);
	wm8994_write(codec, WM8994_AIF1_MASTER_SLAVE, val);

	wm8994_write(codec, WM8994_GPIO_1, 0xA101);

	val = wm8994_read(codec, WM8994_AIF1_CONTROL_1);
	val &= ~(WM8994_AIF1ADCL_SRC_MASK | WM8994_AIF1ADCR_SRC_MASK);
	val |= (WM8994_AIF1ADCR_SRC);
	wm8994_write(codec, WM8994_AIF1_CONTROL_1, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val &= ~(WM8994_ADCL_ENA_MASK | WM8994_AIF1ADC1L_ENA_MASK);
	val |= (WM8994_AIF1ADC1L_ENA | WM8994_ADCL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	/* Enable timeslots */
	val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
	val |= WM8994_ADC1L_TO_AIF1ADC1L;
	wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val &= ~WM8994_MIXINL_TO_SPKMIXL_MASK;
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &= ~WM8994_MIXINL_TO_MIXOUTL_MASK;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~WM8994_MIXINR_TO_MIXOUTR_MASK;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	if (wm8994->input_source == RECOGNITION)
		wm8994_set_codec_gain(codec, RECORDING_MODE,
				RECORDING_REC_MAIN);
	else if (wm8994->input_source == CAMCORDER)
		wm8994_set_codec_gain(codec, RECORDING_MODE,
				RECORDING_CAM_MAIN);
	else
		wm8994_set_codec_gain(codec, RECORDING_MODE, RECORDING_MAIN);

}

void wm8994_record_bluetooth(struct snd_soc_codec *codec)
{
	u16 val;

	DEBUG_LOG("BT Record Path for Voice Command\n");

	wm8994_set_bluetooth_common_setting(codec);

	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_ADC1_TO_DAC2L_MASK);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_ADC1_TO_DAC2R_MASK);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 0x0000);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0000);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val &= ~(WM8994_AIF1ADC1L_ENA_MASK | WM8994_AIF1ADC1R_ENA_MASK);
	val |= (WM8994_AIF1ADC1L_ENA | WM8994_AIF1ADC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4 , val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_6);
	val &= ~(WM8994_AIF3_ADCDAT_SRC_MASK | WM8994_AIF2_DACDAT_SRC_MASK);
	val |= (0x1 << WM8994_AIF3_ADCDAT_SRC_SHIFT | WM8994_AIF2_DACDAT_SRC);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, val);

	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~(WM8994_AIF2DAC_MUTE_MASK);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

	val = wm8994_read(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF2DACL_TO_AIF1ADC1L_MASK);
	val |= (WM8994_AIF2DACL_TO_AIF1ADC1L);
	wm8994_write(codec, WM8994_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF2DACR_TO_AIF1ADC1R_MASK);
	val |= (WM8994_AIF2DACR_TO_AIF1ADC1R);
	wm8994_write(codec, WM8994_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	wm8994_write(codec, WM8994_OVERSAMPLING, 0X0000);

	wm8994_write(codec, WM8994_GPIO_8, WM8994_GP8_DIR | WM8994_GP8_DB);
	wm8994_write(codec, WM8994_GPIO_9, WM8994_GP9_DB);
	wm8994_write(codec, WM8994_GPIO_10, WM8994_GP10_DB);
	wm8994_write(codec, WM8994_GPIO_11, WM8994_GP11_DB);
}

void wm8994_set_playback_receiver(struct snd_soc_codec *codec)
{
	u16 val;

	DEBUG_LOG("");

	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_HPOUT2_VOLUME);
	val &= ~(WM8994_HPOUT2_MUTE_MASK);
	wm8994_write(codec, WM8994_HPOUT2_VOLUME, val);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &= ~(WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |= (WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= (WM8994_DAC1R_TO_MIXOUTR);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_HPOUT2_MIXER);
	val &= ~(WM8994_MIXOUTLVOL_TO_HPOUT2_MASK |
			WM8994_MIXOUTRVOL_TO_HPOUT2_MASK);
	val |= (WM8994_MIXOUTRVOL_TO_HPOUT2 | WM8994_MIXOUTLVOL_TO_HPOUT2);
	wm8994_write(codec, WM8994_HPOUT2_MIXER, val);

	wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_RCV);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
			WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
			WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~(WM8994_DSP_FS1CLK_ENA_MASK | WM8994_DSP_FSINTCLK_ENA_MASK);
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FSINTCLK_ENA);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
	      WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK);
	val |= (WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA |
			WM8994_MIXOUTRVOL_ENA | WM8994_MIXOUTLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
			WM8994_HPOUT2_ENA_MASK | WM8994_HPOUT1L_ENA_MASK |
			WM8994_HPOUT1R_ENA_MASK | WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_HPOUT2_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE | WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

}

void wm8994_set_playback_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	u16 testreturn1 = 0;
	u16 testreturn2 = 0;
	u16 testlow1 = 0;
	u16 testhigh1 = 0;
	u8 testlow = 0;
	u8 testhigh = 0;

	DEBUG_LOG("");

	/* Enable the Timeslot0 to DAC1L */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* Enable the Timeslot0 to DAC1R */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= WM8994_AIF1DAC1R_TO_DAC1R;
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	/* Headset */
	wm8994_write(codec, 0x102, 0x0003);
	wm8994_write(codec, 0x56, 0x0003);
	wm8994_write(codec, 0x102, 0x0000);

	/* Charge pump control : Class W
	 * Using dynamic charge pump control mode whitch allows the power
	 * consumption to be optimised in real time, but can only be used
	 * if a single AIF source is the only signal source
	 */
	val = wm8994_read(codec, WM8994_CLASS_W_1);
	val &= ~(0x0005);
	val |= 0x0005;
	wm8994_write(codec, WM8994_CLASS_W_1, val);

	val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1L_MUTE_N_MASK);
	val |= (WM8994_HPOUT1L_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1R_MUTE_N_MASK);
	val |= (WM8994_HPOUT1R_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	if (wm8994->ringtone_active)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_RING_HP);
	else if (wm8994->cur_path == HP_NO_MIC)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_HP_NO_MIC);
	else
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_HP);

	val = wm8994_read(codec, WM8994_DC_SERVO_2);
	val &= ~(0x03E0);
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	/* Enable vmid,bias, hp left and right */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
		WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK |
		WM8994_SPKOUTR_ENA_MASK | WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL |
		WM8994_HPOUT1R_ENA | WM8994_HPOUT1L_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(0x0022);
	val = 0x0022;
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Enable Charge Pump, this is from wolfson */
	val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
	val &= ~WM8994_CP_ENA_MASK;
	val |= WM8994_CP_ENA | WM8994_CP_ENA_DEFAULT;
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, val);

	msleep(5);

	/* Enable Dac1 and DAC2 and the Timeslot0 for AIF1 */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
		WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* Enable DAC1L to HPOUT1L path */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &=  ~(WM8994_DAC1L_TO_HPOUT1L_MASK | WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |= WM8994_DAC1L_TO_MIXOUTL;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	/* Enable DAC1R to HPOUT1R path */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK | WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= WM8994_DAC1R_TO_MIXOUTR;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK |
		WM8994_SPKRVOL_ENA_MASK | WM8994_SPKLVOL_ENA_MASK);
	val |= (WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA |
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0030);

	if (!wm8994->dc_servo[DCS_MEDIA]) {
		wait_for_dc_servo(codec,
				  WM8994_DCS_TRIG_SERIES_0 |
				  WM8994_DCS_TRIG_SERIES_1);

		testreturn1 = wm8994_read(codec, WM8994_DC_SERVO_4);

		testlow = (signed char)(testreturn1 & 0xff);
		testhigh = (signed char)((testreturn1>>8) & 0xff);

		testlow1 = ((signed short)(testlow-5)) & 0x00ff;
		testhigh1 = (((signed short)(testhigh-5)<<8) & 0xff00);
		testreturn2 = testlow1|testhigh1;
	} else {
		testreturn2 = wm8994->dc_servo[DCS_MEDIA];
	}

	wm8994_write(codec, WM8994_DC_SERVO_4, testreturn2);
	wm8994->dc_servo[DCS_MEDIA] = testreturn2;

	wait_for_dc_servo(codec,
			  WM8994_DCS_TRIG_DAC_WR_0 | WM8994_DCS_TRIG_DAC_WR_1);

	/* Intermediate HP settings */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK | WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val = (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP|
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* Unmute the AF1DAC1 */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= WM8994_AIF1DAC1_UNMUTE;
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

}

void wm8994_set_playback_speaker(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("");

	/* Disable end point for preventing pop up noise.*/
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_SPKOUTL_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK |
		WM8994_SPKRVOL_ENA_MASK | WM8994_SPKLVOL_ENA_MASK);
	val |= WM8994_SPKLVOL_ENA;
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* Speaker Volume Control */
	/* Unmute the SPKMIXVOLUME */
	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val &= ~(WM8994_SPKOUTL_MUTE_N_MASK);
	val |= (WM8994_SPKOUTL_MUTE_N);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
	val &= ~(WM8994_SPKOUTR_MUTE_N_MASK);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK |
		 WM8994_SPKMIXR_TO_SPKOUTL_MASK |
		 WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= WM8994_SPKMIXL_TO_SPKOUTL;
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	/* Unmute the DAC path */
	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
	val |= WM8994_DAC1L_TO_SPKMIXL;
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	/* Eable DAC1 Left and timeslot left */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1L_ENA_MASK | WM8994_AIF1DAC1R_ENA_MASK |
		WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA | WM8994_DAC1L_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	if (wm8994->ringtone_active)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_RING_SPK);
	else
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_SPK);

	/* enable timeslot0 to left dac */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* Enbale bias,vmid and Left speaker */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
		WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK |
		WM8994_SPKOUTR_ENA_MASK | WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_SPKOUTL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	/* Unmute */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE | WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

}

void wm8994_set_playback_speaker_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	u16 nreadservo4val = 0;
	u16 ncompensationresult = 0;
	u16 ncompensationresultlow = 0;
	u16 ncompensationresulthigh = 0;
	u8  nservo4low = 0;
	u8  nservo4high = 0;

	/* Enable the Timeslot0 to DAC1L */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= WM8994_AIF1DAC1L_TO_DAC1L;
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* Enable the Timeslot0 to DAC1R */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= WM8994_AIF1DAC1R_TO_DAC1R;
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	/* Speaker Volume Control */
	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val &= ~(WM8994_SPKOUTL_MUTE_N_MASK);
	val |= (WM8994_SPKOUTL_MUTE_N);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
	val &= ~(WM8994_SPKOUTR_MUTE_N_MASK);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= WM8994_SPKMIXL_TO_SPKOUTL;
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	/* Unmute the DAC path */
	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
	val |= WM8994_DAC1L_TO_SPKMIXL;
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	/* Headset */
	wm8994_write(codec, 0x102, 0x0003);
	wm8994_write(codec, 0x56, 0x0003);
	wm8994_write(codec, 0x102, 0x0000);

	/* Charge pump control : Class W
	 * Using dynamic charge pump control mode whitch allows the power
	 * consumption to be optimised in real time, but can only be used
	 * if a single AIF source is the only signal source
	 */
	val = wm8994_read(codec, WM8994_CLASS_W_1);
	val &= ~(0x0005);
	val |= 0x0005;
	wm8994_write(codec, WM8994_CLASS_W_1, val);

	val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1L_MUTE_N_MASK);
	val |= (WM8994_HPOUT1L_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1R_MUTE_N_MASK);
	val |= (WM8994_HPOUT1R_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

	/* DC Servo Series Count */
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
		WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK |
		WM8994_SPKOUTL_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL |
		WM8994_HPOUT1R_ENA | WM8994_HPOUT1L_ENA |
		WM8994_SPKOUTL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	val = (WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Enable Charge Pump, this is from wolfson */
	val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
	val &= ~WM8994_CP_ENA_MASK ;
	val |= WM8994_CP_ENA | WM8994_CP_ENA_DEFAULT;
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, val);

	msleep(5);

	/* Enable DAC1 and DAC2 and the Timeslot0 for AIF1 */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
		WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* Enbale DAC1L to HPOUT1L path */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &=  ~(WM8994_DAC1L_TO_HPOUT1L_MASK | WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |=  WM8994_DAC1L_TO_MIXOUTL;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	/* Enbale DAC1R to HPOUT1R path */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK | WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= WM8994_DAC1R_TO_MIXOUTR;
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* Enbale bias,vmid, hp left and right and Left speaker */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK |
		WM8994_SPKLVOL_ENA_MASK);
	val |= (WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA | WM8994_SPKLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* DC Servo */
	if (!wm8994->dc_servo[DCS_SPK_HP]) {
		wait_for_dc_servo(codec,
				  WM8994_DCS_TRIG_SERIES_0 |
				  WM8994_DCS_TRIG_SERIES_1);

		nreadservo4val = wm8994_read(codec, WM8994_DC_SERVO_4);
		nservo4low = (signed char)(nreadservo4val & 0xff);
		nservo4high = (signed char)((nreadservo4val>>8) & 0xff);

		ncompensationresultlow = ((signed short)nservo4low - 5)
			& 0x00ff;
		ncompensationresulthigh = ((signed short)(nservo4high - 5)<<8)
			& 0xff00;
		ncompensationresult = ncompensationresultlow |
			ncompensationresulthigh;
	} else {
		ncompensationresult = wm8994->dc_servo[DCS_SPK_HP];
	}

	wm8994_write(codec, WM8994_DC_SERVO_4, ncompensationresult);
	wm8994->dc_servo[DCS_SPK_HP] = ncompensationresult;

	wait_for_dc_servo(codec,
			  WM8994_DCS_TRIG_DAC_WR_1 | WM8994_DCS_TRIG_DAC_WR_0);

	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK |	WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP |
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	if (wm8994->ringtone_active)
		wm8994_set_codec_gain(codec, PLAYBACK_MODE,
				PLAYBACK_RING_SPK_HP);
	else
		wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_SPK_HP);

	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE | WM8994_AIF1DAC1_MONO);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

}

void wm8994_set_playback_bluetooth(struct snd_soc_codec *codec)
{
	u16 val;

	DEBUG_LOG("BT Playback Path for SCO\n");

	wm8994_set_bluetooth_common_setting(codec);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 0x0000);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0000);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val &= ~(WM8994_AIF2ADCL_ENA_MASK | WM8994_AIF2ADCR_ENA_MASK);
	val |= (WM8994_AIF2ADCL_ENA | WM8994_AIF2ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK |
		WM8994_AIF1DAC1L_ENA_MASK | WM8994_AIF1DAC1R_ENA_MASK |
		WM8994_DAC1L_ENA_MASK | WM8994_DAC1R_ENA_MASK);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
		WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_6);
	val &= ~(WM8994_AIF3_ADCDAT_SRC_MASK);
	val |= (0x0001 << WM8994_AIF3_ADCDAT_SRC_SHIFT);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, val);

	/* Mixer Routing*/
	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC2L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC2R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC2R);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	/* Volume*/
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	wm8994_write(codec, WM8994_OVERSAMPLING, 0X0000);

	/* GPIO Configuration*/
	wm8994_write(codec, WM8994_GPIO_8, WM8994_GP8_DIR | WM8994_GP8_DB);
	wm8994_write(codec, WM8994_GPIO_9, WM8994_GP9_DB);
	wm8994_write(codec, WM8994_GPIO_10, WM8994_GP10_DB);
	wm8994_write(codec, WM8994_GPIO_11, WM8994_GP11_DB);

	/* Un-Mute*/
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

}

void wm8994_set_playback_extra_dock_speaker(struct snd_soc_codec *codec)
{
	u16 val;

	DEBUG_LOG("");

	/* Disable end point for preventing pop up noise */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_LINEOUT2N_ENA_MASK | WM8994_LINEOUT2P_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* Unmute */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	wm8994_set_codec_gain(codec, PLAYBACK_MODE, PLAYBACK_EXTRA_DOCK);

	/* For X-talk of VPS's L/R line. It's requested by H/W team. */
	wm8994_write(codec, WM8994_ADDITIONAL_CONTROL, 0x00);
	wm8994_write(codec, WM8994_ANTIPOP_1, 0x80);

	/* Enable DAC1 to mixout */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val &= ~(WM8994_DAC1L_TO_MIXOUTL_MASK);
	val |= (WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val &= ~(WM8994_DAC1R_TO_MIXOUTR_MASK);
	val |= (WM8994_DAC1R_TO_MIXOUTR);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* Enable mixout to lineout */
	val = wm8994_read(codec, WM8994_LINE_MIXER_2);
	val &= ~(WM8994_MIXOUTR_TO_LINEOUT2N_MASK |
		WM8994_MIXOUTL_TO_LINEOUT2N_MASK | WM8994_LINEOUT2_MODE_MASK |
		WM8994_MIXOUTR_TO_LINEOUT2P_MASK);
	val |= (WM8994_MIXOUTL_TO_LINEOUT2N | WM8994_LINEOUT2_MODE |
		WM8994_MIXOUTR_TO_LINEOUT2P);
	wm8994_write(codec, WM8994_LINE_MIXER_2, val);

	/* Enable DAC1 left/right and Timeslot left/right */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_DAC1R_ENA_MASK | WM8994_DAC1L_ENA_MASK |
		WM8994_AIF1DAC1R_ENA_MASK | WM8994_AIF1DAC1L_ENA_MASK);
	val |= (WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA | WM8994_DAC1L_ENA |
		WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* Unmute timeslot 0 */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~(WM8994_AIF1DAC1_MUTE_MASK | WM8994_AIF1DAC1_MONO_MASK);
	val |= (WM8994_AIF1DAC1_UNMUTE);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	/* Unmute Line Outputs Volume */
	val = wm8994_read(codec, WM8994_LINE_OUTPUTS_VOLUME);
	val &= ~(WM8994_LINEOUT2N_MUTE_MASK | WM8994_LINEOUT2P_MUTE_MASK);
	wm8994_write(codec, WM8994_LINE_OUTPUTS_VOLUME, val);

	/* Enable Timeslot0 to left DAC */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* Enable Timeslot0 to right DAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	/* Enable AIF1 processing clock and digital mixing processor clock */
	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~(WM8994_DSP_FS1CLK_ENA_MASK | WM8994_DSP_FSINTCLK_ENA_MASK);
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FSINTCLK_ENA);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	/* Enable lineout and mixout */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~(WM8994_LINEOUT2N_ENA_MASK | WM8994_LINEOUT1P_ENA_MASK |
		WM8994_MIXOUTLVOL_ENA_MASK | WM8994_MIXOUTRVOL_ENA_MASK |
		WM8994_MIXOUTL_ENA_MASK | WM8994_MIXOUTR_ENA_MASK);
	val |= (WM8994_LINEOUT2N_ENA | WM8994_LINEOUT2P_ENA |
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA |
		WM8994_MIXOUTRVOL_ENA | WM8994_MIXOUTLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* Enable bias, vmid */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_BIAS_ENA_MASK | WM8994_VMID_SEL_MASK |
		WM8994_HPOUT2_ENA_MASK);
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

}

void wm8994_set_voicecall_common_setting(struct snd_soc_codec *codec)
{
	int val;

	/* GPIO Configuration */
	wm8994_write(codec, WM8994_GPIO_1, 0xA101);
	wm8994_write(codec, WM8994_GPIO_2, 0x8100);
	wm8994_write(codec, WM8994_GPIO_3, 0x0100);
	wm8994_write(codec, WM8994_GPIO_4, 0x0100);
	wm8994_write(codec, WM8994_GPIO_5, 0x8100);
	wm8994_write(codec, WM8994_GPIO_6, 0xA101);
	wm8994_write(codec, WM8994_GPIO_7, 0x0100);
	wm8994_write(codec, WM8994_GPIO_8, 0xA101);
	wm8994_write(codec, WM8994_GPIO_9, 0xA101);
	wm8994_write(codec, WM8994_GPIO_10, 0xA101);
	wm8994_write(codec, WM8994_GPIO_11, 0xA101);

	wm8994_write(codec, WM8994_FLL2_CONTROL_2, 0x2F00);
	wm8994_write(codec, WM8994_FLL2_CONTROL_3, 0x3126);
	wm8994_write(codec, WM8994_FLL2_CONTROL_4, 0x0100);
	wm8994_write(codec, WM8994_FLL2_CONTROL_5, 0x0C88);
	wm8994_write(codec, WM8994_FLL2_CONTROL_1,
		WM8994_FLL2_FRACN_ENA | WM8994_FLL2_ENA);

	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1);
	if (!(val & WM8994_AIF2CLK_ENA))
		wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0018);

	wm8994_write(codec, WM8994_AIF2_RATE, 0x3 << WM8994_AIF2CLK_RATE_SHIFT);

	/* AIF2 Interface - PCM Stereo mode */
	/* Left Justified, BCLK invert, LRCLK Invert */
	wm8994_write(codec, WM8994_AIF2_CONTROL_1,
		WM8994_AIF2ADCR_SRC | WM8994_AIF2_BCLK_INV | 0x18);

	wm8994_write(codec, WM8994_AIF2_BCLK, 0x70);
	wm8994_write(codec, WM8994_AIF2_CONTROL_2, 0x0000);
	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, WM8994_AIF2_MSTR |
		WM8994_AIF2_CLK_FRC | WM8994_AIF2_LRCLK_FRC);

	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val &= ~(WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK |
		WM8994_AIF1DAC1L_ENA_MASK | WM8994_AIF1DAC1R_ENA_MASK |
		WM8994_DAC1L_ENA_MASK | WM8994_DAC1R_ENA_MASK);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
		WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);

	/* Clocking */
	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val |= (WM8994_DSP_FS2CLK_ENA);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, 0x0);

	/* AIF1 & AIF2 Output is connected to DAC1 */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1L_TO_DAC1L_MASK |
		WM8994_AIF2DACL_TO_DAC1L_MASK);
	val |= (WM8994_AIF1DAC1L_TO_DAC1L | WM8994_AIF2DACL_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM8994_AIF1DAC1R_TO_DAC1R_MASK |
		WM8994_AIF2DACR_TO_DAC1R_MASK);
	val |= (WM8994_AIF1DAC1R_TO_DAC1R | WM8994_AIF2DACR_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	wm8994_write(codec, 0x6, 0x0);
}

void wm8994_set_voicecall_receiver(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	int val;

	DEBUG_LOG("");

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	wm8994_set_voicecall_common_setting(codec);

	/* Disable charge pump */
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, WM8994_CP_ENA_DEFAULT);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2,
		WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS | WM8994_MIXINL_ENA |
		WM8994_IN1L_ENA);

	wm8994_write(codec, WM8994_INPUT_MIXER_2,
			WM8994_IN1LP_TO_IN1L | WM8994_IN1LN_TO_IN1L);

	/* Digital Path Enables and Unmutes */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4,
			WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA);

	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x000C);
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);

	/* Tx -> AIF2 Path */
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC1_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING,
			WM8994_ADC1_TO_DAC2R);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	/* Unmute IN1L PGA, update volume */
	val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM8994_IN1L_MUTE_MASK);
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

	/* Unmute the PGA */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val &= ~(WM8994_IN1L_TO_MIXINL_MASK);
	val |= (WM8994_IN1L_TO_MIXINL);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

	/* Volume Control - Output */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_HPOUT2_VOLUME);
	val &= ~(WM8994_HPOUT2_MUTE_MASK);
	wm8994_write(codec, WM8994_HPOUT2_VOLUME, val);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_RCV);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* Output Mixing */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, WM8994_DAC1R_TO_MIXOUTR);

	/* Analogue Output Configuration */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3,
		WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA |
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_HPOUT2_MIXER, WM8994_MIXOUTLVOL_TO_HPOUT2 |
			WM8994_MIXOUTRVOL_TO_HPOUT2);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1,
		WM8994_HPOUT2_ENA | WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA);

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

}

void wm8994_set_voicecall_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	int val;

	u16 testreturn1 = 0;
	u16 testreturn2 = 0;
	u16 testlow1 = 0;
	u16 testhigh1 = 0;
	u8 testlow = 0;
	u8 testhigh = 0;

	DEBUG_LOG("");

	wm8994_set_voicecall_common_setting(codec);

	/* Digital Path Enables and Unmutes */
	if (wm8994->hw_version == 3) {	/* H/W Rev D */
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC2_TO_DAC2L);
		wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x0180);
		wm8994_write(codec, WM8994_SIDETONE, 0x01C0);
	} else {	/* H/W Rev B */
		wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x000C);
		wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC1_TO_DAC2L);
		wm8994_write(codec, WM8994_SIDETONE, 0x01C1);
	}

	/* Analogue Input Configuration */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val &= ~(WM8994_TSHUT_ENA_MASK | WM8994_TSHUT_OPDIS_MASK |
		WM8994_MIXINR_ENA_MASK | WM8994_IN1R_ENA_MASK);
	val |= (WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS |
		WM8994_MIXINR_ENA | WM8994_IN1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, 0x6110);

	val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM8994_IN1R_MUTE_MASK);
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME, val);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
	val &= ~(WM8994_IN1R_TO_MIXINR_MASK);
	val |= (WM8994_IN1R_TO_MIXINR);
	wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
	val &= ~(WM8994_IN1RP_TO_IN1R_MASK |  WM8994_IN1RN_TO_IN1R_MASK);
	val |= (WM8994_IN1RP_TO_IN1R | WM8994_IN1RN_TO_IN1R);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, 0x0003);

	/* Unmute*/
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, 0x2001);

	/* Headset */
	wm8994_write(codec, 0x102, 0x0003);
	wm8994_write(codec, 0x56, 0x0003);
	wm8994_write(codec, 0x102, 0x0000);

	/* Charge pump control : Class W
	 * Using dynamic charge pump control mode whitch allows the power
	 * consumption to be optimised in real time, but can only be used
	 * if a single AIF source is the only signal source
	 */
	val = wm8994_read(codec, WM8994_CLASS_W_1);
	val &= ~(0x0005);
	val |= 0x0005;
	wm8994_write(codec, WM8994_CLASS_W_1, val);

	val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1L_MUTE_N_MASK);
	val |= (WM8994_HPOUT1L_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1R_MUTE_N_MASK);
	val |= (WM8994_HPOUT1R_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DC_SERVO_2);
	val &= ~(0x03E0);
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0303);

	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x0022);

	/* Enable Charge Pump, This is from wolfson */
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x9F25);

	msleep(5);

	/* Analogue Output Configuration */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, 0x0001);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, 0x0001);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0030);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	if (!wm8994->dc_servo[DCS_VOICE]) {
		wait_for_dc_servo(codec,
				  WM8994_DCS_TRIG_SERIES_0 |
				  WM8994_DCS_TRIG_SERIES_1);

		testreturn1 = wm8994_read(codec, WM8994_DC_SERVO_4);

		testlow = (signed char)(testreturn1 & 0xff);
		testhigh = (signed char)((testreturn1>>8) & 0xff);

		testlow1 = ((signed short)testlow - 5) & 0x00ff;
		testhigh1 = (((signed short)(testhigh - 5)<<8) & 0xff00);
		testreturn2 = testlow1|testhigh1;
	} else {
		testreturn2 = wm8994->dc_servo[DCS_VOICE];
	}

	wm8994_write(codec, WM8994_DC_SERVO_4, testreturn2);

	wait_for_dc_servo(codec,
			  WM8994_DCS_TRIG_DAC_WR_0 | WM8994_DCS_TRIG_DAC_WR_1);

	wm8994->dc_servo[DCS_VOICE] = testreturn2;

	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x00EE);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_HP);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

}

void wm8994_set_voicecall_headphone(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	int val;

	u16 testreturn1 = 0;
	u16 testreturn2 = 0;
	u16 testlow1 = 0;
	u16 testhigh1 = 0;
	u8 testlow = 0;
	u8 testhigh = 0;

	DEBUG_LOG("");
	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	wm8994_set_voicecall_common_setting(codec);

	/* Digital Path Enables and Unmutes */
	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x000C);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC1_TO_DAC2L);

	/* Analogue Input Configuration */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2,
		WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS | WM8994_MIXINL_ENA |
		WM8994_IN1L_ENA);

	/* Unmute IN1L PGA, update volume */
	val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM8994_IN1L_MUTE_MASK);
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

	/* Unmute the PGA */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val &= ~(WM8994_IN1L_TO_MIXINL_MASK);
	val |= (WM8994_IN1L_TO_MIXINL);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

	wm8994_write(codec, WM8994_INPUT_MIXER_2,
			WM8994_IN1LP_TO_IN1L | WM8994_IN1LN_TO_IN1L);

	/* Unmute*/
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTL_MUTE_N_MASK);
	val |= (WM8994_MIXOUTL_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val &= ~(WM8994_MIXOUTR_MUTE_N_MASK);
	val |= (WM8994_MIXOUTR_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* Digital Path Enables and Unmutes */
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4,
			WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA);

	/* Headset */
	wm8994_write(codec, 0x102, 0x0003);
	wm8994_write(codec, 0x56, 0x0003);
	wm8994_write(codec, 0x102, 0x0000);

	/* Charge pump control : Class W
	 * Using dynamic charge pump control mode whitch allows the power
	 * consumption to be optimised in real time, but can only be used
	 * if a single AIF source is the only signal source
	 */
	val = wm8994_read(codec, WM8994_CLASS_W_1);
	val &= ~(0x0005);
	val |= 0x0005;
	wm8994_write(codec, WM8994_CLASS_W_1, val);

	val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1L_MUTE_N_MASK);
	val |= (WM8994_HPOUT1L_MUTE_N);
	wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
	val &= ~(WM8994_HPOUT1R_MUTE_N_MASK);
	val |= (WM8994_HPOUT1R_MUTE_N);
	wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

	val = wm8994_read(codec, WM8994_DC_SERVO_2);
	val &= ~(0x03E0);
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, 0x0303);

	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x0022);

	/* Enable Charge Pump, this is from wolfson */
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x9F25);

	msleep(5);

	/* Analogue Output Configuration */
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, 0x0001);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, 0x0001);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, 0x0030);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	if (!wm8994->dc_servo[DCS_VOICE]) {
		wait_for_dc_servo(codec,
				  WM8994_DCS_TRIG_SERIES_0 |
				  WM8994_DCS_TRIG_SERIES_1);

		testreturn1 = wm8994_read(codec, WM8994_DC_SERVO_4);

		testlow = (signed char)(testreturn1 & 0xff);
		testhigh = (signed char)((testreturn1>>8) & 0xff);

		testlow1 = ((signed short)testlow - 5) & 0x00ff;
		testhigh1 = (((signed short)(testhigh - 5)<<8) & 0xff00);
		testreturn2 = testlow1|testhigh1;
	} else
		testreturn2 = wm8994->dc_servo[DCS_VOICE];

	wm8994_write(codec, WM8994_DC_SERVO_4, testreturn2);

	wait_for_dc_servo(codec,
			  WM8994_DCS_TRIG_DAC_WR_0 | WM8994_DCS_TRIG_DAC_WR_1);

	wm8994->dc_servo[DCS_VOICE] = testreturn2;

	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x00EE);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_HP_NO_MIC);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);

}

void wm8994_set_voicecall_speaker(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	int val;

	DEBUG_LOG("");

	audio_ctrl_mic_bias_gpio(wm8994->pdata, 1);

	wm8994_set_voicecall_common_setting(codec);

	wm8994_write(codec, 0x603, 0x000C);
	/* Tx -> AIF2 Path */
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
			WM8994_ADC1_TO_DAC2L);

	/* Analogue Input Configuration*/
	wm8994_write(codec, 0x02, 0x6240);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, WM8994_IN1LP_TO_IN1L |
			WM8994_IN1LN_TO_IN1L);

	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val &= ~(WM8994_IN1L_TO_MIXINL_MASK);
	val |= (WM8994_IN1L_TO_MIXINL);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

	val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM8994_IN1L_MUTE_MASK);
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME, val);

	/* Analogue Output Configuration*/
	wm8994_write(codec, 0x03, 0x0300);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4,
			WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val &= ~(WM8994_SPKOUTL_MUTE_N_MASK);
	val |= (WM8994_SPKOUTL_MUTE_N);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
	val &= ~(WM8994_SPKOUTR_MUTE_N_MASK | WM8994_SPKOUTR_VOL_MASK);
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= WM8994_SPKMIXL_TO_SPKOUTL;
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	wm8994_write(codec, 0x36, 0x0003);
	/* Digital Path Enables and Unmutes*/

	wm8994_write(codec, WM8994_SIDETONE, 0x01C0);


	wm8994_write(codec, WM8994_ANALOGUE_HP_1, 0x0000);
	wm8994_write(codec, WM8994_DC_SERVO_1, 0x0000);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1,
		WM8994_SPKOUTL_ENA | WM8994_VMID_SEL_NORMAL | WM8994_BIAS_ENA);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_SPK);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, WM8994_AIF1DAC1_UNMUTE);
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, WM8994_AIF1DAC2_UNMUTE);
}

void wm8994_set_voicecall_bluetooth(struct snd_soc_codec *codec)
{
	int val;

	DEBUG_LOG("");

	wm8994_set_voicecall_common_setting(codec);

	/* GPIO Configuration */
	wm8994_write(codec, WM8994_GPIO_8, WM8994_GP8_DIR | WM8994_GP8_DB);
	wm8994_write(codec, WM8994_GPIO_9, WM8994_GP9_DB);
	wm8994_write(codec, WM8994_GPIO_10, WM8994_GP10_DB);
	wm8994_write(codec, WM8994_GPIO_11, WM8994_GP11_DB);

	/* Digital Path Enables and Unmutes */
	val  = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~(WM8994_SPKOUTL_ENA_MASK | WM8994_HPOUT2_ENA_MASK |
		WM8994_HPOUT1L_ENA_MASK | WM8994_HPOUT1R_ENA_MASK);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4,
		WM8994_AIF2ADCL_ENA | WM8994_ADCL_ENA);

	/* If Input MIC is enabled, bluetooth Rx is muted. */
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_1_2_VOLUME,
			WM8994_IN1L_MUTE);
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_1_2_VOLUME,
			WM8994_IN1R_MUTE);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, 0x00);
	wm8994_write(codec, WM8994_INPUT_MIXER_3, 0x00);
	wm8994_write(codec, WM8994_INPUT_MIXER_4, 0x00);

	/*
	 * for BT DTMF Play
	 * Rx Path: AIF2ADCDAT2 select
	 * CP(CALL) Path:GPIO5/DACDAT2 select
	 * AP(DTMF) Path: DACDAT1 select
	 * Tx Path: GPIO8/DACDAT3 select
	 */

	wm8994_write(codec, WM8994_POWER_MANAGEMENT_6, 0x000C);

	/* AIF1 & AIF2 Output is connected to DAC1 */
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING,
		WM8994_AIF2DACL_TO_DAC2L | WM8994_AIF1DAC1L_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING,
		WM8994_AIF2DACR_TO_DAC2R | WM8994_AIF1DAC1R_TO_DAC2R);

	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, 0x0019);

	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x000C);

	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, 0x01C0);
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, 0x01C0);

	wm8994_write(codec, WM8994_OVERSAMPLING, 0X0000);

	wm8994_set_codec_gain(codec, VOICECALL_MODE, VOICECALL_BT);

	/* Unmute DAC1 left */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~(WM8994_DAC1L_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute and volume ctrl RightDAC */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~(WM8994_DAC1R_MUTE_MASK);
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0000);
}

void wm8994_disable_fmradio_path(struct snd_soc_codec *codec,
	enum fmradio_path path)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("Turn off fmradio_path = [%d]", path);

	switch (path) {
	case FMR_SPK:
		/* Disable end point for preventing pop up noise */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_SPKOUTL_ENA_MASK | WM8994_HPOUT1L_ENA_MASK |
			WM8994_HPOUT1R_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1 , val);

		if (!wm8994->testmode_config_flag) {
			/* set 0dB to SPKMIXL_VOL(0dB, -6dB, -12dB, Mute) */
			val = wm8994_read(codec, WM8994_SPKMIXL_ATTENUATION);
			val &= ~(WM8994_SPKMIXL_VOL_MASK);
			wm8994_write(codec, WM8994_SPKMIXL_ATTENUATION, val);

			/* set 0dB to SPKMIXR_VOL(0dB, -6dB, -12dB, Mute) */
			val = wm8994_read(codec, WM8994_SPKMIXR_ATTENUATION);
			val &= ~(WM8994_SPKMIXR_VOL_MASK);
			wm8994_write(codec, WM8994_SPKMIXR_ATTENUATION, val);

			/* SPKOUTL_VOL Control : -57dB (-57dB ~ +6dB)
			 * SPKOUTL_VOL Mute
			 */
			val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
			val &= ~(WM8994_SPKOUTL_MUTE_N_MASK |
				WM8994_SPKOUTL_VOL_MASK);
			wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

			/* SPKOUTR_VOL Control : -57dB (-57dB ~ +6dB)
			 * SPKOUTR_VOL Mute
			 */
			val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
			val &= ~(WM8994_SPKOUTR_MUTE_N_MASK |
				WM8994_SPKOUTR_VOL_MASK);
			wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

			/* SPKOUTL_BOOST Control : 0bB (0dB ~ +12dB) */
			val = wm8994_read(codec, WM8994_CLASSD);
			val &= ~(WM8994_SPKOUTL_BOOST_MASK);
			wm8994_write(codec, WM8994_CLASSD, val);
		}

		/* Disable SPKMIX to SPKOUT */
		val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
		val &= ~(WM8994_SPKMIXL_TO_SPKOUTL_MASK |
			WM8994_SPKMIXR_TO_SPKOUTL_MASK |
			WM8994_SPKMIXR_TO_SPKOUTR_MASK);
		wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

		/* Disable MIXIN to SPKMIX */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_MIXINL_TO_SPKMIXL_MASK |
			WM8994_MIXINR_TO_SPKMIXR_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);
		break;

	case FMR_HP:

		/* Disable end point for preventing pop up noise */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
		val &= ~(WM8994_SPKOUTL_ENA_MASK | WM8994_HPOUT1L_ENA_MASK |
			WM8994_HPOUT1R_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_1 , val);

		/* Cut off the power supply of MIXOUT and MIXOUTVOL */
		val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
		val &= ~(WM8994_MIXOUTLVOL_ENA_MASK |
			WM8994_MIXOUTRVOL_ENA_MASK | WM8994_MIXOUTL_ENA_MASK |
			WM8994_MIXOUTR_ENA_MASK);
		wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

		if (!wm8994->testmode_config_flag) {
			/* HPOUT1L_VOL Control : -57dB (-57dB ~ +6dB)
			 * HPOUT1L_VOL Mute
			 */
			val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1L_MUTE_N_MASK |
				WM8994_HPOUT1L_VOL_MASK);
			wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

			/* HPOUT1L_VOR Control : -57dB (-57dB ~ +6dB)
			 * HPOUT1L_VOR Mute
			 */
			val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
			val &= ~(WM8994_HPOUT1R_MUTE_N_MASK |
				WM8994_HPOUT1R_VOL_MASK);
			wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);
		}

		/* Disable Charge Pump, this is from wolfson */
		val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
		val &= ~WM8994_CP_ENA_MASK;
		val |= WM8994_CP_ENA_DEFAULT;
		wm8994_write(codec, WM8994_CHARGE_PUMP_1, val);

		/* Intermediate HP settings */
		val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
		val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
			WM8994_HPOUT1R_RMV_SHORT_MASK |
			WM8994_HPOUT1L_DLY_MASK | WM8994_HPOUT1L_OUTP_MASK |
			WM8994_HPOUT1L_RMV_SHORT_MASK);
		wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

		/* Disable Output mixer setting */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_DAC1L_TO_HPOUT1L_MASK |
			WM8994_MIXINL_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK |
			WM8994_MIXINR_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);
		break;

	case FMR_DUAL_MIX:
		/* Disable DAC1L to SPKMIXL */
		val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
		val &= ~(WM8994_DAC1L_TO_SPKMIXL_MASK);
		wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

		/* Disable DAC1L to HPOUT1L, DAC1L to MIXOUTL */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
		val &= ~(WM8994_DAC1L_TO_HPOUT1L_MASK |
			WM8994_DAC1L_TO_MIXOUTL_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

		/* Disable DAC1R to HPOUT1R, DAC1R to MIXOUTR */
		val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
		val &= ~(WM8994_DAC1R_TO_HPOUT1R_MASK |
			WM8994_DAC1R_TO_MIXOUTR_MASK);
		wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);
		break;

	default:
		DEBUG_LOG_ERR("fmradio path[%d] is not invaild!\n", path);
		return;
		break;
	}
}

void wm8994_set_fmradio_common(struct snd_soc_codec *codec, int onoff)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("onoff = [%d]", onoff);

	/* Cross Talk (H/W requested) */
	wm8994_write(codec, 0x39, 0x8);

	if (!onoff) {
		/* IN2L_VOL Control : -16.5dB (-16.5dB ~ +30dB)
		 * IN2L_VOL Mute
		 */
		val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_3_4_VOLUME);
		val &= ~(WM8994_IN2L_VU_MASK | WM8994_IN2L_MUTE_MASK |
			WM8994_IN2L_VOL_MASK);
		val |= (WM8994_IN2L_VU | WM8994_IN2L_MUTE);
		wm8994_write(codec, WM8994_LEFT_LINE_INPUT_3_4_VOLUME, val);

		/* IN2R_VOL Control : -16.5dB (-16.5dB ~ +30dB)
		 * IN2R_VOL Mute
		 */
		val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);
		val &= ~(WM8994_IN2R_VU_MASK | WM8994_IN2R_VOL_MASK);
		val |= (WM8994_IN2R_VU_MASK | WM8994_IN2R_MUTE);
		wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME, val);

		/* Disable IN2LN to IN2L, IN2RN to IN2R */
		val = wm8994_read(codec, WM8994_INPUT_MIXER_2);
		val &= ~(WM8994_IN2LN_TO_IN2L | WM8994_IN2RN_TO_IN2R);
		wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

		if (!wm8994->testmode_config_flag) {
			/* Disable IN2L to MIXINL */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
			val &= ~(WM8994_IN2L_TO_MIXINL_MASK);
			wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

			/* Disable IN2R to MIXINR */
			val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
			val &= ~(WM8994_IN2R_TO_MIXINR_MASK);
			wm8994_write(codec, WM8994_INPUT_MIXER_4, val);
		}

		return;
	}

	/* GPIO settings */
	wm8994_write(codec, WM8994_GPIO_3, 0x0100);
	wm8994_write(codec, WM8994_GPIO_4, 0x0100);
	wm8994_write(codec, WM8994_GPIO_5, 0x8100);
	wm8994_write(codec, WM8994_GPIO_6, 0xA101);
	wm8994_write(codec, WM8994_GPIO_7, 0x0100);

	/* Disable AIF1 timesolot0 EQ gain */
	wm8994_write(codec, WM8994_AIF1_DAC1_EQ_GAINS_1, 0x0000);

	/* Enable AIF1CLK */
	val = wm8994_read(codec, WM8994_AIF1_CLOCKING_1);
	val |= WM8994_AIF1CLK_ENA;
	wm8994_write(codec, WM8994_AIF1_CLOCKING_1, val);

	/* AIF2CLK : FLL1 */
	val = wm8994_read(codec, WM8994_AIF2_CLOCKING_1);
	val &= ~WM8994_AIF2CLK_SRC_MASK;
	val |= (WM8994_AIF2CLK_ENA | 0x2 << WM8994_AIF2CLK_SRC_SHIFT);
	wm8994_write(codec, WM8994_AIF2_CLOCKING_1, val);

	/* Enable AIF1, AIF2 processing clock, digital mixing processor clock
	 * SYSCLK source is AIF1CLK
	 */
	val = wm8994_read(codec, WM8994_CLOCKING_1);
	val &= ~WM8994_SYSCLK_SRC_MASK;
	val |= (WM8994_DSP_FS1CLK_ENA | WM8994_DSP_FS2CLK_ENA |
		WM8994_DSP_FSINTCLK_ENA);
	wm8994_write(codec, WM8994_CLOCKING_1, val);

	/* Disable oversampling */
	wm8994_write(codec, WM8994_OVERSAMPLING, 0x0000);

	/* AIF2 Master, Forces CLK to be enabled
	 * when all AIF2 audio channels are disabled
	 */
	val = wm8994_read(codec, WM8994_AIF2_MASTER_SLAVE);
	val |= (WM8994_AIF2_LRCLK_FRC | WM8994_AIF2_CLK_FRC | WM8994_AIF2_MSTR);
	wm8994_write(codec, WM8994_AIF2_MASTER_SLAVE, val);

	/* DAC1L(601h:0x05) settings */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_MIXER_ROUTING);
	val &= ~WM8994_AIF1DAC2L_TO_DAC1L_MASK;
	val |= (WM8994_AIF2DACL_TO_DAC1L | WM8994_AIF1DAC1L_TO_DAC1L);
	wm8994_write(codec, WM8994_DAC1_LEFT_MIXER_ROUTING, val);

	/* DAC1R(602h:0x05) settings */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~WM8994_AIF1DAC2R_TO_DAC1R_MASK;
	val |= (WM8994_AIF1DAC1R_TO_DAC1R | WM8994_AIF2DACR_TO_DAC1R);
	wm8994_write(codec, WM8994_DAC1_RIGHT_MIXER_ROUTING, val);

	/* DAC2 Mixer Vol : 0dB (-36dB ~ 0dB) */
	wm8994_write(codec, WM8994_DAC2_MIXER_VOLUMES, 0x018C);

	/* Enable AIF2 Loopback */
	val = wm8994_read(codec, WM8994_AIF2_CONTROL_2);
	val |= (WM8994_AIF2_LOOPBACK);
	wm8994_write(codec, WM8994_AIF2_CONTROL_2, val);

	/* DRC for Noise-gate (AIF2) */
	wm8994_write(codec, WM8994_AIF2_ADC_FILTERS, 0xF800);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, 0x0036);
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_2, 0x0010);
	wm8994_write(codec, WM8994_AIF2_DRC_2, 0x0840);
	wm8994_write(codec, WM8994_AIF2_DRC_3, 0x2400);
	wm8994_write(codec, WM8994_AIF2_DRC_4, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DRC_5, 0x0000);
	wm8994_write(codec, WM8994_AIF2_DRC_1, 0x019C);

	/* unmute IN2L_VOL */
	val = wm8994_read(codec, WM8994_LEFT_LINE_INPUT_3_4_VOLUME);
	val &= ~WM8994_IN2L_MUTE_MASK;
	wm8994_write(codec, WM8994_LEFT_LINE_INPUT_3_4_VOLUME, val);

	/* unmute IN2R_VOL */
	val = wm8994_read(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME);
	val &= ~WM8994_IN2R_MUTE_MASK;
	wm8994_write(codec, WM8994_RIGHT_LINE_INPUT_3_4_VOLUME, val);

	/* Enable IN2LN to IN2L, IN2RN to IN2R */
	val = (WM8994_IN2LN_TO_IN2L | WM8994_IN2RN_TO_IN2R);
	wm8994_write(codec, WM8994_INPUT_MIXER_2, val);

	/* Enable IN2L to MIXINL */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_3);
	val |= WM8994_IN2L_TO_MIXINL;
	wm8994_write(codec, WM8994_INPUT_MIXER_3, val);

	/* Enable IN2R to MIXINR */
	val = wm8994_read(codec, WM8994_INPUT_MIXER_4);
	val |= WM8994_IN2R_TO_MIXINR;
	wm8994_write(codec, WM8994_INPUT_MIXER_4, val);

	/* Enable ADC1 to DAC2L */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_MIXER_ROUTING);
	val |= (WM8994_ADC1_TO_DAC2L);
	wm8994_write(codec, WM8994_DAC2_LEFT_MIXER_ROUTING, val);

	/* Enable ADC2 to DAC2R */
	val = wm8994_read(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING);
	val |= (WM8994_ADC2_TO_DAC2R);
	wm8994_write(codec, WM8994_DAC2_RIGHT_MIXER_ROUTING, val);

	/* Enable power to MIXIN and IN2 */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val |= (WM8994_MIXINL_ENA | WM8994_MIXINR_ENA | WM8994_IN2L_ENA |
		WM8994_IN2R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2 , val);

	/* Enable power of AIF2ADC and ADC */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_4);
	val |= (WM8994_AIF2ADCL_ENA | WM8994_AIF2ADCR_ENA | WM8994_ADCL_ENA |
		WM8994_ADCR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_4, val);

	/* Enable power of AIF1DAC, AIF2DAC and DAC1 */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_5);
	val |= (WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA |
		WM8994_AIF1DAC1L_ENA | WM8994_AIF1DAC1R_ENA |
		WM8994_DAC1L_ENA | WM8994_DAC1R_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_5, val);
}

void wm8994_set_fmradio_headset(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	u16 nreadservo4val = 0;
	u16 ncompensationresult = 0;
	u16 ncompensationresultlow = 0;
	u16 ncompensationresulthigh = 0;
	u8  nservo4low = 0;
	u8  nservo4high = 0;

	DEBUG_LOG("Routing FM Radio: Headset");

	wm8994->fmradio_path = FMR_HP;

	wm8994_disable_fmradio_path(codec, FMR_SPK);

	/* Enable vmid, bias, HPOUT1 */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~WM8994_VMID_SEL_MASK;
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_HPOUT1R_ENA |
		WM8994_HPOUT1L_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	/* Enable power of MIXOUT */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val |= (WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA |
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* Headset */
	wm8994_write(codec, 0x102, 0x0003);
	wm8994_write(codec, 0x56, 0x0003);
	wm8994_write(codec, 0x102, 0x0000);

	/* Unmute HPOUT1L */
	val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
	val |= WM8994_HPOUT1L_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

	/* Unmute HPOUT1R */
	val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
	val |=  WM8994_HPOUT1R_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

	/* Unmute MIXOUTL */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val |= WM8994_MIXOUTL_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	/* Unmute MIXOUTR */
	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val |= WM8994_MIXOUTR_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	wm8994_set_fmradio_common(codec, 1);

	wm8994_set_codec_gain(codec, FMRADIO_MODE, FMRADIO_HP);

	/* Enable DAC1L to MIXOUTL */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val |= (WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	/* Enable DAC1R to MIXOUTR */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val |= (WM8994_DAC1R_TO_MIXOUTR);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* Enable HPF, cut-off freq is 370Hz */
	wm8994_write(codec, WM8994_SIDETONE, 0x01C0);

	/* DC Servo Series Count */
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	/* Enable HPOUT1 intermediate stage */
	val = (WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Enable Charge Pump */
	val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
	val |= WM8994_CP_ENA | WM8994_CP_ENA_DEFAULT;
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x9F25);

	msleep(5);

	/* DC Servo */
	if (!wm8994->dc_servo[DCS_FMRADIO]) {
		wait_for_dc_servo(codec,
				  WM8994_DCS_TRIG_SERIES_0 |
				  WM8994_DCS_TRIG_SERIES_1);

		nreadservo4val = wm8994_read(codec, WM8994_DC_SERVO_4);
		nservo4low = (signed char)(nreadservo4val & 0xff);
		nservo4high = (signed char)((nreadservo4val>>8) & 0xff);

		ncompensationresultlow = ((signed short)nservo4low - 5)
			& 0x00ff;
		ncompensationresulthigh = ((signed short)(nservo4high - 5)<<8)
			& 0xff00;
		ncompensationresult = ncompensationresultlow |
			ncompensationresulthigh;
	} else
		ncompensationresult = wm8994->dc_servo[DCS_FMRADIO];

	wm8994_write(codec, WM8994_DC_SERVO_4, ncompensationresult);
	wm8994->dc_servo[DCS_FMRADIO] = ncompensationresult;

	wait_for_dc_servo(codec,
			  WM8994_DCS_TRIG_DAC_WR_1 | WM8994_DCS_TRIG_DAC_WR_0);

	/* Enable HPOUT1 output stage and set RMV */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP |
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Soft unmute settings */
	wm8994_write(codec, WM8994_DAC_SOFTMUTE, WM8994_DAC_SOFTMUTEMODE |
		WM8994_DAC_MUTERATE);

	/* Unmute DAC1L */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute DAC1R */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* Unmute DAC2L */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_VOLUME);
	val &= ~WM8994_DAC2L_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, val);

	/* Unmute DAC2R */
	val = wm8994_read(codec, WM8994_DAC2_RIGHT_VOLUME);
	val &= ~WM8994_DAC2R_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, val);

	/* Unmute AIF1DAC1 */
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);

	/* Unmute AIF2DAC */
	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~(WM8994_AIF2DAC_MUTE_MASK);
	val |= WM8994_AIF2DAC_UNMUTE_RAMP | WM8994_AIF2DAC_MUTERATE;
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);


}

void wm8994_set_fmradio_speaker(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	DEBUG_LOG("Routing FM Radio: Speaker");

	wm8994_disable_fmradio_path(codec, FMR_HP);

	wm8994->fmradio_path = FMR_SPK;

	/* Enable power of SPKLVOL */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~WM8994_SPKRVOL_ENA_MASK;
	val |= WM8994_SPKLVOL_ENA;
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* Enable themal sensor, OPCLK */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_2);
	val |= (WM8994_TSHUT_ENA | WM8994_TSHUT_OPDIS | WM8994_OPCLK_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_2, val);

	/* Enable vmid, bias, SPKOUTL */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~WM8994_VMID_SEL_MASK;
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_SPKOUTL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	/* Unmute SPKOUTL */
	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val |= WM8994_SPKOUTL_MUTE_N;
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	/* mute SPKOUTR */
	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_RIGHT);
	val &= ~WM8994_SPKOUTR_MUTE_N_MASK;
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_RIGHT, val);

	wm8994_set_fmradio_common(codec, 1);

	wm8994_set_codec_gain(codec, FMRADIO_MODE, FMRADIO_SPK);

	/* Enable SPKMIXL to SPKOUTL */
	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXR_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= (WM8994_SPKMIXL_TO_SPKOUTL);
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	/* Enable DAC1L to SPKMIXL */
	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val &= ~(WM8994_MIXINL_TO_SPKMIXL_MASK | WM8994_MIXINR_TO_SPKMIXR_MASK |
		WM8994_DAC1R_TO_SPKMIXR_MASK);
	val |= (WM8994_DAC1L_TO_SPKMIXL);
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	/* Soft unmute settings */
	wm8994_write(codec, WM8994_DAC_SOFTMUTE, WM8994_DAC_SOFTMUTEMODE |
		WM8994_DAC_MUTERATE);

	/* Unmute DAC1L */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute DAC1R */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* Unmute DAC2L */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_VOLUME);
	val &= ~WM8994_DAC2L_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, val);

	/* Unmute DAC2R */
	val = wm8994_read(codec, WM8994_DAC2_RIGHT_VOLUME);
	val &= ~WM8994_DAC2R_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, val);

	/* Unmute AIF2DAC */
	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~(WM8994_AIF2DAC_MUTE_MASK);
	val |= WM8994_AIF2DAC_UNMUTE_RAMP | WM8994_AIF2DAC_MUTERATE;
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

	/* Unmute AIF1DAC1 */
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, 0x0000);

}

void wm8994_set_fmradio_speaker_headset_mix(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = codec->drvdata;

	u16 val;

	u16 nreadservo4val = 0;
	u16 ncompensationresult = 0;
	u16 nservo4low = 0;
	u16 nservo4high = 0;
	u8 ncompensationresultlow = 0;
	u8 ncompensationresulthigh = 0;

	DEBUG_LOG("");

	if (wm8994->fmradio_path == FMR_HP)
		wm8994_disable_fmradio_path(codec, FMR_HP);
	else
		wm8994_disable_fmradio_path(codec, FMR_SPK);

	wm8994->fmradio_path = FMR_DUAL_MIX;

	/* Enable power of MIXOUT, MIXOUTVOL and SPKLVOL */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_3);
	val &= ~WM8994_SPKRVOL_ENA_MASK;
	val |= (WM8994_MIXOUTLVOL_ENA | WM8994_MIXOUTRVOL_ENA |
		WM8994_MIXOUTL_ENA | WM8994_MIXOUTR_ENA | WM8994_SPKLVOL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_3, val);

	/* Enable vmid, bias, HPOUT1 and SPKOUTL */
	val = wm8994_read(codec, WM8994_POWER_MANAGEMENT_1);
	val &= ~WM8994_VMID_SEL_MASK;
	val |= (WM8994_BIAS_ENA | WM8994_VMID_SEL_NORMAL | WM8994_HPOUT1R_ENA |
		WM8994_HPOUT1L_ENA | WM8994_SPKOUTL_ENA);
	wm8994_write(codec, WM8994_POWER_MANAGEMENT_1, val);

	wm8994_set_fmradio_common(codec, 1);

	wm8994_set_codec_gain(codec, FMRADIO_MODE, FMRADIO_SPK_HP);

	/* DRC for Noise-gate (AIF2) */
	wm8994_write(codec, 0x541, 0x0850);
	wm8994_write(codec, 0x542, 0x0800);
	wm8994_write(codec, 0x543, 0x0001);
	wm8994_write(codec, 0x544, 0x0008);
	wm8994_write(codec, 0x540, 0x01BC);

	/* Enable DAC1L to MIXOUTL */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_1);
	val |= (WM8994_DAC1L_TO_MIXOUTL);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_1, val);

	/* Enable DAC1R to MIXOUTR */
	val = wm8994_read(codec, WM8994_OUTPUT_MIXER_2);
	val |= (WM8994_DAC1R_TO_MIXOUTR);
	wm8994_write(codec, WM8994_OUTPUT_MIXER_2, val);

	/* Enable SPKMIXL to SPKOUTL */
	val = wm8994_read(codec, WM8994_SPKOUT_MIXERS);
	val &= ~(WM8994_SPKMIXR_TO_SPKOUTL_MASK |
		WM8994_SPKMIXR_TO_SPKOUTR_MASK);
	val |= (WM8994_SPKMIXL_TO_SPKOUTL);
	wm8994_write(codec, WM8994_SPKOUT_MIXERS, val);

	/* Enable DAC1L to SPKMIXL */
	val = wm8994_read(codec, WM8994_SPEAKER_MIXER);
	val |= WM8994_DAC1L_TO_SPKMIXL;
	wm8994_write(codec, WM8994_SPEAKER_MIXER, val);

	/* Unmute MIXOUTL */
	val = wm8994_read(codec, WM8994_LEFT_OPGA_VOLUME);
	val |= WM8994_MIXOUTL_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OPGA_VOLUME, val);

	/* Unmute MIXOUTR */
	val = wm8994_read(codec, WM8994_RIGHT_OPGA_VOLUME);
	val |= WM8994_MIXOUTR_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OPGA_VOLUME, val);

	/* Unmute HPOUT1L */
	val = wm8994_read(codec, WM8994_LEFT_OUTPUT_VOLUME);
	val |= WM8994_HPOUT1L_MUTE_N;
	wm8994_write(codec, WM8994_LEFT_OUTPUT_VOLUME, val);

	/* Unmute HPOUT1R */
	val = wm8994_read(codec, WM8994_RIGHT_OUTPUT_VOLUME);
	val |= WM8994_HPOUT1R_MUTE_N;
	wm8994_write(codec, WM8994_RIGHT_OUTPUT_VOLUME, val);

	/* Unmute SPKOUTL */
	val = wm8994_read(codec, WM8994_SPEAKER_VOLUME_LEFT);
	val |= WM8994_SPKOUTL_MUTE_N;
	wm8994_write(codec, WM8994_SPEAKER_VOLUME_LEFT, val);

	/* Headset */
	wm8994_write(codec, 0x102, 0x0003);
	wm8994_write(codec, 0x56, 0x0003);
	wm8994_write(codec, 0x102, 0x0000);
	wm8994_write(codec, 0x5D, 0x0002);

	/* DC Servo Series Count */
	val = 0x03E0;
	wm8994_write(codec, WM8994_DC_SERVO_2, val);

	/* Enable HPOUT1 intermediate stage */
	val = (WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);


	/* Enable Charge Pump */
	val = wm8994_read(codec, WM8994_CHARGE_PUMP_1);
	val |= WM8994_CP_ENA | WM8994_CP_ENA_DEFAULT;
	wm8994_write(codec, WM8994_CHARGE_PUMP_1, 0x9F25);

	msleep(5);

	/* DC Servo */
	if (!wm8994->dc_servo[DCS_FMRADIO_SPK_HP]) {
		wait_for_dc_servo(codec,
				  WM8994_DCS_TRIG_SERIES_0 |
				  WM8994_DCS_TRIG_SERIES_1);

		nreadservo4val = wm8994_read(codec, WM8994_DC_SERVO_4);
		nservo4low = (signed char)(nreadservo4val & 0xff);
		nservo4high = (signed char)((nreadservo4val>>8) & 0xff);

		ncompensationresultlow = ((signed short)nservo4low - 5)
			& 0x00ff;
		ncompensationresulthigh = ((signed short)(nservo4high - 5)<<8)
			& 0xff00;
		ncompensationresult = ncompensationresultlow |
			ncompensationresulthigh;
	} else
		ncompensationresult = wm8994->dc_servo[DCS_FMRADIO_SPK_HP];

	wm8994_write(codec, WM8994_DC_SERVO_4, ncompensationresult);
	wm8994->dc_servo[DCS_FMRADIO_SPK_HP] = ncompensationresult;

	wait_for_dc_servo(codec,
			  WM8994_DCS_TRIG_DAC_WR_1 | WM8994_DCS_TRIG_DAC_WR_0);

	/* Enable HPOUT1 output stage and set RMV */
	val = wm8994_read(codec, WM8994_ANALOGUE_HP_1);
	val &= ~(WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1R_OUTP_MASK |
		WM8994_HPOUT1R_RMV_SHORT_MASK |	WM8994_HPOUT1L_DLY_MASK |
		WM8994_HPOUT1L_OUTP_MASK | WM8994_HPOUT1L_RMV_SHORT_MASK);
	val |= (WM8994_HPOUT1L_RMV_SHORT | WM8994_HPOUT1L_OUTP |
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_RMV_SHORT |
		WM8994_HPOUT1R_OUTP | WM8994_HPOUT1R_DLY);
	wm8994_write(codec, WM8994_ANALOGUE_HP_1, val);

	/* Soft unmute settings */
	wm8994_write(codec, WM8994_DAC_SOFTMUTE, WM8994_DAC_SOFTMUTEMODE |
		WM8994_DAC_MUTERATE);

	/* Unmute DAC1L */
	val = wm8994_read(codec, WM8994_DAC1_LEFT_VOLUME);
	val &= ~WM8994_DAC1L_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC1_LEFT_VOLUME, val);

	/* Unmute DAC1R */
	val = wm8994_read(codec, WM8994_DAC1_RIGHT_VOLUME);
	val &= ~WM8994_DAC1R_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC1_RIGHT_VOLUME, val);

	/* Unmute DAC2L */
	val = wm8994_read(codec, WM8994_DAC2_LEFT_VOLUME);
	val &= ~WM8994_DAC2L_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC2_LEFT_VOLUME, val);

	/* Unmute DAC2R */
	val = wm8994_read(codec, WM8994_DAC2_RIGHT_VOLUME);
	val &= ~WM8994_DAC2R_MUTE_MASK;
	wm8994_write(codec, WM8994_DAC2_RIGHT_VOLUME, val);

	/* Unmute AIF1DAC1 , mono mix */
	val = wm8994_read(codec, WM8994_AIF1_DAC1_FILTERS_1);
	val &= ~WM8994_AIF1DAC1_MUTE_MASK;
	val |= WM8994_AIF1DAC1_MONO;
	wm8994_write(codec, WM8994_AIF1_DAC1_FILTERS_1, val);

	/* Unmute AIF2DAC */
	val = wm8994_read(codec, WM8994_AIF2_DAC_FILTERS_1);
	val &= ~WM8994_AIF2DAC_MUTE_MASK;
	wm8994_write(codec, WM8994_AIF2_DAC_FILTERS_1, val);

}
int wm8994_set_codec_gain(struct snd_soc_codec *codec, u16 mode, u16 device)
{
	struct gain_info_t *default_gain_table_p = NULL;
	u32 gain_set_bits = COMMON_SET_BIT;
	u16 val;
	int table_num = 0;
	int i;

	if (mode == PLAYBACK_MODE) {
		default_gain_table_p = playback_gain_table;
		table_num = PLAYBACK_GAIN_NUM;

		switch (device) {
		case PLAYBACK_RCV:
			gain_set_bits |= PLAYBACK_RCV;
			break;
		case PLAYBACK_SPK:
			gain_set_bits |= PLAYBACK_SPK;
			break;
		case PLAYBACK_HP:
			gain_set_bits |= PLAYBACK_HP;
			break;
		case PLAYBACK_BT:
			gain_set_bits |= PLAYBACK_BT;
			break;
		case PLAYBACK_SPK_HP:
			gain_set_bits |= PLAYBACK_SPK_HP;
			break;
		case PLAYBACK_RING_SPK:
			gain_set_bits |= (PLAYBACK_SPK | PLAYBACK_RING_SPK);
			break;
		case PLAYBACK_RING_HP:
			gain_set_bits |= (PLAYBACK_HP | PLAYBACK_RING_HP);
			break;
		case PLAYBACK_RING_SPK_HP:
			gain_set_bits |= (PLAYBACK_SPK_HP |
					PLAYBACK_RING_SPK_HP);
			break;
		case PLAYBACK_HP_NO_MIC:
			gain_set_bits |= PLAYBACK_HP_NO_MIC;
			break;
		default:
			pr_err("%s: playback gain flag is wrong\n", __func__);
			break;
		}
	} else if (mode == VOICECALL_MODE) {
		default_gain_table_p = voicecall_gain_table;
		table_num = VOICECALL_GAIN_NUM;

		switch (device) {
		case VOICECALL_RCV:
			gain_set_bits |= VOICECALL_RCV;
			break;
		case VOICECALL_SPK:
			gain_set_bits |= VOICECALL_SPK;
			break;
		case VOICECALL_HP:
			gain_set_bits |= VOICECALL_HP;
			break;
		case VOICECALL_HP_NO_MIC:
			gain_set_bits |= VOICECALL_HP_NO_MIC;
			break;
		case VOICECALL_BT:
			gain_set_bits |= VOICECALL_BT;
			break;
		default:
			pr_err("%s: voicecall gain flag is wrong\n", __func__);
		}
	} else if (mode  == RECORDING_MODE) {
		default_gain_table_p = recording_gain_table;
		table_num = RECORDING_GAIN_NUM;

		switch (device) {
		case RECORDING_MAIN:
			gain_set_bits |= RECORDING_MAIN;
			break;
		case RECORDING_HP:
			gain_set_bits |= RECORDING_HP;
			break;
		case RECORDING_BT:
			gain_set_bits |= RECORDING_BT;
			break;
		case RECORDING_REC_MAIN:
			gain_set_bits |= RECORDING_REC_MAIN;
			break;
		case RECORDING_REC_HP:
			gain_set_bits |= RECORDING_REC_HP;
			break;
		case RECORDING_REC_BT:
			gain_set_bits |= RECORDING_REC_BT;
			break;
		case RECORDING_CAM_MAIN:
			gain_set_bits |= RECORDING_CAM_MAIN;
			break;
		case RECORDING_CAM_HP:
			gain_set_bits |= RECORDING_CAM_HP;
			break;
		case RECORDING_CAM_BT:
			gain_set_bits |= RECORDING_CAM_BT;
			break;
		default:
			pr_err("%s: recording gain flag is wrong\n", __func__);
		}

	} else if (mode == FMRADIO_MODE) {
		default_gain_table_p = fmradio_gain_table;
		table_num = FMRADIO_GAIN_NUM;

		switch (device) {
		case FMRADIO_HP:
			gain_set_bits |= FMRADIO_HP;
			break;
		case FMRADIO_SPK:
			gain_set_bits |= FMRADIO_SPK;
			break;
		case FMRADIO_SPK_HP:
			gain_set_bits |= FMRADIO_SPK_HP;
			break;
		default:
			pr_err("%s: fmradio gain flag is wrong\n", __func__);
		}
	}

	DEBUG_LOG("Set gain mode = 0x%x, device = 0x%x, gain_bits = 0x%x,\
		table_num=%d\n",
		mode, device, gain_set_bits, table_num);

	/* default gain table setting */
	for (i = 0; i < table_num; i++) {
		if ((default_gain_table_p + i)->mode & gain_set_bits) {
			val = wm8994_read(codec,
				(default_gain_table_p + i)->reg);
			val &= ~((default_gain_table_p + i)->mask);
			val |= (default_gain_table_p + i)->gain;
			wm8994_write(codec, (default_gain_table_p + i)->reg,
				val);
		}
	}

	return 0;

}

