#include "LvChess.h"
#include "XQWL.h"
#include <map>
#include <vector>
#include <iostream>

using namespace std;

std::map<uint8_t, const void *> bmpPieces = {
        {8,  &rk},
        {9,  &ra},
        {10, &rb},
        {11, &rn},
        {12, &rr},
        {13, &rc},
        {14, &rp},

        {16, &bk},
        {17, &ba},
        {18, &bb},
        {19, &bn},
        {20, &br},
        {21, &bc},
        {22, &bp},
};

std::map<uint8_t, const void *> bmpPieces2 = {
        {8,  &rk2},
        {9,  &ra2},
        {10, &rb2},
        {11, &rn2},
        {12, &rr2},
        {13, &rc2},
        {14, &rp2},

        {16, &bk2},
        {17, &ba2},
        {18, &bb2},
        {19, &bn2},
        {20, &br2},
        {21, &bc2},
        {22, &bp2},
};

typedef struct {
    lv_obj_t *obj;
    int pc;
    int sq;
    bool ok;
} lv_piece;

// 棋子选择框
lv_obj_t *lv_select = nullptr;
lv_obj_t *lv_board = nullptr;
vector<lv_piece *> lv_pieces;
static lv_timer_t *respond_timer = nullptr;
const extern lv_font_t teng_18;
const BOOL DRAW_SELECTED = TRUE;

// 消息通知
static void MessageBoxMute(const char *txt) {
    auto label = lv_label_create(lv_layer_top());
    if (!label) return;
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_font(label, &teng_18, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_bg_color(label, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_70, 0);
    lv_obj_set_style_pad_all(label, 10, 0);
    lv_obj_set_style_radius(label, 5, 0);
    lv_obj_center(label);
    
    // Auto delete after 2 seconds using animation (safer than global timer)
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, label);
    lv_anim_set_duration(&a, 2000);
    lv_anim_set_values(&a, 255, 254); 
    // Animation automatically stops if the label is deleted by lv_obj_clean()
    lv_anim_set_completed_cb(&a, [](lv_anim_t * anim) {
        lv_obj_delete_async((lv_obj_t *)anim->var);
    });
    lv_anim_start(&a);
}

// 自动置空回调
static void on_obj_delete_cb(lv_event_t *e) {
    lv_obj_t **ptr = (lv_obj_t **)lv_event_get_user_data(e);
    if (ptr) {
        *ptr = nullptr;
    }
}

// 绑定对象生命周期
static void BindObjLife(lv_obj_t *obj, lv_obj_t **ptr) {
    if (!obj || !ptr) return;
    lv_obj_add_event_cb(obj, on_obj_delete_cb, LV_EVENT_DELETE, ptr);
}

// 停止所有棋子动画并解绑
static void ClearAllPieces() {
    for (auto p : lv_pieces) {
        if (p) {
            if (p->obj) {
                // 移除回调，防止 delete p 后回调触发导致野指针写入
                lv_obj_remove_event_cb(p->obj, on_obj_delete_cb);
                lv_anim_delete(p->obj, nullptr);
            }
            delete p;
        }
    }
    lv_pieces.clear();
}

// 移动动画回调
static void anim_x_cb(void *var, int32_t v) {
    lv_obj_set_x((lv_obj_t *)var, v);
}
static void anim_y_cb(void *var, int32_t v) {
    lv_obj_set_y((lv_obj_t *)var, v);
}

// 呼吸动画回调
static void anim_zoom_cb(void *var, int32_t v) {
    lv_img_set_zoom((lv_obj_t *)var, (uint16_t)v);
}

static void StartBreatheAnim(lv_obj_t *obj) {
    if (!obj) return;
    // [原理] 检查对象是否有效且不在删除队列中
    if (lv_obj_is_valid(obj) == false) return; 
    
    if (lv_anim_get(obj, anim_zoom_cb) != nullptr) {
        return;
    }
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, 226, 256);
    lv_anim_set_duration(&a, 500); // LVGL 9 使用 duration
    lv_anim_set_playback_duration(&a, 500); 
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, anim_zoom_cb);
    lv_anim_start(&a);
}

