#include <iostream>
#include <curses.h>
#include <vector>
#include <string>
#include <map>
#include <unistd.h>

#define MWIN_COLOR 1
#define PLWIN_COLOR 2
#define STAT_COLOR 3
#define ENDWIN_COLOR 4
#define SIDEGRASS_COLOR 5
#define PLAYERCAR_COLOR 6
#define TRAFFIC_COLOR 7
#define ROADSIDEFP_COLOR 8
#define RANDOM(min,max)	( (min) + rand()%((max) - (min) + 1))

//Used some concepts of Michail Malafiejski for colision and Car class

using namespace std;

//car class
class Car {
protected:
	WINDOW* wn;
	int y, x, height, width, color, backgroundcol;
public:
	Car() { }
	Car(WINDOW* win, int xpos, int ypos, int wd, int ht, int col, int bcol) {
        IniCar(win,xpos,ypos,wd,ht,col,bcol);
    }

	void IniCar(WINDOW* win, int xpos, int ypos, int wd, int ht, int col, int bcol)
	{
		wn = win;
	    keypad(wn, true);
		y = ypos;
		x = xpos;
		height = ht;
		width = wd;
		color = col;
		backgroundcol = bcol;
	}

	int getCarHeight() { return height; }
	int getCarWidth() { return width; }

	int getCarY() { return y; }
	int getCarX() { return x; }

	void PrintCar()
        {
            wattron(wn,COLOR_PAIR(color));
            mvwaddch(wn, y, x, ACS_ULCORNER); //upper left-hand corner
            mvwaddch(wn, y, x + width, ACS_URCORNER); //upper right-hand corner
            mvwaddch(wn, y+height, x, ACS_LLCORNER); //lower left-hand corner
            mvwaddch(wn, y+height, x+width, ACS_LRCORNER); //lower right-hand corner

            for(int i = 1; i < width; i++){
                mvwaddch(wn, y, x + i, ACS_HLINE); //horizontal line
            }
            for(int i = 1; i < width; i++){
                mvwaddch(wn, y + height, x + i, ACS_HLINE); //horizontal line
            }
        }

	void RemoveTrace()
        {
            wattron(wn,COLOR_PAIR(backgroundcol));
            for(int i = 0; i <= width; i++){
                for(int j = 0; j <= height; j++){
                    mvwprintw(wn,y + j, x + i,"%c",' ');
                }
            }
        }
};

//player car class
class PlayerCar : public Car {
public:
	PlayerCar() { }
	void IniPCar(WINDOW* wn, int xpos, int ypos, int wd, int ht, int col, int bcol)
	{
		((Car*)(this))->IniCar(wn, xpos, ypos, wd, ht, col, bcol);
	}

	int PlayerCarMV(char inp)
	{
		switch(inp){
        case 'a':
            RemoveTrace();
			if (x > 31) x -= 1;
			return 1;
			break;
        case (char)KEY_LEFT:
            RemoveTrace();
			if (x > 31) x -= 1;
			return 1;
			break;
        case 'd':
            RemoveTrace();
			if (x < getmaxx(wn) - width - 32) x += 1;
			return 1;
			break;
        case (char)KEY_RIGHT:
            RemoveTrace();
			if (x < getmaxx(wn) - width - 32) x += 1;
			return 1;
			break;
        default:
            return 0;
            break;
		}
	}

	int CarCrush(Car& car)
	{
	    if(x == car.getCarX() || x +  width == car.getCarX() || x - width == car.getCarX()){
            if((y == car.getCarY() + car.getCarHeight() - 2) || (y == car.getCarY() + car.getCarHeight() - 3)){
                return 1;
            }
	    }
		if (y == car.getCarY() + car.getCarHeight() + 1)
		{
			if (((x >= car.getCarX()) && ( x <= car.getCarX() + car.getCarWidth())) || ( (x + width >= car.getCarX()) && (x + width <= car.getCarX() + car.getCarWidth()))){
                return 1;
			}
		}
		return 0;
	}

};

// moving car class
class MovingCar : public Car {
private:
	int mvdel;
public:
	MovingCar(WINDOW* wn, int xposition, int yposition, int width, int height, int col, int bcol, int mdl) {
	    IniMVCar(wn, xposition, yposition, width, height, col, bcol, mdl);
    }
	void IniMVCar(WINDOW* wn, int xposition, int yposition, int width, int height, int col, int bcol, int mdl)
	{
		mvdel = mdl;
		((Car*)(this))->IniCar(wn, xposition, yposition, width, height, col, bcol);
	}

	void MovingCarMV() { y += 1; }
	int ViewEndCheck()
	{
		if (y + height == getmaxy(wn) - 2) return 1; //check if traffic car approached window border
		return 0;
	}
	int getMvdel(){
        return mvdel;
	}
};

