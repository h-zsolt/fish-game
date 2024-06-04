#pragma once
#include <Engine/OGLGame.h>
#include <string>

/**
 *  An OpenGL Game based on ASGE.
 */
enum
{
  MAX_FISHCOUNT = 20,
  FISH_TYPE_COUNT = 8,
  DIFFICULTY_BRACKET_COUNT = 10,
  DIFFICULTY1 = 3,
  DIFFICULTY2 = 10,
  DIFFICULTY3 = 25,
  DIFFICULTY4 = 60,
  DIFFICULTY5 = 100,
  DIFFICULTY6 = 200,
  DIFFICULTY7 = 350,
  DIFFICULTY8 = 500,
  DIFFICULTY9 = 750,
  DIFFICULTY10 = 1000
};

class MyASGEGame : public ASGE::OGLGame
{
 public:
  MyASGEGame();

  ~MyASGEGame() final;

  bool init() override;

 private:
  void keyHandler(ASGE::SharedEventData data);

  void clickHandler(ASGE::SharedEventData data);

  void setupResolution();

  void update(const ASGE::GameTime&) override;

  void render(const ASGE::GameTime&) override;

  bool isInside(const ASGE::Sprite* sprite, float x, float y) const;

  int key_callback_id = -1;   /**< Key Input Callback ID. */
  int mouse_callback_id = -1; /**< Mouse Input Callback ID. */
  bool in_menu = true;
  int menu_option = 0;
  int fish_count = 0;
  int score = 0;
  int difficulty_state = 0;
  int difficulty_limits[DIFFICULTY_BRACKET_COUNT] = {
    DIFFICULTY1, DIFFICULTY2, DIFFICULTY3, DIFFICULTY4, DIFFICULTY5,
    DIFFICULTY6, DIFFICULTY7, DIFFICULTY8, DIFFICULTY9, DIFFICULTY10
  };
  int fish_pool[FISH_TYPE_COUNT] = { 0 };
  int gamemode = 0;
  float life = 0;
  std::string welcome = "Would you like to start the game?";
  std::string score_fluff = "Score: ";

  class Clownfishes
  {
   public:
    float xPos = 0;
    float yPos = 0;
    int fish_size = 32;
    float speed = 100;
    float angle = 1;
    bool y_negative = false;
    bool x_negative = false;
    float state_progress = 0;
    int state_goal = 0;
    int score_value = 0;
    int type = 0;
  };
  Clownfishes fishes[MAX_FISHCOUNT];
  void createFish(int type, int target);
  int fishChoice(int type_lost, bool chance_to_stay);
  void difficultyCalculation();
  void fishPoolConstructor(int difficulty_progress);
  void fishSpecialAbility(int type, int target);
  void fishFlipper(int target);

  // art assets for the game
  bool initBackground();
  ASGE::Sprite* background = nullptr;

  bool initClownfish();
  ASGE::Sprite* clownfish[MAX_FISHCOUNT] = { nullptr };
  // ASGE::Sprite *clownfish = nullptr;

  bool initLifeBar();
  ASGE::Sprite* life_bar = nullptr;

  void backToMenu();
  void updateFishLocation(const ASGE::GameTime& game_time, int i);
  int menuLocationX(int menu_order, int text_length);
  void gameStateInit();
};
