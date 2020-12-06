/*
 * zp.cpp - ZP-coder from DjVuLibre, an adaptive arithmetical binary coder
 */

/* The DjVuLibre code is a bit reorganized, but the algorithms are the same.
 * Z-coder was stripped.
 */

#include "../base/mdjvucfg.h"
#include <minidjvu-mod/minidjvu-mod.h>
#include <stdlib.h>
#include "zp.h"

ZPMemoryWatcher::~ZPMemoryWatcher()
{
}


// NumContext {{{

enum {numcontext_first_allocation_size = 512};
void ZPNumContext::init()/*{{{*/
{
    n = 1;
    nodes[0].value = 0;
    left[0] = right[0] = 0;
}/*}}}*/
ZPNumContext::ZPNumContext(int32 amin, int32 amax, ZPMemoryWatcher *w)/*{{{*/
    : min(amin), max(amax), watcher(w)
{
    assert(amin <= amax);
    allocated = numcontext_first_allocation_size;
    nodes = (ZPBitContext *) malloc(allocated * sizeof(ZPBitContext));
    left  = (uint16 *) malloc(allocated * sizeof(uint16));
    right = (uint16 *) malloc(allocated * sizeof(uint16));
    init();
}/*}}}*/
ZPNumContext::~ZPNumContext()/*{{{*/
{
    free(nodes);
    free(left);
    free(right);
}/*}}}*/
uint16 ZPNumContext::get_left(uint16 i)/*{{{*/
{
    assert(i < n);
    uint16 r = left[i];
    if (r) return r;
    r = new_node();
    left[i] = r;
    return r;
}/*}}}*/
uint16 ZPNumContext::get_right(uint16 i)/*{{{*/
{
    assert(i < n);
    uint16 r = right[i];
    if (r) return r;
    r = new_node();
    right[i] = r;
    return r;
}/*}}}*/
uint16 ZPNumContext::new_node()/*{{{*/
{
    if (n == allocated)
    {
        allocated <<= 1;
        nodes = (ZPBitContext *) realloc(nodes, allocated * sizeof(*nodes));
        left  = (uint16 *)       realloc(left , allocated * sizeof(*left));
        right = (uint16 *)       realloc(right, allocated * sizeof(*right));
    }
    nodes[n].value = 0;
    left[n] = 0;
    right[n] = 0;
    if (watcher)
        watcher->handle_allocation();
    return n++;
}/*}}}*/
void ZPNumContext::reset()/*{{{*/
{
    allocated = numcontext_first_allocation_size;
    nodes = (ZPBitContext *) realloc(nodes, allocated * sizeof(ZPBitContext));
    left  = (uint16 *) realloc(left, allocated * sizeof(uint16));
    right = (uint16 *) realloc(right, allocated * sizeof(uint16));
    init();
}/*}}}*/
void ZPNumContext::set_interval(int32 new_min, int32 new_max)/*{{{*/
{
    assert(new_min <= new_max);
    min = new_min;
    max = new_max;
}/*}}}*/

// NumContext }}}

// Table {{{

static signed char ZP_FFZ_table[256];

// The following tables are taken from DjVuLibre.