//traffic class
class TrafficCars {
private:
	WINDOW* wn;
	vector <MovingCar> MVCVec;
public:
	TrafficCars() { }
	void IniTraffic(WINDOW *w)
	{
		wn = w;
	}
	void Create(int mvde)
	{
		int height = 1;
		int width = 2;
		int spawnposition = RANDOM(32,getmaxx(wn)-34); //random position on screen
		MVCVec.push_back(MovingCar(wn, spawnposition,6,width,height,TRAFFIC_COLOR,MWIN_COLOR,mvde));
	}

	void TrafficCarsMV(int frame)
	{
		for(int i = 0; i < MVCVec.size(); i++)
            if (frame % MVCVec[i].getMvdel() == 0)
            {
                if (MVCVec[i].ViewEndCheck())
                {
                    MVCVec[i].RemoveTrace();
                    MVCVec.erase(MVCVec.begin()+i); //erasing the MVCVec at i
                }
                else
                {
                    MVCVec[i].RemoveTrace();
                    MVCVec[i].MovingCarMV();
                    MVCVec[i].PrintCar();
                }
            }
	}

	bool CarCrush(PlayerCar& playercar, int frm)
	{
		for(int i = 0; i < MVCVec.size(); i++){
		if (frm % MVCVec[i].getMvdel() == 0)
		{
			int answ = playercar.CarCrush(MVCVec[i]);
			if (answ == 1) return true;
		}
        }
	}
};

//roadside class
class ROADSIDE{
private:
    WINDOW * w;
    string inpstr, emp;
    int yposition, xposition, color, length, fpcolor;
    char side;
public:
    ROADSIDE(){ }
    ROADSIDE(WINDOW * w, string str, int xpos, int ypos, int color, int fpcolor, char side){
        IniRoasdside(w, str, xpos, ypos, color, fpcolor, side);
    }
    void IniRoasdside(WINDOW * w, string str, int xpos, int ypos, int color, int fpcolor, char side){
        this->w = w;
        inpstr = str;
        length = inpstr.size();
        emp = string(length, ' ');
        yposition = ypos;
        xposition = xpos;
        this->color = color;
        this->fpcolor = fpcolor;
        this->side = side;
    }

    void ShowFP(){
        wattron(w, COLOR_PAIR(fpcolor));
        if(side == 'l'){
            for(int i = 1; i < LINES - 6; i++){
            for(int j = 0; j < 31; j++){
                if(j == 30 || j == 29){

                    mvwprintw(w, i, j, " ");
                }
            }
        }
        }
        else if(side == 'r'){
            for(int i = 1; i < LINES - 6; i++){
            for(int j = COLS - 32; j < COLS; j++){
                if(j == COLS - 31 || j == COLS - 30){

                    mvwprintw(w, i, j, " ");
                }
            }
        }
        }
        wattroff(w, COLOR_PAIR(fpcolor));
    }

    void ShowRoasdside(){
        wattron(w, COLOR_PAIR(color));
        for(int i = 1; i < LINES - 6; i++){
            mvwprintw(w, i, xposition, "%s", emp.c_str());
        }
        ShowFP();
        wrefresh(w);
    }

};

//gamecontrol class
class GAMECONTROL{
private:
    WINDOW * status;
    unsigned int frm_tm = 20;
    char uinp;
    int frm;
    int SPAWN_COOLDOWN;
    int mvdelay;
    int score; int substitute;
public:
    void IniGameControl(WINDOW * mainwin){
        SPAWN_COOLDOWN = 50;
        mvdelay = 7;
        frm = 0;
        score = 0;
		status = subwin(mainwin, 5, COLS, 0, 0);
		wattron(status, COLOR_PAIR(STAT_COLOR));
		box(status,0,0);
		nodelay(status,TRUE);
		StatCL();
		wrefresh(status);
    }

    void StatCL(){
        for(int i = 1; i <= 3; i++){
            string subst(getmaxx(status) - 2,' ');
			mvwprintw(status, i, 1, "%s", subst.c_str());
        }
    }

    int setUserInp()
	{
		uinp = wgetch(status);
		if (uinp == ' ') return 1;
		return 0;
	}

	void PrintTimer() { mvwprintw(status,1,COLS/2-4,"Time: %.2lf", (frm * 5) / 1000.0); }
	void PrintScore(int val) { mvwprintw(status,3,COLS/2-4,"Score: %d", val); }

