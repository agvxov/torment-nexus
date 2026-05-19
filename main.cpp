// @BAKE clang++ -o torment-nexus.out $@ -std=c++20 -lm -lraylib -Wall -Wpedantic -Wno-c99-extensions -ggdb
// XXX nounused but for functions only?
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <vector>
#include <raylib.h>
#include <raymath.h>
#include "roman.h"
#include "DenseObjectPool.hpp"
#include "Timeout.hpp"
#include "global_arena.hpp"
#include "resource_manager.hpp"
#include "dyrect.h"
#include "XXX.h"

const int W = 1200;
const int H = 800;

size_t TICK = 0;
const size_t TICKS_PER_SEC = 60;

resource_manager<Texture2D> * rms;

using namespace std;

//static inline float signum(float x) { return (x > 0) - (x < 0); }

static inline
Color hex2color(unsigned int hex) {
    return Color{
        (unsigned char)((hex >> 16) & 0xFF), // R
        (unsigned char)((hex >>  8) & 0xFF), // G
        (unsigned char)( hex        & 0xFF), // B
        255                                  // A
    };
}

static inline
float is_in_range(float what, float min, float max) {
    return what >  min
        && what <= max
    ;
}

static
void save_screenshot_with_timestamp(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char file_name[128];
    snprintf(
        file_name,
        sizeof(file_name),
        "screenshot_%04d-%02d-%02d_%02d-%02d-%02d.png",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec
    );

    TakeScreenshot(file_name);
}

class Entity {
  public:
    bool is_enabled = true;
    virtual void draw(void)   { ; }
    virtual void update(void) { ; }
};

#include "Bullet.hpp"
#include "static_prop.hpp"

class Room : public Entity {
  public:
    int x;
    int y;
    int w;
    int h;

    float tile_scale = 5.0f;
    RenderTexture2D texture_cache{};
    bool has_cache = false;

    ~Room() {
        destroy_cache();
    }

    void destroy_cache() {
        if (!has_cache) {
            return;
        }
        UnloadRenderTexture(texture_cache);
        texture_cache = {};
        has_cache = false;
    }

    void cache() {
        destroy_cache();

        texture_cache = LoadRenderTexture(w, h);
        has_cache = true;

        Texture2D stone = rms->get("stone.png");

        const float tileW = stone.width  * tile_scale;
        const float tileH = stone.height * tile_scale;

        BeginTextureMode(texture_cache);
        ClearBackground(BLANK);

        for (float py = 0.0f; py < (float)h; py += tileH) {
            const float dstH = std::min(tileH, (float)h - py);
            const float srcH = dstH / tile_scale;

            for (float px = 0.0f; px < (float)w; px += tileW) {
                const float dstW = std::min(tileW, (float)w - px);
                const float srcW = dstW / tile_scale;

                Rectangle src = { 0.0f, 0.0f, srcW, srcH };
                Rectangle dst = { px, py, dstW, dstH };
                Vector2 origin = { 0.0f, 0.0f };

                DrawTexturePro(stone, src, dst, origin, 0.0f, WHITE);
            }
        }

        EndTextureMode();
    }

    inline
    void force_into(float * x_, float * y_, float hitbox_w, float hitbox_h) {
        if (*x_ < x) { *x_ = x; }
        if (*y_ < y) { *y_ = y; }
        if (*x_ + hitbox_w > x + w) {
            *x_ = x + w - hitbox_w;
        }
        if (*y_ + hitbox_h > y + h) {
            *y_ = y + h - hitbox_h;
        }
    }

    void update(void) override {
        ;
    }

    void draw(void) override {
        if (!has_cache) {
            DrawRectangle(
                x,
                y,
                w,
                h,
                RAYWHITE
            );
            return;
        }

        DrawTextureRec(
            texture_cache.texture,
            Rectangle{
                0.0f,
                0.0f,
                (float)texture_cache.texture.width,
                -(float)texture_cache.texture.height
            },
            Vector2{ (float)x, (float)y },
            WHITE
        );
    }
};

