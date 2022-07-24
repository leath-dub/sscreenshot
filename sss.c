// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
// XCB headers
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_cursor.h>
// Image headers
#include <png.h>

// Macros
#define DIE(MSG) internal_die(MSG) /* safer, clears resources */
#define EXIT(MSG) internal_exit(MSG)
#define LENGTH(X) (sizeof X / sizeof X[0])
#define MIN(A, B) A < B ? A : B
// Resource types
#define RES_CONNECTION 0
#define RES_GC         1
#define RES_COLORMAP   2
#define RES_PIXMAP     3
#define RES_CURSOR     4
// other symbols
#define ERROR 0
#define ALLPLANES ~0
#define TRUE 1
#define FALSE 0
#define BUTTON_1 1
#define BUTTON_2 2
#define BUTTON_3 3
#define BUTTON_4 4

// user defined configs
#include "config.h"

// global vars
static xcb_connection_t *conn;
static xcb_screen_t *screen;
static xcb_window_t win;
static xcb_gcontext_t dgc;
static xcb_gcontext_t pen;
static xcb_pixmap_t pm;
static xcb_generic_event_t *ev;
static uint32_t value_mask;
static uint32_t value_list[4];
static uint32_t allplanes = ~0;

// template for resource 32 bytes
typedef struct {
    uint32_t id;
    void *next;
    void *prev; // for back freeing of structs
    xcb_void_cookie_t (*free_func)(xcb_connection_t *, uint32_t);
    uint8_t type;
} _res;
_res *res_head;
_res *res_incr;

typedef struct {
    uint8_t *data;
    uint16_t width;
    uint16_t height;
    int len;
} _img;

typedef struct {
    const char *opt;
    uint8_t optid;
} _opt;

typedef struct {
    _opt head;

} _opt_f;

// function declarations
static void connect_init(void);
static void res_add(uint32_t res_id, int type);
static void res_clear(void);
static void internal_die(const char *msg);
static void internal_exit(const char *msg);
static void create_window(void);
static void event_loop(int charc, char *argv[]);
static void data_to_png(_img img, const char *fname);
static xcb_cursor_t load_cursor(const char *name);

void
connect_init(void)
{
    int sn;
    if (!(
        conn = xcb_connect(NULL, &sn)
    )) EXIT("failed to connect to xserver");
    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    res_head = malloc(sizeof(_res));
    res_incr = res_head;
    /* set some default resources */
    // default gc
    dgc = xcb_generate_id(conn);
    value_mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    value_list[0] = screen->black_pixel;
    value_list[1] = screen->white_pixel;
    xcb_create_gc(conn, dgc, screen->root, value_mask, value_list);
    res_add(dgc, RES_GC);
}

// in general resources have a function to free then of arg form
// (connection, resource_ptr);
void
res_add(uint32_t res_id, int type)
{
    _res *res = malloc(sizeof(_res));
    res->id = res_id;
    res->type = type;
    switch(res->type) {
        case RES_GC:
            res->free_func = xcb_free_gc;
            break;
        case RES_COLORMAP:
            res->free_func = xcb_free_colormap;
            break;
        case RES_PIXMAP:
            res->free_func = xcb_free_pixmap;
            break;
        case RES_CURSOR:
            res->free_func = xcb_free_cursor;
            break;
        default:
            DIE("Invalid RES type");
            break;
    }
    // set the current res next field to new res
    _res *prev = res_incr;
    res_incr->next = res;
    res_incr = res;
    res_incr->prev = prev;
    // set next to be null, top of linked list
    res->next = NULL;
}

void
res_clear(void)
{
    // we have res_head to start
    while (res_head = res_head->next) {
        free(res_head->prev);
        res_head->free_func(conn, res_head->id);
    }
    // no next, free current struct
    free(res_head);
    // stop the connection to x
    xcb_disconnect(conn);
    exit(0);
}

