#ifndef EXPLOSION
#   define EXPLOSION
#else
#   error "Unity build is broken."
#endif

#include <array>
#include <cmath>
#include <algorithm>

class Explosion_effect : public Entity {
  public:
    struct particle_t {
        Vector2 pos;
        Vector2 start_pos;
        Vector2 drift;
        float radius;
        float start_radius;
        float peak_radius;
        float life;
        float visible_time;
        float hidden_time;
        float phase_offset;
        Color color;
        bool active;
        bool from_center;
    };

    int x;
    int y;
    bool is_finished = false;

    std::array<particle_t, 30> particles{};

    bool initialized = false;
    float time_alive = 0.0f;

    float circle_delay_sec = 0.08f;
    float circle_radius = 0.0f;
    float circle_speed_px_s = 260.0f;
    float screen_cover_radius = 0.0f;

    int center_particle_count = 18;
    int screen_particle_count = 12;

    void update(void) override {
        if (!initialized) {
            initialized = true;
            screen_cover_radius = compute_screen_cover_radius();
            init_particles();
        }

        if (is_finished) {
            raise(SIGSEGV); // L:
            return;
        }

        float dt = GetFrameTime();
        time_alive += dt;

        for (int i = 0; i < (int)particles.size(); ++i) {
            particle_t &p = particles[i];

            p.life += dt;

            if (p.active) {
                float t = p.life / p.visible_time;
                if (t >= 1.0f) {
                    p.active = false;
                    p.life = 0.0f;
                    p.radius = 0.0f;
                } else {
                    float grow_t = t * t;
                    p.radius = p.start_radius + (p.peak_radius - p.start_radius) * grow_t;

                    p.pos.x += p.drift.x * dt;
                    p.pos.y += p.drift.y * dt;
                }
            } else {
                if (p.life >= p.hidden_time) {
                    respawn_particle(i);
                }
            }
        }

        if (time_alive >= circle_delay_sec) {
            circle_radius += circle_speed_px_s * dt;

            if (circle_radius >= screen_cover_radius) {
                circle_radius = screen_cover_radius;
                is_finished = true;
            }
        }
    }

    void draw(void) override {
        if (!initialized) {
            return;
        }

        for (const particle_t &p : particles) {
            if (!p.active) {
                continue;
            }

            DrawCircleV(p.pos, p.radius, p.color);
        }

        if (time_alive >= circle_delay_sec) {
            DrawCircleV(Vector2{(float)x, (float)y}, circle_radius, WHITE);
        }
    }

    void init_particles() {
        for (int i = 0; i < (int)particles.size(); ++i) {
            respawn_particle(i);
        }
    }

    void respawn_particle(int i) {
        particle_t &p = particles[i];

        p.from_center = (i < center_particle_count);

        Vector2 spawn_pos;
        Vector2 drift_dir;

        if (p.from_center) {
            float angle = random_float(0.0f, 2.0f * PI);
            float dir_x = std::cos(angle);
            float dir_y = std::sin(angle);

            float spread = random_float(0.0f, 14.0f);
            spawn_pos = Vector2{
                (float)x + dir_x * spread,
                (float)y + dir_y * spread
            };

            drift_dir = Vector2{dir_x, dir_y};
        } else {
            spawn_pos = Vector2{
                random_float(0.0f, (float)W),
                random_float(0.0f, (float)H)
            };

            float angle = random_float(0.0f, 2.0f * PI);
            drift_dir = Vector2{std::cos(angle), std::sin(angle)};
        }

        float drift_speed = p.from_center
            ? random_float(12.0f, 45.0f)
            : random_float(6.0f, 24.0f);

        p.start_pos = spawn_pos;
        p.pos = spawn_pos;
        p.drift = Vector2{
            drift_dir.x * drift_speed,
            drift_dir.y * drift_speed
        };

        p.start_radius = random_float(3.0f, 7.0f);
        p.peak_radius = random_float(12.0f, 24.0f);
        p.radius = p.start_radius;

        p.visible_time = random_float(0.05f, 0.14f);
        p.hidden_time = random_float(0.03f, 0.10f);
        p.phase_offset = random_float(0.0f, 1.0f);

        p.color = random_particle_color();
        p.life = 0.0f;
        p.active = true;

        if (!p.from_center) {
            p.pos.x += random_float(-3.0f, 3.0f);
            p.pos.y += random_float(-3.0f, 3.0f);
        }
    }

    float compute_screen_cover_radius() const {
        float dx = std::max((float)x, (float)(W - x));
        float dy = std::max((float)y, (float)(H - y));
        return std::sqrt(dx * dx + dy * dy) + 24.0f;
    }

    static float random_float(float min_v, float max_v) {
        float t = (float)GetRandomValue(0, 10000) / 10000.0f;
        return min_v + (max_v - min_v) * t;
    }

    static Color random_particle_color() {
        switch (GetRandomValue(0, 4)) {
            case 0: return YELLOW;
            case 1: return ORANGE;
            case 2: return RED;
            case 3: return DARKGRAY;
            default: return BLACK;
        }
    }
};
