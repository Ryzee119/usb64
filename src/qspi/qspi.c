/*
 * Portions of this software were developed at http://www.pjrc.com/
 * Those portions licensed under MIT License Agreement, (the "License");
 * You may not use these files except in compliance with the License.
 * You may obtain a copy of the License at: http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
*/

#include <stdint.h>
#include <Arduino.h>
#include "printf.h"
#include "qspi.h"

#define LUT0(opcode, pads, operand) (FLEXSPI_LUT_INSTRUCTION((opcode), (pads), (operand)))
#define LUT1(opcode, pads, operand) (FLEXSPI_LUT_INSTRUCTION((opcode), (pads), (operand)) << 16)
#define CMD_SDR FLEXSPI_LUT_OPCODE_CMD_SDR
#define ADDR_SDR FLEXSPI_LUT_OPCODE_RADDR_SDR
#define READ_SDR FLEXSPI_LUT_OPCODE_READ_SDR
#define WRITE_SDR FLEXSPI_LUT_OPCODE_WRITE_SDR
#define DUMMY_SDR FLEXSPI_LUT_OPCODE_DUMMY_SDR
#define PINS1 FLEXSPI_LUT_NUM_PADS_1
#define PINS4 FLEXSPI_LUT_NUM_PADS_4

//#define FLASH_MEMMAP 1 //Use memory-mapped access

static const uint32_t flashBaseAddr = 0x01000000u;
static char flashID[8];

uint8_t cs = 51, clk = 53, miso = 49, mosi = 52;
static void spi_bitbang_init()
{
    digitalWrite(cs, HIGH);
    digitalWrite(clk, LOW);
    pinMode(cs, OUTPUT);
    pinMode(clk, OUTPUT);
    pinMode(mosi, OUTPUT);
    pinMode(miso, INPUT);

    pinMode(48, INPUT); //Disable RAM
}
static uint8_t spi_bitbang(uint8_t data, bool final)
{
    uint8_t ret = 0;
    digitalWrite(cs, LOW);
    delay(1);
    for (int i = 7; i >= 0; i--)
    {
        delay(1);
        digitalWrite(clk, LOW);
        (data & (1 << i)) ? digitalWrite(mosi, HIGH) : digitalWrite(mosi, LOW);
        delay(1);
        ret |= (digitalRead(miso) << i);
        digitalWrite(clk, HIGH);
    }
    if (final)
        digitalWrite(cs, HIGH);
    return ret;
}

