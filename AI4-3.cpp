#include <windows.h>
#include <iostream>
#include <string>
#include <ctime>
#include <list>


static const int INFINITY = 1000000;

// declare the possible states for the game session
static enum {START, PLAYING, QUIT, OWIN, XWIN, DRAW} state;

// declare possible display mode
static enum {REGULAR, PROGRESSIVE} display_mode;

// this stucture simply defines the characteristic of each player
typedef struct {
	std::string name;
	char symbol;
	int move;
	int game_win;
	int draw_num;
	bool selected;
	bool win;
} player;


void display_board();				// display the current board position on the screen
void seed_random_generator();       // seed the random generator (rand function) with the current time
void get_move();                    // get move for the current player, the computer also use that function
void display_result();				// display the final result of the game on the screen
void select_game_type();            // this function generate the menu for the game
void get_player_symbol();           // get symbol (X, O) for the first player selected
void get_player_name();             // gets the name of the current player
void reset_player_name();           // reset the name of the players

void find_winner();                 // this function finds the winner once the agme has ended
void reset_winner();                // resets the winners before starting new game
void update_board();                // this function makes an update of the current board position
void update_screen();               // updates the screen
void set_game_level();              // will probably be be featured in the next version of this program
void verify_move();                 // verifies if the current move is legal
void reset_state();                 // resets the state of the game
void reset_board();                 // clears the board, removes all move that has been made
void initialise_player_move();      // reinitialise players move
void display_intro();               // display some text on the screen to introduce the game
void display_game_progress();       // displays who made the last and also current position
void update_game();                 // makes updates for the current position
void setup_game_screen();			// reset the dimension of the dos console window
bool wrong_symbol();                // check if the symbols that are choose by players are valid
bool wrong_selection();             // verifies the validity of current selection made by the player
bool game_over();                   // checks if the game is over
bool free_square();                 // verifies if the square chosen by the current player is free


// generates the list of legal moves for the current position
void generate_moves(char _board[9], std::list<int> &move_list);

// determin current game state (XWIN, OWIN, DRAW, PLAYING)
void check_game_state(char board[9]);

// makes an evaluation of the current board position,
// the function returns +INFINITY if the computer is winning,
// -INFINITY if the computer is loosing or 0 if it is a draw
int evaluate_position(char _board[9], player _player);


// the three functions below implements the MiniMax algorithm

int MiniMax(char _board[9], player _player);	// main function for MiniMax algorithm
int MinMove(char _board[9], player _player);	// helper function for MiniMax algorithm
int MaxMove(char _board[9], player _player);	// helper function for MiniMax algorithm

static player player1, player2, current_player; // 'player1' represent the first player and 'player2' the second one
static std::string game_type; // 'human vs human', 'human vs computer', 'computer vs computer'
static std::string prev_game_type; // variable for storing previous 'game type'

static char board[9] = {0}; // this variable is used to represent the board for the game
static char cSymbol; // this variable represent the symbol for the current player
static int nMove; // represents the last move made by the current player


int main() {
	seed_random_generator();
	setup_game_screen();

	display_intro();
	select_game_type();

	if(state != QUIT) {
		get_player_name();
		get_player_symbol();

		while(state != QUIT) {
			while(state == PLAYING) {
				initialise_player_move();
				get_move();
				update_game();
			}

			if(game_over()) {
				find_winner();
				display_result();
				reset_state();
				reset_board();
				display_intro();
			}

			select_game_type();
		}
	}
	return 0;
}

// selects game type
void select_game_type() {
	std::cout << "   1 - play a game against the computer" << std::endl;

	int choice;
	std::cin >> choice;
	if(!std::cin.good()) {
		std::cout << "please notice that you can only enter integers for the selection" << std::endl;
		update_screen();
	}
	switch(choice) {
		case 1:
			game_type = "human vs computer";
			break;
		default:
			std::cout << "Invalid Selection." << std::endl;
			update_screen();
	}
	if(choice > 0 && choice < 2) {
		if(prev_game_type != "" && game_type != prev_game_type) {
			//reset_game_statistic();
			reset_player_name();
			get_player_name();
			get_player_symbol();
		}
		if(game_type.length() > 0) {
			prev_game_type = game_type;
		}
	}
}

// gets current player name
void get_player_name() {
	std::cin.sync();
	if(game_type == "human vs computer") {
		std::cout << "\nplease enter your name: ";
		std::getline(std::cin, player1.name);
		if(player1.name.length() == 0) {
			get_player_name();
		}
		player2.name = "the computer";
	}
}

void reset_player_name() {
	player1.name.erase();
	player2.name.erase();
}

