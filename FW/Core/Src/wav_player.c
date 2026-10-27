/**
 ******************************************************************************
 * @file    wav_player.c
 * @brief   Lecteur WAV : carte SD (FatFs/SDIO) -> I2S2 (DMA circulaire) -> PCM5102A
 *
 * Principe
 *   - Le DMA envoie en boucle un buffer circulaire vers SPI2/I2S2.
 *   - Le HAL appelle un callback à la mi-parcours (1re moitié libre) et en
 *     fin de parcours (2e moitié libre). Ces callbacks ne font que lever un
 *     drapeau ; la lecture SD + conversion se font dans la boucle principale.
 *   - La fréquence I2S est recalculée pour chaque fichier (PLLI2S).
 *
 ******************************************************************************
 */
#include "wav_player.h"
#include "main.h"
#include "fatfs.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Handles générés par CubeMX */
extern I2S_HandleTypeDef hi2s2;
#if WAV_ENABLE_UART_LOG
extern UART_HandleTypeDef huart3;
#endif

/* LED d'état : on utilise le label CubeMX "LED_STATUS" s'il existe */
#ifndef LED_STATUS_Pin
#define LED_STATUS_Pin        GPIO_PIN_2
#define LED_STATUS_GPIO_Port  GPIOA
#endif
#define LED_ON()   HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_SET)
#define LED_OFF()  HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_RESET)

/* Journal optionnel */
#if WAV_ENABLE_UART_LOG
static void wav_log(const char *fmt, ...)
{
    char    line[128];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(line, sizeof line - 2u, fmt, ap);
    va_end(ap);
    if (n < 0) {
        return;
    }
    if ((size_t)n > sizeof line - 3u) {
        n = (int)sizeof line - 3;
    }
    line[n++] = '\r';
    line[n++] = '\n';
    (void)HAL_UART_Transmit(&huart3, (uint8_t *)line, (uint16_t)n, 50);
}
#define LOG(...)  wav_log(__VA_ARGS__)
#else
#define LOG(...)  ((void)0)
#endif

/* ------------------------------ Constantes -------------------------------- */

#define HALF_FRAMES      (WAV_BUF_FRAMES / 2u)
#define DMA_HALFWORDS    (WAV_BUF_FRAMES * 2u)     /* 2 canaux x 16 bits */
#define WAV_STAGE_BYTES  1536u                     /* multiple de 1,2,3,4,6 */
#define WAV_MIN_RATE     8000u
#define WAV_MAX_RATE     96000u

#define EV_HALF0         0x01u   /* 1re moitié du buffer libre  */
#define EV_HALF1         0x02u   /* 2e moitié du buffer libre   */
#define EV_ERROR         0x80u   /* erreur I2S/DMA              */

/* Codes retour internes */
enum {
    WAV_OK = 0,
    WAV_ERR_OPEN,
    WAV_ERR_IO,
    WAV_ERR_NOT_WAV,
    WAV_ERR_NO_DATA,
    WAV_ERR_FORMAT,
    WAV_ERR_RATE,
    WAV_ERR_I2S
};

/* ------------------------------- État ------------------------------------- */

/* Buffer DMA stéréo entrelacé L,R,L,R... (RAM principale, pas la CCM) */
static int16_t dma_buf[WAV_BUF_FRAMES * 2u] __attribute__((aligned(32)));

/* Zone de transit pour les formats à convertir (8/24 bits, mono) */
static uint8_t stage[WAV_STAGE_BYTES] __attribute__((aligned(4)));

static volatile uint8_t  ev_flags;
static volatile uint32_t gain_q15 =
    ((uint32_t)WAV_DEFAULT_VOLUME * WAV_DEFAULT_VOLUME * 32768u) / 10000u;

static struct {
    FIL      f;
    uint32_t rate;
    uint16_t channels;
    uint16_t bits;
    uint16_t block_align;   /* octets par trame (tous canaux confondus) */
    uint32_t data_left;     /* octets de données encore à lire          */
    uint8_t  io_error;
} S;

/* --------------------------- Callbacks HAL (ISR) --------------------------- */

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2) {
        ev_flags |= EV_HALF0;
    }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2) {
        ev_flags |= EV_HALF1;
    }
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI2) {
        ev_flags |= EV_ERROR;
    }
}

/* ------------------------------- Volume ----------------------------------- */

void WavPlayer_SetVolume(uint8_t percent)
{
    if (percent > 100u) {
        percent = 100u;
    }
    /* Courbe quadratique : 50 % ~ -12 dB, plus proche de l'oreille qu'une rampe linéaire */
    gain_q15 = ((uint32_t)percent * percent * 32768u) / 10000u;
}

