#ifdef __APPLE__
	#include <GLUT/glut.h> 
#else
	#include <GL/glut.h> 
#endif

#include <iostream>
#include <algorithm>                  
#include <cstdlib>
#include <ctime>
#include <vector>
#include <array>
#include <string>
#include <string.h>
#include <stddef.h>

void draw_text(const char* text, int scale_factor=1)
{
	/* Draw text at current location - scaled by scale_factor */
	const float scale = 0.005f * scale_factor;

	glPushMatrix();
	glScalef(scale, scale, 1.0f);
	size_t len = strlen(text);
	for (size_t i = 0;i < len;i++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);
	glPopMatrix();
}


void init_lights(const GLenum shade_model = GL_FLAT)
{
	float light0_position[] = { 1.0, 1.0, 2.0, 0.0 };
	float light1_position[] = { -2.0, 0.0, 2.0, 0.0 };
	float light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	float light_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);

	glFrontFace(GL_CW);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);


	glShadeModel(shade_model); 
}

void init_material()
{
	float mat_ambient[] = { 0.05, 0.05, 0.05, 1.0 };
	float mat_diffuse[] = { 0.75, 0.75, 0.75, 1.0 };
	float mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	float mat_shininess[] = { 50.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

// Define some constants for clarity
const int LEFT = -1;
const int RIGHT = 1;

const int CLOCKWISE = 1;
const int COUNTERCLOCKWISE = -1;

// Enum to store colour of tiles in game grid
enum class TileState { EMPTY, RED, GREEN, BLUE, PURPLE, CYAN, YELLOW, PINK};

/* These structs are conceptually different despite being structurally identical and so are separated for clarity */

struct absolutecoords {
	int x;
	int y;
};

struct relativecoords {
	int relx;
	int rely;
};

/* --------------------------------------------------------------------------------------------------------------- */

void setColour(TileState colour) {
	/* Function to set GLUT to the given colour */
	switch (colour) {
	case TileState::RED:
		glColor3f(1.0f, 0.0f, 0.0f);
		break;
	case TileState::GREEN:
		glColor3f(0.0f, 1.0f, 0.0f);
		break;
	case TileState::BLUE:
		glColor3f(0.0f, 0.0f, 1.0f);
		break;
	case TileState::PURPLE:
		glColor3f(1.0f, 0.0f, 1.0f);
		break;
	case TileState::CYAN:
		glColor3f(0.0f, 1.0f, 1.0f);
		break;
	case TileState::YELLOW:
		glColor3f(1.0f, 1.0f, 0.0f);
		break;
	case TileState::PINK:
		glColor3f(1.0f, 0.6f, 0.6f);
		break;
	}
}

class Shape {
	/* Collection of attributes and methods for controlling the currently falling shape */
protected:
	// Starting position
	absolutecoords grid_position{ 3,20 };

	TileState colour = TileState::EMPTY;

	// Adjust where the shape appears as a lookahead
	float look_ahead_y_adjust = 0;
	float look_ahead_x_adjust = 0;

	//Store the possible rotations of the shape and the current one
	std::vector<std::array<relativecoords, 4>> shaperotations;
	int currentrotation = 0;

public:
	void descend() {
		grid_position.y -= 1;
	}

	void left() {
		grid_position.x -= 1;
	}

	void right() {
		grid_position.x += 1;
	}

	void rotateclockwise() {
		currentrotation += 1;

		if (currentrotation > (shaperotations.size() - 1)) {
			currentrotation = 0;
		}
	}

	void rotatecounterclockwise() {
		currentrotation -= 1;

		if (currentrotation < 0) {
			currentrotation = shaperotations.size() - 1;
		}
	}

	void draw3d() {
		/* Draw the shape on the board */
		setColour(colour);
		std::array<relativecoords, 4> tile_positions = shaperotations.at(currentrotation);
		glPushMatrix();
		
		// Move to the shape's absolute reference point
		glTranslatef(grid_position.x * 0.5, grid_position.y * 0.5, 0.0f);
		for (auto tile : tile_positions) {
			glPushMatrix();
			// Move from the reference point to the exact location of this tile
			glTranslatef(tile.relx * 0.5, tile.rely * 0.5, 0.0f);
			glutSolidCube(0.5f);
			glPopMatrix();
		}
		glPopMatrix();
	}

	void drawLookahead3d() {
		/* Draw the shape as a lookahead */
		setColour(colour);
		std::array<relativecoords, 4> tile_positions = shaperotations.at(currentrotation);
		for (auto tile : tile_positions) {
			glPushMatrix();
			glTranslatef((tile.relx * 0.5) + look_ahead_x_adjust, (tile.rely * 0.5) + look_ahead_y_adjust, 0.0f);
			glutSolidCube(0.5f);
			glPopMatrix();
		}
	}

	void absoluteTilePositions(absolutecoords coords[4], int rotationDelta) {
		/* Returns the absolute grid positions of each tile in the shape 
		   RotationDelta allows the user to get the absolute positions of the shapes clockwise or anticlockwise rotation
		*/
		
		int neededRotation = currentrotation + rotationDelta;

		if (neededRotation < 0) {
			neededRotation = shaperotations.size() - 1;
		}
		else if (neededRotation > (shaperotations.size() - 1)) {
			neededRotation = 0;
		}

		std::array<relativecoords, 4> tile_positions = shaperotations.at(neededRotation);
		for (int i = 0; i < 4; i++) {
			absolutecoords current;
			relativecoords tile_position = tile_positions[i];
			current.x = grid_position.x + tile_position.relx;
			current.y = grid_position.y + tile_position.rely;
			coords[i] = current;
		}
	}

	TileState getColour() {
		return colour;
	}
};

class Line : public Shape {
public:
	Line() {
		
		std::array<relativecoords, 4> tile_positions;
		tile_positions[0] = relativecoords{ 1,1 };
		tile_positions[1] = relativecoords{ 1,2 };
		tile_positions[2] = relativecoords{ 1,3 };
		tile_positions[3] = relativecoords{ 1,4 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 0,1 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 2,1 };
		tile_positions[3] = relativecoords{ 3,1 };
		shaperotations.push_back(tile_positions);

		look_ahead_y_adjust = -1.25;
		look_ahead_x_adjust = -0.25;
		colour = TileState::BLUE;
	}
};


class LShape : public Shape {
public:
	LShape() {

		std::array<relativecoords, 4> tile_positions;
		tile_positions[0] = relativecoords{ 1,0 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 1,2 };
		tile_positions[3] = relativecoords{ 2,0 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 0,0 };
		tile_positions[1] = relativecoords{ 0,1 };
		tile_positions[2] = relativecoords{ 1,1 };
		tile_positions[3] = relativecoords{ 2,1 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 0,2 };
		tile_positions[1] = relativecoords{ 1,2 };
		tile_positions[2] = relativecoords{ 1,1 };
		tile_positions[3] = relativecoords{ 1,0 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 0,1 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 2,1 };
		tile_positions[3] = relativecoords{ 2,2 };
		shaperotations.push_back(tile_positions);

		look_ahead_y_adjust = -0.25;
		look_ahead_x_adjust = -0.25;

		colour = TileState::YELLOW;
	}
};


class TShape : public Shape {
public:
	TShape() {
		std::array<relativecoords, 4> tile_positions;
		tile_positions[0] = relativecoords{ 0,1 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 1,2 };
		tile_positions[3] = relativecoords{ 2,1 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 1,2 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 1,0 };
		tile_positions[3] = relativecoords{ 2,1 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 0,1 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 2,1 };
		tile_positions[3] = relativecoords{ 1,0 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 1,2 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 1,0 };
		tile_positions[3] = relativecoords{ 0,1 };
		shaperotations.push_back(tile_positions);

		look_ahead_y_adjust = -0.5;
		look_ahead_x_adjust = -0.25;

		colour = TileState::RED;
	}
};


class SShape : public Shape {
public:
	SShape() {
		std::array<relativecoords, 4> tile_positions;
		tile_positions[0] = relativecoords{ 0,0 };
		tile_positions[1] = relativecoords{ 1,0 };
		tile_positions[2] = relativecoords{ 1,1 };
		tile_positions[3] = relativecoords{ 2,1 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 1,2 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 2,1 };
		tile_positions[3] = relativecoords{ 2,0 };
		shaperotations.push_back(tile_positions);

		colour = TileState::PURPLE;
	}
};


class ZShape : public Shape {
public:
	ZShape() {
		std::array<relativecoords, 4> tile_positions;
		tile_positions[0] = relativecoords{ 0,1 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 1,0 };
		tile_positions[3] = relativecoords{ 2,0 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 1,0 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 2,1 };
		tile_positions[3] = relativecoords{ 2,2 };
		shaperotations.push_back(tile_positions);

		colour = TileState::PINK;
	}
};

class JShape : public Shape {
public:
	JShape() {
		std::array<relativecoords, 4> tile_positions;
		tile_positions[0] = relativecoords{ 0,0 };
		tile_positions[1] = relativecoords{ 1,0 };
		tile_positions[2] = relativecoords{ 1,1 };
		tile_positions[3] = relativecoords{ 1,2 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 0,2 };
		tile_positions[1] = relativecoords{ 0,1 };
		tile_positions[2] = relativecoords{ 1,1 };
		tile_positions[3] = relativecoords{ 2,1 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 1,0 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 1,2 };
		tile_positions[3] = relativecoords{ 2,2 };
		shaperotations.push_back(tile_positions);

		tile_positions[0] = relativecoords{ 0,1 };
		tile_positions[1] = relativecoords{ 1,1 };
		tile_positions[2] = relativecoords{ 2,1 };
		tile_positions[3] = relativecoords{ 2,0 };
		shaperotations.push_back(tile_positions);

		look_ahead_y_adjust = -0.25;

		colour = TileState::GREEN;
	}
};


class Square : public Shape {
public:
	Square() {
		std::array<relativecoords, 4> tile_positions;
		tile_positions[0] = relativecoords{ 0,1 };
		tile_positions[1] = relativecoords{ 1,0 };
		tile_positions[2] = relativecoords{ 1,1 };
		tile_positions[3] = relativecoords{ 0,0 };
		shaperotations.push_back(tile_positions);

		colour = TileState::CYAN;
	}
};

Shape generateRandomShape() {
	/* Returns a random shape from the seven tetris pieces */
	float random_val = (float)std::rand() / RAND_MAX;
	Shape shape;
	if (random_val < (float)1 / 7) {
		shape = Line();
	}
	else if (random_val < (float)2 / 7) {
		shape = LShape();
	}
	else if (random_val < (float)3 / 7) {
		shape = TShape();
	}
	else if (random_val < (float)4 / 7) {
		shape = SShape();
	}
	else if (random_val < (float)5 / 7) {
		shape = ZShape();
	}
	else if (random_val < (float)6 / 7) {
		shape = JShape();
	}
	else {
		shape = Square();
	}

	return shape;
}

class LookAheadShape {
private:
	Shape next_shape;

public:
	LookAheadShape() {
		next_shape = generateRandomShape();
	}

	void draw3d() {
		glPushMatrix();
		glTranslatef(7.5, 7, 0);
		next_shape.drawLookahead3d();
		glTranslatef(-1.5, 1.5, 0);
		glColor3f(0.0f, 0.0f, 0.0f);
		draw_text("Next Piece");
		glPopMatrix();
	}

	Shape doTransition() {
		Shape oldShape = next_shape;
		next_shape = generateRandomShape();
		return oldShape;
	}

};


bool slamming = false;
int slamming_length = 0;
bool game_over = false;
int game_score = 0;
int count = 0;
int current_gravity = 20;
int game_level = 1;
int total_rows_cleared = 0;
bool flat_perspective = false;

void increase_level() {
	game_level++;
	game_score += game_level * 50;

	if (current_gravity > 2) {
		current_gravity -= 3;
	}
}

class Game {
private:
	TileState Board[10][25];
	Shape currentshape;
	LookAheadShape lookahead = LookAheadShape();

public:
	Game() {
		/* Create first shape and setup empty board */
		currentshape = generateRandomShape();
		for (int x = 0; x < 10; x++) {
			for (int y = 0; y < 25; y++) {
				Board[x][y] = TileState::EMPTY;
			}
		}
	}

	bool checkShapeRotate(int direction) {
		/* Check whether the shape can turn in the given direction.
		
			Return true if so, else false.
		*/
		absolutecoords coords[4];
		currentshape.absoluteTilePositions(coords, direction);

		for (auto coord : coords) {
			int x = coord.x;
			int y = coord.y;

			// Some part of the shape is against leftmost wall and trying to move left
			if (x < 0) {
				return false;
			}
			// Some part of the shape is against rightmost wall and trying to move right
			else if (x > 9) {
				return false;
			}

			// Some part of the shape is on top of another part
			if (y < 20) {
				if (Board[x][y] != TileState::EMPTY) {
					return false;
				}
			}
		}
		return true;
	}

	bool checkShapeMove(int direction) {
		/* Check whether the shape can move in the given direction 
		
			Returns true if so, else false
		*/
		absolutecoords coords[4];
		currentshape.absoluteTilePositions(coords, 0);

		for (auto coord : coords) {
			int x = coord.x + direction;
			int y = coord.y; 

			// Some part of the shape is against leftmost wall and trying to move left
			if (x < 0) {
				return false;
			}
			// Some part of the shape is against rightmost wall and trying to move right
			else if (x > 9) {
				return false;
			}

			// Some part of the shape is on top of another part
			if (y < 20) {
				if (Board[x][y] != TileState::EMPTY) {
					return false;
				}
			}
		}
		return true;
	}

	void left() {
		if (checkShapeMove(LEFT)) {
			currentshape.left();
		}
	}

	void right() {
		if (checkShapeMove(RIGHT)) {
			currentshape.right();
		}
	}

	void rotateclockwise() {
		if (checkShapeRotate(CLOCKWISE)) {
			currentshape.rotateclockwise();
		}
	}

	void rotatecounterclockwise() {
		if (checkShapeRotate(COUNTERCLOCKWISE)) {
			currentshape.rotatecounterclockwise();
		}
	}

	TileState getTile(int x, int y) {
		return Board[x][y];
	}

	void clearRows(int min, int max) {
		bool empty_row = false;
		int range = max - min;
		total_rows_cleared += range + 1;
		if (total_rows_cleared >= (game_level * 5)) {
			increase_level();
		}

		// Increment game score by (100 * 2^(rows cleared-1)) + ((30 * game_level) * rows_cleared)
		game_score += (100 << range) + ((30 * game_level) * (range + 1));
		int numCompleted = 0;
		while ((!empty_row) && (max + numCompleted + 1 < 20)) {
			empty_row = true;
			for (int i = 0; i < 10; i++) {
				if (Board[i][max + 1 + numCompleted] != TileState::EMPTY) {
					empty_row = false;
				}

				Board[i][min + numCompleted] = Board[i][max + 1 + numCompleted];
				Board[i][max + 1 + numCompleted] = TileState::EMPTY;
			}
			numCompleted++;
		}
		while (numCompleted <= (range + 1)) {
			for (int i = 0; i < 10; i++) {
				Board[i][max + 1 + numCompleted] = TileState::EMPTY;
			}
			numCompleted++;
		}
	}

	void do_game_over() {
		game_over = true;

	}
	void addShapeToBoard() {
		/* Add the colour of the shape to all the tiles occupied by it
		   Check to see if a line is completed
		*/
		absolutecoords tiles[4];
		currentshape.absoluteTilePositions(tiles, 0);
		for (auto tile : tiles) {
			if (tile.y > 20) {
				do_game_over();
			}
			Board[tile.x][tile.y] = currentshape.getColour();
		}

		bool encounteredEmpty;
		int i;
		int min = -1;
		int max = -1;

		// Check to see if a row is completed
		for (auto tile : tiles) {
			encounteredEmpty = false;
			i = 0;
			// Go through each row that has has a tile added to it until an empty space is found
			// If there is no empty space(ie. i=10), the row is completed
			while (!(encounteredEmpty) && (i < 10)) {
				if (Board[i][tile.y] == TileState::EMPTY) {
					encounteredEmpty = true;
				}
				i++;
			}
			
			// Find the lowest and highest rows completed
			if (!encounteredEmpty) {
				if ((min == -1) or (tile.y < min)) {
					min = tile.y;
				}

				if ((max == -1) or (tile.y > max)) {
					max = tile.y;
				}
			}
		}

		// If at least one row is completed, clear rows
		if (min != -1) {
			clearRows(min, max);
		}	
	}

	

	bool checkShapeCanFall() {
		/* Check the tiles below the falling shape and make sure they are all valid to be occupied
		   Returns true if so, else false
		*/
		absolutecoords coords[4];
		currentshape.absoluteTilePositions(coords, 0);

		for (auto coord : coords) {
			int x = coord.x;
			int y = coord.y - 1; // We are interested in the tile below the shape

			// Some part of the shape is on the floor
			if (y < 0) {
				return false;
			}

			// Some part of the shape is on top of another part
			if (y < 20) {
				if (Board[x][y] != TileState::EMPTY) {
					return false;
				}
			}
		}

		return true;
	}

	void doGravity() {
		/* If the shape can fall, then it does.
		   Otherwise, it has hit the floor. Add it to the board and choose a new shape.
		*/
		if (checkShapeCanFall()) {
			currentshape.descend();
		}
		else {
			slamming = false;
			game_score += slamming_length;
			slamming_length = 0;
			addShapeToBoard();
			currentshape = lookahead.doTransition();
		}
	}

	void slam() {
		/* Set the slamming flag */
		slamming = true;
	}

	void draw3d() {
		/* Method for drawing the game */

		// Draw the board, add a cube of the correct colour wherever the board is not empty.
		for (int x = 0; x < 10; x++) {
			for (int y = 0; y < 20; y++) {
				if (Board[x][y] != TileState::EMPTY) {
					glPushMatrix();
					glTranslatef(x * 0.5, y * 0.5, 0.0f);
					setColour(Board[x][y]);
					glutSolidCube(0.5);
					glPopMatrix();
				}
			}
		}

		// Draw the currentshape and the lookahead
		currentshape.draw3d();
		lookahead.draw3d();

		// Add the level and score texts
		glPushMatrix();
		glTranslatef(6.5, 3.0, 0.0);
		glColor3f(0.0f, 0.0f, 0.0f);
		std::string text = "Level: " + std::to_string(game_level);
		draw_text(text.c_str());
		glTranslatef(0.0, -1, 0.0);
		text = "Score: " + std::to_string(game_score);
		draw_text(text.c_str());

		glPopMatrix();

	}
};

Game game;

void draw_board3d() {
	/* Draws the lines that make up the 3d board */
	glColor3f(0.0f, 0.0f, 0.0f);

	//Bottom
	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.25f, -0.25f, 0.25f);
	glVertex3f(-0.25f, -0.25f, -0.25f);
	glVertex3f(4.75f, -0.25f, -0.25f);
	glVertex3f(4.75f, -0.25f, 0.25f);
	glEnd();

	//Side
	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.25f, 9.75, -0.25);
	glVertex3f(-0.25f, 9.75, 0.25f);
	glVertex3f(-0.25f, -0.25, 0.25f);
	glVertex3f(-0.25f, -0.25f, -0.25f);
	glEnd();

	//Back
	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.25f, -0.25f, -0.25);
	glVertex3f(-0.25f, 9.75f, -0.25);
	glVertex3f(4.75f, 9.75f, -0.25);
	glVertex3f(4.75f, -0.25f, -0.25);
	glEnd();
}