static void setupFlexSPI2()
{
    memset(flashID, 0, sizeof(flashID));
    // initialize pins
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_22 = 0x1B0F9; // 100K pullup, strong drive, max speed, hyst
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_23 = 0x110F9; // keeper, strong drive, max speed, hyst
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_24 = 0x1B0F9; // 100K pullup, strong drive, max speed, hyst
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_25 = 0x100F9; // strong drive, max speed, hyst
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_26 = 0x170F9; // 47K pullup, strong drive, max speed, hyst
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_27 = 0x170F9; // 47K pullup, strong drive, max speed, hyst
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_28 = 0x170F9; // 47K pullup, strong drive, max speed, hyst
    IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_29 = 0x170F9; // 47K pullup, strong drive, max speed, hyst

    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_22 = 8 | 0x10; // ALT1 = FLEXSPI2_A_SS1_B (Flash)
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_23 = 8 | 0x10; // ALT1 = FLEXSPI2_A_DQS
    //IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_24 = 8 | 0x10; // ALT1 = FLEXSPI2_A_SS0_B (RAM)
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_25 = 8 | 0x10; // ALT1 = FLEXSPI2_A_SCLK
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_26 = 8 | 0x10; // ALT1 = FLEXSPI2_A_DATA0
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_27 = 8 | 0x10; // ALT1 = FLEXSPI2_A_DATA1
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28 = 8 | 0x10; // ALT1 = FLEXSPI2_A_DATA2
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29 = 8 | 0x10; // ALT1 = FLEXSPI2_A_DATA3

    IOMUXC_FLEXSPI2_IPP_IND_DQS_FA_SELECT_INPUT = 1;     // GPIO_EMC_23 for Mode: ALT8, pg 986
    IOMUXC_FLEXSPI2_IPP_IND_IO_FA_BIT0_SELECT_INPUT = 1; // GPIO_EMC_26 for Mode: ALT8
    IOMUXC_FLEXSPI2_IPP_IND_IO_FA_BIT1_SELECT_INPUT = 1; // GPIO_EMC_27 for Mode: ALT8
    IOMUXC_FLEXSPI2_IPP_IND_IO_FA_BIT2_SELECT_INPUT = 1; // GPIO_EMC_28 for Mode: ALT8
    IOMUXC_FLEXSPI2_IPP_IND_IO_FA_BIT3_SELECT_INPUT = 1; // GPIO_EMC_29 for Mode: ALT8
    IOMUXC_FLEXSPI2_IPP_IND_SCK_FA_SELECT_INPUT = 1;     // GPIO_EMC_25 for Mode: ALT8

    // turn on clock
    //clocks[4] = {396.0f, 720.0f, 664.62f, 528.0f} / CCM_CBCMR_FLEXSPI2_PODF + 1
    CCM_CBCMR = (CCM_CBCMR & ~(CCM_CBCMR_FLEXSPI2_PODF_MASK | CCM_CBCMR_FLEXSPI2_CLK_SEL_MASK)) | CCM_CBCMR_FLEXSPI2_PODF(7) | CCM_CBCMR_FLEXSPI2_CLK_SEL(0); // 99 MHz
    CCM_CCGR7 |= CCM_CCGR7_FLEXSPI2(CCM_CCGR_ON);

    FLEXSPI2_MCR0 |= FLEXSPI_MCR0_MDIS;
    FLEXSPI2_MCR0 = (FLEXSPI2_MCR0 & ~(FLEXSPI_MCR0_AHBGRANTWAIT_MASK | FLEXSPI_MCR0_IPGRANTWAIT_MASK | FLEXSPI_MCR0_SCKFREERUNEN | FLEXSPI_MCR0_COMBINATIONEN | FLEXSPI_MCR0_DOZEEN | FLEXSPI_MCR0_HSEN | FLEXSPI_MCR0_ATDFEN | FLEXSPI_MCR0_ARDFEN | FLEXSPI_MCR0_RXCLKSRC_MASK | FLEXSPI_MCR0_SWRESET)) | FLEXSPI_MCR0_AHBGRANTWAIT(0xFF) | FLEXSPI_MCR0_IPGRANTWAIT(0xFF) | FLEXSPI_MCR0_RXCLKSRC(1) | FLEXSPI_MCR0_MDIS;
    FLEXSPI2_MCR1 = FLEXSPI_MCR1_SEQWAIT(0xFFFF) | FLEXSPI_MCR1_AHBBUSWAIT(0xFFFF);
    FLEXSPI2_MCR2 = (FLEXSPI_MCR2 & ~(FLEXSPI_MCR2_RESUMEWAIT_MASK | FLEXSPI_MCR2_SCKBDIFFOPT | FLEXSPI_MCR2_SAMEDEVICEEN | FLEXSPI_MCR2_CLRLEARNPHASE | FLEXSPI_MCR2_CLRAHBBUFOPT)) | FLEXSPI_MCR2_RESUMEWAIT(0x20) /*| FLEXSPI_MCR2_SAMEDEVICEEN*/;

    FLEXSPI2_AHBCR = FLEXSPI2_AHBCR & ~(FLEXSPI_AHBCR_READADDROPT | FLEXSPI_AHBCR_PREFETCHEN | FLEXSPI_AHBCR_BUFFERABLEEN | FLEXSPI_AHBCR_CACHABLEEN);
    uint32_t mask = (FLEXSPI_AHBRXBUFCR0_PREFETCHEN | FLEXSPI_AHBRXBUFCR0_PRIORITY_MASK | FLEXSPI_AHBRXBUFCR0_MSTRID_MASK | FLEXSPI_AHBRXBUFCR0_BUFSZ_MASK);
    FLEXSPI2_AHBRXBUF0CR0 = (FLEXSPI2_AHBRXBUF0CR0 & ~mask) | FLEXSPI_AHBRXBUFCR0_PREFETCHEN | FLEXSPI_AHBRXBUFCR0_BUFSZ(64);
    FLEXSPI2_AHBRXBUF1CR0 = (FLEXSPI2_AHBRXBUF0CR0 & ~mask) | FLEXSPI_AHBRXBUFCR0_PREFETCHEN | FLEXSPI_AHBRXBUFCR0_BUFSZ(64);
    FLEXSPI2_AHBRXBUF2CR0 = mask;
    FLEXSPI2_AHBRXBUF3CR0 = mask;

    // RX watermark = one 64 bit line
    FLEXSPI2_IPRXFCR = (FLEXSPI_IPRXFCR & 0xFFFFFFC0) | FLEXSPI_IPRXFCR_CLRIPRXF;
    // TX watermark = one 64 bit line
    FLEXSPI2_IPTXFCR = (FLEXSPI_IPTXFCR & 0xFFFFFFC0) | FLEXSPI_IPTXFCR_CLRIPTXF;

    FLEXSPI2_INTEN = 0;
    FLEXSPI2_FLSHA1CR0 = 0x4000;
    FLEXSPI2_FLSHA1CR1 = FLEXSPI_FLSHCR1_CSINTERVAL(2) | FLEXSPI_FLSHCR1_TCSH(3) | FLEXSPI_FLSHCR1_TCSS(3);
    FLEXSPI2_FLSHA1CR2 = FLEXSPI_FLSHCR2_AWRSEQID(6) | FLEXSPI_FLSHCR2_AWRSEQNUM(0) | FLEXSPI_FLSHCR2_ARDSEQID(5) | FLEXSPI_FLSHCR2_ARDSEQNUM(0);

    FLEXSPI2_FLSHA2CR0 = 0x40000;
    FLEXSPI2_FLSHA2CR1 = FLEXSPI_FLSHCR1_CSINTERVAL(2) | FLEXSPI_FLSHCR1_TCSH(3) | FLEXSPI_FLSHCR1_TCSS(3);
    FLEXSPI2_FLSHA2CR2 = FLEXSPI_FLSHCR2_AWRSEQID(6) | FLEXSPI_FLSHCR2_AWRSEQNUM(0) | FLEXSPI_FLSHCR2_ARDSEQID(5) | FLEXSPI_FLSHCR2_ARDSEQNUM(0);

    FLEXSPI2_MCR0 &= ~FLEXSPI_MCR0_MDIS;

    FLEXSPI2_LUTKEY = FLEXSPI_LUTKEY_VALUE;
    FLEXSPI2_LUTCR = FLEXSPI_LUTCR_UNLOCK;
    volatile uint32_t *luttable = &FLEXSPI2_LUT0;
    for (int i = 0; i < 64; i++)
        luttable[i] = 0;
    FLEXSPI2_MCR0 |= FLEXSPI_MCR0_SWRESET;
    while (FLEXSPI2_MCR0 & FLEXSPI_MCR0_SWRESET)
        ; // wait

    // CBCMR[FLEXSPI2_SEL]
    // CBCMR[FLEXSPI2_PODF]

    FLEXSPI2_LUTKEY = FLEXSPI_LUTKEY_VALUE;
    FLEXSPI2_LUTCR = FLEXSPI_LUTCR_UNLOCK;

    // cmd index 0 = exit QPI mode
    FLEXSPI2_LUT0 = LUT0(CMD_SDR, PINS4, 0xF5); // RAM

    // cmd index 1 = reset enable
    FLEXSPI2_LUT4 = LUT0(CMD_SDR, PINS1, 0x66); // RAM, FLASH

    // cmd index 2 = reset
    FLEXSPI2_LUT8 = LUT0(CMD_SDR, PINS1, 0x99); // RAM, FLASH

    // cmd index 3 = read ID bytes
    FLEXSPI2_LUT12 = LUT0(CMD_SDR, PINS1, 0x9F) | LUT1(DUMMY_SDR, PINS1, 24);
    FLEXSPI2_LUT13 = LUT0(READ_SDR, PINS1, 1);

    // cmd index 4 = enter QPI mode
    FLEXSPI2_LUT16 = LUT0(CMD_SDR, PINS1, 0x35); //RAM

    // cmd index 5 = read QPI
    FLEXSPI2_LUT20 = LUT0(CMD_SDR, PINS4, 0xEB) | LUT1(ADDR_SDR, PINS4, 24);
    FLEXSPI2_LUT21 = LUT0(DUMMY_SDR, PINS4, 6) | LUT1(READ_SDR, PINS4, 1); //RAM, FLASH

    // cmd index 6 = write QPI
    FLEXSPI2_LUT24 = LUT0(CMD_SDR, PINS4, 0x35) | LUT1(ADDR_SDR, PINS4, 24);
    FLEXSPI2_LUT25 = LUT0(WRITE_SDR, PINS4, 1); // RAM

    // cmd index 7 = read ID bytes SPI
    FLEXSPI2_LUT28 = LUT0(CMD_SDR, PINS1, 0x9F) | LUT1(READ_SDR, PINS1, 1); //RAM, FLASH

    // ----------------- FLASH only ----------------------------------------------

    // cmd index 8 = read Status register #1 SPI
    FLEXSPI2_LUT32 = LUT0(CMD_SDR, PINS1, 0x05) | LUT1(READ_SDR, PINS1, 1);

    // cmd index 9 = read Status register #2 SPI
    FLEXSPI2_LUT36 = LUT0(CMD_SDR, PINS1, 0x35) | LUT1(READ_SDR, PINS1, 1);

    //cmd index 10 = exit QPI mode
    FLEXSPI2_LUT40 = LUT0(CMD_SDR, PINS4, 0xFF);

    //cmd index 11 = write enable QPI
    FLEXSPI2_LUT44 = LUT0(CMD_SDR, PINS4, 0x06);

    //cmd index 12 = sector erase
    FLEXSPI2_LUT48 = LUT0(CMD_SDR, PINS4, 0x20) | LUT1(ADDR_SDR, PINS4, 24);

    //cmd index 13 = page program
    FLEXSPI2_LUT52 = LUT0(CMD_SDR, PINS4, 0x02) | LUT1(ADDR_SDR, PINS4, 24);
    FLEXSPI2_LUT53 = LUT0(WRITE_SDR, PINS4, 1);

    //cmd index 14 = set read parameters
    FLEXSPI2_LUT56 = LUT0(CMD_SDR, PINS4, 0xc0) | LUT1(CMD_SDR, PINS4, 0x20);

    //cmd index 15 = enter QPI mode
    FLEXSPI2_LUT60 = LUT0(CMD_SDR, PINS1, 0x38);
}

