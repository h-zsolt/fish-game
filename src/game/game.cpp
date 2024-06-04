#include <cstdlib>
#include <ctime>
#include <string>

#include <Engine/DebugPrinter.h>
#include <Engine/Input.h>
#include <Engine/InputEvents.h>
#include <Engine/Keys.h>
#include <Engine/Sprite.h>

#include "game.h"

enum
{
  WINDOWX = 1280,
  WINDOWY = 720,
  DISTANCE_BETWEEN_CHOICES = 120,
  AVERAGE_FONT_LENGTH = 5,
  CLOWNFISH_STANDARD_SIZE_MIN = 50,
  CLOWNFISH_STANDARD_SIZE_MAX = 64,
  CLOWNFISH_SMALL_SIZE_MIN = 40,
  CLOWNFISH_SMALL_SIZE_MAX = 48,
  CLOWNFISH_TINY_SIZE_MIN = 32,
  CLOWNFISH_TINY_SIZE_MAX = 38,
  MENU_MIN = 0,
  MENU_MAX = 2,
  STANDARD_SPEED_MIN = 100,
  STANDARD_SPEED_MAX = 200,
  FAST_SPEED_MIN = 250,
  FAST_SPEED_MAX = 400,
  FASTER_SPEED_MIN = 600,
  FASTER_SPEED_MAX = 800,
  STANDARD_FISH = 0,
  FAST_FISH = 1,
  ANGLED_FISH = 2,
  FAST_ANGLED_FISH = 3,
  FASTER_FISH = 4,
  SLIPPERY_FISH = 5,
  TURNING_FISH = 6,
  ULTIMATE_FISH = 7,
  SCORE_Y_LOCATION = 40,
  SPECIAL_POWER_GAIN = 200,
  LIFE_LOSS = 10
};

/**
 *   @brief   Default Constructor.
 *   @details Consider setting the game's width and height
 *            and even seeding the random number generator.
 */

MyASGEGame::MyASGEGame()
{
  game_name = "Not a Nemo game by Csongor-Zsolt Horosnyi";
}

/**
 *   @brief   Destructor.
 *   @details Remove any non-managed memory and callbacks.
 */

MyASGEGame::~MyASGEGame()
{
  this->inputs->unregisterCallback(static_cast<unsigned int>(key_callback_id));

  this->inputs->unregisterCallback(
    static_cast<unsigned int>(mouse_callback_id));
}

/**
 *   @brief   Initialises the game.
 *   @details The game window is created and all assets required to
 *            run the game are loaded. The keyHandler and clickHandler
 *            callback should also be set in the initialise function.
 *   @return  True if the game initialised correctly.
 */

bool MyASGEGame::init()
{
  setupResolution();
  if (!initAPI())
  {
    return false;
  }

  toggleFPS();

  // input handling functions
  inputs->use_threads = false;

  renderer->setSpriteMode(ASGE::SpriteSortMode::IMMEDIATE);

  key_callback_id =
    inputs->addCallbackFnc(ASGE::E_KEY, &MyASGEGame::keyHandler, this);

  mouse_callback_id = inputs->addCallbackFnc(
    ASGE::E_MOUSE_CLICK, &MyASGEGame::clickHandler, this);

  if (initBackground())
  {
    ASGE::DebugPrinter{} << "init::Background init success" << std::endl;
  }
  else
    return false;
  if (initClownfish())
  {
    ASGE::DebugPrinter{} << "init::Clownfish init success" << std::endl;
  }
  else
    return false;
  if (initLifeBar())
  {
    ASGE::DebugPrinter{} << "init::Life_bar init success" << std::endl;
  }
  else
    return false;
  life_bar->yPos(WINDOWY - 20);
  std::srand(std::time(0));
  gameStateInit();
  return true;
}

void MyASGEGame::gameStateInit()
{
  fish_count = 2;
  difficulty_state = 0;
  createFish(STANDARD_FISH, 0);
  createFish(STANDARD_FISH, 1);
  score = 0;
}

/**
 *   @brief   Picks a fish to spawn
 *   @details Builds a new pool of fish based on difficulty after
 *            checking difficulty. Then adds a 25% chance to re-spawn
 *            the fish that was just clicked, if it receives a
 *            true boolean value.
 *   @return  The ID of the fish that was chosen
 */