void display()
{
	// clears to current background colour
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (flat_perspective) {
		gluLookAt(5.0f, 5.0f, 20.0f, // eye position
			5, 5.0f, 0.0f, // reference point
			0, 1, 0  // up vector
		);
	}
	else {
		gluLookAt(15.0f, 5.0f, 15.0f, // eye position
			5, 5.0f, 0.0f, // reference point
			0, 1, 0  // up vector
		);
	}
	glDisable(GL_LIGHTING);
	draw_board3d();

	glEnable(GL_LIGHTING);
	game.draw3d();

	if (game_over) {
		// Show game over text
		glPushMatrix();
		glTranslatef(1.2f, 5.0f, 0.5f);
		glColor3f(0.0f, 0.0f, 0.0f);
		draw_text("GAME OVER", 2);
		glPopMatrix();
	}
	
	glutSwapBuffers();
	
}

void gravity(int) {
	/* Each time the function is called, increments a variable.
	   When the variable reaches the gravity delay, tell the game to simulate gravity then resets the variable
	*/
	if (!game_over) {
		if ((count == current_gravity) or slamming) {
			if (slamming) {
				slamming_length++;
			}
			count = 0;
			game.doGravity();
			display();
		}
		else {
			count += 1;
		}
		glutTimerFunc(50, gravity, 0);
	}
}