class Player : public Entity {
  public:
    float x;
    float y;
    float hitbox_w;
    float hitbox_h;
    float speed;
    float base_speed;
    float sprint_speed;
    float max_stamina;
    float stamina;
    bool is_sprinting = false;
    bool is_parrying  = false;

    static constexpr int n_blood_particles = 100;
    class BloodParticle : public Entity {
      public:
        const float G = 98.0f / 64;

        Vector2 position;
        Vector2 velocity;

        BloodParticle(void) {
            is_enabled = false;
        }

        void init_random_trajectory(void) {
            velocity.y = (rand() % 40) + 3;
            velocity.x = (rand() % (5*2)) - 5;
        }

        void update(void) override {
            if (not is_enabled) {
                return;
            }

            position.x += velocity.x;
            position.y -= velocity.y;

            velocity.y -= G;
        }

        void draw(void) override {
            if (not is_enabled) {
                return;
            }

            DrawRectangle(
                position.x,
                position.y,
                5,
                5,
                RED
            );
        }
    } blood_particles[n_blood_particles];

    void play_death_animation(void) {
        is_enabled = false;
        for (auto & p : blood_particles) {
            p.position.x = x;
            p.position.y = y;
            p.init_random_trajectory();
            p.is_enabled = true;
        }
    }

    void stop_death_animation(void) {
        is_enabled = true;
        for (auto & p : blood_particles) {
            p.is_enabled = false;
        }
    }

    Rectangle rect(void) {
        if (not is_sprinting) {
            return (Rectangle) {
                .x      = x,
                .y      = y,
                .width  = hitbox_w,
                .height = hitbox_h,
            };
        } else {
            return (Rectangle) {
                .x      = x + hitbox_w / 4,
                .y      = y + hitbox_h / 4,
                .width  = hitbox_w / 2,
                .height = hitbox_h / 2,
            };
        }
    }

    void draw_stamina_bar(void) {
        Rectangle screen_rect = get_screen_rect();
        Rectangle r = screen_rect;
        r.height = 20;
        r.width *= 0.8;

        float h = stamina / max_stamina;
        r.width *= h;

        r = ride(after(screen_rect, 1), r);
        r = balance(screen_rect, r);

        DrawRectangleRec(
            r,
            GREEN
        );
    }

    void update(void) override {
        for (auto & p : blood_particles) {
            p.update();
        }

        do {
            Timeout stamina_depleted_timeout(60);
            stamina_depleted_timeout.bump();
            const int stamina_loss = 3;
            const int stamina_gain = 1;

            if (is_sprinting
            &&  stamina_depleted_timeout.is_expired()) {
                if (stamina <= 0) {
                    is_sprinting = false;
                    speed = base_speed;
                    stamina_depleted_timeout.start(100);
                    break;
                }
                stamina -= stamina_loss;
                speed = sprint_speed;
                break;
            } else {
                if (stamina < max_stamina) {
                    stamina += stamina_gain;
                }
                speed = base_speed;
            }

            is_sprinting = false;
        } while (0);

        do {
            static Timeout timeout(60);
            timeout.bump();

            if (is_parrying
            && timeout.is_expired_once()) {
                timeout.start(1'000);
                break;
            }

            if (!timeout.is_expired()
            &&  timeout.rem() > 500) {
                is_parrying = true;
                break;
            }

            is_parrying = false;
        } while (0);
    }

    void draw(void) override {
        draw_stamina_bar();

        for (auto & p : blood_particles) {
            p.draw();
        }

        if (not is_enabled) {
            return;
        }

        Color c = is_parrying
                ? (Color){ 255, 255, 120, 255 } // yellowish overlay
                : WHITE
        ;

        Texture2D tex = rms->get("player.png");

        Rectangle src = {
            0.0f,
            0.0f,
            (float)tex.width,
            (float)tex.height
        };

        Rectangle dst = rect();

        // XXX #define ORIGO ?
        Vector2 origin = { 0.0f, 0.0f };

        DrawTexturePro(
            tex,
            src,
            dst,
            origin,
            0.0f,
            c
        );
    }
};

void trigger_endgame(void);
void face_player(void); // horrific indirection
void trigger_end_of_game(void);

class Boss : public Entity {
  public:
    float x;
    float y;
    float hitbox_w;
    float hitbox_h;
    float max_health;
    float health;
    /* Attacks come based on the Boss's health.
     * This is for attacks which should "fire only once",
     *  mostlikely doing a unique pattern by using Bullet::latency.
     */
    float health_prev;
    
