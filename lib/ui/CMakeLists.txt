SET(SOURCES ui_Screen1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_thermometer_30_png.c
    ui_img_1191103926.c
    ui_img_00d_75_png.c
    ui_img_00n_75_png.c
    ui_img_01d_75_png.c
    ui_img_01n_75_png.c
    ui_img_02_75_png.c
    ui_img_03_75_png.c
    ui_img_45_75_png.c
    ui_img_48_75_png.c
    ui_img_51_55_61_75_png.c
    ui_img_56_57_66_67_75_png.c
    ui_img_63_75_png.c
    ui_img_65_80_82_75_png.c
    ui_img_85_86_75_png.c
    ui_img_95_75_png.c
    ui_img_96_99_75_png.c
    ui_img_71_77_75_png.c
    fonts/ui_font_dsdigital128.c
    fonts/ui_font_dsdigital48.c
    fonts/ui_font_roboto20.c
    fonts/ui_font_roboto32.c
    fonts/ui_font_roboto36.c)

add_library(ui ${SOURCES})