static uint16 ZP_p_table[256] = {
0x8000,0x8000,0x8000,0x6bbd,0x6bbd,0x5d45,0x5d45,0x51b9,0x51b9,0x4813,0x4813,
0x3fd5,0x3fd5,0x38b1,0x38b1,0x3275,0x3275,0x2cfd,0x2cfd,0x2825,0x2825,0x23ab,
0x23ab,0x1f87,0x1f87,0x1bbb,0x1bbb,0x1845,0x1845,0x1523,0x1523,0x1253,0x1253,
0x0fcf,0x0fcf,0x0d95,0x0d95,0x0b9d,0x0b9d,0x09e3,0x09e3,0x0861,0x0861,0x0711,
0x0711,0x05f1,0x05f1,0x04f9,0x04f9,0x0425,0x0425,0x0371,0x0371,0x02d9,0x02d9,
0x0259,0x0259,0x01ed,0x01ed,0x0193,0x0193,0x0149,0x0149,0x010b,0x010b,0x00d5,
0x00d5,0x00a5,0x00a5,0x007b,0x007b,0x0057,0x0057,0x003b,0x003b,0x0023,0x0023,
0x0013,0x0013,0x0007,0x0007,0x0001,0x0001,0x5695,0x24ee,0x8000,0x0d30,0x481a,
0x0481,0x3579,0x017a,0x24ef,0x007b,0x1978,0x0028,0x10ca,0x000d,0x0b5d,0x0034,
0x078a,0x00a0,0x050f,0x0117,0x0358,0x01ea,0x0234,0x0144,0x0173,0x0234,0x00f5,
0x0353,0x00a1,0x05c5,0x011a,0x03cf,0x01aa,0x0285,0x0286,0x01ab,0x03d3,0x011a,
0x05c5,0x00ba,0x08ad,0x007a,0x0ccc,0x01eb,0x1302,0x02e6,0x1b81,0x045e,0x24ef,
0x0690,0x2865,0x09de,0x3987,0x0dc8,0x2c99,0x10ca,0x3b5f,0x0b5d,0x5695,0x078a,
0x8000,0x050f,0x24ee,0x0358,0x0d30,0x0234,0x0481,0x0173,0x017a,0x00f5,0x007b,
0x00a1,0x0028,0x011a,0x000d,0x01aa,0x0034,0x0286,0x00a0,0x03d3,0x0117,0x05c5,
0x01ea,0x08ad,0x0144,0x0ccc,0x0234,0x1302,0x0353,0x1b81,0x05c5,0x24ef,0x03cf,
0x2b74,0x0285,0x201d,0x01ab,0x1715,0x011a,0x0fb7,0x00ba,0x0a67,0x01eb,0x06e7,
0x02e6,0x0496,0x045e,0x030d,0x0690,0x0206,0x09de,0x0155,0x0dc8,0x00e1,0x2b74,
0x0094,0x201d,0x0188,0x1715,0x0252,0x0fb7,0x0383,0x0a67,0x0547,0x06e7,0x07e2,
0x0496,0x0bc0,0x030d,0x1178,0x0206,0x19da,0x0155,0x24ef,0x00e1,0x320e,0x0094,
0x432a,0x0188,0x447d,0x0252,0x5ece,0x0383,0x8000,0x0547,0x481a,0x07e2,0x3579,
0x0bc0,0x24ef,0x1178,0x1978,0x19da,0x2865,0x24ef,0x3987,0x320e,0x2c99,0x432a,
0x3b5f,0x447d,0x5695,0x5ece,0x8000,0x8000,0x5695,0x481a,0x481a};

static uint16 ZP_m_table[256] = {
0x0000,0x0000,0x0000,0x10a5,0x10a5,0x1f28,0x1f28,0x2bd3,0x2bd3,0x36e3,0x36e3,
0x408c,0x408c,0x48fd,0x48fd,0x505d,0x505d,0x56d0,0x56d0,0x5c71,0x5c71,0x615b,
0x615b,0x65a5,0x65a5,0x6962,0x6962,0x6ca2,0x6ca2,0x6f74,0x6f74,0x71e6,0x71e6,
0x7404,0x7404,0x75d6,0x75d6,0x7768,0x7768,0x78c2,0x78c2,0x79ea,0x79ea,0x7ae7,
0x7ae7,0x7bbe,0x7bbe,0x7c75,0x7c75,0x7d0f,0x7d0f,0x7d91,0x7d91,0x7dfe,0x7dfe,
0x7e5a,0x7e5a,0x7ea6,0x7ea6,0x7ee6,0x7ee6,0x7f1a,0x7f1a,0x7f45,0x7f45,0x7f6b,
0x7f6b,0x7f8d,0x7f8d,0x7faa,0x7faa,0x7fc3,0x7fc3,0x7fd7,0x7fd7,0x7fe7,0x7fe7,
0x7ff2,0x7ff2,0x7ffa,0x7ffa,0x7fff,0x7fff,0xFFFF}; // last value is a signal

static unsigned char ZP_up_table[256] = {
84,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,
22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,
74,75,76,77,78,79,80,81,82,81,82,9,86,5,88,89,90,91,92,93,94,95,96,97,82,99,76,
101,70,103,66,105,106,107,66,109,60,111,56,69,114,65,116,61,118,57,120,53,122,
49,124,43,72,39,60,33,56,29,52,23,48,23,42,137,38,21,140,15,142,9,144,141,146,
147,148,149,150,151,152,153,154,155,70,157,66,81,62,75,58,69,54,65,50,167,44,
65,40,59,34,55,30,175,24,177,178,179,180,181,182,183,184,69,186,59,188,55,190,
51,192,47,194,41,196,37,198,199,72,201,62,203,58,205,54,207,50,209,46,211,40,
213,36,215,30,217,26,219,20,71,14,61,14,57,8,53,228,49,230,45,232,39,234,35,
138,29,24,25,240,19,22,13,16,13,10,7,244,249,10,89,230};

