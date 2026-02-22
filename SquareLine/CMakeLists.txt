SET(SOURCES ui_Screen1.c
    ui_Screen2.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_1605233879.c
    ui_img_1620110196.c
    ui_img_463316836.c
    fonts/ui_font_DSEG7100.c
    fonts/ui_font_DSEG780.c
    fonts/ui_font_DSEG788.c
    fonts/ui_font_DSEG794.c
    fonts/ui_font_digital24.c
    fonts/ui_font_digital48.c
    fonts/ui_font_dsdigital120.c
    fonts/ui_font_dsdigital128.c
    fonts/ui_font_dsdigital48.c
    fonts/ui_font_dsdigital64.c
    fonts/ui_font_dsdigital72.c
    fonts/ui_font_dsdigital96.c
    fonts/ui_font_roboto24.c
    fonts/ui_font_roboto48.c
    fonts/ui_font_roboto64.c
    fonts/ui_font_dsdigital24.c
    fonts/ui_font_dsdigital32.c)

add_library(ui ${SOURCES})