static void apply_gain(int16_t *p, uint32_t n)
{
    const int32_t g = (int32_t)gain_q15;
    if (g >= 32768) {
        return;
    }
    while (n--) {
        *p = (int16_t)(((int32_t)*p * g) >> 15);
        p++;
    }
}

/* ----------------------------- Parsing WAV -------------------------------- */

static uint16_t le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/**
 * Lit l'en-tête RIFF/WAVE, saute les chunks inconnus (LIST, fact, ...) et
 * laisse le pointeur de fichier au début des données audio.
 */
static int wav_parse(void)
{
    FIL     *f = &S.f;
    UINT     br;
    uint8_t  riff[12];
    uint8_t  fmt[64];
    int      have_fmt = 0;
    uint16_t tag = 0;

    if (f_read(f, riff, sizeof riff, &br) != FR_OK || br != sizeof riff) {
        return WAV_ERR_IO;
    }
    if (memcmp(riff, "RIFF", 4) != 0 || memcmp(riff + 8, "WAVE", 4) != 0) {
        return WAV_ERR_NOT_WAV;
    }

    for (;;) {
        uint8_t  ck[8];
        uint32_t sz, left;

        if (f_read(f, ck, sizeof ck, &br) != FR_OK) {
            return WAV_ERR_IO;
        }
        if (br != sizeof ck) {
            return WAV_ERR_NO_DATA;
        }
        sz   = le32(ck + 4);
        left = (uint32_t)f_size(f) - (uint32_t)f_tell(f);

        if (memcmp(ck, "fmt ", 4) == 0) {
            if (sz < 16u || sz > sizeof fmt || sz > left) {
                return WAV_ERR_FORMAT;
            }
            if (f_read(f, fmt, sz, &br) != FR_OK || br != sz) {
                return WAV_ERR_IO;
            }
            tag         = le16(fmt);
            S.channels  = le16(fmt + 2);
            S.rate      = le32(fmt + 4);
            S.bits      = le16(fmt + 14);
            if (tag == 0xFFFEu && sz >= 26u) {   /* WAVE_FORMAT_EXTENSIBLE */
                tag = le16(fmt + 24);            /* 1er mot du GUID SubFormat */
            }
            have_fmt = 1;
            if ((sz & 1u) && f_lseek(f, f_tell(f) + 1u) != FR_OK) {
                return WAV_ERR_IO;
            }
        } else if (memcmp(ck, "data", 4) == 0) {
            if (!have_fmt) {
                return WAV_ERR_FORMAT;
            }
            if (tag != 1u || (S.bits != 8u && S.bits != 16u && S.bits != 24u) ||
                (S.channels != 1u && S.channels != 2u)) {
                return WAV_ERR_FORMAT;
            }
            if (S.rate < WAV_MIN_RATE || S.rate > WAV_MAX_RATE) {
                return WAV_ERR_RATE;
            }
            /* Taille déclarée absente/fausse (WAV "streamé") : on prend le reste du fichier */
            if (sz == 0u || sz > left) {
                sz = left;
            }
            S.block_align = (uint16_t)(S.channels * (S.bits / 8u));
            S.data_left   = sz - (sz % S.block_align);
            return (S.data_left >= S.block_align) ? WAV_OK : WAV_ERR_NO_DATA;
        } else {
            /* chunk inconnu : on saute (taille paire, padding inclus) */
            uint32_t skip = sz + (sz & 1u);
            if (sz > left) {
                return WAV_ERR_NO_DATA;
            }
            if (skip > left) {
                skip = left;
            }
            if (f_lseek(f, f_tell(f) + skip) != FR_OK) {
                return WAV_ERR_IO;
            }
        }
    }
}

/* ------------------------- Lecture + conversion --------------------------- */

static int16_t rd_sample(const uint8_t *p, uint16_t bits)
{
    if (bits == 8u) {
        return (int16_t)(((int32_t)p[0] - 128) * 256);         /* non signé -> signé 16 bits (pas de << sur négatif) */
    }
    if (bits == 16u) {
        return (int16_t)le16(p);
    }
    return (int16_t)le16(p + 1);                                /* 24 bits : on garde les 16 bits de poids fort */
}

static void convert(int16_t *dst, const uint8_t *src, uint32_t frames)
{
    const uint16_t bytes = S.bits / 8u;
    const int      stereo = (S.channels == 2u);

    while (frames--) {
        const int16_t l = rd_sample(src, S.bits);
        src += bytes;
        int16_t r = l;                                           /* mono : même signal à gauche et à droite */
        if (stereo) {
            r = rd_sample(src, S.bits);
            src += bytes;
        }
        *dst++ = l;
        *dst++ = r;
    }
}

/**
 * Remplit `frames` trames stéréo 16 bits dans dst (silence si le fichier est fini).
 * @return nombre de trames réellement issues du fichier
 */