static void flexspi_ip_command(uint32_t index, uint32_t addr)
{
    uint32_t n;
    FLEXSPI2_IPCR0 = addr;
    FLEXSPI2_IPCR1 = FLEXSPI_IPCR1_ISEQID(index);
    FLEXSPI2_IPCMD = FLEXSPI_IPCMD_TRG;
    while (!((n = FLEXSPI2_INTR) & FLEXSPI_INTR_IPCMDDONE))
        ; // wait
    if (n & FLEXSPI_INTR_IPCMDERR)
    {
        printf("Error: FLEXSPI2_IPRXFSTS=%08lX\n", FLEXSPI2_IPRXFSTS);
    }
    FLEXSPI2_INTR = FLEXSPI_INTR_IPCMDDONE;
}

static void flexspi_ip_read(uint32_t index, uint32_t addr, void *data, uint32_t length)
{
    uint32_t n;
    uint8_t *p = (uint8_t *)data;
    const uint8_t *src;

    FLEXSPI2_IPCR0 = addr;
    FLEXSPI2_IPCR1 = FLEXSPI_IPCR1_ISEQID(index) | FLEXSPI_IPCR1_IDATSZ(length);
    FLEXSPI2_IPCMD = FLEXSPI_IPCMD_TRG;
    while (!((n = FLEXSPI2_INTR) & FLEXSPI_INTR_IPCMDDONE))
    {
        if (n & FLEXSPI_INTR_IPRXWA)
        {
            if (length >= 8)
            {
                length -= 8;
                *(uint32_t *)(p + 0) = FLEXSPI2_RFDR0;
                *(uint32_t *)(p + 4) = FLEXSPI2_RFDR1;
                p += 8;
            }
            else
            {
                src = (const uint8_t *)&FLEXSPI2_RFDR0;
                while (length > 0)
                {
                    length--;
                    *p++ = *src++;
                }
            }
            FLEXSPI2_INTR = FLEXSPI_INTR_IPRXWA;
        }
    }
    if (n & FLEXSPI_INTR_IPCMDERR)
    {
        printf("Error: FLEXSPI2_IPRXFSTS=%08lX\r\n", FLEXSPI2_IPRXFSTS);
    }
    FLEXSPI2_INTR = FLEXSPI_INTR_IPCMDDONE;
    src = (const uint8_t *)&FLEXSPI2_RFDR0;
    while (length > 0)
    {
        *p++ = *src++;
        length--;
    }
    if (FLEXSPI2_INTR & FLEXSPI_INTR_IPRXWA)
        FLEXSPI2_INTR = FLEXSPI_INTR_IPRXWA;
}