static unsigned char ZP_dn_table[256] = {
145,4,3,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,
19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,
71,72,73,74,75,76,77,78,79,80,85,226,6,176,143,138,141,112,135,104,133,100,129,
98,127,72,125,102,123,60,121,110,119,108,117,54,115,48,113,134,59,132,55,130,
51,128,47,126,41,62,37,66,31,54,25,50,131,46,17,40,15,136,7,32,139,172,9,170,
85,168,248,166,247,164,197,162,95,160,173,158,165,156,161,60,159,56,71,52,163,
48,59,42,171,38,169,32,53,26,47,174,193,18,191,222,189,218,187,216,185,214,61,
212,53,210,49,208,45,206,39,204,195,202,31,200,243,64,239,56,237,52,235,48,233,
44,231,38,229,34,227,28,225,22,223,16,221,220,63,8,55,224,51,2,47,87,43,246,37,
244,33,238,27,236,21,16,15,8,241,242,7,10,245,2,1,83,250,2,143,246};

static void init_ffz_table()/*{{{*/
{
    for (int i = 0; i < 256; i++)
    {
        ZP_FFZ_table[i] = 0;
        for (int j = i; j & 0x80; j <<= 1)
            ZP_FFZ_table[i] += 1;
    }
}/*}}}*/
static void init_tables()/*{{{*/
{
    for (int i = 0; i < 256; i++)
    {
        if (ZP_m_table[i] == 0xFFFF)
        {
            for (int j = i; j < 256; j++) ZP_m_table[j] = 0;
            break;
        }
    }
}/*}}}*/

static void init()/*{{{*/
{
    init_ffz_table();
    init_tables();
}/*}}}*/

// call function init() on startup
static struct Initer {Initer(){init();}} initer;

// Table }}}

// Encoder {{{

inline void ZPEncoder::emit_byte(unsigned char b)/*{{{*/
{
    fputc(b, file);
}/*}}}*/
ZPEncoder::ZPEncoder(FILE *f)/*{{{*/
    : file(f), a(0), nrun(0), subend(0), buffer(0xffffff),
      delay(25), byte(0), scount(0), closed(false)
{
    assert(f);
}/*}}}*/
void ZPEncoder::close()/*{{{*/
{
    /* adjust subend */
    if (subend > 0x8000)
        subend = 0x10000;
    else if (subend > 0)
        subend = 0x8000;

    /* zemit many mps bits */
    while (buffer != 0xffffff  || subend )
    {
        zemit(1 - (subend >> 15) );
        subend = (unsigned short)(subend<<1);
    }

    /* zemit pending run */
    outbit(1);
    while (nrun-- > 0)
        outbit(0);
    nrun = 0;

    /* zemit 1 until full byte */
    while (scount > 0)
        outbit(1);

    /* prevent further emission */
    delay = 0xff;

    closed = true;
}/*}}}*/
ZPEncoder::~ZPEncoder()/*{{{*/
{
    if (!closed) close();
}/*}}}*/
void ZPEncoder::outbit(Bit bit)/*{{{*/
{
    if (delay > 0)
    {
        if (delay < 0xff) // delay=0xff suspends emission forever
            delay -= 1;
    }
    else
    {
        /* Insert a bit */
        byte = (byte<<1) | bit;

        /* Output a byte */
        if (++scount == 8)
        {
            emit_byte(byte);
            scount = 0;
            byte = 0;
        }
    }
}/*}}}*/
void ZPEncoder::zemit(Bit b)/*{{{*/
{
    /* Shift new bit into 3bytes buffer */
    buffer = (buffer << 1) + b;

    /* Examine bit going out of the 3bytes buffer */
    b = buffer >> 24;
    buffer = buffer & 0xffffff;

    /* The following lines have been changed in order to emphazise the
     * similarity between this bit counting and the scheme of Witten, Neal & Cleary
     * (WN&C).  Corresponding changes have been made in outbit and eflush.
     * Variable 'nrun' is similar to the 'bits_to_follow' in the W&N code.
     */
    switch(b)
    {
        /* Similar to WN&C upper renormalization */
        case 1:
            outbit(1);
            while (nrun-- > 0)
                outbit(0);
            nrun = 0;
        break;

        /* Similar to WN&C lower renormalization */
        case 0xff:
            outbit(0);
            while (nrun-- > 0)
                outbit(1);
            nrun = 0;
        break;

        /* Similar to WN&C central renormalization */
        case 0:
            nrun += 1;
        break;

        default:
            assert(0);
    }
}/*}}}*/