void keyboard(unsigned char key, int, int) {
	/* Function to handle user keyboard input */
	if (!game_over) {
		// These commands can only be given when the game is in progress
		switch (key)
		{
		// Handle user game controls
		case 'a': game.left(); break;
		case 'd': game.right(); break;
		case 's': game.slam(); break;
		case 'e': game.rotateclockwise(); break;
		case 'q': game.rotatecounterclockwise(); break;
		}
	}

	// These commands can be given even if the game is over
	switch (key) {
	// Restart game
	case 'p': game = Game(); 
		if (game_over) 
		{
			glutTimerFunc(50, gravity, 0);
		};
		game_over = false;  
		current_gravity = 20;
		game_score = 0;
		game_level = 1;
		slamming = false;
		total_rows_cleared = 0;
		slamming_length = 0;
		break;
	// Change perspective
	case 'r': flat_perspective = !flat_perspective; break;
	case 'z': exit(1); // quit!
	}
	glutPostRedisplay();
}



void reshape(int w, int h)
{
	/* Handles reshaping the screen */
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect_ratio = (float) w / h;
	glOrtho(-1 * aspect_ratio, 1 * aspect_ratio, -1, 1, 1.0, -1.0);

	gluPerspective(40.0, 1, 1.0, 50.0);
}

void init()
{
	// Initialize lights, materials, and clear colour
	init_lights();
	init_material();
	glEnable(GL_COLOR_MATERIAL);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glClearColor(0.0f, 0.8f, 1.0f, 0.0f);

	gluPerspective(40.0, 1.0f, 1.0, 50.0);
	
}

int main(int argc, char* argv[])
{
	// Seed random number generator
	std::srand(std::time(nullptr));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // flags bitwise OR'd together

	// Setup display window
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Tetris");

	// Setup methods
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutTimerFunc(50, gravity, 0);

	init(); 
	
	glutMainLoop();

	return 0;
}