static void flexspi_ip_write(uint32_t index, uint32_t addr, const void *data, uint32_t length)
{
    const uint8_t *src;
    uint32_t n, wrlen;

    FLEXSPI2_IPCR0 = addr;
    FLEXSPI2_IPCR1 = FLEXSPI_IPCR1_ISEQID(index) | FLEXSPI_IPCR1_IDATSZ(length);
    src = (const uint8_t *)data;
    FLEXSPI2_IPCMD = FLEXSPI_IPCMD_TRG;

    while (!((n = FLEXSPI2_INTR) & FLEXSPI_INTR_IPCMDDONE))
    {

        if (n & FLEXSPI_INTR_IPTXWE)
        {
            wrlen = length;
            if (wrlen > 8)
                wrlen = 8;
            if (wrlen > 0)
            {

                //memcpy((void *)&FLEXSPI2_TFDR0, src, wrlen); !crashes sometimes!
                //src += wrlen;
                uint8_t *p = (uint8_t *)&FLEXSPI2_TFDR0;
                for (unsigned i = 0; i < wrlen; i++)
                    *p++ = *src++;
                length -= wrlen;
                FLEXSPI2_INTR = FLEXSPI_INTR_IPTXWE;
            }
        }
    }

    if (n & FLEXSPI_INTR_IPCMDERR)
    {
        printf("Error: FLEXSPI2_IPRXFSTS=%08lX\r\n", FLEXSPI2_IPRXFSTS);
    }

    FLEXSPI2_INTR = FLEXSPI_INTR_IPCMDDONE;
}

