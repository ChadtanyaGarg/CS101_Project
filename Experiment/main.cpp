#include <simplecpp>
#include "shooter.h"
#include "bubble.h"

/* Simulation Vars */
const double STEP_TIME = 0.02;

/* Game Vars */
const int LEFT_MARGIN = 70;
const int TOP_MARGIN = 20;
const int BOTTOM_MARGIN = (PLAY_Y_HEIGHT + TOP_MARGIN);
const int RIGHT_MARGIN = (WINDOW_X - 70);

void move_bullets(vector<Bullet> &bullets)
{
    // move all bullets
    for (unsigned int i = 0; i < bullets.size(); i++)
    {
        if (!bullets[i].nextStep(STEP_TIME))
        {
            bullets.erase(bullets.begin() + i);
        }
    }
}

void move_bubbles(vector<Bubble> &bubbles)
{
    // move all bubbles
    for (unsigned int i = 0; i < bubbles.size(); i++)
    {
        bubbles[i].nextStep(STEP_TIME);
    }
}

// Checks if there is collision between the given bubble and any of the bullets
bool bubble_bullet_collision(vector<Bullet> &bullets, Bubble bubble)
{
    for (unsigned int j = 0; j < bullets.size(); j++)
    {
        double dx = bubble.get_center_x() - bullets[j].get_center_x();
        double dy = bubble.get_center_y() - bullets[j].get_center_y();
        double max_dist = bubble.get_radius() + bullets[j].get_width() / 2; // max distance of collision
        if (dx * dx + dy * dy < max_dist * max_dist)
        {
            bullets.erase(bullets.begin() + j);
            return true;
        }
    }
    return false;
}

// Checks if any bubble collides with shooter
bool bubble_shooter_collision(vector<Bubble> bubbles, Shooter shooter)
{
    double shooter_body_y_max = shooter.get_body_center_y() + shooter.get_body_height() / 2;
    double shooter_body_y_min = shooter.get_body_center_y() - shooter.get_body_height() / 2;
    for (unsigned int i = 0; i < bubbles.size(); i++)
    {
        // Overlap with head of shooter
        double dx_head = abs(bubbles[i].get_center_x() - shooter.get_head_center_x());
        double dy_head = abs(bubbles[i].get_center_y() - shooter.get_head_center_y());
        double max_dr_head = bubbles[i].get_radius() + shooter.get_head_radius();
        if (dx_head * dx_head + dy_head * dy_head < max_dr_head * max_dr_head) // both are circles, so condition is easy
        {
            return true;
        }
        // Overlap with body of shooter
        double dx_body = abs(bubbles[i].get_center_x() - shooter.get_body_center_x());
        double max_dx_body = bubbles[i].get_radius() + shooter.get_body_width() / 2;
        double bubble_y_max = bubbles[i].get_center_y() + bubbles[i].get_radius();
        double bubble_y_min = bubbles[i].get_center_y() - bubbles[i].get_radius();
        if (dx_body < max_dx_body && bubble_y_min < shooter_body_y_max && bubble_y_max > shooter_body_y_min)
        { // conditon assuming bubble as the circumscribing square as an approximation. Otherwise it is very complicated and computational
            return true;
        }
    }
    return false;
}