inline void ZPEncoder::export_bits()/*{{{*/
{
    zemit(1 - (subend >> 15));
    subend = (unsigned short) (subend << 1);
    a = (unsigned short) (a << 1);
}/*}}}*/
void ZPEncoder::encode_mps(ZPBitContext &context, uint32 z)/*{{{*/
{
    /* Adaptation */
    if (a >= ZP_m_table[context.value])
        context.value = ZP_up_table[context.value];

    /* Code MPS */
    a = z;

    if (a >= 0x8000) export_bits();
}/*}}}*/
void ZPEncoder::encode_lps(ZPBitContext &context, uint32 z)/*{{{*/
{
    /* Adaptation */
    context.value = ZP_dn_table[context.value];

    /* Code LPS */
    z = 0x10000 - z;
    subend += z;
    a += z;

    while (a >= 0x8000) export_bits();
}/*}}}*/
void ZPEncoder::encode_mps_simple(unsigned int z)
{
    /* Code MPS */
    a = z;
    /* Export bits */
    if (a >= 0x8000) export_bits();
}

void ZPEncoder::encode_lps_simple(unsigned int z)
{
    /* Code LPS */
    z = 0x10000 - z;
    subend += z;
    a += z;
    
    /* Export bits */
    while (a >= 0x8000) export_bits();
}
void ZPEncoder::encode(int32 n, ZPNumContext &context)/*{{{*/
{
    bool negative =false;
    int32 cutoff = 0;
    uint32 range = 0xFFFFFFFF;
    uint16 current_node = 0;
    int phase = 1;
    int32 low = context.min;
    int32 high = context.max;
    assert(low <= n && n <= high);

    while(range != 1)
    {
        // encoding-specific
        bool decision;
        decision = n >= cutoff;
        if (low < cutoff && cutoff <= high)
            encode(decision, context.nodes[current_node]);

        // context for new bit
        current_node = decision
            ? context.get_right(current_node)
            : context.get_left (current_node);

        // phase dependent part
        switch (phase)
        {
	    case 1:
                negative = !decision;
                if (negative)
                {
                    // encoding-specific
                    n = - n - 1;

                    const int temp = - low - 1;
                    low = - high - 1;
                    high = temp;
	        }
                phase = 2; cutoff = 1;
            break;

	    case 2:
                if (!decision)
                {
                    phase = 3;
                    range = (cutoff + 1) / 2;
                    if (range == 1)
                        cutoff = 0;
                    else
                        cutoff -= range / 2;
	        }
                else
                {
                    cutoff += cutoff + 1;
                }
            break;

	    case 3:
                range /= 2;
                if (range != 1)
                {
                    if (!decision)
                        cutoff -= range / 2;
                    else
                        cutoff += range / 2;
	        }
                else if (!decision)
                {
                    cutoff--;
	        }
            break;
	}
    }
}/*}}}*/
void ZPEncoder::encode_without_context(Bit bit)/*{{{*/
{
    assert(bit == 0 || bit == 1);
    uint32 z = 0x8000 + (a >> 1);
    if (bit)
        encode_lps_simple(z);
    else
        encode_mps_simple(z);
}/*}}}*/
void ZPEncoder::encode(Bit bit, ZPBitContext &context) /*{{{*/
{
    uint32 z = a + ZP_p_table[context.value];

    assert(bit == 0 || bit == 1);
    if (bit != (context.value & 1))
    {
        /* Avoid interval reversion */
        uint32 d = 0x6000 + ((z + a) >> 2);
        if (z > d) z = d;
        encode_lps(context, z);
    }
    else if (z >= 0x8000)
    {
        /* Avoid interval reversion */
        uint32 d = 0x6000 + ((z + a) >> 2);
        if (z > d) z = d;
        encode_mps(context, z);
    }
    else
        a = z;
}/*}}}*/

// Encoder }}}

// Decoder {{{