int MyASGEGame::fishChoice(int type_lost, bool chance_to_stay)
{
  int fish_to_spawn = -1;
  int random_counter = std::rand();

  difficultyCalculation();
  fishPoolConstructor(difficulty_state);

  if (chance_to_stay)
  {
    fish_pool[type_lost] += 25;
    random_counter = random_counter % 100;
    random_counter++;
  }
  else
  {
    random_counter = random_counter % 75;
    random_counter++;
  }

  for (int i = 0; i < FISH_TYPE_COUNT && fish_to_spawn == -1; i++)
  {
    if (fish_pool[i] >= random_counter)
    {
      fish_to_spawn = i;
    }
    random_counter -= fish_pool[i];
  }
  return fish_to_spawn;
}

/**
 *   @brief   Checks Score against the difficulty gate
 *   @details Checks if the score surpassed the current difficulty gate
 *            then adds a new fish and raises difficulty if it was.
 */

void MyASGEGame::difficultyCalculation()
{
  if (difficulty_state < DIFFICULTY_BRACKET_COUNT &&
      score >= difficulty_limits[difficulty_state])
  {
    difficulty_state++;
    fish_count++;
    createFish(fishChoice(0, false), fish_count - 1);
  }
}

/**
 *   @brief   Rebuilds the fish_pool based on current difficulty
 *   @details Chooses which types of fish have a chance to spawn.
 *            The total in a pool should add up to 75.
 */

void MyASGEGame::fishPoolConstructor(int difficulty_progress)
{
  switch (difficulty_progress)
  {
    case 0:
      fish_pool[0] = 75; // Standard
      fish_pool[1] = 0;  // Fast
      fish_pool[2] = 0;  // Angled
      fish_pool[3] = 0;  // Angled Fast
      fish_pool[4] = 0;  // Faster
      fish_pool[5] = 0;  // Slippery
      fish_pool[6] = 0;  // Turning
      fish_pool[7] = 0;  // Ultimate
      break;
    case 1:
      fish_pool[0] = 50;
      fish_pool[1] = 25;
      fish_pool[2] = 0;
      fish_pool[3] = 0;
      fish_pool[4] = 0;
      fish_pool[5] = 0;
      fish_pool[6] = 0;
      fish_pool[7] = 0;
      break;
    case 2:
      fish_pool[0] = 20;
      fish_pool[1] = 45;
      fish_pool[2] = 10;
      fish_pool[3] = 0;
      fish_pool[4] = 0;
      fish_pool[5] = 0;
      fish_pool[6] = 0;
      fish_pool[7] = 0;
      break;
    case 3:
      fish_pool[0] = 0;
      fish_pool[1] = 30;
      fish_pool[2] = 30;
      fish_pool[3] = 15;
      fish_pool[4] = 0;
      fish_pool[5] = 0;
      fish_pool[6] = 0;
      fish_pool[7] = 0;
      break;
    case 4:
      fish_pool[0] = 0;
      fish_pool[1] = 0;
      fish_pool[2] = 10;
      fish_pool[3] = 35;
      fish_pool[4] = 20;
      fish_pool[5] = 10;
      fish_pool[6] = 0;
      fish_pool[7] = 0;
      break;
    case 5:
      fish_pool[0] = 0;
      fish_pool[1] = 0;
      fish_pool[2] = 0;
      fish_pool[3] = 20;
      fish_pool[4] = 10;
      fish_pool[5] = 35;
      fish_pool[6] = 10;
      fish_pool[7] = 0;
      break;
    case 6:
      fish_pool[0] = 0;
      fish_pool[1] = 0;
      fish_pool[2] = 0;
      fish_pool[3] = 20;
      fish_pool[4] = 10;
      fish_pool[5] = 20;
      fish_pool[6] = 25;
      fish_pool[7] = 0;
      break;
    case 7:
      fish_pool[0] = 0;
      fish_pool[1] = 0;
      fish_pool[2] = 0;
      fish_pool[3] = 20;
      fish_pool[4] = 0;
      fish_pool[5] = 25;
      fish_pool[6] = 25;
      fish_pool[7] = 5;
      break;
    case 8:
      fish_pool[0] = 0;
      fish_pool[1] = 0;
      fish_pool[2] = 0;
      fish_pool[3] = 20;
      fish_pool[4] = 0;
      fish_pool[5] = 20;
      fish_pool[6] = 20;
      fish_pool[7] = 15;
      break;
    case 9:
      fish_pool[0] = 0;
      fish_pool[1] = 0;
      fish_pool[2] = 0;
      fish_pool[3] = 10;
      fish_pool[4] = 0;
      fish_pool[5] = 5;
      fish_pool[6] = 10;
      fish_pool[7] = 50;
      break;
    case 10:
      fish_pool[0] = 0;
      fish_pool[1] = 0;
      fish_pool[2] = 0;
      fish_pool[3] = 0;
      fish_pool[4] = 0;
      fish_pool[5] = 0;
      fish_pool[6] = 0;
      fish_pool[7] = 75;
      break;
    default:
      fish_pool[0] = 0;
      fish_pool[1] = 0;
      fish_pool[2] = 0;
      fish_pool[3] = 0;
      fish_pool[4] = 0;
      fish_pool[5] = 0;
      fish_pool[6] = 0;
      fish_pool[7] = 75;
      break;
  }
}

