/*
 * zp.h - ZP-coder from DjVuLibre, an adaptive arithmetical binary coder
 */

/* The DjVuLibre code is a bit reorganized, but the algorithms are the same.
 * Z-coder was stripped.
 */


#ifndef ZP_H
#define ZP_H


#include <minidjvu-mod/minidjvu-mod.h>
#include <stdio.h>
#include <assert.h>

typedef int Bit;


/* ZPBitContext is an adaptation variable.
 * A ZP-coder works together with a context,
 *    and both are changed in the process,
 *    but the context is detachable.
 */

class ZPBitContext
{
    public:
        ZPBitContext();
    private:
        unsigned char value;

    friend class ZPNumContext;
    friend class ZPEncoder;
    friend class ZPDecoder;

    public:
        inline Bit get_more_probable_bit() {return value & 1;}
        inline void assign(ZPBitContext &c) {value = c.value;}
};

inline ZPBitContext::ZPBitContext(): value(0) {}


/* ZPMemoryWatcher is a purely abstract class.
 * Its only goal is to receive notifications
 *     when a ZPNumContext has allocated a bit context.
 * This unit contains no implementation of ZPMemoryWatcher.
 */
class ZPMemoryWatcher
{
    public:
        virtual void handle_allocation() = 0;
    virtual ~ZPMemoryWatcher();
};


/* ZPNumContext is a tree of ZPBitContexts.
 * Its nodes are accessed by indices.
 * Root node always has the index of 0.
 *
 * Nodes are created automatically.
 */
class ZPNumContext
{
    public:
        ZPNumContext(int32 amin, int32 amax, ZPMemoryWatcher *w = NULL);
        ~ZPNumContext();
        void set_interval(int32 new_min, int32 new_max);
        void reset();
    private:
        int32 min, max;
        ZPMemoryWatcher *watcher;
        ZPBitContext *nodes;
        uint16 n;
        uint16 allocated;
        uint16 *left, *right; // 0 means nil
        uint16 new_node();
        uint16 get_left(uint16);
        uint16 get_right(uint16);
        void init();
    friend class ZPEncoder;
    friend class ZPDecoder;
};


class ZPEncoder
{
    public:
        ZPEncoder(FILE *); // does not close it on destruction
        virtual ~ZPEncoder();
        void encode_without_context(Bit);
        void encode(Bit, ZPBitContext &);
        void encode(int32, ZPNumContext &);
        void close();

    private:
        FILE *file;
        void emit_byte(unsigned char);
        uint32 a, nrun, subend, buffer;
        unsigned char delay, byte, scount;
        bool closed;

        void outbit(Bit);
        void zemit(Bit);
        void export_bits();
        void encode_mps(ZPBitContext &, uint32);
        void encode_lps(ZPBitContext &, uint32);
        void encode_mps_simple(uint32);
        void encode_lps_simple(uint32);
};


class ZPDecoder
{
    public:
        ZPDecoder(FILE *, int32 length); // does not close it on destruction
        Bit decode_without_context();
        Bit decode(ZPBitContext &);
        int32 decode(ZPNumContext &);
    private:
        FILE *file;
        uint32 a, code, fence, buffer;
        int32 bytes_left;
        unsigned char byte, scount, delay;
        bool next_byte(unsigned char &);
        void open();
        void preload();
        int32 ffz(uint32);
        Bit decode_sub(ZPBitContext &, uint32);
        Bit decode_sub_simple(uint32);
};


#endif