	int ControlRefresh()
	{
	    if(score >= 500){
            mvdelay = 6;
            SPAWN_COOLDOWN = 40;
	    }
	    if(score >= 1000){
            mvdelay = 5;
            SPAWN_COOLDOWN = 30;
	    }
	    if(score >= 1500){
            mvdelay = 4;
            SPAWN_COOLDOWN = 20;
	    }
	    if(score >= 2000){
            mvdelay = 3;
            SPAWN_COOLDOWN = 15;
	    }
	    if(score >= 2500){
            mvdelay = 2;
	        SPAWN_COOLDOWN = 10;
	    }
		frm++;
		substitute++;

		if(substitute / 2 >= 100){
            score += 100;
            substitute = 0;
		}

		PrintTimer();
		PrintScore(score);
		wrefresh(status);
		sleep();

		return 0;
	}

    void sleep() { usleep(frm_tm * 1000); }
	char getUserInp() { return uinp; }
	int getScore(){
        return score;
	}
	int getFrame(){
        return frm;
	}
	int getDelay(){
        return mvdelay;
	}
	int getSPAWN_COOLDOWN(){
        return SPAWN_COOLDOWN;
	}

};

class Game{
private:
    WINDOW * mwin, * plwin;
    ROADSIDE roadside_left;
    ROADSIDE roadside_right;
    GAMECONTROL gamecontrol;
    PlayerCar playercar;

    TrafficCars trafficcars;
public:
    Game(){
        if((mwin = initscr()) == NULL){
            printf("Game Could not Start!\n");
            exit(-1);
        }

        start_color();

        init_pair(MWIN_COLOR, COLOR_WHITE, COLOR_BLACK);// mwin color
		init_pair(STAT_COLOR, COLOR_WHITE, COLOR_BLUE);
		init_pair(TRAFFIC_COLOR, COLOR_BLACK, COLOR_BLUE);
		init_pair(PLWIN_COLOR, COLOR_WHITE, COLOR_CYAN); // plwin color
		init_pair(ENDWIN_COLOR, COLOR_BLACK, COLOR_BLUE);
		init_pair(SIDEGRASS_COLOR, COLOR_GREEN, COLOR_GREEN);
		init_pair(PLAYERCAR_COLOR, COLOR_BLACK, COLOR_YELLOW);
		init_color(COLOR_MAGENTA, 211, 211, 211);
		init_pair(ROADSIDEFP_COLOR, COLOR_BLACK, COLOR_MAGENTA);

        plwin = subwin(mwin, LINES - 5, COLS, 5, 0); //creating plwin
        wattron(plwin, COLOR_PAIR(PLWIN_COLOR));
        box(plwin, 0, 0);
        wrefresh(plwin);

        curs_set(0);
        noecho();

        //roadside
        string fillerln(30,' ');
		roadside_left.IniRoasdside(plwin, fillerln, 1, 1, SIDEGRASS_COLOR, ROADSIDEFP_COLOR, 'l');
		roadside_right.IniRoasdside(plwin, fillerln, COLS - 31, getmaxy(plwin)-2, SIDEGRASS_COLOR, ROADSIDEFP_COLOR, 'r');
		roadside_left.ShowRoasdside();
		roadside_right.ShowRoasdside();

		srand(time(NULL));
		nodelay(mwin,TRUE);

		trafficcars.IniTraffic(mwin);

		playercar.IniPCar(mwin, COLS/2 - 1, LINES-4, 2, 1, PLAYERCAR_COLOR, MWIN_COLOR); //width 2, height 1
		playercar.PrintCar();

		gamecontrol.IniGameControl(mwin);
    }

        void GameEnd(){
        clear();
        wattron(mwin, COLOR_PAIR(ENDWIN_COLOR));
        mvwprintw(mwin,10,COLS/2-17,"GAME OVER | YOUR FINAL SCORE: %d",gamecontrol.getScore());
        wrefresh(mwin);
    }

    void MainLoop()
	{
		while (!gamecontrol.setUserInp()) // MAIN LOOP RECIEVE INPUT
		{
			if(trafficcars.CarCrush(playercar, gamecontrol.getFrame())){
               GameEnd();
               break;
			}

			if (gamecontrol.getFrame() % gamecontrol.getSPAWN_COOLDOWN() == 0) trafficcars.Create(gamecontrol.getDelay()); // add traffic

			wrefresh(mwin);
			flushinp();
			trafficcars.TrafficCarsMV(gamecontrol.getFrame());

			if (playercar.PlayerCarMV(gamecontrol.getUserInp())){
                playercar.PrintCar();
			}
			gamecontrol.ControlRefresh();
		}
		usleep(3 * 1000000); //wait after game over
	}

	~Game(){
	    wclear(mwin);
        endwin();
    }

};

int main()
{
    Game game;
    game.MainLoop();
    return 0;
}