/**
 *   @brief   Creates a new fish with randomized attributes and location
 *   @details It spawns a new fish based off of the type it is told to create
 *            and it may replace a fish if the target number is already existing
 */

void MyASGEGame::createFish(int type, int target)
{
  switch (type)
  {
    case STANDARD_FISH:
      fishes[target].fish_size = CLOWNFISH_STANDARD_SIZE_MIN +
                                 (std::rand() % (CLOWNFISH_STANDARD_SIZE_MAX -
                                                 CLOWNFISH_STANDARD_SIZE_MIN));
      clownfish[target]->height(fishes[target].fish_size);
      clownfish[target]->width(fishes[target].fish_size);
      fishes[target].speed =
        STANDARD_SPEED_MIN +
        (std::rand() % (STANDARD_SPEED_MAX - STANDARD_SPEED_MIN));
      fishes[target].angle = 1;
      fishes[target].x_negative = std::rand() % 2;
      fishes[target].y_negative = false;
      fishes[target].score_value = 1;
      fishes[target].xPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWX - fishes[target].fish_size);
      fishes[target].yPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWY - fishes[target].fish_size);
      fishes[target].state_goal = 1000;
      fishes[target].state_progress = 0;
      fishes[target].type = STANDARD_FISH;
      fishFlipper(target);
      clownfish[target]->colour(ASGE::COLOURS::WHITE);
      break;
    case FAST_FISH:
      fishes[target].fish_size =
        CLOWNFISH_SMALL_SIZE_MIN +
        (std::rand() % (CLOWNFISH_SMALL_SIZE_MAX - CLOWNFISH_SMALL_SIZE_MIN));
      clownfish[target]->height(fishes[target].fish_size);
      clownfish[target]->width(fishes[target].fish_size);
      fishes[target].speed =
        FAST_SPEED_MIN + (std::rand() % (FAST_SPEED_MAX - FAST_SPEED_MIN));
      fishes[target].angle = 1;
      fishes[target].x_negative = std::rand() % 2;
      fishes[target].y_negative = false;
      fishes[target].score_value = 3;
      fishes[target].xPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWX - fishes[target].fish_size);
      fishes[target].yPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWY - fishes[target].fish_size);
      fishes[target].state_goal = 1000;
      fishes[target].state_progress = 0;
      fishes[target].type = FAST_FISH;
      fishFlipper(target);
      clownfish[target]->colour(ASGE::COLOURS::WHITE);
      break;
    case ANGLED_FISH:
      fishes[target].fish_size =
        CLOWNFISH_SMALL_SIZE_MIN +
        (std::rand() % (CLOWNFISH_SMALL_SIZE_MAX - CLOWNFISH_SMALL_SIZE_MIN));
      clownfish[target]->height(fishes[target].fish_size);
      clownfish[target]->width(fishes[target].fish_size);
      fishes[target].speed =
        STANDARD_SPEED_MIN +
        (std::rand() % (STANDARD_SPEED_MAX - STANDARD_SPEED_MIN));
      fishes[target].angle = (0.1 * (std::rand() % 7 + 1)) + 0.2;
      fishes[target].x_negative = std::rand() % 2;
      fishes[target].y_negative = std::rand() % 2;
      fishes[target].score_value = 3;
      fishes[target].xPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWX - fishes[target].fish_size);
      fishes[target].yPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWY - fishes[target].fish_size);
      fishes[target].state_goal = 1000;
      fishes[target].state_progress = 0;
      fishes[target].type = ANGLED_FISH;
      fishFlipper(target);
      clownfish[target]->colour(ASGE::COLOURS::WHITE);
      break;
    case FAST_ANGLED_FISH:
      fishes[target].fish_size =
        CLOWNFISH_SMALL_SIZE_MIN +
        (std::rand() % (CLOWNFISH_SMALL_SIZE_MAX - CLOWNFISH_SMALL_SIZE_MIN));
      clownfish[target]->height(fishes[target].fish_size);
      clownfish[target]->width(fishes[target].fish_size);
      fishes[target].speed =
        FAST_SPEED_MIN + (std::rand() % (FAST_SPEED_MAX - FAST_SPEED_MIN));
      fishes[target].angle = 0.1 * (std::rand() % 9 + 1);
      fishes[target].x_negative = std::rand() % 2;
      fishes[target].y_negative = std::rand() % 2;
      fishes[target].score_value = 5;
      fishes[target].xPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWX - fishes[target].fish_size);
      fishes[target].yPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWY - fishes[target].fish_size);
      fishes[target].state_goal = 500 + 100 * (std::rand() % 6);
      fishes[target].state_progress = 0;
      fishes[target].type = FAST_ANGLED_FISH;
      fishFlipper(target);
      clownfish[target]->colour(ASGE::COLOURS::WHITE);
      break;
    case FASTER_FISH:
      fishes[target].fish_size =
        CLOWNFISH_SMALL_SIZE_MIN +
        (std::rand() % (CLOWNFISH_SMALL_SIZE_MAX - CLOWNFISH_SMALL_SIZE_MIN));
      clownfish[target]->height(fishes[target].fish_size);
      clownfish[target]->width(fishes[target].fish_size);
      fishes[target].speed =
        FASTER_SPEED_MIN +
        (std::rand() % (FASTER_SPEED_MAX - FASTER_SPEED_MIN));
      fishes[target].angle = 1;
      fishes[target].x_negative = std::rand() % 2;
      fishes[target].y_negative = false;
      fishes[target].score_value = 5;
      fishes[target].xPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWX - fishes[target].fish_size);
      fishes[target].yPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWY - fishes[target].fish_size);
      fishes[target].state_goal = 1000;
      fishes[target].state_progress = 0;
      fishes[target].type = FASTER_FISH;
      fishFlipper(target);
      clownfish[target]->colour(ASGE::COLOURS::WHITE);
      break;
    case SLIPPERY_FISH:
      fishes[target].fish_size =
        CLOWNFISH_TINY_SIZE_MIN +
        (std::rand() % (CLOWNFISH_TINY_SIZE_MAX - CLOWNFISH_TINY_SIZE_MIN));
      clownfish[target]->height(fishes[target].fish_size);
      clownfish[target]->width(fishes[target].fish_size);
      fishes[target].speed =
        FAST_SPEED_MIN + (std::rand() % (FAST_SPEED_MAX - FAST_SPEED_MIN));
      fishes[target].angle = 1;
      fishes[target].x_negative = std::rand() % 2;
      fishes[target].y_negative = false;
      fishes[target].score_value = 8;
      fishes[target].xPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWX - fishes[target].fish_size);
      fishes[target].yPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWY - fishes[target].fish_size);
      fishes[target].state_goal = 400 + 20 * (std::rand() % 11);
      fishes[target].state_progress = 0;
      fishes[target].type = SLIPPERY_FISH;
      fishFlipper(target);
      clownfish[target]->colour(ASGE::COLOURS::WHITE);
      break;
    case TURNING_FISH:
      fishes[target].fish_size =
        CLOWNFISH_SMALL_SIZE_MIN +
        (std::rand() % (CLOWNFISH_SMALL_SIZE_MAX - CLOWNFISH_SMALL_SIZE_MIN));
      clownfish[target]->height(fishes[target].fish_size);
      clownfish[target]->width(fishes[target].fish_size);
      fishes[target].speed =
        FASTER_SPEED_MIN +
        (std::rand() % (FASTER_SPEED_MAX - FASTER_SPEED_MIN));
      fishes[target].angle = 0.1 * (std::rand() % 9 + 1);
      fishes[target].x_negative = std::rand() % 2;
      fishes[target].y_negative = std::rand() % 2;
      fishes[target].score_value = 8;
      fishes[target].xPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWX - fishes[target].fish_size);
      fishes[target].yPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWY - fishes[target].fish_size);
      fishes[target].state_goal = 250 + 50 * (std::rand() % 11);
      fishes[target].state_progress = 0;
      fishes[target].type = TURNING_FISH;
      fishFlipper(target);
      clownfish[target]->colour(ASGE::COLOURS::WHITE);
      break;
    case ULTIMATE_FISH:
      fishes[target].fish_size =
        CLOWNFISH_TINY_SIZE_MIN +
        (std::rand() % (CLOWNFISH_TINY_SIZE_MAX - CLOWNFISH_TINY_SIZE_MIN));
      clownfish[target]->height(fishes[target].fish_size);
      clownfish[target]->width(fishes[target].fish_size);
      fishes[target].speed = FASTER_SPEED_MAX;
      fishes[target].angle = 0.1 * (std::rand() % 9 + 1);
      fishes[target].x_negative = std::rand() % 2;
      fishes[target].y_negative = std::rand() % 2;
      fishes[target].score_value = 10;
      fishes[target].xPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWX - fishes[target].fish_size);
      fishes[target].yPos = (fishes[target].fish_size / 2) +
                            std::rand() % (WINDOWY - fishes[target].fish_size);
      fishes[target].state_goal = 100 + 100 * (std::rand() % 3);
      fishes[target].state_progress = 0;
      fishes[target].type = ULTIMATE_FISH;
      fishFlipper(target);
      clownfish[target]->colour(ASGE::COLOURS::CORAL);
      break;
    default:
      break;
  }
}