static void printStatusRegs()
{
#if 0
    uint8_t val;

    flexspi_ip_read(8, flashBaseAddr, &val, 1);
    printf("Status 1:");
    printf(" %02X", val);
    printf("\n");

    // cmd index 9 = read Status register #2 SPI
    flexspi_ip_read(9, flashBaseAddr, &val, 1);
    printf("Status 2:");
    printf(" %02X", val);
    printf("\n");
#endif
}

/*
   Waits for busy bit = 0 (statusregister #1 )
   Timeout is optional
*/
static bool waitFlash(uint32_t timeout)
{
    uint8_t val;
    uint32_t t = millis();
    FLEXSPI_IPRXFCR = FLEXSPI_IPRXFCR_CLRIPRXF; // clear rx fifo
    do
    {
        flexspi_ip_read(8, flashBaseAddr, &val, 1);
        if (timeout && (millis() - t > timeout))
            return 1;
    } while ((val & 0x01) == 1);
    return 0;
}

static void setupFlexSPI2Flash()
{

    // reset the chip
    flexspi_ip_command(10, flashBaseAddr); //exit QPI
    flexspi_ip_command(1, flashBaseAddr);  //reset enable
    flexspi_ip_command(2, flashBaseAddr);  //reset
    delayMicroseconds(50);

    flexspi_ip_read(7, flashBaseAddr, flashID, sizeof(flashID));

#if 0
    printf("ID:");
    for (unsigned i = 0; i < sizeof(flashID); i++)
        printf(" %02X", flashID[i]);
    printf("\n");
#endif

    printStatusRegs();
    //TODO!!!!! set QPI enable bit in status reg #2 if not factory set!!!!!

    //  printf("ENTER QPI MODE");
    flexspi_ip_command(15, flashBaseAddr);

    //patch LUT for QPI:
    // cmd index 8 = read Status register #1
    FLEXSPI2_LUT32 = LUT0(CMD_SDR, PINS4, 0x05) | LUT1(READ_SDR, PINS4, 1);
    // cmd index 9 = read Status register #2
    FLEXSPI2_LUT36 = LUT0(CMD_SDR, PINS4, 0x35) | LUT1(READ_SDR, PINS4, 1);

    flexspi_ip_command(14, flashBaseAddr);

    printStatusRegs();

    flexspi_ip_command(1, flashBaseAddr); //reset enable
    flexspi_ip_command(2, flashBaseAddr); //reset
}

