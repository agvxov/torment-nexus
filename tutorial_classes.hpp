#ifndef TUTORIAL_CLASSES
#   define TUTORIAL_CLASSES
#else
#   error "Unity build is broken."
#endif

class Ball : public Entity {
  public:
    Vector2 position;
    float size;

    bool is_homing = false;
    Vector2 homing_point;

    float speed = 3.0f;

    Rectangle rect(void) {
        if (not is_enabled) {
            return (Rectangle) {
                .x      = 0,
                .y      = 0,
                .width  = 0,
                .height = 0,
            };
        }

        return (Rectangle) {
            .x      = position.x,
            .y      = position.y,
            .width  = size,
            .height = size,
        };
    }

    void bump(void) {
        is_homing = true;
    }

    void update(void) override {
        if (!is_homing) {
            return;
        }

        Vector2 delta = {
            homing_point.x - position.x,
            homing_point.y - position.y
        };

        float dist = sqrtf(delta.x * delta.x + delta.y * delta.y);

        float inv = speed / dist;
        position.x += delta.x * inv;
        position.y += delta.y * inv;
    }

    void draw(void) override {
        if (not is_enabled) {
            return;
        }

        DrawTextureEx(
            rms->get("bullet-3.png"),
            position,
            0,
            3.0f,
            WHITE
        );
    }
};

class Wall : public Entity {
  public:
    float x;
    float y;
    float w;
    float h;

    Rectangle rect(void) {
        return (Rectangle) {
            .x      = x,
            .y      = y,
            .width  = w,
            .height = h,
        };
    }

    void update(void) override {
        ;
    }

    void draw(void) override {
        if (not is_enabled) {
            return;
        }

        DrawRectangle(
            x,
            y,
            w,
            h,
            hex2color(0x6e6e6e)
        );
    }
};