    /* The boss has a head which rotates,
     *  either following the player, moving according to an attack animation or similar.
     */
    Vector2 facing;

    // i HATE C++ enums
    // HATE
    // HATE
    // HATE
    static constexpr int STAGE_I   = 0;
    static constexpr int HEALING_I = 1;
    static constexpr int STAGE_II  = 2;
    static constexpr int DEATH     = 3;
    // XXX
    int stage = STAGE_I;
    //int stage = HEALING_I;
    //int stage = STAGE_II;

    int damage_effect = 0;  // amount of tocks to perform the damage animation

    void set_facing(float x, float y) {
        facing = (Vector2) {
            .x = x,
            .y = y,
        };
    }

    void set_facing(int degrees) {
        float radians = (degrees % 360) * (M_PI / 180.0f);

        /* Account for the head not being placed at the entitys x/y
         */
        const float distance = 1000.0f;

        set_facing(
            x + std::cos(radians) * distance,
            y - std::sin(radians) * distance
        );
    }

    Rectangle rect(void) {
        return (Rectangle) {
            .x      = x,
            .y      = y,
            .width  = hitbox_w,
            .height = hitbox_h,
        };
    }

    void do_wall(int latency) {
        for (int i = 0; i < 360; i++) {
            auto & bullet = bullets.emplace_back();
            bullet.x           = x + hitbox_w / 2;
            bullet.y           = y + hitbox_h / 2;
            bullet.speed       = 2.0;
            bullet.face_degree = i;
            bullet.move_function = move_bullet_straigt;
            bullet.is_parriable = true;
            bullet.damage = 1;
            bullet.latency = latency;
        }
    }

