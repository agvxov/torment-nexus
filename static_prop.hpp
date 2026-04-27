#ifndef STATIC_PROP
#   define STATIC_PROP
#else
#   error "Unity build is broken."
#endif

class StaticTexture : public Entity {
  public:
    float x;
    float y;
    float scale = 1.0;
    const char * key;

    void draw(void) override {
        DrawTextureEx(
            rms->get(key),
            (Vector2) {
                .x = x,
                .y = y,
            },
            0,
            scale,
            RAYWHITE
        );
    }

    virtual ~StaticTexture() = default;
};

class StaticText : public Entity {
  public:
    float x;
    float y;
    float scale;
    const char * text;
    Color c = WHITE;

    void draw(void) override {
        DrawText(
            text,
            x,
            y,
            scale,
            c
        );
    }

    virtual ~StaticText() = default;
};

class StaticTextureCollection : public Entity {
  public:
    vector<StaticTexture> static_textures;

    void draw(void) override {
        for (auto && e : static_textures) {
            e.draw();
        }
    }
};
