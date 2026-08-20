#include <stdio.h>
#include <math.h>
#include "raylib.h"

#define MAX_ENEMIES 25
#define MAX_BULLETS 30
#define MAX_B_ENEMIES 5
#define MAX_ENEMY_BULLETS 50

int SCREEN_WIDTH;
int SCREEN_HEIGHT;

typedef struct {
    Vector2 position;
    Vector2 speed;
    int active;
} Bullet;

typedef struct {
    Vector2 position;
    float speed;
    int hp;
} Player;

typedef struct {
    Vector2 position;
    Vector2 speed;
    int active;
    int hp;
} Enemy;

typedef struct{
    Vector2 position;
    Vector2 speed;
    int active;
} BoundEnemy;


typedef struct {
    Vector2 position;
    Vector2 speed;
    int active;
    int hp;
} Boss;

typedef struct {
    Vector2 position;
    Vector2 speed;
    int active;
} EnemyBullet;

EnemyBullet E_bullets[MAX_ENEMY_BULLETS] = {0};
BoundEnemy B_enemies [MAX_B_ENEMIES] = {0};

void Shoot_5Way(Vector2 basePos, float speed, float degree) {
    float angles[] = {-degree *2, -degree, 0.0f, degree, degree *2}; 

    for (int k = 0; k < 5; k++) {
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
            if (!E_bullets[i].active) {
                E_bullets[i].position = basePos;

                float rad = angles[k] * DEG2RAD;

                E_bullets[i].speed.x = sinf(rad) * (speed);
                E_bullets[i].speed.y = cosf(rad) * (speed);
                E_bullets[i].active = 1;
                break;
            }
        }
    }
}       