    void update(void) override {
        bool sprinkler  = false;
        bool shotgun    = false;
        bool pistol     = false;
        bool parry_drip = true;
        bool whip       = false;
        bool sine       = false;

        if (damage_effect) {
            --damage_effect;
        }

        face_player();

      #if 1
        switch (stage) {
            case STAGE_I: {
                if (is_in_range(health, 85, 100)) {
                    shotgun = true;
                } else
                if (is_in_range(health, 70, 85)) {
                    sprinkler = true;
                } else 
                if (health == 70
                &&  health != health_prev) {
                    for (int i = 0; i < 3; i++) {
                        do_wall(i * 20);
                    }
                    parry_drip = false; // XXX doesnt work, but its fine
                } else
                if (is_in_range(health, 45, 60)) {
                    sprinkler = true;
                }
                if (is_in_range(health, 0, 45)) {
                    sprinkler = true;
                    shotgun   = true;
                }

                if (health <= 0) {
                    ++stage;
                    return;
                }
            } break;
            case HEALING_I: {
                ++health;

                damage_effect = 1;

                if (health >= max_health) {
                    health      = max_health;
                    health_prev = -1;
                    ++stage;
                    return;
                }
            } break;
            case STAGE_II: {
                parry_drip = false;

                if (health == max_health
                &&  health != health_prev) {
                    int gap = 0;
                    for (int i = 0; i < 3; i++) {
                        do_wall(i * 20 + gap);
                    }
                    gap += 120;
                    for (int i = 0; i < 3; i++) {
                        do_wall(i * 20 + gap);
                    }
                    gap += 120;
                    for (int i = 0; i < 5; i++) {
                        do_wall(i * 20 + gap);
                    }
                } else
                if (is_in_range(health, 0, 80)) {
                    whip = true;
                    pistol = true;
                    trigger_endgame();
                }
                if (health <= 0) {
                    ++stage;
                    return;
                }
            } break;
            case DEATH: {
                parry_drip = false;
                trigger_end_of_game();
            } break;
            //laser = true;
            //sine = true;
            //override_facing = true;
        }
      #endif


        if (shotgun) {
            if (!(TICK % 40)) {
                float general_directoin_degree = 150 + ((rand() % 6) * 10);

                for (int i = 0; i < 20; i++) {
                    auto & bullet = bullets.emplace_back();
                    bullet.x           = x + hitbox_w / 2;
                    bullet.y           = y + hitbox_h / 2;
                    bullet.speed       = 10.8;
                    bullet.face_degree = general_directoin_degree + (rand() % 12);
                    bullet.move_function = move_bullet_straigt;
                    bullet.is_parriable = false;

                    for (int i = rand() % 10; i > 0; i--) {
                        bullet.update();
                    }
                }
            }
        }

        if (pistol) {
            if (!(TICK % 30)) {
                auto & bullet = bullets.emplace_back();
                bullet.x           = x + hitbox_w / 2;
                bullet.y           = y + hitbox_h / 2;
                bullet.speed       = 2.0;
                bullet.face_degree = 160 + ((rand() % 30) * 2);
                bullet.move_function = move_bullet_straigt;
                bullet.is_parriable = false;
            }
        }

        if (sprinkler) {
            static int rotation = 0;

            if (!(TICK % 20)) {
                for (int i = 0; i < 8; i++) {
                    auto & bullet = bullets.emplace_back();
                    bullet.x           = x + hitbox_w / 2;
                    bullet.y           = y + hitbox_h / 2;
                    bullet.speed       = 3.0;
                    bullet.face_degree = rotation * (i % 2 ? 1 : -1) + i * 45;
                    bullet.move_function = move_bullet_straigt;
                    bullet.is_parriable = !(rand() % 10);
                }

                rotation += 10;
                if (rotation > 360) {
                    rotation = 10;
                }
            }
        }

        if (parry_drip) {
            if (!(TICK % 120)) {
                float general_directoin_degree = 170 + ((rand() % 6) * 8);

                auto & bullet = bullets.emplace_back();
                bullet.x           = x + hitbox_w / 2;
                bullet.y           = y + hitbox_h / 2;
                bullet.speed       = 4.0;
                bullet.face_degree = general_directoin_degree;
                bullet.move_function = move_bullet_straigt;
                bullet.is_parriable = true;
            }
        }

        if (whip) {
            static int iterih      = 0;
            const int iterih_delta = 3;

            iterih -= iterih_delta;

            auto degree = 90 - iterih;

            auto & bullet = bullets.emplace_back();
            bullet.x           = x + hitbox_w / 2;
            bullet.y           = y + hitbox_h / 2;
            bullet.speed       = 120.0;
            bullet.face_degree = degree;
            bullet.move_function = move_bullet_straigt;
            bullet.is_parriable = false;
            bullet.visual_type  = Bullet::RED_BULLET;

            set_facing(degree);
        }

        if (sine) {
            if (!(TICK % 15)) {
                float degree = 150 + ((rand() % 12) * 5);

                for (int i = 0; i < 2; i++) {
                    auto & bullet = bullets.emplace_back();
                    bullet.x           = x + hitbox_w / 2;
                    bullet.y           = y + hitbox_h / 2;
                    bullet.speed       = 4.0;
                    bullet.face_degree = degree;
                    bullet.is_parriable = false;
                    bullet.visual_type  = Bullet::YELLOW_BULLET;
                    bullet.move_function = move_bullet_sine;
                    bullet.sine_phase     = M_PI * 0.25 * (rand() % 8);
                    bullet.sine_amplitude = 12;
                    bullet.sine_sign      = i == 1 ? i : -1;
                }
            }
        }

        health_prev = health;
    }