/**
 *   @brief   Sets the correct FlipFlag to the targeted Sprite
 *   @details Target Sprite replaces it's FlipFlag based off of the fishes
 * horizontal orientation
 */

void MyASGEGame::fishFlipper(int target)
{
  if (!fishes[target].x_negative)
  {
    clownfish[target]->setFlipFlags(ASGE::Sprite::FlipFlags::FLIP_X);
  }
  if (fishes[target].x_negative)
  {
    clownfish[target]->setFlipFlags(ASGE::Sprite::FlipFlags::NORMAL);
  }
}

/**
 *   @brief   Initialises the game background
 *   @details Attempts to load the background used throughout the
 *            game. The function will return if the loading was
 *            successful or not.
 *   @return  true if loaded, false if not
 */

bool MyASGEGame::initBackground()
{
  // load the background sprite
  background = renderer->createRawSprite();

  if (!background->loadTexture("/data/images/background.jpg"))
  {
    ASGE::DebugPrinter{} << "init::Failed to load background" << std::endl;
    return false;
  }

  background->width(WINDOWX);
  background->height(WINDOWY);
  return true;
}

bool MyASGEGame::initLifeBar()
{
  // load the lifebar sprite
  life_bar = renderer->createRawSprite();

  if (!life_bar->loadTexture("/data/images/lifebar.png"))
  {
    ASGE::DebugPrinter{} << "init::Failed to load lifebar" << std::endl;
    return false;
  }

  life_bar->width(WINDOWX);
  life_bar->height(20);
  return true;
}

