"""Keep the stock LVGL package but compile only the modules this GUI uses.

PlatformIO normally compiles every LVGL source file (551 in LVGL 9.6.0),
even for features disabled in lv_conf.h. Source-level filtering is distinct
from preprocessor-based feature configuration. This PRE build middleware
skips unrelated upstream sources WITHOUT patching installed dependencies.

Paths are relative to the lvgl/src directory of the pinned v9.6.0 tag.
Whenever LVGL is upgraded, qualify this explicit allowlist again in CI.
"""

Import("env")

# PlatformIO IDE runs integration discovery independently of building.
if env.IsIntegrationDump():
    Return()

# Safe base modules: object tree, RGB565 SW drawing, font, display/timing,
# generic memory/text/style services, and exactly one widget (label).
# The list is intentionally narrow and auditable, not inferred from source
# includes or heuristics.
KEEP_DIRS = (
    "core/",
    "display/",
    "draw/sw/",
    "font/fmt_txt/",
    "indev/",
    "logging/",
    "misc/",
    "stdlib/clib/",
    "tick/",
    "widgets/label/",
)
KEEP_FILES = (
    "lv_init.c",
    "draw/lv_draw.c",
    "draw/lv_draw_3d.c",
    "draw/lv_draw_arc.c",
    "draw/lv_draw_blur.c",
    "draw/lv_draw_buf.c",
    "draw/lv_draw_image.c",
    "draw/lv_draw_label.c",
    "draw/lv_draw_line.c",
    "draw/lv_draw_mask.c",
    "draw/lv_draw_rect.c",
    "draw/lv_draw_triangle.c",
    "draw/lv_draw_utils.c",
    "draw/lv_draw_vector.c",
    "draw/convert/lv_draw_buf_convert.c",
    "font/lv_font.c",
    "font/lv_font_unscii_8.c",
    "fs/lv_fs.c",
    "image/lv_bin_decoder.c",
    "image/lv_image_decoder.c",
    "layouts/lv_layout.c",
    "osal/lv_os.c",
    "osal/lv_os_none.c",
    "stdlib/lv_mem.c",
    "themes/lv_theme.c",
)


def filter_lvgl_sources(build_env, node):
    original = node.srcnode().get_abspath().replace("\\", "/")
    marker = "/lvgl/src/"
    if marker not in original:
        return node

    relative = original.rsplit(marker, 1)[1]
    if relative in KEEP_FILES or relative.startswith(KEEP_DIRS):
        return node

    # Skip drivers, graphics backends, demos, disabled widgets, fonts, etc.
    return None


env.AddBuildMiddleware(filter_lvgl_sources)
print("LVGL v9.6.0 minimal source-filter active (no vendor patches)")