inline bool ZPDecoder::next_byte(unsigned char &b)/*{{{*/
{
    if (!bytes_left) return false;
    int c = fgetc(file);
    if (c == EOF) return false;
    bytes_left--;
    b = c;
    return true;
}/*}}}*/
ZPDecoder::ZPDecoder(FILE *f, int32 len)/*{{{*/
    : file(f), a(0), fence(0), bytes_left(len)
{
    open();
}/*}}}*/
void ZPDecoder::open()/*{{{*/
{
    /* Read first 16 bits of code */
    if (!next_byte(byte))
        byte = 0xff;
    code = byte << 8;
    if (!next_byte(byte))
        byte = 0xff;
    code = code | byte;

    /* Preload buffer */
    delay = 25;
    scount = 0;
    preload();

    /* Compute initial fence */
    fence = code;
    if (code >= 0x8000)
        fence = 0x7fff;
}/*}}}*/
void ZPDecoder::preload(void)/*{{{*/
{
    while (scount<=24)
    {
        if (!next_byte(byte))
        {
            byte = 0xff;
            delay--;
            assert(delay);
        }
        buffer = (buffer<<8) | byte;
        scount += 8;
    }
}/*}}}*/
inline int32 ZPDecoder::ffz(uint32 x)/*{{{*/
{
  return x >= 0xff00
      ? ZP_FFZ_table[x & 0xff] + 8
      : ZP_FFZ_table[(x >> 8) & 0xff];
}/*}}}*/
Bit ZPDecoder::decode_sub(ZPBitContext &context, uint32 z)/*{{{*/
{
    /* Save bit */
    int bit = context.value & 1;

    /* Test MPS/LPS */
    if (z > code)
    {
        /* LPS branch */
        z = 0x10000 - z;
        a += z;
        code = code + z;

        /* LPS adaptation */
        context.value = ZP_dn_table[context.value];

        /* LPS renormalization */
        int shift = ffz(a);
        scount -= shift;
        a = (unsigned short) (a << shift);
        code = (unsigned short) (code << shift) |
                ((buffer >> scount) & ((1 << shift) - 1));
        if (scount<16) preload();

        /* Adjust fence */
        fence = code;
        if (code >= 0x8000)
        fence = 0x7fff;
        return bit ^ 1;
    }
    else
    {
        /* MPS adaptation */
        if (a >= ZP_m_table[context.value])
            context.value = ZP_up_table[context.value];

        /* MPS renormalization */
        scount -= 1;
        a = (unsigned short)(z<<1);
        code = (unsigned short)(code<<1) | ((buffer>>scount) & 1);
        if (scount<16) preload();

        /* Adjust fence */
        fence = code;
        if (code >= 0x8000)
            fence = 0x7fff;
        return bit;
    }
}/*}}}*/
int32 ZPDecoder::decode(ZPNumContext &context)/*{{{*/
{
    bool negative=false;
    int32 cutoff = 0;
    uint32 range = 0xFFFFFFFF;
    uint16 current_node = 0;
    int phase = 1;
    int32 low = context.min;
    int32 high = context.max;

    while(range != 1)
    {
        // decoding-specific
        bool decision;
        decision = low >= cutoff ||
                   (high >= cutoff && decode(context.nodes[current_node]));

        // context for new bit
        current_node = decision
            ? context.get_right(current_node)
            : context.get_left (current_node);

        // phase dependent part
        switch (phase)
        {
	    case 1:
                negative = !decision;
                if (negative)
                {
                    const int temp = - low - 1;
                    low = - high - 1;
                    high = temp;
	        }
                phase = 2; cutoff = 1;
            break;

	    case 2:
                if (!decision)
                {
                    phase = 3;
                    range = (cutoff + 1) / 2;
                    if (range == 1)
                        cutoff = 0;
                    else
                        cutoff -= range / 2;
	        }
                else
                {
                    cutoff += cutoff + 1;
                }
            break;

	    case 3:
                range /= 2;
                if (range != 1)
                {
                    if (!decision)
                        cutoff -= range / 2;
                    else
                        cutoff += range / 2;
	        }
                else if (!decision)
                {
                    cutoff--;
	        }
            break;
	}
    }
    return negative ? -cutoff-1 : cutoff;
}/*}}}*/
Bit ZPDecoder::decode_without_context()/*{{{*/
{
    ZPBitContext dummy;
    return decode_sub(dummy, 0x8000 + (a >> 1));
}/*}}}*/
Bit ZPDecoder::decode(ZPBitContext &context)/*{{{*/
{
    uint32 z = a + ZP_p_table[context.value];
    if (z <= fence)
    {
        a = z;
        return context.value & 1;
    }

    /* Avoid interval reversion */
    uint32 d = 0x6000 + ((z + a) >> 2);
    if (z > d) z = d;

    return decode_sub(context, z);
}/*}}}*/

// Decoder }}}