static uint32_t fill_frames(int16_t *dst, uint32_t frames)
{
    const uint32_t ba   = S.block_align;
    const int      fast = (S.bits == 16u && S.channels == 2u);   /* lecture directe dans le buffer DMA */
    uint32_t       done = 0;

    while (done < frames && S.data_left >= ba) {
        uint32_t n = frames - done;
        UINT     br = 0;
        FRESULT  fr;

        if (!fast && n > WAV_STAGE_BYTES / ba) {
            n = WAV_STAGE_BYTES / ba;
        }
        if (n * ba > S.data_left) {
            n = S.data_left / ba;
        }

        fr = f_read(&S.f, fast ? (void *)(dst + 2u * done) : (void *)stage, n * ba, &br);
        if (fr != FR_OK) {
            S.io_error  = 1u;
            S.data_left = 0u;
            break;
        }
        n = br / ba;
        if (n == 0u) {                       /* fichier plus court que prévu */
            S.data_left = 0u;
            break;
        }
        if (!fast) {
            convert(dst + 2u * done, stage, n);
        }
        S.data_left -= n * ba;
        done        += n;
    }

    if (done < frames) {
        memset(dst + 2u * done, 0, (size_t)(frames - done) * 4u);
    }
    apply_gain(dst, done * 2u);
    return done;
}

/** Remplit une moitié du buffer DMA. @return 1 si la source est épuisée. */
static int fill_half(unsigned half)
{
    const uint32_t n = fill_frames(dma_buf + (size_t)half * HALF_FRAMES * 2u, HALF_FRAMES);
    return (n < HALF_FRAMES) || (S.data_left < S.block_align);
}

/* --------------------------- Horloge audio I2S ---------------------------- */

/**
 * Cherche (N, R) du PLLI2S pour obtenir la fréquence d'échantillonnage la plus
 * juste, en reproduisant le calcul de diviseur du HAL (16 bits, sans MCLK).
 * Le PLLI2S partage le diviseur M du PLL principal.
 */
static int audio_set_rate(uint32_t fs)
{
    const uint32_t pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
    const uint32_t src  = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) ? HSE_VALUE : HSI_VALUE;
    uint32_t best_err = 0xFFFFFFFFu, best_n = 0u, best_r = 0u;

    if (pllm == 0u) {
        return -1;
    }
    const uint32_t vco_in = src / pllm;

    for (uint32_t r = 2u; r <= 7u; r++) {
        for (uint32_t n = 50u; n <= 432u; n++) {
            const uint32_t vco = vco_in * n;
            if (vco < 100000000u || vco > 432000000u) {
                continue;
            }
            const uint32_t clk = vco / r;
            if (clk > 192000000u) {                      /* limite de l'horloge I2S (RM0090, PLLI2SR) */
                continue;
            }
            const uint32_t tmp = ((((clk / 32u) * 10u) / fs) + 5u) / 10u;   /* = 2*I2SDIV + ODD */
            if ((tmp >> 1) < 2u || (tmp >> 1) > 255u) {
                continue;
            }
            const uint32_t real = clk / (32u * tmp);
            const uint32_t err  = (real > fs) ? (real - fs) : (fs - real);
            if (err < best_err) {
                best_err = err;
                best_n   = n;
                best_r   = r;
            }
        }
    }
    if (best_n == 0u || best_err * 100u > fs) {          /* erreur > 1 % : refus */
        return -2;
    }

    RCC_PeriphCLKInitTypeDef pc;
    memset(&pc, 0, sizeof pc);
    pc.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    pc.PLLI2S.PLLI2SN       = best_n;
    pc.PLLI2S.PLLI2SR       = best_r;
    if (HAL_RCCEx_PeriphCLKConfig(&pc) != HAL_OK) {
        return -3;
    }

    hi2s2.Init.AudioFreq = fs;
    if (HAL_I2S_Init(&hi2s2) != HAL_OK) {                /* recalcule I2SDIV/ODD */
        return -4;
    }
    LOG("I2S: PLLI2S N=%lu R=%lu, erreur %lu Hz", (unsigned long)best_n, (unsigned long)best_r,
        (unsigned long)best_err);
    return 0;
}

/* ------------------------------- Lecture ---------------------------------- */