    void draw_health_bar(void) {
        Rectangle screen_rect = get_screen_rect();
        Rectangle r = screen_rect;
        r.height = 20;
        r.width *= 0.8;

        if (damage_effect) {
            Rectangle b = r;
            b = hang(screen_rect, b);
            b = balance(screen_rect, b);
            DrawRectangleRec(
                b,
                WHITE
            );
        }

        float h = health / max_health;
        r.width *= h;

        r = hang(screen_rect, r);
        r = balance(screen_rect, r);

        DrawRectangleRec(
            r,
            RED
        );
    }

    void draw(void) override {
        draw_health_bar();

        constexpr float scale = 2.4;

        do {
            DrawTextureEx(
                rms->get("am_body.png"),
                (Vector2) {
                    .x = x,
                    .y = y,
                },
                0,
                scale,
                WHITE
            );
        } while (0);

        do {
            auto tex = rms->get("am_head.png");

            // XXX is there no to_bounding_rect helper?
            Rectangle src = {
                0,
                0,
                (float)tex.width,
                (float)tex.height,
            };

            Rectangle dst = {
                x + 40,
                y,
                tex.width  * scale,
                tex.height * scale,
            };

            Vector2 pivot = {
                tex.width  * scale * 0.85f,
                tex.height * scale / 2.0f,
            };

            Vector2 direction = {
                facing.x - dst.x + pivot.x,
                facing.y - dst.y + pivot.y
            };

            float rotation = atan2f(direction.y, direction.x) * RAD2DEG + 180.0f;

            DrawTexturePro(
                tex,
                src,
                dst,
                pivot,
                rotation,
                WHITE
            );
        } while (0);
    }
};

#include "Explosion_effect.hpp"
#include "tutorial_classes.hpp"

vector<Entity*> entities;

Room   * room   = nullptr;
Player * player = nullptr;
Boss   * boss   = nullptr;
Ball   * ball   = nullptr;
Wall   * wall   = nullptr;
StaticTextureCollection * dead_bodies = nullptr;

/* Utter fucking retardmaxxing, horrific architecture,
 *  scope hacking and sanity violation.
 * At the same time this is the last thing I need to implement, so I dont care.
 */
void trigger_endgame(void) {
    if (ball->is_enabled) {
        return;
    }

    ball->is_enabled   = true;
    ball->position.x   = 100;
    ball->position.y   = H / 2;
    ball->is_homing    = false;
    ball->homing_point = (Vector2) {
        .x = boss->x,
        .y = boss->y + boss->hitbox_h / 2,
    };
    ball->speed = 3.3;

    entities.push_back(ball);
}

bool do_not_fucking_reset_the_player_because_the_ending_scene_is_playing_and_they_have_already_won = false;
void trigger_end_of_game(void) {
    entities.clear();

    Explosion_effect * e = new Explosion_effect;
    e->x = boss->x;
    e->y = boss->y;
    entities.push_back(e);

    do_not_fucking_reset_the_player_because_the_ending_scene_is_playing_and_they_have_already_won = true;
}

void face_player(void) {
    boss->set_facing(player->x, player->y);
}

bool is_player_alive = true;
int death_counter = 0;

#include "scenes.hpp"

signed main(void) {
    srand(0);
    InitWindow(W, H, "Torment Nexus I");
    SetTargetFPS(TICKS_PER_SEC);
    HideCursor();

    rms = new resource_manager<Texture2D>(
        LoadTexture,
        "default.png",
        +[](Texture2D t) -> bool {
            return !(t.id == 0);
        },
        UnloadTexture
    );

    SetWindowIcon(LoadImageFromTexture(rms->get("player.png")));

    room   = new Room;
    player = new Player;
    boss   = new Boss;
    ball   = new Ball;
    wall   = new Wall;
    dead_bodies = new StaticTextureCollection;

    Timeout deadringer(60); // brief delay before respawn for death animation

    init_game(scene_e::TUTORIAL);

    while (!WindowShouldClose()) {
        // Input
        do {
            float dx = 0.0f;
            float dy = 0.0f;

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) { dy -= 1.0f; }
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) { dy += 1.0f; }
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) { dx -= 1.0f; }
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { dx += 1.0f; }