/**
 *   @brief   Initialises the clownfish
 *   @details Attempts to load the clownfish used throughout the
 *            game. The function will return if the loading was
 *            successful or not.
 *   @return  true if loaded, false if not
 */

bool MyASGEGame::initClownfish()
{
  // load the clownfish
  for (int i = 0; i < MAX_FISHCOUNT; i++)
  {
    clownfish[i] = renderer->createRawSprite();

    if (!clownfish[i]->loadTexture("/data/images/clown-fish-icon.png"))
    {
      ASGE::DebugPrinter{} << "init::Failed to load clownfish" << std::endl;
      return false;
    }

    clownfish[i]->width(64);
    clownfish[i]->height(64);
    // clownfish[i]->setFlipFlags(ASGE::Sprite::FlipFlags::FLIP_X);
    clownfish[i]->yPos(50);
  }
  return true;
}

/**
 *   @brief   Sets the game window resolution
 *   @details This function is designed to create the window size, any
 *            aspect ratio scaling factors and safe zones to ensure the
 *            game frames when resolutions are changed in size.
 *   @return  void
 */

void MyASGEGame::setupResolution()
{
  // how will you calculate the game's resolution?
  // will it scale correctly in full screen? what AR will you use?
  // how will the game be framed in native 16:9 resolutions?
  // here are some arbitrary values for you to adjust as you see fit
  // https://www.gamasutra.com/blogs/KenanBolukbasi/20171002/306822/
  // Scaling_and_MultiResolution_in_2D_Games.php

  // 720p is a pretty modest starting point, consider 1080p
  game_width = WINDOWX;
  game_height = WINDOWY;
}

/**
 *   @brief   Processes any key inputs
 *   @details This function is added as a callback to handle the game's
 *            keyboard input. For this game, calls to this function
 *            are thread safe, so you may alter the game's state as
 *            you see fit.
 *   @param   data The event data relating to key input.
 *   @see     KeyEvent
 *   @return  void
 */