vector<Bubble> create_bubbles(int level_counter)
{
    // create initial bubbles in the game
    vector<Bubble> bubbles;
    if (level_counter == 1)
    {
        bubbles.push_back(Bubble(WINDOW_X / 2.0, BUBBLE_START_Y, BUBBLE_DEFAULT_RADIUS, -BUBBLE_DEFAULT_VX, 0, COLOR(255, 105, 180)));
        bubbles.push_back(Bubble(WINDOW_X / 4.0, BUBBLE_START_Y, BUBBLE_DEFAULT_RADIUS, BUBBLE_DEFAULT_VX, 0, COLOR(255, 105, 180)));
    }
    else if (level_counter == 2)
    {
        bubbles.push_back(Bubble(1 * WINDOW_X / 4.0, BUBBLE_START_Y, BUBBLE_DEFAULT_RADIUS * 2, BUBBLE_DEFAULT_VX * 1.6, 0, COLOR(255, 139, 139)));
        bubbles.push_back(Bubble(2 * WINDOW_X / 4.0, BUBBLE_START_Y, BUBBLE_DEFAULT_RADIUS * 2, -BUBBLE_DEFAULT_VX * 1.8, 0, COLOR(255, 139, 139)));
        bubbles.push_back(Bubble(3 * WINDOW_X / 4.0, BUBBLE_START_Y, BUBBLE_DEFAULT_RADIUS * 2, BUBBLE_DEFAULT_VX * 2.1, 0, COLOR(255, 139, 139)));
    }
    else if (level_counter == 3)
    {
        bubbles.push_back(Bubble(1 * WINDOW_X / 4.0, BUBBLE_START_Y, BUBBLE_DEFAULT_RADIUS * 4, BUBBLE_DEFAULT_VX * 2.9, 0, COLOR(214, 163, 84)));
        bubbles.push_back(Bubble(2 * WINDOW_X / 4.0, BUBBLE_START_Y, BUBBLE_DEFAULT_RADIUS * 4, -BUBBLE_DEFAULT_VX * 2.5, 0, COLOR(214, 163, 84)));
        bubbles.push_back(Bubble(3 * WINDOW_X / 4.0, BUBBLE_START_Y, BUBBLE_DEFAULT_RADIUS * 4, BUBBLE_DEFAULT_VX * 2.4, 0, COLOR(214, 163, 84)));
    }
    return bubbles;
}