void
internal_die(const char *msg)
{
    fprintf(stderr, "pixcap: %s\n", msg);
    res_clear();
    exit(-1);
}

void
internal_exit(const char *msg)
{
    fprintf(stderr, "pixcap: %s\n", msg);
    exit(-1);
}

void
create_window(void)
{
    win = xcb_generate_id(conn);
    value_mask = XCB_CW_BACK_PIXEL        |
                 XCB_CW_BORDER_PIXEL      |
                 XCB_CW_OVERRIDE_REDIRECT |
                 XCB_CW_EVENT_MASK;
    value_list[0] = 0;
    value_list[1] = 0;
    value_list[2] = 1;
    value_list[3] = XCB_EVENT_MASK_BUTTON_PRESS    |
                    XCB_EVENT_MASK_BUTTON_1_MOTION |
                    XCB_EVENT_MASK_BUTTON_RELEASE;
    xcb_create_window(
        conn,
        screen->root_depth,
        win,
        screen->root,
        0, 0,
        screen->width_in_pixels, screen->height_in_pixels,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        value_mask, value_list
    );
}

void
event_loop(int charc, char *argv[])
{
    xcb_point_t start;
    xcb_point_t end;
    xcb_rectangle_t rect;
    xcb_rectangle_t prev;
    xcb_get_image_cookie_t cookie;
    xcb_get_image_reply_t *img_rep;
    _img ss;
    while(ev = xcb_wait_for_event(conn)) {
        if (ev->response_type == 0) {
            DIE("Error reported by x server");
        }
        switch(ev->response_type & ~0x80) {
            case XCB_BUTTON_PRESS: {
                xcb_button_press_event_t *btn;
                btn = (xcb_button_press_event_t *)ev;
                switch(btn->detail) {
                    case BUTTON_1: {
                        start.x = btn->event_x;
                        start.y = btn->event_y;
                        break;
                    }
                    case BUTTON_3:
                        // get the image reply
                        if (rect.width - LINE_WIDTH > 0 &&
                            rect.height - LINE_WIDTH > 0) {
                            img_rep = xcb_get_image_reply(
                                conn, cookie,
                                NULL
                            );
                            ss.data = xcb_get_image_data(img_rep);
                            if (charc >= 3 && !strcmp(argv[1], "-n")) {
                                data_to_png(ss, argv[2]);
                            } else data_to_png(ss, "screenshot.png");
                            free(img_rep);
                        }
                        res_clear();
                        break;
                }
            }
            case XCB_MOTION_NOTIFY: {
                xcb_motion_notify_event_t *mot;
                mot = (xcb_motion_notify_event_t *)ev;
                prev = rect;
                // top line
                xcb_clear_area(
                    conn, FALSE, win,
                    prev.x - LINE_WIDTH / 2, prev.y - LINE_WIDTH / 2,
                    prev.width + LINE_WIDTH, LINE_WIDTH
                );
                // left line
                xcb_clear_area(
                    conn, FALSE, win,
                    prev.x - LINE_WIDTH / 2, prev.y,
                    LINE_WIDTH, prev.height
                );
                // bottom line
                xcb_clear_area(
                    conn, FALSE, win,
                    prev.x - LINE_WIDTH / 2,
                    (prev.y - LINE_WIDTH / 2) + prev.height,
                    prev.width + LINE_WIDTH, LINE_WIDTH
                );
                // right line
                xcb_clear_area(
                    conn, FALSE, win,
                    (prev.x - LINE_WIDTH / 2) + prev.width,
                    prev.y - LINE_WIDTH / 2,
                    LINE_WIDTH, prev.height
                );

                rect = (xcb_rectangle_t){
                    MIN(mot->event_x, start.x), MIN(mot->event_y, start.y),
                    abs(mot->event_x - start.x), abs(mot->event_y - start.y)
                };
                xcb_poly_rectangle(conn, win, pen, 1, &rect);
                xcb_flush(conn);
                break;
            }
            case XCB_BUTTON_RELEASE:
                // req an image now, get reply before exit
                if (rect.width - LINE_WIDTH > 0 &&
                    rect.height - LINE_WIDTH > 0) {
                    cookie = xcb_get_image(
                        conn, XCB_IMAGE_FORMAT_Z_PIXMAP,
                        win, rect.x + LINE_WIDTH / 2, rect.y + LINE_WIDTH / 2,
                        rect.width - LINE_WIDTH, rect.height - LINE_WIDTH,
                        ALLPLANES
                    );
                    ss.width = rect.width - LINE_WIDTH;
                    ss.height = rect.height - LINE_WIDTH;
                }
                break;
        }
        free(ev);
    }
}

