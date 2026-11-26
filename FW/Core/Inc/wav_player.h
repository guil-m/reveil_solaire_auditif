/**
 ******************************************************************************
 * @file    wav_player.h
 * @brief   Lecteur WAV : carte SD (FatFs/SDIO) -> I2S2 (DMA) -> PCM5102A
 *
 * Cible   : STM32F405RGT6 + HAL STM32CubeF4 (projet généré par CubeMX)
 * Formats : WAV PCM 8 / 16 / 24 bits, mono ou stéréo, 8 kHz à 96 kHz
 *
 * Contexte matériel pris en compte (carte "STM32F4 Test Board") :
 *   - I2S2 : PB12 = WS, PB13 = CK (BCK), PB15 = SD, MCLK non utilisée
 *            (PCM5102A en mode 3 fils, SCK relié à la masse)
 *   - SDIO 4 bits (PC8..PC12, PD2) + détection de carte sur PA15 (DSIO_DET),
 *     tirée au + par une résistance externe, niveau bas = carte présente
 *   - LED d'état sur PA2 (LED_STATUS), journal optionnel sur USART3
 *
 * Sources et références : voir le bloc "Références" en tête de wav_player.c.
 ******************************************************************************
 */
#ifndef WAV_PLAYER_H
#define WAV_PLAYER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------- Configuration (surchargeable) ------------------- */

/** Taille du buffer DMA complet, en trames stéréo (2 moitiés ping-pong).
 *  4096 trames = 16 Ko de RAM ; chaque moitié dure ~46 ms à 44,1 kHz.
 *  Plus grand = plus tolérant aux pauses de la carte SD. */
#ifndef WAV_BUF_FRAMES
#define WAV_BUF_FRAMES        4096u
#endif

/** Volume initial en pourcent (0..100), courbe quadratique (plus naturelle). */
#ifndef WAV_DEFAULT_VOLUME
#define WAV_DEFAULT_VOLUME    80u
#endif

/** 1 = rejoue en boucle tous les .wav de la racine ; 0 = une seule passe. */
#ifndef WAV_LOOP_ALL
#define WAV_LOOP_ALL          1
#endif

/** 1 = journal texte sur USART3 (huart3, header J4 de ta carte, 115200 bauds).
 *  Activé par défaut : ton main.c configure déjà USART3. */
#ifndef WAV_ENABLE_UART_LOG
#define WAV_ENABLE_UART_LOG   1
#endif

/** 1 = utilise la broche de détection de carte DSIO_DET (PA15). Si le label
 *  DSIO_DET_Pin n'existe pas dans main.h, la détection est désactivée. */
#ifndef WAV_USE_CARD_DETECT
#define WAV_USE_CARD_DETECT   1
#endif

/** Niveau logique de DSIO_DET quand une carte est insérée (GPIO_PIN_RESET = 0
 *  = contact fermé vers la masse, pull-up externe sur ta carte). */
#ifndef WAV_CD_PRESENT_LEVEL
#define WAV_CD_PRESENT_LEVEL  GPIO_PIN_RESET
#endif

/** 1 = si CubeMX a laissé "SDIO ClockBypass = ENABLE" (horloge SD = 48 MHz,
 *  hors spécification 25 MHz), le lecteur repasse l'horloge à 24 MHz.
 *  Le bon réglage reste de corriger CubeMX (voir README). */
#ifndef WAV_SD_CLAMP_24MHZ
#define WAV_SD_CLAMP_24MHZ    1
#endif

/* ------------------------------- API --------------------------------------- */

/**
 * @brief  Attend une carte SD, la monte, puis lit en boucle les fichiers .wav
 *         de la racine.
 * @note   Ne retourne jamais (sauf WAV_LOOP_ALL == 0 : alors elle attend en WFI).
 *         À appeler une seule fois, dans USER CODE 2 de main(), après
 *         MX_DMA_Init(), MX_I2S2_Init(), MX_SDIO_SD_Init() et MX_FATFS_Init().
 *         Tout code placé après l'appel (boucle while(1) comprise) n'est
 *         donc jamais exécuté.
 */
void WavPlayer_Run(void);

/**
 * @brief  Règle le volume logiciel (utilisable pendant la lecture).
 * @param  percent  0..100 (au-delà : saturé à 100).
 */
void WavPlayer_SetVolume(uint8_t percent);

#ifdef __cplusplus
}
#endif

#endif /* WAV_PLAYER_H */
