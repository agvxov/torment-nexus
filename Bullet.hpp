#ifndef BULLETS
#   define BULLETS
#else
#   error "Unity build is broken."
#endif

class Bullet {
  public:
    float x;
    float y;
    float speed;
    float face_degree;
    bool is_parriable;
    bool is_parried = false;
    int damage = 5;
    int latency = 0;

    float sine_phase;
    float sine_amplitude;
    float sine_sign;

    enum {
        BLUE_BULLET,
        YELLOW_BULLET,
        RED_BULLET,
    } visual_type = BLUE_BULLET;

    static constexpr int size = 20;

    typedef void (*move_function_t)(Bullet*);
    move_function_t move_function = nullptr;

    bool is_on_screen(void) {
        return x > 0
            && y > 0
            && x < W
            && y < H
        ;
    }

    Rectangle rect(void) {
        return (Rectangle) {
            .x      = x,
            .y      = y,
            .width  = size,
            .height = size,
        };
    }

    void flip(void) {
        face_degree += 180;
    }

    void parry(void) {
        flip();
        is_parried = true;
    }

    void update(void) {
        if (latency) {
            --latency;
            return;
        }

        if (move_function) {
            move_function(this);
        }
    }

    void draw(void) {
        if (latency) {
            return;
        }

        Texture2D t;
        do {
            if (is_parriable) {
                t = rms->get("bullet-2.png");
                break;
            }
            switch (visual_type) {
                case BLUE_BULLET:   t = rms->get("bullet-1.png"); break;
                case YELLOW_BULLET: t = rms->get("bullet-5.png"); break;
                case RED_BULLET:    t = rms->get("bullet-4.png"); break;
            }
        } while (0);

        DrawTexture(
            t,
            x,
            y,
            WHITE
        );
    }
};

void move_bullet_straigt(Bullet * b) {
    float rad = (360 - b->face_degree) * DEG2RAD;

    float dx = cosf(rad);
    float dy = sinf(rad);

    b->x += dx * b->speed;
    b->y += dy * b->speed;
}

void move_bullet_sine(Bullet * b) {
    float rad = (360.0f - b->face_degree) * DEG2RAD;

    float forward_x = cosf(rad);
    float forward_y = sinf(rad);

    float side_x = -forward_y * b->sine_sign;
    float side_y =  forward_x * b->sine_sign;

    /* Do NOT touch this! It breaks everything.
     * This proves that even if I used to have elementary knowledge about math
     *  -which I'm no longer sure about- I have forgotten it all regardless.
     */
    b->sine_phase += b->speed * 0.01;

    float wobble = sinf(b->sine_phase) * b->sine_amplitude;

    b->x += forward_x * b->speed + side_x * wobble;
    b->y += forward_y * b->speed + side_y * wobble;
}

/* The dense object container has the side effect of modifying the draw order.
 * The resulting visual artifact is technically a bug,
 *  but I'm leaving it it because I think it looks kinda neet.
 */
DenseObjectPool<Bullet> bullets;
