#ifdef PLATFORM_PC
#include "ppu.h"
#include "gba_mem.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// ─── GBA IO register offsets ────────────────────────────────────────────────

#define IO_DISPCNT   0x000
#define IO_DISPSTAT  0x004
#define IO_BG0CNT    0x008
#define IO_BG1CNT    0x00A
#define IO_BG2CNT    0x00C
#define IO_BG3CNT    0x00E
#define IO_BG0HOFS   0x010
#define IO_BG0VOFS   0x012
#define IO_BG1HOFS   0x014
#define IO_BG1VOFS   0x016
#define IO_BG2HOFS   0x018
#define IO_BG2VOFS   0x01A
#define IO_BG3HOFS   0x01C
#define IO_BG3VOFS   0x01E
#define IO_BG2PA     0x020
#define IO_BG2PB     0x022
#define IO_BG2PC     0x024
#define IO_BG2PD     0x026
#define IO_BG2X      0x028
#define IO_BG2Y      0x02C
#define IO_BG3PA     0x030
#define IO_BG3PB     0x032
#define IO_BG3PC     0x034
#define IO_BG3PD     0x036
#define IO_BG3X      0x038
#define IO_BG3Y      0x03C
#define IO_WIN0H     0x040
#define IO_WIN0V     0x044
#define IO_WIN1H     0x042
#define IO_WIN1V     0x046
#define IO_WININ     0x048
#define IO_WINOUT    0x04A
#define IO_BLDMOD    0x050
#define IO_COLEV     0x052
#define IO_COLEY     0x054

#define IOREG16(off) (*(volatile uint16_t *)(gba_io + (off)))
#define IOREG32(off) (*(volatile uint32_t *)(gba_io + (off)))

// ─── colour conversion ──────────────────────────────────────────────────────