static void StopBreatheAnim(lv_obj_t *obj) {
    if (!obj) return;
    if (lv_obj_is_valid(obj) == false) return;
    
    if (lv_anim_delete(obj, anim_zoom_cb)) {
         lv_image_set_scale(obj, 256); // LVGL 9 使用 image_set_scale
    }
}

static void StartMoveAnim(lv_obj_t *obj, int x, int y, lv_anim_completed_cb_t completed_cb = nullptr) {
    if (!obj) return;
    if (lv_obj_is_valid(obj) == false) return;

    int cur_x = lv_obj_get_x(obj);
    int cur_y = lv_obj_get_y(obj);
    
    lv_anim_delete(obj, anim_x_cb);
    lv_anim_delete(obj, anim_y_cb);

    if (cur_x == x && cur_y == y) {
        if (completed_cb) {
            // 如果已经在目标位置，直接触发完成回调
            completed_cb(nullptr);
        }
        return;
    }

    if (cur_x != x) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, obj);
        lv_anim_set_values(&a, cur_x, x);
        lv_anim_set_duration(&a, 300);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a, anim_x_cb);
        if (cur_y == y && completed_cb) {
            // 如果 Y 轴不动，则在 X 轴完成时触发回调
            lv_anim_set_completed_cb(&a, completed_cb);
        }
        lv_anim_start(&a);
    }

    if (cur_y != y) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, obj);
        lv_anim_set_values(&a, cur_y, y);
        lv_anim_set_duration(&a, 300);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a, anim_y_cb);
        if (completed_cb) {
            // 无论 X 轴是否动，在 Y 轴完成时触发回调是安全的
            lv_anim_set_completed_cb(&a, completed_cb);
        }
        lv_anim_start(&a);
    }
}