void
data_to_png(_img img, const char *fname)
{
    xcb_image_t *img_t;
    img_t = xcb_image_create_native(
        conn,
        img.width, img.height,
        XCB_IMAGE_FORMAT_Z_PIXMAP, screen->root_depth,
        NULL, 0, img.data
    );

    png_structp ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL
    );
    if (!ptr) DIE("NULL png_ptr");

    png_infop infop = png_create_info_struct(ptr);
    if (!infop) DIE("NULL infop");

    // Create a file
    char *path = malloc(sizeof(char) * strlen(SS_DIR) + strlen(fname));
    strcpy(path, SS_DIR);
    strcat(path, fname);
    FILE *fp = fopen(path, "wb");
    free(path);

    png_init_io(ptr, fp);
    png_set_IHDR(
        ptr, infop, img.width, img.height,
        8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    char *title = "ss";
    png_text ttext;
    ttext.compression = PNG_TEXT_COMPRESSION_NONE;
    ttext.key = "Title";
    ttext.text = title;
    png_set_text(ptr, infop, &ttext, 1);

    png_write_info(ptr, infop);

    png_bytep bytep = (png_bytep)malloc(3 * img.width * sizeof(png_byte));

    // Get root visual
    xcb_visualtype_t *vt = NULL;
    xcb_depth_iterator_t di;
    di = xcb_screen_allowed_depths_iterator (screen);
    for (; di.rem; xcb_depth_next(&di)) {
        xcb_visualtype_iterator_t vi;
        vi = xcb_depth_visuals_iterator (di.data);
        for (; vi.rem; xcb_visualtype_next (&vi)) {
            if (screen->root_visual == vi.data->visual_id) {
                vt = vi.data;
                break;
            }
        }
    }

    for (size_t y = 0; y < img.height; ++y) {
        for (size_t x = 0; x < img.width; ++x) {
            uint32_t pix = xcb_image_get_pixel(img_t, x, y);
            uint8_t blue = pix & vt->blue_mask;
            uint8_t green = (pix & vt->green_mask) >> 8;
            uint8_t red = (pix & vt->red_mask) >> 16;
            png_byte *bptr = &(bytep[x * 3]);
            bptr[0] = red;
            bptr[1] = green;
            bptr[2] = blue;
        }
        png_write_row(ptr, bytep);
    }
    png_write_end(ptr, NULL);
    fclose(fp);
    png_free_data(ptr, infop, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&ptr, (png_infopp)NULL);
    free(bytep);
    xcb_image_destroy(img_t);
}

xcb_cursor_t
load_cursor(const char *name) {
    xcb_cursor_context_t *ctx;
    xcb_cursor_t cid;

    if (xcb_cursor_context_new(conn, screen, &ctx) < 0)
        DIE("could not create cursor context");

    cid = xcb_cursor_load_cursor(ctx, name); // "crosshair"
    res_add(cid, RES_CURSOR);
    xcb_cursor_context_free(ctx);
    return cid;
}