void Resetgame(Player *player, Bullet bullets[], Enemy enemies[], Boss *boss, int *score, int *speedup, int *shot, int *frame_count, int *max_bullets_now, Vector2 *boss_target, int *stage, int *start, int * B_enemies_timer) {
    player->position = (Vector2){(float)SCREEN_WIDTH / 2, (float)SCREEN_HEIGHT - 50};
    player->speed = 5.0f;
    player->hp = 5;

    boss->position = (Vector2){-(float)SCREEN_WIDTH / 2, -100.0f};
    boss->hp = 50;
    boss->active = 0;

    boss_target->x = (float)SCREEN_WIDTH / 2;
    boss_target->y = 200.0f;
    
    *score = 0;
    *speedup = 0;
    *shot = 0;
    *frame_count = 0;
    *max_bullets_now = 10;
    *B_enemies_timer = 0;
    
    *stage = 0;
    *start = 0; 
    SetTargetFPS(90);

    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = 0;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) E_bullets[i].active = 0;
    for (int i = 0; i < MAX_B_ENEMIES; i++) B_enemies[i].active = 0;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(1, 1, "Very Normal Game");
    int monitor = GetCurrentMonitor();
    SCREEN_WIDTH = GetMonitorWidth(monitor);
    SCREEN_HEIGHT = GetMonitorHeight(monitor);
    
    SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetWindowPosition(0, 0);

    InitAudioDevice();
    Sound shootSound = LoadSound("shoot.mp3");
    Sound hitSound = LoadSound("hit.mp3");
    Sound damageSound = LoadSound("damage.mp3");
    Sound boundSound = LoadSound("bound.mp3");

    int stage = 0;
    int start_text = 1;

    int speedup = 0;
    int shot = 0;
    int score = 0;

    int frame_count = 0;

    int max_bullets_now = 10;
    int timer = 0;
    int start = 0;
    int shoots_timer = 0;
    int B_enemies_timer = 0;

    SetTargetFPS(90);
    
    Player player = {
        .position = {(float)SCREEN_WIDTH / 2, (float)SCREEN_HEIGHT - 50},
        .speed = 6.0f,
        .hp = 5
    };
    Boss boss = {
        .position = {-(float)SCREEN_WIDTH / 2, -100.0f},
        .active = 0,
        .hp = 50
    };
    
    Vector2 boss_target = {(float)SCREEN_WIDTH / 2, 200.0f};
    Bullet bullets[MAX_BULLETS] = {0};
    Enemy enemies[MAX_ENEMIES] = {0};

    // メインゲームループ
    while (!WindowShouldClose()) {
        // ---スタート画面---
        if (start == 0) {
            if (IsKeyPressed(KEY_C)) {
                start = 1;
            }
        } 
        else {  
            if (player.hp > 0 && stage < 3) {
                frame_count++;
                shoots_timer++;
                B_enemies_timer++;

                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.position.x += player.speed;
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.position.x -= player.speed;

                if (player.position.x < 30) player.position.x = 30;
                if (player.position.x > SCREEN_WIDTH - 30) player.position.x = SCREEN_WIDTH - 30;

                //単発発射
                
                if (IsKeyPressed(KEY_SPACE) && shot < max_bullets_now) {
                    for (int i = 0; i < max_bullets_now; i++) {
                        if (!bullets[i].active) {
                            bullets[i].position = player.position;
                            bullets[i].speed = (Vector2){0.0f, -10.0f};
                            bullets[i].active = 1;
                            shot += 1;
                            PlaySound(shootSound);
                            break;
                        }
                    }
                }

                //連射

                if (IsKeyDown(KEY_ENTER) && shot < max_bullets_now){
                    if (shoots_timer >= 90){
                        shoots_timer = 0;
                        for (int k = 0; k < max_bullets_now - 4; k++){
                            for (int i = 0; i < max_bullets_now; i++) {
                                if (!bullets[i].active){
                                    bullets[i].position = player.position;
                                    bullets[i].speed = (Vector2){0.0f, -10.0f};
                                    bullets[i].active = 1;
                                    shot += 1;
                                    PlaySound(shootSound);
                                    break;
                                }
                            }
                        }
                    }
                }
                //玉の移動と削除
                for (int i = 0; i < max_bullets_now; i++) {
                    if (bullets[i].active) {
                        bullets[i].position.x += bullets[i].speed.x;
                        bullets[i].position.y += bullets[i].speed.y;

                        if (bullets[i].position.y < 0 || bullets[i].position.x < 0 || bullets[i].position.x > SCREEN_WIDTH) {
                            bullets[i].active = 0;
                            shot -= 1;
                            if (shot < 0) { shot = 0; }
                        }
                    }
                }
                // ボス出現
                if (speedup / 5 >= 10 && !boss.active) {
                    for (int i = 0; i < MAX_ENEMIES; i++){
                        enemies[i].active = 0 ;
                    } 
                    for (int i = 0; i < MAX_B_ENEMIES; i++){
                        B_enemies[i].active = 0;
                    } 
                    boss.active = 1;
                    boss.position = (Vector2){(float)SCREEN_WIDTH / 2, -100.0f};
                    speedup = 0;
                }    
                if (boss.active) {
                    float dx = boss_target.x - boss.position.x;
                    float dy = boss_target.y - boss.position.y;
                    if (fabsf(dx) < 2.0f && fabsf(dy) < 2.0f) {
                        boss.position = boss_target;
                        boss_target.x = (float)GetRandomValue(100, SCREEN_WIDTH - 100);
                        boss_target.y = (float)GetRandomValue(100, SCREEN_HEIGHT / 2);
                    } else {
                        boss.position.x += dx * 0.03f + stage / 25;
                        boss.position.y += dy * 0.03f + stage / 25;
                    }

                    // ボス弾発射
                    if (boss.hp > (stage + 1) * 5) {
                        if (frame_count % 30 == 0) {
                            for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
                                if (!E_bullets[i].active) {
                                    E_bullets[i].position = boss.position;
                                    E_bullets[i].speed = (Vector2){0.0f, 5.0f + stage *2};
                                    E_bullets[i].active = 1;
                                    PlaySound(shootSound);
                                    break;
                                }
                            }
                        }
                    } else {
                            //5way弾
                            if (frame_count % 60 == 0)
                            Shoot_5Way(boss.position, 4.0f, 20.0f);
                    }
                    
                } else {
                    //雑魚的追加
                    for (int i = 0; i < MAX_ENEMIES; i++) {
                        if (!enemies[i].active && frame_count % (36 / (stage + 1)) == 0) {
                            enemies[i].position = (Vector2){(float)GetRandomValue(50, SCREEN_WIDTH - 50), 0.0f};
                            enemies[i].speed = (Vector2){(float)GetRandomValue(-2, 2), (float)GetRandomValue(2, 8)};
                            enemies[i].active = 1;
                            enemies[i].hp = 1 + stage; 
                            break;
                        }
                    }

                    //バウンド弾の初期位置
                    if (B_enemies_timer >= 3600){
                        if (frame_count % 1350 == 0){
                            for (int i = 0;i < MAX_B_ENEMIES; i++){
                                if (!B_enemies[i].active){
                                    B_enemies[i].active = 1;
                                    B_enemies[i].position = (Vector2){(float)SCREEN_WIDTH / 2, 100};
                                    float rx = (float)GetRandomValue(2, 8);
                                    float ry = (float)GetRandomValue(4, 12);
                                    
                                    if (GetRandomValue(0,1) == 0) rx *= -1.0f;

                                    B_enemies[i].speed.x = rx;
                                    B_enemies[i].speed.y = ry;
                                    break;
                                } 
                            }
                        }
                    }
                    //バウンド弾の移動
                    for (int i = 0;i < MAX_B_ENEMIES; i++){
                        if (B_enemies[i].active == 1){
                            B_enemies[i].position.y += B_enemies[i].speed.y;
                            B_enemies[i].position.x += B_enemies[i].speed.x;

                            if (B_enemies[i].position.x > SCREEN_WIDTH - 10 || B_enemies[i].position.x < 10){
                                B_enemies[i].speed.x *= -1.0f;
                                PlaySound(boundSound);
                            }
                            if (B_enemies[i].position.y > SCREEN_HEIGHT - 10 || B_enemies[i].position.y < 10){
                                B_enemies[i].speed.y *= -1.0f;
                                PlaySound(boundSound);
                            } 
                        }
                    }
                }
                //ボス弾削除&移動
                for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
                    if (E_bullets[i].active) {
                        E_bullets[i].position.y += E_bullets[i].speed.y;
                        E_bullets[i].position.x += E_bullets[i].speed.x;

                        if (E_bullets[i].position.y > SCREEN_HEIGHT || E_bullets[i].position.x < 0 || E_bullets[i].position.x > SCREEN_WIDTH) {
                            E_bullets[i].active = 0;
                        }
                    }
                }

                //ザコ敵削除
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) {
                        enemies[i].position.y += enemies[i].speed.y;
                        enemies[i].position.x += enemies[i].speed.x;

                        if (enemies[i].position.y > SCREEN_HEIGHT || enemies[i].position.x < 0 || enemies[i].position.x > SCREEN_WIDTH) {
                            enemies[i].active = 0;
                        }
                    }
                }

                // --- 当たり判定処理 ---
                for (int i = 0; i < max_bullets_now; i++) {
                    if (!bullets[i].active) continue;

                    for (int j = 0; j < MAX_ENEMIES; j++) {
                        if (!enemies[j].active) continue;

                        if (CheckCollisionCircles(bullets[i].position, 12.0f, enemies[j].position, 14.0f + (speedup / 5.0f) + (stage * 2.0f))) {
                            PlaySound(hitSound);
                            bullets[i].active = 0;
                            shot -= 1;
                            if (shot < 0) { shot = 0; }
                            enemies[j].hp -= 1;

                            if (enemies[j].hp <= 0) {
                                enemies[j].active = 0;
                                score += 100;
                                
                                if (score % 1500 == 0 && max_bullets_now < MAX_BULLETS) {
                                    max_bullets_now++;
                                }

                                if (score % 500 == 0) {
                                    speedup += 5;
                                }
                            }
                            break;
                        }
                    }
                }
                for (int i = 0;i < MAX_B_ENEMIES; i++){
                    if (!B_enemies[i].active) continue;

                    if (CheckCollisionCircles(player.position, 30.0f + stage * 3, B_enemies[i].position, 20.0f + stage *2)) {
                        B_enemies[i].speed.x *= -1.0f;
                        B_enemies[i].position = (Vector2){10.0f, 100.0f};
                        B_enemies[i].speed.y *= -1.0f;
                        player.hp -= 1;
                        PlaySound(damageSound);
                    }
                }
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) {
                        if (CheckCollisionCircles(player.position, 30.0f + stage * 3, enemies[i].position, 14.0f + (speedup / 5.0f) + (stage * 2.0f))) {
                            enemies[i].active = 0;
                            player.hp -= 1;
                            PlaySound(damageSound);
                            break;
                        }
                    }
                }


                for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
                    if (E_bullets[i].active) {
                        if (CheckCollisionCircles(player.position, 30.0f + stage * 3, E_bullets[i].position, 15.0f + stage)) {
                            E_bullets[i].active = 0;
                            player.hp -= 1;
                            PlaySound(damageSound);
                            break;
                        }
                    }
                }

                if (boss.active) {
                    for (int i = 0; i < max_bullets_now; i++) {
                        if (bullets[i].active) { 
                            if (CheckCollisionCircles(boss.position, 100.0f - stage * 10, bullets[i].position, 12.0f)) {
                                bullets[i].active = 0;
                                boss.hp -= 1;
                                shot -= 1;
                                if (shot < 0) { shot = 0; }
                                PlaySound(hitSound);
                                if (boss.hp <= 0) {
                                    boss.active = 0;
                                }
                                break;
                            }
                        }
                    }
                }
            } 
            else if (player.hp <= 0 || stage >= 3) {
                if (IsKeyPressed(KEY_R)) {
                    Resetgame(&player, bullets, enemies, &boss, &score, &speedup, &shot, &frame_count, &max_bullets_now, &boss_target, &stage, &start, &B_enemies_timer);
                }
            }
        }

        // --- 2. 描画処理 ---
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        if (start == 0) {
            // タイトル画面の描画
            DrawText("Very\n       Normal\n                Game", SCREEN_WIDTH / 4 - 100, 100, 130, RED);
            DrawText("Press 'C' to Start", SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT / 2 + 100, 40, DARKGRAY);
        }
        else {
            // ゲーム画面の描画
            if (player.hp > 0 && stage < 3) {
                if (start_text == 1) {
                    DrawText("A,D:MOVE ENTER,SPACE:ATTACK", SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2, 35, RED);
                    if (frame_count > 900 + stage * 90) {
                        start_text = 0;
                    }
                }

                DrawCircleV(player.position, 30.0f + stage * 3, BLUE);

                if (boss.active) {
                    DrawCircleV(boss.position, 100.0f - stage * 10, RED);
                }
                for (int i = 0;i < MAX_B_ENEMIES; i++){
                    if (B_enemies[i].active) {
                        DrawCircleV(B_enemies[i].position, 20.0f + stage *2, DARKPURPLE);
                    }
                }
                for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
                    if (E_bullets[i].active) {
                        DrawCircleV(E_bullets[i].position, 15.0f + stage, DARKPURPLE);
                    }
                }

                for (int i = 0; i < max_bullets_now; i++) {
                    if (bullets[i].active) {
                        DrawCircleV(bullets[i].position, 12.0f, YELLOW);
                    }
                }

                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].active) {
                        DrawCircleV(enemies[i].position, 14.0f + (speedup / 5.0f) + (stage * 2.0f), DARKGREEN);
                    }
                }

                DrawText(TextFormat("Score: %d", score), 10, 10, 30, BLACK);
                DrawText(TextFormat("Shots: %d / %d", max_bullets_now - shot, max_bullets_now), 10, 50, 30, BLACK);
                DrawText(TextFormat("Stage: %d", stage + 1), 10, 90, 30, BLACK);
                DrawText(TextFormat("Hp: %d", player.hp), 10, SCREEN_HEIGHT - 50, 40, RED);

                if (boss.active && boss.hp > 0) {
                    DrawText(TextFormat("Boss hp: %d", boss.hp), SCREEN_WIDTH / 2 - 100, 20, 40, RED);
                }

                if (boss.hp <= 0 && !boss.active && timer < 180 && frame_count > 100) {
                    timer++;
                    DrawText("STAGE CLEAR!", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 50, 60, RED);
                    for (int i = 0; i < MAX_ENEMIES; i++){
                        enemies[i].active = 0;
                    }
                    B_enemies_timer = 0;
                    if (timer >= 180) {
                        stage += 1;
                        boss.hp = 50 + stage * 25;
                        speedup = 0;
                        timer = 0;
                        frame_count = 0;
                        SetTargetFPS(90);
                        player.hp = 4 + stage * 2;
                        for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = 0;
                        shot = 0;
                    }
                }
            } else if (stage >= 3) {
                DrawText("YOU WIN!", SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT / 2 - 100, 70, GOLD);
                DrawText("Press 'R' to Restart\nPress 'Esc' to Quit", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 + 20, 40, BLACK);
            } else {
                DrawText("Game Over\nPress 'R' to Restart\nPress 'Esc' to Quit", SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2 - 100, 50, BLACK);
            }
        }

        EndDrawing();
    }

    // --- 3. クリーンアップ（whileループの外側） ---
    UnloadSound(shootSound);
    UnloadSound(hitSound);
    UnloadSound(damageSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