static void RefreshBoard(int mvAnim = 0, lv_anim_completed_cb_t completed_cb = nullptr) {
    int x, y, xx, yy, sq, pc;
    
    // 增加对核心全局对象的有效性检查
    if (!lv_board || lv_pieces.empty()) return;

    // 1. 重置所有棋子的状态，并停止未选中的呼吸动画
    size_t piece_count = lv_pieces.size();
    for (size_t i = 0; i < piece_count; i++) {
        lv_piece *p = lv_pieces[i];
        if (!p || !p->obj) continue;
        p->ok = false;
        StopBreatheAnim(p->obj);
    }

    // 记录是否有动画正在进行
    bool any_anim = false;
    lv_obj_t *anim_target_obj = nullptr;
    
    // 2. 第一遍扫描：优先匹配未移动的棋子（位置和类型都匹配）
    for (size_t i = 0; i < piece_count; i++) {
        lv_piece *p = lv_pieces[i];
        if (!p || !p->obj || lv_obj_is_valid(p->obj) == false) continue;
        if (p->sq >= 0 && p->sq < 256) {
            int board_pc = pos.ucpcSquares[p->sq];
            if (board_pc == p->pc) {
                p->ok = true;
                int visual_sq = Xqwl.bFlipped ? SQUARE_FLIP(p->sq) : p->sq;
                xx = BOARD_EDGE_H + (FILE_X(visual_sq) - FILE_LEFT) * SQUARE_SIZE;
                yy = BOARD_EDGE_V + (RANK_Y(visual_sq) - RANK_TOP) * SQUARE_SIZE;
                
                if (lv_obj_has_flag(p->obj, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_remove_flag(p->obj, LV_OBJ_FLAG_HIDDEN);
                }
                
                // 检查是否是本次移动的目标棋子
                if (mvAnim != 0 && p->sq == DST(mvAnim)) {
                    any_anim = true;
                    anim_target_obj = p->obj;
                    StartMoveAnim(p->obj, xx, yy, completed_cb);
                } else {
                    StartMoveAnim(p->obj, xx, yy);
                }

                if (p->sq == Xqwl.sqSelected) {
                    StartBreatheAnim(p->obj);
                }
            }
        }
    }

    // 3. 第二遍扫描：匹配移动过的棋子
    for (x = FILE_LEFT; x <= FILE_RIGHT; x++) {
        for (y = RANK_TOP; y <= RANK_BOTTOM; y++) {
            sq = COORD_XY(x, y);
            int logic_sq = Xqwl.bFlipped ? SQUARE_FLIP(sq) : sq;
            pc = pos.ucpcSquares[logic_sq];
            
            if (pc != 0) {
                bool matched = false;
                for (size_t i = 0; i < piece_count; i++) {
                    lv_piece *p = lv_pieces[i];
                    if (p && p->ok && p->sq == logic_sq) {
                        matched = true;
                        break;
                    }
                }
                
                if (!matched) {
                    for (size_t i = 0; i < piece_count; i++) {
                        lv_piece *p = lv_pieces[i];
                        if (p && !p->ok && p->pc == pc && p->obj && lv_obj_is_valid(p->obj)) {
                            p->ok = true;
                            p->sq = logic_sq;
                            xx = BOARD_EDGE_H + (x - FILE_LEFT) * SQUARE_SIZE;
                            yy = BOARD_EDGE_V + (y - RANK_TOP) * SQUARE_SIZE;
                            
                            if (lv_obj_has_flag(p->obj, LV_OBJ_FLAG_HIDDEN)) {
                                lv_obj_remove_flag(p->obj, LV_OBJ_FLAG_HIDDEN);
                            }
                            
                            // 检查是否是本次移动的目标棋子
                            if (mvAnim != 0 && p->sq == DST(mvAnim)) {
                                any_anim = true;
                                anim_target_obj = p->obj;
                                StartMoveAnim(p->obj, xx, yy, completed_cb);
                            } else {
                                StartMoveAnim(p->obj, xx, yy);
                            }

                            if (logic_sq == Xqwl.sqSelected) {
                                StartBreatheAnim(p->obj);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    
    // 如果没有任何棋子触发动画且有完成回调，则立即执行回调
    if (!any_anim && completed_cb) {
        completed_cb(nullptr);
    }

    // 4. 处理隐藏和遮罩
    for (size_t i = 0; i < piece_count; i++) {
        lv_piece *p = lv_pieces[i];
        if (!p || !p->obj || lv_obj_is_valid(p->obj) == false) continue;
        if (!p->ok) {
            if (!lv_obj_has_flag(p->obj, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(p->obj, LV_OBJ_FLAG_HIDDEN);
            }
            StopBreatheAnim(p->obj);
        }
    }

    if (lv_select && lv_obj_is_valid(lv_select)) {
        if (Xqwl.sqSelected == 0) {
            lv_obj_add_flag(lv_select, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_remove_flag(lv_select, LV_OBJ_FLAG_HIDDEN);
            int visual_sq = Xqwl.bFlipped ? SQUARE_FLIP(Xqwl.sqSelected) : Xqwl.sqSelected;
            xx = BOARD_EDGE_H + (FILE_X(visual_sq) - FILE_LEFT) * SQUARE_SIZE;
            yy = BOARD_EDGE_V + (RANK_Y(visual_sq) - RANK_TOP) * SQUARE_SIZE;
            lv_obj_set_pos(lv_select, xx, yy);
            lv_obj_move_foreground(lv_select);
        }
    }
}

// 绘制格子
static void DrawSquare(int sq, BOOL bSelected = FALSE) {
    int sqFlipped, xx, yy;
    sqFlipped = Xqwl.bFlipped ? SQUARE_FLIP(sq) : sq;
    xx = BOARD_EDGE_H + (FILE_X(sqFlipped) - FILE_LEFT) * SQUARE_SIZE;
    yy = BOARD_EDGE_V + (RANK_Y(sqFlipped) - RANK_TOP) * SQUARE_SIZE;
    
    if (bSelected && lv_select) {
        lv_obj_set_pos(lv_select, xx, yy);
    }
}

// 电脑回应一步棋
static void ResponseMove() {
    int vlRep;
    
    // 移除之前的 lv_timer_handler()，防止在 timer 回调内部重入导致的崩溃
    SearchMain();
    
    // 检查搜索结果是否合法
    if (Search.mvResult == 0) return;

    pos.MakeMove(Search.mvResult);
    Xqwl.mvLast = Search.mvResult;
    
    // 电脑走棋完成后，开始电脑的动画
    RefreshBoard(Search.mvResult);
    
    if (pos.Checked()) {
        MessageBoxMute("将军！");
    }
    
    vlRep = pos.RepStatus(3);
    if (pos.IsMate()) {
        MessageBoxMute("请再接再厉！");
        Xqwl.bGameOver = TRUE;
    } else if (vlRep > 0) {
        vlRep = pos.RepValue(vlRep);
        MessageBoxMute(vlRep < -WIN_VALUE ? "长打作负，请不要气馁！" :
                       vlRep > WIN_VALUE ? "电脑长打作负，祝贺你取得胜利！" : "双方不变作和，辛苦了！");
        Xqwl.bGameOver = TRUE;
    } else if (pos.nMoveNum > 100) {
        MessageBoxMute("超过自然限着作和，辛苦了！");
        Xqwl.bGameOver = TRUE;
    }
}

void DelayRespondMove() {
    if (respond_timer) {
        lv_timer_delete(respond_timer);
        respond_timer = nullptr;
    }
    // 增加延迟到 200ms，确保 UI 线程有足够时间完成当前的渲染
    respond_timer = lv_timer_create([](lv_timer_t *t) {
        ResponseMove();
        respond_timer = nullptr;
        lv_timer_delete(t);
    }, 200, nullptr);
}

// 点击格子事件处理
static void ClickSquare(int sq) {
    int pc, mv, vlRep;
    //Xqwl.hdc = GetDC(Xqwl.hWnd);
    //Xqwl.hdcTmp = CreateCompatibleDC(Xqwl.hdc);
    sq = Xqwl.bFlipped ? SQUARE_FLIP(sq) : sq;
    pc = pos.ucpcSquares[sq];

    if ((pc & SIDE_TAG(pos.sdPlayer)) != 0) {
        // 如果点击自己的子，那么直接选中该子
        Xqwl.sqSelected = sq;
        DrawSquare(sq, DRAW_SELECTED);
        RefreshBoard();
        //PlayResWav(IDR_CLICK); // 播放点击的声音
    } else if (Xqwl.sqSelected != 0 && !Xqwl.bGameOver) {
        // 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子
        mv = MOVE(Xqwl.sqSelected, sq);
        if (pos.LegalMove(mv)) {
            if (pos.MakeMove(mv)) {
                Xqwl.mvLast = mv;
                Xqwl.sqSelected = 0;
                
                // 1. 用户走子后，启动动画，并在动画完成后触发电脑响应
                RefreshBoard(mv, [](lv_anim_t *a) {
                    // 2. 检查用户这一步是否将军
                    if (pos.Checked()) {
                        MessageBoxMute("将军！");
                    }

                    // 3. 检查游戏是否结束，如果没有，则触发电脑响应
                    int vlRep = pos.RepStatus(3);
                    if (pos.IsMate()) {
                        MessageBoxMute("祝贺你取得胜利！");
                        Xqwl.bGameOver = TRUE;
                    } else if (vlRep > 0) {
                        vlRep = pos.RepValue(vlRep);
                        MessageBoxMute(vlRep > WIN_VALUE ? "长打作负，请不要气馁！" :
                                       vlRep < -WIN_VALUE ? "电脑长打作负，祝贺你取得胜利！" : "双方不变作和，辛苦了！");
                        Xqwl.bGameOver = TRUE;
                    } else if (pos.nMoveNum > 100) {
                        MessageBoxMute("超过自然限着作和，辛苦了！");
                        Xqwl.bGameOver = TRUE;
                    } else {
                        // 正常走子，轮到电脑
                        DelayRespondMove();
                    }
                });
            } else {
                //PlayResWav(IDR_ILLEGAL); // 播放被将军的声音
            }
        }
        // 如果根本就不符合走法(例如马不走日字)，那么程序不予理会
    }
}

// 触摸响应
static void on_board_click(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        auto indev = lv_indev_active();
        if (!indev) return;
        lv_point_t tp;
        lv_indev_get_point(indev, &tp);
        int x = FILE_LEFT + (tp.x - BOARD_EDGE_H) / SQUARE_SIZE;
        int y = RANK_TOP + (tp.y - BOARD_EDGE_V) / SQUARE_SIZE; // 修复了 Y 轴偏移使用 BOARD_EDGE_V
        if (x >= FILE_LEFT && x <= FILE_RIGHT && y >= RANK_TOP && y <= RANK_BOTTOM) {
            ClickSquare(COORD_XY(x, y));
        }
    }
}

// 绘制棋盘
static void DrawInitBoard() {
    int x, y, xx = 0, yy = 0, sq, pc;

    // 1. 立即清理待处理的计时器
    if (respond_timer) {
        lv_timer_delete(respond_timer);
        respond_timer = nullptr;
    }

    auto act = lv_scr_act();
    
    // 2. 先解绑并删除所有的 lv_piece 结构体，然后再清理 LVGL 对象
    // 这样可以确保 LVGL 的清理过程不会触发任何已经失效的 struct 指针
    ClearAllPieces();
    
    // 3. 清理屏幕上的所有 LVGL 对象
    lv_obj_clean(act);
    lv_board = nullptr;
    lv_select = nullptr;

    // 画棋盘
    lv_board = lv_img_create(act);
    lv_img_set_src(lv_board, &board);
    BindObjLife(lv_board, &lv_board);
    // 棋盘事件
    lv_obj_add_flag(lv_board, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(lv_board, on_board_click, LV_EVENT_PRESSED, nullptr);

    // 画棋子
    for (x = FILE_LEFT; x <= FILE_RIGHT; x++) {
        for (y = RANK_TOP; y <= RANK_BOTTOM; y++) {
            sq = COORD_XY(x, y);
            pc = pos.ucpcSquares[sq];
            if (pc != 0) {
                xx = BOARD_EDGE_H + (x - FILE_LEFT) * SQUARE_SIZE;
                yy = BOARD_EDGE_V + (y - RANK_TOP) * SQUARE_SIZE;
                auto img = bmpPieces[pc];
                auto lvg = lv_img_create(act);
                lv_img_set_src(lvg, img);
                lv_obj_set_pos(lvg, (int16_t) xx, (int16_t) yy);
                auto *lv_p = new lv_piece;
                lv_p->obj = lvg;
                BindObjLife(lv_p->obj, &lv_p->obj); // 绑定棋子对象生命周期
                lv_p->pc = pc;
                lv_p->sq = sq;
                lv_p->ok = true; // 初始标记为 ok
                lv_pieces.push_back(lv_p);
            }
        }
    }

    // 选中遮罩
    lv_select = lv_img_create(act);
    lv_img_set_src(lv_select, &selected);
    BindObjLife(lv_select, &lv_select);
    lv_obj_add_flag(lv_select, LV_OBJ_FLAG_HIDDEN); // 初始隐藏

    // 按钮保持不变...
    // 重新开始按钮
    auto btn_restart = lv_btn_create(act);
    lv_obj_set_size(btn_restart, 120, 40);
    lv_obj_set_pos(btn_restart, 30, 400);
    auto label_restart = lv_label_create(btn_restart);
    lv_label_set_text(label_restart, "重新开始");
    lv_obj_set_style_text_font(label_restart, &teng_18, 0);
    lv_obj_center(label_restart);
    lv_obj_add_event_cb(btn_restart, [](lv_event_t *e) {
        lv_chess_start(); 
    }, LV_EVENT_CLICKED, nullptr);

    // 悔棋按钮
    auto btn_undo = lv_btn_create(act);
    lv_obj_set_size(btn_undo, 120, 40);
    lv_obj_set_pos(btn_undo, 180, 400);
    auto label_undo = lv_label_create(btn_undo);
    lv_label_set_text(label_undo, "悔棋");
    lv_obj_set_style_text_font(label_undo, &teng_18, 0);
    lv_obj_center(label_undo);
    lv_obj_add_event_cb(btn_undo, [](lv_event_t *e) {
        if (pos.nMoveNum > 2) {
            pos.UndoMakeMove(); 
            pos.UndoMakeMove(); 
            Xqwl.sqSelected = 0;
            Xqwl.mvLast = pos.mvsList[pos.nMoveNum - 1].wmv;
            Xqwl.bGameOver = FALSE;
            RefreshBoard(); 
        }
    }, LV_EVENT_CLICKED, nullptr);
}

//开始游戏
void lv_chess_start() {
    InitZobrist();
    LoadBook();
    Xqwl.bFlipped = FALSE;
    Xqwl.sqSelected = 0;
    Xqwl.mvLast = 0;
    Xqwl.bGameOver = FALSE;
    Startup();
    DrawInitBoard();
}