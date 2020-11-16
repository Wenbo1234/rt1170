//
//  tiger_paths.h
//  tiger
//
//  Created by sw-builder on 6/25/14.
//  Copyright (c) 2014 Vivante Corporation. All rights reserved.
//

//#ifndef tiger_tiger_paths_h
//#define tiger_tiger_paths_h

#include "vg_lite.h"


signed char wenbopath_data[] = {
    2, 0, 1,
    4, -40, 0,
    4, 0, -1,
    1,
    0
};
#if 0 //wenbo 
signed char right_needle_data[] = {
    2, 2, 1,
    4, -38, 0,
    4, 2, -1,
    1,
    0
};
#endif 
signed char right_needle_data[] = {
    2, 2, 10,
    4, -20, 10,
    4, -20, -10,
    4, 2, -10,
    1,
    0
};

//int pathCount = 4;
int pathCount = 1;

vg_lite_path_t path[5] = {
    {
        {-80, -40, // left,top
            80, 90}, // right,bottom
        VG_LITE_HIGH, // quality
        VG_LITE_S8, // -128 to 127 coordinate range
        {0}, // uploaded
        sizeof(wenbopath_data), // path length
        wenbopath_data, // path data
        1           // Initially, path is changed for uploaded.
    },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(wenbopath_data), wenbopath_data, 1 },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(wenbopath_data), wenbopath_data, 1 },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(wenbopath_data), wenbopath_data, 1 },
};


vg_lite_path_t path_needle2[5] = {
    {
        {-80, -40, // left,top
            80, 90}, // right,bottom
        VG_LITE_HIGH, // quality
        VG_LITE_S8, // -128 to 127 coordinate range
        {0}, // uploaded
        sizeof(right_needle_data), // path length
        right_needle_data, // path data
        1           // Initially, path is changed for uploaded.
    },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(right_needle_data), right_needle_data, 1 },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(right_needle_data), right_needle_data, 1 },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(right_needle_data), right_needle_data, 1 },
};

uint32_t color_data[] = {
    //green added by wenbo
    //0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800,
    //0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00  ,
    //0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    //0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E,
    //0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C,
    //0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F,
    //0x001F001F, 
    //0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E,
    //0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0,
    0xffb1e4e4, 0xffb1e4e4, 0xff2672cc, 0xff508ee9, 0xff7caaef, 0xffa8c6f4, 0xffd3e2f9, 0xffffffff,
};

uint32_t needle2_color_data[] = {
    //green added by wenbo
    //0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800,
    0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00,
    //0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00,
    //0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    //0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E,
    //0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C,
    //0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F,
    //0x001F001F, 
    //0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E,
    //0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0,
    0xffb1e4e4, 0xffb1e4e4, 0xff2672cc, 0xff508ee9, 0xff7caaef, 0xffa8c6f4, 0xffd3e2f9, 0xffffffff,
};

//2:jump to 
//4:line to 
//1:close  


signed char left_arrow_data1[] = {
    2, -37, -35,  
    4, -40, -35,
    4, -40, -36,
    4, -43, -34,
    4, -40, -32,
    4, -40, -33,
    4, -37, -33,
    1,
    0
};

signed char left_arrow_data2[] = {
    2, -36, 84,  
    4, -41, 84,
    4, -41, 86,
    4, -45, 81,
    4, -41, 77,
    4, -41, 79,
    4, -36, 79,
    1,
    0
};

signed char left_arrow_data3[] = {
    2, -36, 44,  
    4, -41, 44,
    4, -41, 46,
    4, -45, 41,
    4, -41, 37,
    4, -41, 39,
    4, -36, 39,
    1,
    0
};

signed char left_arrow_data[] = {
    2, -36-6, 47+5,  
    4, -41-6, 47+5,
    4, -41-6, 49+5,
    4, -45-6, 44+5,
    4, -41-6, 40+5,
    4, -41-6, 42+5,
    4, -36-6, 42+5,
    1,
    0
};

signed char right_arrow_data[] = {
    2, 36+6, 47+5,  
    4, 41+6, 47+5,
    4, 41+6, 49+5,
    4, 45+6, 44+5,
    4, 41+6, 40+5,
    4, 41+6, 42+5,
    4, 36+6, 42+5,
    1,
    0
};

vg_lite_path_t path_left_arrow[5] = {
    {
        {-80, -40, // left,top
            80, 90}, // right,bottom
        VG_LITE_HIGH, // quality
        VG_LITE_S8, // -128 to 127 coordinate range
        {0}, // uploaded
        sizeof(left_arrow_data), // path length
        left_arrow_data, // path data
        1           // Initially, path is changed for uploaded.
    },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(left_arrow_data), left_arrow_data, 1 },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(left_arrow_data), left_arrow_data, 1 },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(left_arrow_data), left_arrow_data, 1 },
};


vg_lite_path_t path_rgiht_arrow[5] = {
    {
        {-80, -40, // left,top
            80, 90}, // right,bottom
        VG_LITE_HIGH, // quality
        VG_LITE_S8, // -128 to 127 coordinate range
        {0}, // uploaded
        sizeof(right_arrow_data), // path length
        right_arrow_data, // path data
        1           // Initially, path is changed for uploaded.
    },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(right_arrow_data), right_arrow_data, 1 },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(right_arrow_data), right_arrow_data, 1 },
    { {-80, -40, 80, 90}, VG_LITE_HIGH, VG_LITE_S8, {0}, sizeof(right_arrow_data), right_arrow_data, 1 },
};

uint32_t arrow_color_data[] = {
    //green added by wenbo
    //0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800,
    //0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00, 0xFC00FC00  ,
    //0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00, 0xFE00FE00,
    0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800, 0xF800F800,
    //0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E, 0x003E003E,
    //0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C, 0x003C003C,
    //0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F, 0x001F001F,
    //0x001F001F, 
    //0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E, 0x001E001E,
    //0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0, 0x07E007E0,
    0xffb1e4e4, 0xffb1e4e4, 0xff2672cc, 0xff508ee9, 0xff7caaef, 0xffa8c6f4, 0xffd3e2f9, 0xffffffff,
};
//#endif