            float len = sqrtf(dx * dx + dy * dy);
            if (len > 0.0f) {
                dx /= len;
                dy /= len;
            }

            player->x += dx * player->speed;
            player->y += dy * player->speed;

            room->force_into(
                &player->x,
                &player->y,
                player->hitbox_w,
                player->hitbox_h
            );
        } while (0);

        do {
            if (IsKeyPressed(KEY_F2)) {
                save_screenshot_with_timestamp();
            }
        } while (0);

        player->is_parrying  = IsKeyDown(KEY_SPACE);
        player->is_sprinting = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        
        // Update
        for (auto && e : entities) {
            e->update();
        }

        for (auto it = bullets.mut_begin(); it != bullets.mut_end(); ++it) {
            it->update();

            if (not it->is_parried
            &&  CheckCollisionRecs(it->rect(), player->rect())) {
                if (it->is_parriable
                &&  player->is_parrying) {
                    it->parry();
                } else {
                    is_player_alive = false;
                }
            } else
            if (it->is_parried
            &&  CheckCollisionRecs(it->rect(), boss->rect())) {
                boss->health -= it->damage;
                boss->damage_effect = 10;
                it.kill_current();
            } else
            if (not it->is_on_screen()) {
                it.kill_current();
            }
        }

        if (boss->is_enabled) {
            if (CheckCollisionRecs(boss->rect(), player->rect())) {
                is_player_alive = false;
            }

            if (CheckCollisionRecs(boss->rect(), ball->rect())) {
                boss->health = -1000;
            }
        }

        if (ball->is_enabled) {
            if (player->is_parrying
            &&  CheckCollisionRecs(player->rect(), ball->rect())) {
                ball->bump();
            }
        }

        if (wall->is_enabled) {
            if (CheckCollisionRecs(ball->rect(), wall->rect())) {
                ball->is_enabled = false;
                wall->is_enabled = false;
            }

            if (player->x + player->hitbox_w > wall->x) {
                player->x = wall->x - player->hitbox_w;
            }
        }

        if (player->x >= W) {
            init_game(scene_e::BOSSFIGHT);
        }

        if (not is_player_alive
        &&  not do_not_fucking_reset_the_player_because_the_ending_scene_is_playing_and_they_have_already_won) {
            if (not deadringer.is_primed) {
                player->play_death_animation();

                ++death_counter;

                auto & t = dead_bodies->static_textures.emplace_back();
                t.key = "dead.png";
                t.x   = player->x;
                t.y   = player->y;

                deadringer.start(700);
            }

            if (deadringer.is_primed
            &&  deadringer.is_expired_once()) {
                player->stop_death_animation();
                init_game(scene_e::WALKBACK);
            }

            if (deadringer.is_primed) {
                deadringer.bump();
            }
        }

        ++TICK;

        // Draw
      BeginDrawing();
        ClearBackground(BLACK);
        for (auto && e : entities) {
            e->draw();
        }

        for (auto && b : bullets) {
            b.draw();
        }

        //DrawText(
        //    TextFormat("%d", GetFPS()),
        //    5,
        //    5,
        //    64,
        //    WHITE
        //);
        
        //DrawRectangle(
        //    0,
        //    0,
        //    W,
        //    H,
        //    (Color){ 255, 0, 0, 140 }
        //);
      EndDrawing();
    }

    CloseWindow();
    return 0;
}
