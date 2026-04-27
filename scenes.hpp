#ifndef SCENES
#   define SCENES
#else
#   error "Unity build is broken."
#endif

enum class scene_e {
    TUTORIAL,
    BOSSFIGHT,
    WALKBACK,
} scene;

void init_tutorial(void) {
    scene = scene_e::TUTORIAL;

    boss->is_enabled = false;
    ball->is_enabled = true;

    entities.push_back(ball);

    do {
        player->x = 100;
        player->y = 250;
    } while (0);

    do {
        room->x = 50;
        room->y = 200;
        room->w = W;
        room->h = H - 400;
        room->cache();
    } while (0);

    do {
        wall->x = 50 + W - 300;
        wall->y = 200;
        wall->w = 100;
        wall->h = room->h;
        entities.push_back(wall);

        ball->position.x = 500;
        ball->position.y = 400;
        ball->size = 50;
        ball->homing_point.x = wall->x + (wall->w / 2);
        ball->homing_point.y = wall->y + (wall->h / 2) - (ball->size / 2);
        entities.push_back(ball);
    } while (0);

    do {
        auto t = arena_new<StaticTexture>();
        t->key = "wasd.png";
        t->x = 100;
        t->y = 450;
        entities.push_back(t);
    } while (0);

    do {
        auto t = arena_new<StaticTexture>();
        t->key = "torment-nexus.png";
        t->x = 201;
        t->y = 20;
        entities.push_back(t);
    } while (0);
}

void init_walkback(void) {
    scene = scene_e::WALKBACK;

    boss->is_enabled = false;

    do {
        player->x = 450 + death_counter * 10;
        player->y = 250;
        if (player->x > W - 100) {
            player->x = W - 100;
        }
    } while (0);

    do {
        room->x = 50;
        room->y = 200;
        room->w = W;
        room->h = H - 400;
        room->cache();
    } while (0);

    do {
        auto t = arena_new<StaticTexture>();
        t->key = "torment-nexus.png";
        t->x = 201;
        t->y = 20;
        entities.push_back(t);
    } while (0);

    do {
        static char death_counter_roman[128]; // roman.c uses the magick minimum requirement of 81
        auto t = arena_new<StaticText>();
        t->text = decimalToRoman(death_counter, death_counter_roman);
        t->scale = 50;
        t->x = 75;
        t->y = H - 100;
        t->c = RED;
        entities.push_back(t);
    } while (0);
}

void init_bossfight(void) {
    scene = scene_e::BOSSFIGHT;

    boss->is_enabled = true;
    ball->is_enabled = false;

    boss->stage = Boss::STAGE_I;

    entities.push_back(dead_bodies);
    entities.push_back(boss);

    do {
        player->x = 100;
    } while (0);

    do {
        room->x = 50;
        room->y = 50;
        room->w = W - 100;
        room->h = H - 100;
        room->cache();
    } while (0);

    do {
        boss->x          = 1000;
        boss->y          = 300;
        boss->hitbox_w   = 100;
        boss->hitbox_h   = 100;
        boss->max_health = 100;
        boss->health     = boss->max_health;
    } while (0);

    do {
        auto t = arena_new<StaticText>();
        t->scale = 20;
        t->text  = "AM";
        t->x     = W / 2 - MeasureText(t->text, t->scale) / 2;
        t->y     = 1;
        entities.push_back(t);
    } while (0);
}

void init_game(scene_e scene) {
    TICK = 0;

    global_arena.release();

    player->is_enabled = true;
    is_player_alive = true;

    bullets.clear();
    entities.clear();

    room->destroy_cache();
    entities.push_back(room);

    boss->health_prev = -1;

    switch (scene) {
        case scene_e::TUTORIAL:  init_tutorial();  break;
        case scene_e::BOSSFIGHT: init_bossfight(); break;
        case scene_e::WALKBACK:  init_walkback();  break;
    }

    do {
        player->hitbox_w     = 27; //- 5;
        player->hitbox_h     = 24; //- 5;
        player->max_stamina  = 120;
        player->stamina      = 100;
        player->base_speed   = 3.0;
        player->sprint_speed = 9.0;
    } while (0);
    entities.push_back(player);
}