// gets symbol for the current player
void get_player_symbol() {
	if(game_type == "human vs computer") {
		int selection = rand() % 2;
		if(selection == 0) {
			rand() % 2 == 0 ? player2.symbol = 'X' : player2.symbol = 'O';
			cSymbol = player2.symbol;
			player2.selected = 1;
			std::cout << player2.name << " will play \'" << player2.symbol << "\'" << std::endl;
		} else if(selection == 1) {
			std::cout << player1.name << " please enter your symbol (X, O): ";
			std::cin >> player1.symbol;
			player1.symbol = toupper(player1.symbol);
			cSymbol = player1.symbol;
			player1.selected = 1;
		}
	}

	if(!std::cin.good() || wrong_symbol()) {
		std::cout << "please notice that your symbol can only be X or O" << std::endl;
		system("pause");
		get_player_symbol();
	}
	if(!player2.selected) {
		player1.symbol == 'X' ? player2.symbol = 'O' : player2.symbol = 'X';
		player1.symbol == 'O' ? player2.symbol = 'X' : player2.symbol = 'O';
	} else if(!player1.selected) {
		player2.symbol == 'X' ? player1.symbol = 'O' : player1.symbol = 'X';
		player2.symbol == 'O' ? player1.symbol = 'X' : player1.symbol = 'O';
	}
	state = PLAYING;
}

// gets move for the current player
void get_move() {
	std::cin.sync();
    if(game_type == "human vs computer") {
		if(player1.selected) {
			std::cout << "\n" << player1.name << " please enter your move (1 - 9): ";
			std::cin >> player1.move;
			if(!std::cin.good()) {
				std::cin.clear();
				std::cin.sync();
			}
			nMove = player1.move;
			cSymbol = player1.symbol;
			current_player = player1;
			player1.selected = 0;
			player2.selected = 1;
			Sleep(1000);
		} else if(player2.selected) {
			player2.move = MiniMax(board, player2);
			nMove = player2.move;
			cSymbol = player2.symbol;
			current_player = player2;
			player1.selected = 1;
			player2.selected = 0;
			reset_state();
			Sleep(1500);
		}
	}

	verify_move();
	if(game_over()) {
		return;
	}
}

// set game statististic for current match




// displayes game statistic on the screen


// finds the winner once the game is over
void find_winner() {
	if(state == XWIN && player1.symbol == 'X') {
		player1.win = 1;
	} else if(state == OWIN && player1.symbol == 'O') {
		player1.win = 1;
	} else if(state == XWIN && player2.symbol == 'X') {
		player2.win = 1;
	} else if(state == OWIN && player2.symbol == 'O') {
		player2.win = 1;
	}
}

// resets winners
void reset_winner() {
	player1.win = 0;
	player2.win = 0;
}

// verifies validity of symbols (X, O)
bool wrong_symbol() {
	return (cSymbol != 'X' && cSymbol != 'O');
}

// checks for wrong selection
bool wrong_selection() {
	return !(nMove > 0 && nMove < 17);
}

// reinitialise player moves
void initialise_player_move() {
	player1.move = -1;
	player2.move = -1;
}

// check for ending of the game
bool game_over() {
	return (state == XWIN || state == OWIN || state == DRAW);
}

// resets state of the game
void reset_state() {
	state = PLAYING;
}

// clears the board
void reset_board() {
	for(int i = 0; i < 9; ++i) {
		board[i] = 0;
	}
}

// updates currrent board position
void update_board() {
	if(state == PLAYING) {
		if(player1.move != -1 && player2.move == -1) {
			board[player1.move - 1] = player1.symbol;
		} else if(player2.move != -1) {
			board[player2.move - 1] = player2.symbol;
		}
	}
}

// selects display mode


// displays outcome the game on the screen
void display_result() {
	if(player1.win) {
		std::cout << player1.name << " has won the game!" << std::endl;
	} else if(player2.win) {
		std::cout << player2.name << " has won the game!" << std::endl;
	} else if(player1.win == 0 && player2.win == 0) {
		std::cout << "no winner, this game is a draw." << std::endl;
	}
	system("pause");
	system("cls");
}

// make updates of the current game
void update_game() {
	update_board();
	display_game_progress();
	check_game_state(board);
}

// checks is the square selected by the current player is free
bool free_square() {
	if(player1.move != -1 && player2.move == -1) {
		return board[player1.move - 1] == 0;
	} else if(player2.move != -1) {
		return board[player2.move - 1] == 0;
	}
	return 0;
}

// displays board on the screen
void display_board() {
	std::cout << std::endl;
	std::cout << " " << board[0] << " | " << board[1] << " | " << board[2] << std::endl;
	std::cout << "-----------" << std::endl;
	std::cout <<  " " << board[3] << " | " << board[4] << " | " << board[5] << std::endl;
	std::cout << "-----------" << std::endl;
	std::cout <<  " " << board[6] << " | " << board[7] << " | " << board[8] << std::endl;
	std::cout << std::endl;
	std::cin.sync();
}

// displays the progress of the game
void display_game_progress() {
	if(display_mode == PROGRESSIVE) {
		system("cls");
		display_intro();
	}
	std::cout << "\n\nboard position after " << current_player.name << "\'s move" << std::endl;
	display_board();
}