void MyASGEGame::keyHandler(ASGE::SharedEventData data)
{
  auto key = static_cast<const ASGE::KeyEvent*>(data.get());
  if (key->key == ASGE::KEYS::KEY_Q && key->action == ASGE::KEYS::KEY_PRESSED)
  {
    toggleFPS();
  }
  if (key->key == ASGE::KEYS::KEY_RIGHT &&
      key->action == ASGE::KEYS::KEY_PRESSED)
  {
    menu_option++;
  }
  if (key->key == ASGE::KEYS::KEY_LEFT &&
      key->action == ASGE::KEYS::KEY_PRESSED)
  {
    menu_option--;
  }

  if (menu_option > MENU_MAX)
  {
    menu_option = MENU_MAX;
  }
  if (menu_option < MENU_MIN)
  {
    menu_option = MENU_MIN;
  }

  if (key->key == ASGE::KEYS::KEY_ENTER &&
      key->action == ASGE::KEYS::KEY_RELEASED)
  {
    if (menu_option == 2)
    {
      signalExit();
    }
    if (menu_option == 0)
    {
      in_menu = false;
      gamemode = 0;
    }
    if (menu_option == 1)
    {
      in_menu = false;
      gamemode = 1;
      life = 1000;
    }
  }

  if (key->key == ASGE::KEYS::KEY_ESCAPE &&
      key->action == ASGE::KEYS::KEY_RELEASED)
  {
    if (in_menu)
    {
      signalExit();
    }
    else
    {
      backToMenu();
    }
  }
}

/**
 *   @brief   Processes any click inputs
 *   @details This function is added as a callback to handle the game's
 *            mouse button input. For this game, calls to this function
 *            are thread safe, so you may alter the game's state as you
 *            see fit.
 *   @param   data The event data relating to key input.
 *   @see     ClickEvent
 *   @return  void
 */

void MyASGEGame::clickHandler(ASGE::SharedEventData data)
{
  auto click = static_cast<const ASGE::ClickEvent*>(data.get());

  double x_pos = click->xpos;
  double y_pos = click->ypos;

  ASGE::DebugPrinter{} << "x_pos: " << x_pos << std::endl;
  ASGE::DebugPrinter{} << "y_pos: " << y_pos << std::endl;
  if (click->action == ASGE::MOUSE::BUTTON_PRESSED &&
      click->button == ASGE::MOUSE::MOUSE_BTN1)
  {
    for (int i = 0; i < fish_count; i++)
    {
      if (isInside(clownfish[i], x_pos, y_pos))
      {
        score += fishes[i].score_value;
        if (gamemode == 1)
        {
          life += ((fishes[i].score_value / (difficulty_state + 1)) * 100) + 50;
        }
        createFish(
          fishChoice(fishes[i].type, (fishes[i].type + 2) < difficulty_state),
          i);
      }
    }
    if (gamemode == 1)
    {
      life -= 50;
    }
  }
}

/**
 *   @brief   Updates the scene
 *   @details Prepares the renderer subsystem before drawing the
 *            current frame. Once the current frame is has finished
 *            the buffers are swapped accordingly and the image shown.
 *   @return  void
 */

void MyASGEGame::update(const ASGE::GameTime& game_time)
{
  // auto dt_sec = game_time.delta.count() / 1000.0;;
  // make sure you use delta time in any movement calculations!

  if (!in_menu)
  {
    if (gamemode == 1)
    {
      life -= LIFE_LOSS * (game_time.delta.count() / 1000.0);

      if (life > 1000)
        life = 1000;

      life_bar->xPos(0 - (WINDOWX * ((1000.0 - life) / 1000.0)));
      if (life <= 0)
        backToMenu();
    }
    for (int i = 0; i < fish_count; i++)
    {
      fishes[i].state_progress +=
        SPECIAL_POWER_GAIN * (game_time.delta.count() / 1000.0);
      if (fishes[i].state_goal <= fishes[i].state_progress)
      {
        fishes[i].state_progress = 0;
        fishSpecialAbility(fishes[i].type, i);
      }

      updateFishLocation(game_time, i);
    }
  }
}

/**
 *   @brief   Triggers a special ability for the fish that triggers this
 *   @details Depending on the type of fish it fires an "ability" that changes
 * one or more of the fishes attributes.
 */

