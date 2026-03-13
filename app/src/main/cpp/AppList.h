#ifndef LVGLPORTANDROID_APPLIST_H
#define LVGLPORTANDROID_APPLIST_H

#include <iostream>
#include <map>
#include <lv_demos.h>
#include "chess/lvChess.h"

using namespace std;

const map<string, void (*)(void)> lv_ci_example_list{
        {"default",   lv_demo_widgets},
        {"benchmark", lv_demo_benchmark},
        {"widgets",   lv_demo_widgets},
        {"music",     lv_demo_music},
        {"chess",     lv_chess_start},
};

#endif //LVGLPORTANDROID_APPLIST_H