static int play_file(const char *path, const char *name)
{
    int      rc;
    int      eof       = 0;
    unsigned last_half = 0u;
    uint32_t underruns = 0u;
    int      result    = WAV_OK;

    memset(&S, 0, sizeof S);
    if (f_open(&S.f, path, FA_READ) != FR_OK) {
        return WAV_ERR_OPEN;
    }

    rc = wav_parse();
    if (rc != WAV_OK) {
        LOG("%s: en-tete WAV non supporte (code %d)", name, rc);
        f_close(&S.f);
        return rc;
    }
    LOG("Lecture %s : %lu Hz, %u bits, %u canal(aux)", name, (unsigned long)S.rate, (unsigned)S.bits,
        (unsigned)S.channels);

    if (audio_set_rate(S.rate) != 0) {
        LOG("%s: horloge I2S impossible", name);
        f_close(&S.f);
        return WAV_ERR_RATE;
    }

    /* Pré-remplissage des deux moitiés avant de lancer le DMA */
    memset(dma_buf, 0, sizeof dma_buf);
    if (fill_half(0u)) {
        eof = 1;
        last_half = 0u;
    }
    if (fill_half(1u) && !eof) {
        eof = 1;
        last_half = 1u;
    }

    ev_flags = 0u;
    if (HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t *)dma_buf, (uint16_t)DMA_HALFWORDS) != HAL_OK) {
        f_close(&S.f);
        return WAV_ERR_I2S;
    }
    LED_ON();

    for (;;) {
        uint8_t f;

        __disable_irq();
        f = ev_flags;
        ev_flags = 0u;
        __enable_irq();

        if (f == 0u) {
            __WFI();
            continue;
        }
        if (f & EV_ERROR) {
            result = WAV_ERR_I2S;
            break;
        }
        if ((f & (EV_HALF0 | EV_HALF1)) == (EV_HALF0 | EV_HALF1)) {
            underruns++;                                  /* on a raté une moitié : micro-coupure */
        }

        int finished = 0;
        for (unsigned h = 0u; h < 2u; h++) {
            if (!(f & (1u << h))) {
                continue;
            }
            if (eof && h == last_half) {
                finished = 1;                             /* dernière moitié de données entièrement émise */
                break;
            }
            if (fill_half(h) && !eof) {
                eof = 1;
                last_half = h;
            }
        }
        if (finished) {
            break;
        }
    }

    (void)HAL_I2S_DMAStop(&hi2s2);
    LED_OFF();
    if (result == WAV_OK && S.io_error) {
        result = WAV_ERR_IO;
    }
    if (underruns != 0u) {
        LOG("%s: %lu micro-coupure(s) (carte SD trop lente ?)", name, (unsigned long)underruns);
    }
    f_close(&S.f);
    return result;
}

/* ------------------------- Boucle principale ------------------------------ */

static int has_wav_ext(const char *name)
{
    const size_t n = strlen(name);
    if (n < 5u) {
        return 0;
    }
    const char *e = name + n - 4u;
    return e[0] == '.' && tolower((unsigned char)e[1]) == 'w' && tolower((unsigned char)e[2]) == 'a' &&
           tolower((unsigned char)e[3]) == 'v';
}

/** Code lumineux : n clignotements puis une pause. 1 = SD, 2 = aucun WAV, 3 = fichier refusé, 4 = I2S. */
static void blink(unsigned n)
{
    while (n--) {
        LED_ON();
        HAL_Delay(150);
        LED_OFF();
        HAL_Delay(150);
    }
    HAL_Delay(900);
}

void WavPlayer_Run(void)
{
    static DIR     dir;
    static FILINFO fno;
    static char    path[4 + sizeof(fno.fname) + 4];

    LED_OFF();

    for (;;) {
        unsigned played = 0u;
        FRESULT  fr     = f_mount(&SDFatFS, (const TCHAR *)SDPath, 1);

        if (fr != FR_OK) {
            LOG("Montage SD impossible (FRESULT %d)", (int)fr);
            (void)f_mount(NULL, (const TCHAR *)SDPath, 0);
            blink(1u);
            continue;                                     /* on retente : carte insérée plus tard */
        }
        if (f_opendir(&dir, (const TCHAR *)SDPath) != FR_OK) {
            blink(1u);
            continue;
        }

        for (;;) {
            fr = f_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == '\0') {
                break;
            }
            if ((fno.fattrib & (AM_DIR | AM_HID | AM_SYS)) || fno.fname[0] == '.' || !has_wav_ext(fno.fname)) {
                continue;                                 /* ignore dossiers, cachés, ._xxx de macOS */
            }
            (void)snprintf(path, sizeof path, "%s%s", SDPath, fno.fname);

            const int rc = play_file(path, fno.fname);
            if (rc == WAV_OK) {
                played++;
            } else if (rc == WAV_ERR_I2S) {
                blink(4u);
            } else {
                blink(3u);
            }
        }
        (void)f_closedir(&dir);

        if (played == 0u) {
            LOG("Aucun fichier .wav lisible a la racine de la carte");
            blink(2u);
        }
#if !WAV_LOOP_ALL
        if (played != 0u) {
            for (;;) {
                __WFI();
            }
        }
#endif
    }
}