void MyASGEGame::fishSpecialAbility(int type, int id)
{
  switch (type)
  {
    case FAST_ANGLED_FISH:
      fishes[id].y_negative = !fishes[id].y_negative;
      break;
    case SLIPPERY_FISH:
      if (fishes[id].state_goal >= 400)
      {
        fishes[id].speed = FASTER_SPEED_MAX;
        fishes[id].state_goal = 200 + 10 * (std::rand() % 11);
      }
      else
      {
        fishes[id].speed =
          FAST_SPEED_MIN + (std::rand() % (FAST_SPEED_MAX - FAST_SPEED_MIN));
        ;
        fishes[id].state_goal = 400 + 20 * (std::rand() % 11);
      }
      break;
    case TURNING_FISH:
      fishes[id].x_negative = !fishes[id].x_negative;
      fishFlipper(id);
      fishes[id].y_negative = std::rand() % 2;
      fishes[id].angle = 0.1 * (std::rand() % 9 + 1);
      fishes[id].state_goal = 250 + 50 * (std::rand() % 11);
      break;
    case ULTIMATE_FISH:
      fishes[id].state_goal = 100 + 100 * (std::rand() % 3);
      switch (std::rand() % 6)
      {
        case 0:
          fishes[id].x_negative = !fishes[id].x_negative;
          fishFlipper(id);
          break;
        case 1:
          fishes[id].y_negative = !fishes[id].y_negative;
          break;
        case 2:
          fishes[id].y_negative = std::rand() % 2;
          fishes[id].angle = 0.1 * (std::rand() % 9 + 1);
          break;
        case 3:
          if (fishes[id].speed <= FASTER_SPEED_MAX)
          {
            fishes[id].speed += 200;
          }
          else
          {
            fishes[id].speed -= 200;
          }
          break;
        default:
          // chance to do nothing
          break;
      }
      break;

    default:
      // no special ability
      break;
  }
}

/**
 *   @brief   Updates the target fish's location
 *   @details Checks the direction of the fish then updates it's
 *            x and y locations based on their speed and angle.
 */

void MyASGEGame::updateFishLocation(const ASGE::GameTime& game_time, int target)
{
  float x_pos = fishes[target].xPos;
  float y_pos = fishes[target].yPos;
  if (fishes[target].x_negative)
  {
    x_pos -= fishes[target].speed * (game_time.delta.count() / 1000.0) *
             fishes[target].angle;
    if (x_pos < -fishes[target].fish_size)
    {
      x_pos = (fishes[target].speed / 10) + WINDOWX;
    }
  }
  else
  {
    x_pos += fishes[target].speed * (game_time.delta.count() / 1000.0) *
             fishes[target].angle;
    if (x_pos > WINDOWX)
    {
      x_pos = -(fishes[target].speed / 10) - fishes[target].fish_size;
    }
  }

  if (fishes[target].y_negative)
  {
    y_pos -= fishes[target].speed * (game_time.delta.count() / 1000.0) *
             (1 - fishes[target].angle);
    if (y_pos < -fishes[target].fish_size)
    {
      y_pos = (fishes[target].speed / 10) + WINDOWY;
    }
  }
  else
  {
    y_pos += fishes[target].speed * (game_time.delta.count() / 1000.0) *
             (1 - fishes[target].angle);
    if (y_pos > WINDOWY)
    {
      y_pos = -(fishes[target].speed / 10) - fishes[target].fish_size;
    }
  }

  fishes[target].xPos = x_pos;
  fishes[target].yPos = y_pos;
  clownfish[target]->xPos(x_pos);
  clownfish[target]->yPos(y_pos);
}

/**
 *   @brief   Renders the scene
 *   @details Renders all the game objects to the current frame.
 *            Once the current frame is has finished the buffers are
 *            swapped accordingly and the image shown.
 *   @return  void
 */