// GBA palette entries are 15-bit BGR555: xBBBBBGGGGGRRRRR
static inline uint32_t bgr555_to_rgba8888(uint16_t c)
{
    uint8_t r = (c & 0x1F) << 3;
    uint8_t g = ((c >> 5) & 0x1F) << 3;
    uint8_t b = ((c >> 10) & 0x1F) << 3;
    // propagate the low 3 bits so 0x1F → 0xFF
    r |= r >> 5;
    g |= g >> 5;
    b |= b >> 5;
    return (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | 0xFF000000u;
}

// ─── framebuffer ────────────────────────────────────────────────────────────

// Per-pixel output: RGBA8888
static uint32_t s_fb[GBA_W * GBA_H];

int gPcFrameNo = 0;   // frame counter, also used by the scene trace

// Per-pixel priority + alpha tracking (lower priority number wins)
// High nibble = priority of BG/OBJ that drew this pixel (0-3, 4=none yet)
// Low bit     = 1 if a non-transparent OBJ was drawn here
static uint8_t s_prio[GBA_W * GBA_H];

static SDL_Texture *s_tex = NULL;

// ─── palette helpers ─────────────────────────────────────────────────────────

static inline uint16_t bg_palette16(int pal, int idx)
{
    return *(uint16_t *)(gba_palette + (pal * 16 + idx) * 2);
}
static inline uint16_t bg_palette256(int idx)
{
    return *(uint16_t *)(gba_palette + idx * 2);
}
static inline uint16_t obj_palette16(int pal, int idx)
{
    return *(uint16_t *)(gba_palette + 0x200 + (pal * 16 + idx) * 2);
}
static inline uint16_t obj_palette256(int idx)
{
    return *(uint16_t *)(gba_palette + 0x200 + idx * 2);
}

// ─── tile helpers ───────────────────────────────────────────────────────────

// Return one 4-bit palette index from a 4bpp tile row.
// tilebase = address in gba_vram, row 0-7, col 0-7
static inline int tile4bpp_pixel(int tilebase, int row, int col)
{
    uint8_t byte = gba_vram[tilebase + row * 4 + col / 2];
    return (col & 1) ? (byte >> 4) : (byte & 0xF);
}

// Return one 8-bit palette index from an 8bpp tile row.
static inline int tile8bpp_pixel(int tilebase, int row, int col)
{
    return gba_vram[tilebase + row * 8 + col];
}

// ─── text BG rendering ──────────────────────────────────────────────────────

// BGCNT bit fields
#define BGCNT_PRIO(v)       ((v) & 3)
#define BGCNT_TILEDATA(v)   (((v) >> 2) & 3)
#define BGCNT_PALETTE256(v) (!!((v) & (1 << 7)))
#define BGCNT_MAPBASE(v)    (((v) >> 8) & 0x1F)
#define BGCNT_OVERFLOW(v)   (!!((v) & (1 << 13)))
#define BGCNT_MAPSIZE(v)    (((v) >> 14) & 3)

// Text BG map entry bit fields
#define MAP_TILEID(e)   ((e) & 0x3FF)
#define MAP_HFLIP(e)    (!!((e) & (1 << 10)))
#define MAP_VFLIP(e)    (!!((e) & (1 << 11)))
#define MAP_PAL(e)      (((e) >> 12) & 0xF)

static void render_text_bg(int bgn, int prio)
{
    // IO offsets for this BG
    static const int cnt_off[4]  = {IO_BG0CNT, IO_BG1CNT, IO_BG2CNT, IO_BG3CNT};
    static const int hofs_off[4] = {IO_BG0HOFS, IO_BG1HOFS, IO_BG2HOFS, IO_BG3HOFS};
    static const int vofs_off[4] = {IO_BG0VOFS, IO_BG1VOFS, IO_BG2VOFS, IO_BG3VOFS};

    uint16_t bgcnt = IOREG16(cnt_off[bgn]);
    if (BGCNT_PRIO(bgcnt) != prio) return;

    int hofs      = IOREG16(hofs_off[bgn]) & 0x1FF;
    int vofs      = IOREG16(vofs_off[bgn]) & 0x1FF;
    int tiledata  = BGCNT_TILEDATA(bgcnt) * 0x4000;
    int mapbase   = BGCNT_MAPBASE(bgcnt) * 0x800;
    int is256     = BGCNT_PALETTE256(bgcnt);
    int mapsize   = BGCNT_MAPSIZE(bgcnt);

    // Map dimensions in 8-pixel tiles (text BG)
    // mapsize: 0=32×32, 1=64×32, 2=32×64, 3=64×64
    int map_w = (mapsize & 1) ? 64 : 32;
    int map_h = (mapsize & 2) ? 64 : 32;

    for (int y = 0; y < GBA_H; y++) {
        int ty = (y + vofs) & (map_h * 8 - 1);
        int tile_row_in_map = ty / 8;
        int tile_py = ty % 8;

        for (int x = 0; x < GBA_W; x++) {
            int px = s_prio[y * GBA_W + x];
            // Already covered by a higher- or equal-priority layer
            if ((px >> 4) <= prio) continue;

            int tx = (x + hofs) & (map_w * 8 - 1);
            int tile_col_in_map = tx / 8;
            int tile_px = tx % 8;

            // Map entry: pick the right 32×32 screen based on position
            int screen = 0;
            if (mapsize == 1 || mapsize == 3) screen |= (tile_col_in_map >= 32) ? 1 : 0;
            if (mapsize == 2 || mapsize == 3) screen |= (tile_row_in_map >= 32) ? 2 : 0;

            int local_col = tile_col_in_map & 31;
            int local_row = tile_row_in_map & 31;
            int map_off   = mapbase + screen * 0x800 + (local_row * 32 + local_col) * 2;

            uint16_t entry = *(uint16_t *)(gba_vram + map_off);
            int tileid = MAP_TILEID(entry);
            int hflip  = MAP_HFLIP(entry);
            int vflip  = MAP_VFLIP(entry);
            int pal    = MAP_PAL(entry);

            int px_in_tile = hflip ? (7 - tile_px) : tile_px;
            int py_in_tile = vflip ? (7 - tile_py) : tile_py;

            int colidx;
            if (is256) {
                int tbase = tiledata + tileid * 64;
                colidx = tile8bpp_pixel(tbase, py_in_tile, px_in_tile);
                if (colidx == 0) continue; // transparent
                uint16_t colour = bg_palette256(colidx);
                s_fb[y * GBA_W + x]   = bgr555_to_rgba8888(colour);
                s_prio[y * GBA_W + x] = (prio << 4);
            } else {
                int tbase = tiledata + tileid * 32;
                colidx = tile4bpp_pixel(tbase, py_in_tile, px_in_tile);
                if (colidx == 0) continue; // transparent
                uint16_t colour = bg_palette16(pal, colidx);
                s_fb[y * GBA_W + x]   = bgr555_to_rgba8888(colour);
                s_prio[y * GBA_W + x] = (prio << 4);
            }
        }
    }
}

// ─── affine/rotscale BG rendering ───────────────────────────────────────────

static void render_affine_bg(int bgn, int prio)
{
    static const int cnt_off[4]  = {IO_BG0CNT, IO_BG1CNT, IO_BG2CNT, IO_BG3CNT};
    uint16_t bgcnt = IOREG16(cnt_off[bgn]);
    if (BGCNT_PRIO(bgcnt) != prio) return;

    // Affine BG always uses 8bpp and 256-colour palette
    int tiledata = BGCNT_TILEDATA(bgcnt) * 0x4000;
    int mapbase  = BGCNT_MAPBASE(bgcnt) * 0x800;
    int overflow = BGCNT_OVERFLOW(bgcnt);
    // Map size: 0=128×128, 1=256×256, 2=512×512, 3=1024×1024 px
    int map_size_px = 128 << BGCNT_MAPSIZE(bgcnt);
    int map_tiles   = map_size_px / 8;

    // Affine parameters for BG2/BG3
    int pa_off, pb_off, pc_off, pd_off, px_off, py_off;
    if (bgn == 2) {
        pa_off = IO_BG2PA; pb_off = IO_BG2PB;
        pc_off = IO_BG2PC; pd_off = IO_BG2PD;
        px_off = IO_BG2X;  py_off = IO_BG2Y;
    } else {
        pa_off = IO_BG3PA; pb_off = IO_BG3PB;
        pc_off = IO_BG3PC; pd_off = IO_BG3PD;
        px_off = IO_BG3X;  py_off = IO_BG3Y;
    }

    int16_t pa = (int16_t)IOREG16(pa_off);
    int16_t pb = (int16_t)IOREG16(pb_off);
    int16_t pc = (int16_t)IOREG16(pc_off);
    int16_t pd = (int16_t)IOREG16(pd_off);
    int32_t ref_x = (int32_t)IOREG32(px_off);
    int32_t ref_y = (int32_t)IOREG32(py_off);
    // Sign-extend 28-bit reference point
    if (ref_x & 0x08000000) ref_x |= 0xF0000000;
    if (ref_y & 0x08000000) ref_y |= 0xF0000000;

    for (int y = 0; y < GBA_H; y++) {
        // Screen-space (cx,cy) → texture (tx,ty) via the affine matrix
        int32_t tx_fp = ref_x + pb * y;
        int32_t ty_fp = ref_y + pd * y;

        for (int x = 0; x < GBA_W; x++) {
            int px = s_prio[y * GBA_W + x];
            if ((px >> 4) <= prio) { tx_fp += pa; ty_fp += pc; continue; }

            // Convert fixed-point (8 frac bits) to tile coordinates
            int tx = tx_fp >> 8;
            int ty = ty_fp >> 8;

            if (overflow) {
                tx &= (map_size_px - 1);
                ty &= (map_size_px - 1);
            } else if (tx < 0 || tx >= map_size_px || ty < 0 || ty >= map_size_px) {
                tx_fp += pa; ty_fp += pc;
                continue;
            }

            int tile_col = tx / 8;
            int tile_row = ty / 8;
            int tileid   = gba_vram[mapbase + tile_row * map_tiles + tile_col];
            int px_in_tile = tx & 7;
            int py_in_tile = ty & 7;

            int tbase = tiledata + tileid * 64;
            int colidx = tile8bpp_pixel(tbase, py_in_tile, px_in_tile);
            if (colidx != 0) {
                uint16_t colour = bg_palette256(colidx);
                s_fb[y * GBA_W + x]   = bgr555_to_rgba8888(colour);
                s_prio[y * GBA_W + x] = (prio << 4);
            }
            tx_fp += pa;
            ty_fp += pc;
        }
    }
}

// ─── mode 3/4/5 bitmap rendering ────────────────────────────────────────────

static void render_bitmap_mode3(void)
{
    // 240×160 15-bit direct colour, single frame buffer
    for (int y = 0; y < GBA_H; y++) {
        for (int x = 0; x < GBA_W; x++) {
            uint16_t c = *(uint16_t *)(gba_vram + (y * GBA_W + x) * 2);
            s_fb[y * GBA_W + x] = bgr555_to_rgba8888(c);
        }
    }
}

static void render_bitmap_mode4(void)
{
    // 240×160 8-bit paletted, two page frames (page0=0x0, page1=0xA000)
    uint16_t dispcnt = IOREG16(IO_DISPCNT);
    int page_off = (dispcnt & (1 << 4)) ? 0xA000 : 0x0;
    for (int y = 0; y < GBA_H; y++) {
        for (int x = 0; x < GBA_W; x++) {
            uint8_t idx = gba_vram[page_off + y * GBA_W + x];
            uint16_t c  = bg_palette256(idx);
            s_fb[y * GBA_W + x] = bgr555_to_rgba8888(c);
        }
    }
}

// ─── OAM (sprite) rendering ─────────────────────────────────────────────────

// OAM entry: 3 × u16 attributes + 1 × u16 rotation field
struct OBJ_Attr {
    uint16_t attr0;
    uint16_t attr1;
    uint16_t attr2;
    uint16_t affine_param; // used in rotation groups, not per-sprite
};

// Sprite size lookup: [ATTR1_SIZE][ATTR0_SHAPE] → {w, h}
static const int sprite_dims[4][3][2] = {
    {{8,8},   {16,8},  {8,16}},
    {{16,16}, {32,8},  {8,32}},
    {{32,32}, {32,16}, {16,32}},
    {{64,64}, {64,32}, {32,64}},
};

static void render_sprites(int prio)
{
    uint16_t dispcnt = IOREG16(IO_DISPCNT);
    int obj_1d = !!(dispcnt & (1 << 6)); // 1D tile mapping

    // OBJ tile data starts at 0x10000 in VRAM (6K into the 96K VRAM)
    int tilebase = 0x10000;

    // Iterate sprites in reverse order (sprite 0 has highest priority)
    for (int s = 127; s >= 0; s--) {
        struct OBJ_Attr *obj = (struct OBJ_Attr *)(gba_oam + s * 8);
        uint16_t a0 = obj->attr0;
        uint16_t a1 = obj->attr1;
        uint16_t a2 = obj->attr2;

        // Object mode: 00=normal, 01=affine, 10=hidden, 11=affine double
        int obj_mode = (a0 >> 8) & 3;
        if (obj_mode == 2) continue; // hidden

        int shape = (a0 >> 14) & 3;
        int size  = (a1 >> 14) & 3;
        if (shape == 3) continue; // prohibited

        int sw = sprite_dims[size][shape][0];
        int sh = sprite_dims[size][shape][1];

        int y_screen = a0 & 0xFF;
        if (y_screen >= 160) y_screen -= 256; // sign
        int x_screen = a1 & 0x1FF;
        if (x_screen >= 240) x_screen -= 512;

        int is_8bpp   = !!(a0 & (1 << 13));
        int hflip     = (obj_mode == 0) && !!(a1 & (1 << 12));
        int vflip     = (obj_mode == 0) && !!(a1 & (1 << 13));
        int tileid    = a2 & 0x3FF;
        int obj_prio  = (a2 >> 10) & 3;
        int pal_bank  = (a2 >> 12) & 0xF;

        if (obj_prio != prio) continue;

        // Affine parameters
        int affine_idx = (a1 >> 9) & 0x1F;
        int16_t aff_pa = 0x100, aff_pb = 0, aff_pc = 0, aff_pd = 0x100;
        if (obj_mode == 1 || obj_mode == 3) {
            aff_pa = (int16_t)(*(uint16_t *)(gba_oam + affine_idx * 32 + 6));
            aff_pb = (int16_t)(*(uint16_t *)(gba_oam + affine_idx * 32 + 14));
            aff_pc = (int16_t)(*(uint16_t *)(gba_oam + affine_idx * 32 + 22));
            aff_pd = (int16_t)(*(uint16_t *)(gba_oam + affine_idx * 32 + 30));
        }

        // Render visible rows
        for (int py = 0; py < sh; py++) {
            int screen_y = y_screen + py;
            if (screen_y < 0 || screen_y >= GBA_H) continue;

            for (int px_s = 0; px_s < sw; px_s++) {
                int screen_x = x_screen + px_s;
                if (screen_x < 0 || screen_x >= GBA_W) continue;

                int ppx = hflip ? (sw - 1 - px_s) : px_s;
                int ppy = vflip ? (sh - 1 - py) : py;

                // Affine: transform (ppx - sw/2, ppy - sh/2) using matrix
                int src_px, src_py;
                if (obj_mode == 1 || obj_mode == 3) {
                    int dx = (px_s - sw / 2) * 256;
                    int dy = (py   - sh / 2) * 256;
                    src_px = (aff_pa * dx + aff_pb * dy) / 65536 + sw / 2;
                    src_py = (aff_pc * dx + aff_pd * dy) / 65536 + sh / 2;
                    if (src_px < 0 || src_px >= sw || src_py < 0 || src_py >= sh) continue;
                    ppx = src_px;
                    ppy = src_py;
                }

                // Map pixel position to tile and pixel within tile
                int tile_x = ppx / 8;
                int tile_y = ppy / 8;
                int pix_x  = ppx & 7;
                int pix_y  = ppy & 7;

                // Find which tile in VRAM
                int tid;
                if (is_8bpp) {
                    if (obj_1d) {
                        tid = tileid + tile_y * (sw / 8) * 2 + tile_x * 2;
                    } else {
                        tid = tileid + tile_y * 16 * 2 + tile_x * 2;
                    }
                    int taddr  = tilebase + tid * 32;
                    int colidx = tile8bpp_pixel(taddr, pix_y, pix_x);
                    if (colidx == 0) continue;
                    uint16_t colour = obj_palette256(colidx);
                    s_fb[screen_y * GBA_W + screen_x]   = bgr555_to_rgba8888(colour);
                    s_prio[screen_y * GBA_W + screen_x] = (obj_prio << 4) | 1;
                } else {
                    if (obj_1d) {
                        tid = tileid + tile_y * (sw / 8) + tile_x;
                    } else {
                        tid = tileid + tile_y * 32 + tile_x;
                    }
                    int taddr  = tilebase + tid * 32;
                    int colidx = tile4bpp_pixel(taddr, pix_y, pix_x);
                    if (colidx == 0) continue;
                    uint16_t colour = obj_palette16(pal_bank, colidx);
                    s_fb[screen_y * GBA_W + screen_x]   = bgr555_to_rgba8888(colour);
                    s_prio[screen_y * GBA_W + screen_x] = (obj_prio << 4) | 1;
                }
            }
        }
    }
}

// ─── public API ─────────────────────────────────────────────────────────────

int ppu_init(SDL_Renderer *renderer)
{
    // Framebuffer layout: r | (g<<8) | (b<<16) | (a<<24)
    // → byte order in memory: R, G, B, A  → SDL_PIXELFORMAT_ABGR8888
    s_tex = SDL_CreateTexture(renderer,
                              SDL_PIXELFORMAT_ABGR8888,
                              SDL_TEXTUREACCESS_STREAMING,
                              GBA_W, GBA_H);
    return s_tex ? 0 : -1;
}

void ppu_destroy(void)
{
    if (s_tex) { SDL_DestroyTexture(s_tex); s_tex = NULL; }
}

void ppu_render_frame(void)
{
    uint16_t dispcnt = IOREG16(IO_DISPCNT);
    int mode = dispcnt & 7;

    // Backdrop colour (palette entry 0)
    uint16_t backdrop_c = *(uint16_t *)gba_palette;
    uint32_t backdrop   = bgr555_to_rgba8888(backdrop_c);

    // Fill with backdrop and reset priority
    for (int i = 0; i < GBA_W * GBA_H; i++) {
        s_fb[i]   = backdrop;
        s_prio[i] = 0xFF; // no layer drawn yet
    }

    if (dispcnt & (1 << 7)) {
        // FORCE BLANK
        memset(s_fb, 0xFF, sizeof(s_fb));
        goto upload;
    }

    switch (mode) {
    case 0:
        // Text modes: render in priority order (3 lowest … 0 highest)
        for (int p = 3; p >= 0; p--) {
            if (dispcnt & (1 << 12)) render_sprites(p);  // bit12 = OBJ enable
            if (dispcnt & (1 <<  8)) render_text_bg(0, p);
            if (dispcnt & (1 <<  9)) render_text_bg(1, p);
            if (dispcnt & (1 << 10)) render_text_bg(2, p);
            if (dispcnt & (1 << 11)) render_text_bg(3, p);
        }
        break;
    case 1:
        for (int p = 3; p >= 0; p--) {
            if (dispcnt & (1 << 12)) render_sprites(p);
            if (dispcnt & (1 <<  8)) render_text_bg(0, p);
            if (dispcnt & (1 <<  9)) render_text_bg(1, p);
            if (dispcnt & (1 << 10)) render_affine_bg(2, p);
        }
        break;
    case 2:
        for (int p = 3; p >= 0; p--) {
            if (dispcnt & (1 << 12)) render_sprites(p);
            if (dispcnt & (1 << 10)) render_affine_bg(2, p);
            if (dispcnt & (1 << 11)) render_affine_bg(3, p);
        }
        break;
    case 3:
        render_bitmap_mode3();
        break;
    case 4:
        render_bitmap_mode4();
        break;
    default:
        break;
    }

upload:
    SDL_UpdateTexture(s_tex, NULL, s_fb, GBA_W * sizeof(uint32_t));

    // Auto-save frames at key points for diagnostics (build with RTPC_FRAMEDUMP=1)
#ifdef RTPC_FRAMEDUMP
    {
        static int s_frame = 0;
        // RTPC_SHOT_DIR redirects both kinds of dump.  Without it a scene sweep
        // has to run one scene at a time, because every process writes the same
        // /tmp/shot_NNNNN.bmp; with it each run gets its own directory and the
        // sweep parallelises.
        static const char *dir = NULL;
        if (dir == NULL) {
            const char *e = getenv("RTPC_SHOT_DIR");
            dir = (e && *e) ? e : "/tmp";
        }
        s_frame++;
        gPcFrameNo = s_frame;
        // RTPC_SHOTS=N : also save a frame every N frames.
        {
            static int every = -1;
            if (every < 0) { const char *e = getenv("RTPC_SHOTS"); every = e ? atoi(e) : 0; }
            if (every > 0 && (s_frame % every) == 0) {
                char p[512];
                snprintf(p, sizeof(p), "%s/shot_%05d.bmp", dir, s_frame);
                SDL_Surface *sf = SDL_CreateRGBSurfaceFrom(s_fb, GBA_W, GBA_H, 32, GBA_W*4,
                    0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
                if (sf) { SDL_SaveBMP(sf, p); SDL_FreeSurface(sf); }
            }
        }
        // Save at regular intervals
        static const int save_at[] = {120, 900, 1800, 2700, 3600, 4200, 4500, 5400, 6000, -1};
        for (int i = 0; save_at[i] >= 0; i++) {
            if (s_frame == save_at[i]) {
                char path[512];
                snprintf(path, sizeof(path), "%s/rtpc_f%04d.bmp", dir, s_frame);
                // s_fb pixels: byte0=R, byte1=G, byte2=B, byte3=A
                // BMP mask must match: R=low byte, B=high byte
                SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(
                    s_fb, GBA_W, GBA_H, 32, GBA_W * 4,
                    0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
                if (surf) { SDL_SaveBMP(surf, path); SDL_FreeSurface(surf); }
                fprintf(stderr, "[PPU] saved %s\n", path);
            }
        }
    }
#endif
}

void ppu_present(SDL_Renderer *renderer)
{
    SDL_RenderCopy(renderer, s_tex, NULL, NULL);
}

#endif // PLATFORM_PC