int
main(int charc, char *argv[])
{
    connect_init();

    // req image of display
    xcb_get_image_cookie_t cookie = xcb_get_image(
        conn,
        XCB_IMAGE_FORMAT_Z_PIXMAP,
        screen->root,
        0, 0,
        screen->width_in_pixels, screen->height_in_pixels,
        ALLPLANES
    );
    if (charc >= 2 && !strcmp(argv[1], "-f")) goto image;

    // req a pointer grab
    xcb_cursor_t cid = load_cursor(CURSOR);
    value_mask = XCB_EVENT_MASK_BUTTON_PRESS    |
                 XCB_EVENT_MASK_BUTTON_1_MOTION |
                 XCB_EVENT_MASK_BUTTON_RELEASE;
    xcb_grab_pointer_cookie_t pcookie = xcb_grab_pointer(
        conn,
        FALSE,
        screen->root,
        value_mask,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
        XCB_NONE, cid,
        XCB_CURRENT_TIME
    );

    // create window to draw on with pixmap as bg
    create_window();

    // create a pen gc
    pen = xcb_generate_id(conn);
    value_mask = XCB_GC_FOREGROUND         |
                 XCB_GC_LINE_WIDTH         |
                 XCB_GC_LINE_STYLE         |
                 XCB_GC_GRAPHICS_EXPOSURES;
    value_list[0] = LINE_COLOR;
    value_list[1] = LINE_WIDTH;
    value_list[2] = XCB_LINE_STYLE_SOLID;
    value_list[3] = 0;
    xcb_create_gc(
        conn,
        pen,
        screen->root,
        value_mask, value_list
    );
    res_add(pen, RES_GC);

    // create a pixmap to copy image to
    pm = xcb_generate_id(conn);
    xcb_create_pixmap(
        conn,
        screen->root_depth,
        pm, win,
        screen->width_in_pixels, screen->height_in_pixels
    );
    res_add(pm, RES_PIXMAP);

    // Get reply for image data
    image: {
        xcb_get_image_reply_t *img_rep = xcb_get_image_reply(conn, cookie, NULL);
        _img bg_img;
        bg_img.data = xcb_get_image_data(img_rep);
        bg_img.len = xcb_get_image_data_length(img_rep);
        bg_img.width = screen->width_in_pixels;
        bg_img.height = screen->height_in_pixels;

        // exit with ss of fullscreen
        if (charc >= 3 && !strcmp(argv[1], "-f")) {
            const char *name;
            name = argv[2];
            data_to_png(bg_img, argv[2]);
            free(img_rep);
            res_clear();
        } else if (charc >= 2 && !strcmp(argv[1], "-f")) {
            data_to_png(bg_img, "fullscreen.png");
            free(img_rep);
            res_clear();
        }

        // put image on pixmap
        xcb_put_image(
            conn,
            XCB_IMAGE_FORMAT_Z_PIXMAP,
            pm,
            dgc,
            screen->width_in_pixels,
            screen->height_in_pixels,
            0, 0, 0,
            screen->root_depth,
            bg_img.len, bg_img.data
        );
        free(img_rep);
    }

    // change window attributes to set pixmap as background
    value_mask = XCB_CW_BACK_PIXMAP       |
                 XCB_CW_BORDER_PIXEL      |
                 XCB_CW_OVERRIDE_REDIRECT |
                 XCB_CW_EVENT_MASK;
    value_list[0] = pm;
    value_list[1] = 0;
    value_list[2] = 1;
    value_list[3] = XCB_EVENT_MASK_BUTTON_PRESS    |
                    XCB_EVENT_MASK_BUTTON_1_MOTION |
                    XCB_EVENT_MASK_BUTTON_RELEASE;
    xcb_change_window_attributes(conn, win, value_mask, value_list);

    xcb_map_window(conn, win);
    xcb_flush(conn);

    // get a reply for pointer grab
    xcb_grab_pointer_reply_t *gp_rep = xcb_grab_pointer_reply(
        conn,
        pcookie,
        NULL
    );
    if (gp_rep->status != XCB_GRAB_STATUS_SUCCESS) {
        free(gp_rep);
        DIE("failed to grab the pointer");
    } free(gp_rep);

    event_loop(charc, argv);
}