void MyASGEGame::render(const ASGE::GameTime&)
{
  renderer->setFont(0);
  renderer->renderSprite(*background);
  if (in_menu)
  {
    renderer->renderText(welcome,
                         WINDOWX / 2 - (welcome.length() * AVERAGE_FONT_LENGTH),
                         WINDOWY / 2 - DISTANCE_BETWEEN_CHOICES,
                         1.0,
                         ASGE::COLOURS::DARKORANGE);

    renderer->renderText(menu_option == 0 ? ">Play" : "Play",
                         menu_option == 0 ? menuLocationX(1, 5)
                                          : menuLocationX(1, 4),
                         WINDOWY / 2 + DISTANCE_BETWEEN_CHOICES,
                         1.0,
                         ASGE::COLOURS::DARKORANGE);

    renderer->renderText(menu_option == 2 ? ">Exit" : "Exit",
                         menu_option == 2 ? menuLocationX(3, 5)
                                          : menuLocationX(3, 4),
                         WINDOWY / 2 + DISTANCE_BETWEEN_CHOICES,
                         1.0,
                         ASGE::COLOURS::DARKORANGE);

    renderer->renderText(menu_option == 1 ? ">Arcade" : "Arcade",
                         menu_option == 1 ? menuLocationX(2, 7)
                                          : menuLocationX(2, 6),
                         WINDOWY / 2 + DISTANCE_BETWEEN_CHOICES,
                         1.0,
                         ASGE::COLOURS::DARKORANGE);
  }
  else
  {
    renderer->renderSprite(*life_bar);
    for (int i = 0; i < fish_count; i++)
    {
      renderer->renderSprite(*clownfish[i]);
    }
    renderer->renderText(score_fluff + std::to_string(score),
                         WINDOWX - (AVERAGE_FONT_LENGTH * 24),
                         SCORE_Y_LOCATION,
                         1.0,
                         ASGE::COLOURS::DARKORANGE);
  }
}

/**
 *   @brief   Collision checks a point and an AABB
 *   @details Designed to check if a point resides inside an
 *            AABB. It uses the sprite to form the rectangle
 *            and the x,y floats to establish the point.
 *   @param   sprite, the sprite to check against
 *   @param   mouse_x, the x position of the point
 *   @param   mouse_y, the y position of the point
 *   @return  void
 */

bool MyASGEGame::isInside(const ASGE::Sprite* sprite,
                          float mouse_x,
                          float mouse_y) const
{
  bool x_truth = false;
  bool y_truth = false;
  if (sprite->xPos() < mouse_x && mouse_x < sprite->xPos() + sprite->width())
    x_truth = true;
  if (sprite->yPos() < mouse_y && mouse_y < sprite->yPos() + sprite->height())
    y_truth = true;
  if (x_truth && y_truth)
    return true;
  else
    return false;
}

/**
 *   @brief   Gives menu x values based on the text
 *            length it's given
 *   @details Travis doesnt like a constant if statement,
 *            so I just commented it out in case I expand
 *            the menu again. Laziness, eh? ¯\_(ツ)_/¯
 *   @return  int
 */

int MyASGEGame::menuLocationX(int menu_order, int text_length)
{
  /*if (MENU_MAX % 2 == 1)
  {
    if (menu_order > MENU_MAX + 1 / 2)
    {
      return WINDOWX / 2 +
             ((menu_order - ((MENU_MAX + 1) / 2)) * DISTANCE_BETWEEN_CHOICES) -
             (AVERAGE_FONT_LENGTH * text_length);
    }
    else
    {
      return WINDOWX / 2 -
             ((MENU_MAX + 3 / 2) - menu_order * DISTANCE_BETWEEN_CHOICES) -
             (AVERAGE_FONT_LENGTH * text_length);
    }
  }
  else
  {
    if (menu_order > (MENU_MAX + 2) / 2)
    {
      return WINDOWX / 2 +
             ((menu_order - ((MENU_MAX + 2) / 2)) * DISTANCE_BETWEEN_CHOICES) -
             (AVERAGE_FONT_LENGTH * text_length);
    }
    if (menu_order < (MENU_MAX + 2) / 2)
    {
      return WINDOWX / 2 -
             ((((MENU_MAX + 2) / 2) - menu_order) * DISTANCE_BETWEEN_CHOICES) -
             (AVERAGE_FONT_LENGTH * text_length);
    }
    return WINDOWX / 2 - (AVERAGE_FONT_LENGTH * text_length);
  }*/
  if (menu_order > (MENU_MAX + 2) / 2)
  {
    return WINDOWX / 2 +
           ((menu_order - ((MENU_MAX + 2) / 2)) * DISTANCE_BETWEEN_CHOICES) -
           (AVERAGE_FONT_LENGTH * text_length);
  }
  if (menu_order < (MENU_MAX + 2) / 2)
  {
    return WINDOWX / 2 -
           ((((MENU_MAX + 2) / 2) - menu_order) * DISTANCE_BETWEEN_CHOICES) -
           (AVERAGE_FONT_LENGTH * text_length);
  }
  return WINDOWX / 2 - (AVERAGE_FONT_LENGTH * text_length);
}

void MyASGEGame::backToMenu()
{
  in_menu = true;
  gameStateInit();
}