void qspi_erase_chip()
{
    pinMode(48, INPUT); //Disable RAM
    setupFlexSPI2();
    setupFlexSPI2Flash();

    waitFlash(0);
    flexspi_ip_command(11, flashBaseAddr);

    printf("Erasing... (may take some time)\r\n");
    uint32_t t = millis();
    FLEXSPI2_LUT60 = LUT0(CMD_SDR, PINS4, 0x60); //Chip erase
    flexspi_ip_command(15, flashBaseAddr);

#ifdef FLASH_MEMMAP
    arm_dcache_delete((void *)((uint32_t)extBase + flashBaseAddr), flashCapacity);
#endif

    while (waitFlash(500))
    {
        printf(".");
    }

    t = millis() - t;
    printf("\nChip erased in %d seconds.\n", t / 1000);
}

//********************************************************************************************************
//********************************************************************************************************
//********************************************************************************************************
/*
   SPIFFS interface
*/

#define LOG_PAGE_SIZE 256

//********************************************************************************************************
static const uint32_t _flashsize = 1024 * 1024 * 16; //16 Mbyte
static const uint32_t _blocksize = 4096U;
static const uint32_t _pagesize = 256U;

void qspi_init(uint32_t *sector_size, uint32_t *flash_size)
{
    spi_bitbang_init();
    spi_bitbang(0x66, true); //Reset enable
    spi_bitbang(0x99, true); //Reset
    delay(50);
    spi_bitbang(0x06, true);  //Write enable
    spi_bitbang(0x31, false); //Write status reg 2 command
    spi_bitbang(0x02, true);  //Write QSPI flag in status reg 2
    delay(100);
    setupFlexSPI2();
    setupFlexSPI2Flash();
    waitFlash(0);
    flexspi_ip_command(11, flashBaseAddr);
    if (sector_size)
        *sector_size = _blocksize;
    if (flash_size)
        *flash_size = _flashsize;
}

void qspi_get_flash_properties(uint32_t *sector_size, uint32_t *flash_size)
{
    if (sector_size)
        *sector_size = _blocksize;
    if (flash_size)
        *flash_size = _flashsize;
}

uint8_t qspi_read(uint32_t addr, uint32_t size, uint8_t *dst)
{
    flexspi_ip_read(5, flashBaseAddr + addr, dst, size);
    return 0;
}

uint8_t qspi_write(uint32_t addr, uint32_t size, uint8_t *src)
{
    int s = size;
    while (s > 0)
    {
        flexspi_ip_command(11, flashBaseAddr);                      // write enable
        flexspi_ip_write(13, flashBaseAddr + addr, src, _pagesize); // write
        //delay(3);
        waitFlash(0);

#ifdef FLASH_MEMMAP
        arm_dcache_delete((void *)((uint32_t)extBase + addr), _pagesize);
#endif
        src += _pagesize;
        addr += _pagesize;
        s -= _pagesize;
    }
    return 0;
}

uint8_t qspi_erase(uint32_t addr, uint32_t size)
{
    int s = size;
    while (s > 0)
    {
        flexspi_ip_command(11, flashBaseAddr);
        flexspi_ip_command(12, flashBaseAddr + addr);

#ifdef FLASH_MEMMAP
        arm_dcache_delete((void *)((uint32_t)extBase + addr), _blocksize);
#endif

        addr += _blocksize;
        s -= _blocksize;
        waitFlash(0);
    }
    return 0;
}