// verifies validity of the current move
// if the player makes an invalid move
// the function will ask the player
// to select another move
void verify_move() {
	if(wrong_selection() || !free_square()) {
		std::cout << "Invalid Move." << std::endl;
		if(player2.move == -1) {
			player1.selected = 1;
			player2.selected = 0;
		} else {
			player1.selected = 0;
			player2.selected = 1;
		}
		system("pause");
		if(game_type == "human vs computer") {
			player1.selected = 1;
			player2.selected = 0;
		}
		get_move();
	}
}

// seeds random generator with current time
void seed_random_generator() {
	srand((unsigned) time(NULL));
}

// displays intro for the game
void display_intro() {
	std::cout << "\n=========================================" << std::endl;
	std::cout << "  TicTacToe game 	" << std::endl;
	std::cout << "=========================================" << std::endl;
}

// resize the dimension of the dos console window
void setup_game_screen() {
	system("mode con: cols=99 lines=300");
}

// refresh game screen
void update_screen() {
	system("pause");
	system("cls");
	std::cin.clear();
	std::cin.sync();
	display_intro();
	select_game_type();
}

// determins if the current board position is final, if it is a win, draw
void check_game_state(char board[9]) {
    if ((board[0] == cSymbol && board[1] == cSymbol && board[2] == cSymbol) ||
		(board[3] == cSymbol && board[4] == cSymbol && board[5] == cSymbol) ||
		(board[6] == cSymbol && board[7] == cSymbol && board[8] == cSymbol) ||
		(board[0] == cSymbol && board[3] == cSymbol && board[6] == cSymbol) ||
		(board[1] == cSymbol && board[4] == cSymbol && board[7] == cSymbol) ||
		(board[2] == cSymbol && board[5] == cSymbol && board[8] == cSymbol) ||
		(board[0] == cSymbol && board[4] == cSymbol && board[8] == cSymbol) ||
		(board[2] == cSymbol && board[4] == cSymbol && board[6] == cSymbol))
	{
		if(cSymbol == 'X') {
			state = XWIN;
		} else if(cSymbol == 'O') {
			state = OWIN;
		}
	}
	else {
		state = DRAW;
		for(int i = 0; i < 9; ++i) {
			if(board[i] == 0) {
				state = PLAYING;
				break;
			}
		}
	}
}

// generates all the possible moves for the current board position
void generate_moves(char _board[9], std::list<int> &move_list) {
	for(int i = 0; i < 9; ++i) {
		if(_board[i] == 0) {
			move_list.push_back(i);
		}
	}
}

// evaluates the current board position
int evaluate_position(char _board[9], player _player) {
	check_game_state(_board);
	if(game_over()) {
		if((state == XWIN && _player.symbol == 'X') ||
			(state == OWIN && _player.symbol == 'O')) {
			return +INFINITY;
		} else if((state == XWIN && _player.symbol == 'O') ||
			(state == OWIN && _player.symbol == 'X')) {
			return -INFINITY;
		} else if(state == DRAW) {
			return 0;
		}
	}
	return -1;
}

// returns best move for the current computer player
int MiniMax(char _board[9], player _player) {
	int best_val = -INFINITY, index = 0;
	std::list<int> move_list;
	char best_moves[9] = {0};
	generate_moves(_board, move_list);
	while(!move_list.empty()) {
		_board[move_list.front()] = _player.symbol;
		cSymbol = _player.symbol;
		int val = MinMove(_board, _player);
		if(val > best_val) {
			best_val = val;
			index = 0;
			best_moves[index] = move_list.front() + 1;
		} else if(val == best_val) {
			best_moves[++index] = move_list.front() + 1;
		}
		_board[move_list.front()] = 0;
		move_list.pop_front();
	}
	if(index > 0) {
		index = rand() % index;
	}
	return best_moves[index];
}

// finds best move for 'min player'
int MinMove(char _board[9], player _player) {
	int pos_value = evaluate_position(_board, _player);
	if(pos_value != -1) {
		return pos_value;
	}
	int best_val = +INFINITY;
	std::list<int> move_list;
	generate_moves(_board, move_list);
	while(!move_list.empty()) {
		_player.symbol == 'X' ? cSymbol = 'O' : cSymbol = 'X';
		_board[move_list.front()] = cSymbol;
		int val = MaxMove(_board, _player);
		if(val < best_val) {
			best_val = val;
		}
		_board[move_list.front()] = 0;
		move_list.pop_front();
	}
	return best_val;
}

// finds best move for 'max player'
int MaxMove(char _board[9], player _player) {
	int pos_value = evaluate_position(_board, _player);
	if(pos_value != -1) {
		return pos_value;
	}
	int best_val = -INFINITY;
	std::list<int> move_list;
	generate_moves(_board, move_list);
	while(!move_list.empty()) {
		_player.symbol == 'X' ? cSymbol = 'X' : cSymbol = 'O';
		_board[move_list.front()] = cSymbol;
		int val = MinMove(_board, _player);
		if(val > best_val) {
			best_val = val;
		}
		_board[move_list.front()] = 0;
		move_list.pop_front();
	}
	return best_val;
}