int main()
{
    initCanvas("Bubble Trouble", WINDOW_X, WINDOW_Y);

    Line b1(0, PLAY_Y_HEIGHT, WINDOW_X, PLAY_Y_HEIGHT);
    b1.setColor(COLOR(0, 0, 255));

    string msg_cmd("Cmd: _");
    Text charPressed(LEFT_MARGIN, BOTTOM_MARGIN, msg_cmd);

    XEvent event;

    // We need a boolean to keep track if game is won or lost.
    bool win = false;

    // Time Counter
    string time("Timer: 45 sec");
    Text TIME(LEFT_MARGIN, TOP_MARGIN, time);
    int time_limit = 45;
    double time_counter = 45;
    // each iteration waits 0.02 seconds, to account for computation time, I am adding a 0.04 seconds
    // hence each iteration is 0.06 sec, accourding to me
    // this does not produce exact 1 sec as in real world, but is reasonable.
    double time_wait = 0.06;

    // Strings for Health
    string health("Health: 3/3");
    Text HEALTH(RIGHT_MARGIN, TOP_MARGIN, health);
    int health_counter = 3;
    bool collision_counted = false;

    // Strings for Score
    string score("Score:  0");
    Text SCORE(RIGHT_MARGIN, BOTTOM_MARGIN, score);
    int score_counter = 0;

    // Setting up levels
    string level_flash("Level 1!");
    Text LEVEL_FLASH(WINDOW_X / 2, WINDOW_Y / 2, level_flash);
    LEVEL_FLASH.setColor(COLOR(20, 20, 255));
    int level_counter = 1;
    // the Text LEVEL_FLASH is what flashs in middle before level begins
    // the Text LEVEL remains at the bottom of the screen
    string level("Level: 1/3");
    Text LEVEL(WINDOW_X / 2, BOTTOM_MARGIN, level);

    // loop for levels
    while (true)
    {
        win = false;

        // updating level info
        level[level.length() - 3] = level_counter + '0';
        LEVEL.setMessage(level);
        level_flash[level_flash.length() - 2] = level_counter + '0';
        LEVEL_FLASH.setMessage(level_flash);
        nextEvent(event);
        LEVEL_FLASH.setMessage("");

        // set timer back to original time limit
        time_counter = time_limit;
        time[time.length() - 6] = (time_counter / 10) + '0';
        time[time.length() - 5] = (int(time_counter) % 10) + '0';
        TIME.setMessage(time);
        TIME.setColor(COLOR(0, 0, 0));

        // Intialize the shooter
        Shooter shooter(SHOOTER_START_X, SHOOTER_START_Y, SHOOTER_VX);

        // Initialize the bubbles
        vector<Bubble> bubbles = create_bubbles(level_counter);

        // Initialize the bullets (empty)
        vector<Bullet> bullets;

        // Main game loop
        while (true)
        {
            bool pendingEvent = checkEvent(event);
            if (pendingEvent)
            {
                // Get the key pressed
                char c = charFromEvent(event);
                msg_cmd[msg_cmd.length() - 1] = c;
                charPressed.setMessage(msg_cmd);

                // Update the shooter
                if (c == 'a' || c == 'K') // Adding control for arrow keys
                    shooter.move(STEP_TIME, true);
                else if (c == 'd' || c == 'M')
                    shooter.move(STEP_TIME, false);
                else if (c == 'w' || c == 'H')
                    bullets.push_back(shooter.shoot());
                else if (c == 'q')
                    return 0;
            }

            // Update Time counter.
            if (time_counter - int(time_counter) < time_wait) // means time_counter is integer
            {
                if (int(time_counter) / 10 == 0)
                    time[time.length() - 6] = ' '; // if else to make it 5 sec, and not 05 sec
                else
                    time[time.length() - 6] = (time_counter / 10) + '0';
                time[time.length() - 5] = (int(time_counter) % 10) + '0';
                // % operator works only on integers, we know here time_count is integer, but still stored in double, hence thiss conversion
                TIME.setMessage(time);
                if (time_counter <= 10)
                    TIME.setColor(COLOR(227, 39, 39)); // Make Time reddish is it is too low
            }

            // We check for bubbles starting from last so as to not miss any bubble when we erase one
            for (int i = bubbles.size() - 1; i >= 0; i--)
            {
                if (bubble_bullet_collision(bullets, bubbles[i]))
                {
                    if (bubbles[i].get_radius() / 2 >= BUBBLE_RADIUS_THRESHOLD)
                    {
                        bubbles.push_back(Bubble(bubbles[i].get_center_x(), bubbles[i].get_center_y(), bubbles[i].get_radius() / 2, bubbles[i].get_vx(), 0, bubbles[i].get_color()));
                        bubbles.push_back(Bubble(bubbles[i].get_center_x(), bubbles[i].get_center_y(), bubbles[i].get_radius() / 2, -bubbles[i].get_vx(), 0, bubbles[i].get_color()));
                    }
                    bubbles.erase(bubbles.begin() + i);
                    score_counter++;
                    score[score.length() - 1] = (score_counter % 10) + '0';
                    if (int(score_counter) / 10 == 0)
                        score[score.length() - 2] = ' '; // if else to make it 1, and not 01
                    else
                        score[score.length() - 2] = (score_counter / 10) + '0';
                    SCORE.setMessage(score);
                }
            }

            // If bubble collides with shooter, we loss a health and the shooter turns cyan during it.
            if (bubble_shooter_collision(bubbles, shooter) && !collision_counted)
            {
                health_counter--;
                health[health.length() - 3] = health_counter + '0';
                HEALTH.setMessage(health);
                if (health_counter == 1)
                    HEALTH.setColor(COLOR(227, 39, 39)); // Make Health reddish is it is too low
                collision_counted = true;
                shooter.setColor(COLOR(0, 255, 255));
            }
            if (!bubble_shooter_collision(bubbles, shooter))
            {
                collision_counted = false;
                shooter.setColor(COLOR(0, 255, 0));
            }

            // Conditions for losing:
            // Health becomes 0;
            // Time counter reachs time limit, we loss
            if (health_counter == 0 || time_counter <= 0)
            {
                win = false;
                break;
            }

            // If the bubble vector is empty, then all bubbles are destroyed, hence we will and break out of the game
            if (bubbles.size() == 0)
            {
                win = true;
                break;
            }

            // Update the bubbles
            move_bubbles(bubbles);

            // Update the bullets
            move_bullets(bullets);

            time_counter -= time_wait; // decrease time
            wait(STEP_TIME);
        }

        if (win)
        {
            level_counter++;
            gravity += 1; // increase gravity at each level
            if (level_counter == 4)
            {
                Text WIN(WINDOW_X / 2, WINDOW_Y / 2, "Congratulations!!");
                WIN.setColor(COLOR(0, 255, 0));
                WIN.imprint();
                break;
            }
        }
        else
        {
            Text LOSS(WINDOW_X / 2, WINDOW_Y / 2, "Game Over");
            LOSS.setColor(COLOR(255, 0, 0));
            LOSS.imprint();
            break;
        }
    }

    nextEvent(event); // Wait till some event before closing canvas
}