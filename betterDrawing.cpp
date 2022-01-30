// character chart that seems to be working https://www.asciitable.com/

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <thread>

const char SYMBOLS[] = {'*','\\','/','|','_',' '}; 

struct Point{
	double x;
	double y;

	Point(){
		x = 0;
		y = 0;
	}

	Point(double in_x, double in_y){
		x = in_x;
		y = in_y;
	}

	bool operator==(const Point& p) const
	{
		return (x == p.x && y == p.y);
	}
};

// Overload ostream operator to print Point
std::ostream& operator<<(std::ostream& os, const Point& p) { 
	os << "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
	return os;
}

// not great how it's structured
struct CanvasPoint{
	char symbol;
	std::array<uint8_t, 3> colour;

	CanvasPoint(){
		symbol = 0;
	}

	CanvasPoint(char s, std::array<uint8_t, 3> c){
		symbol = s;
		colour = c;
	}

	bool operator==(const CanvasPoint& cp) const 
	{
		return (cp.symbol == symbol && cp.colour[0] == colour[0] && cp.colour[1] == colour[1] && cp.colour[2] == colour[2]);
	}
};

struct Canvas{
	int width;
	int height;
	Point corner;
	// pointer to two dimensional arrays
	std::vector<std::vector<CanvasPoint>> canvas;
	
	Canvas(const int& w, const int& h, const Point c){
		// Width is twice the height to account for spacing in console
		// Coner is in the top left and acts as a reference for all the points
		width = 2*w;
		height = h;
		corner = c;

		//std::cout << "Initializing canvas with: " << height << " " << width << corner << std::endl;
		
		std::vector<CanvasPoint> tmp_arr;
		for(int y = 0; y <= height; y++){
			for(int x = 0; x <= width; x++){
				tmp_arr.push_back(CanvasPoint());
			}
			canvas.push_back(tmp_arr);
			tmp_arr = std::vector<CanvasPoint>();
		}

		
	}

	//Draw first bit of canvas on the screen
	void start(){
		//hide cursor
		printf("\033[?25l"); 
		
		//clear the screen;
		printf("\033[2J");

		//set 16 bit color mode
		//printf("\033[=19h");

		//print empty characters on line so it's preserved
		for(int y = 0; y <= height; y++){
			// end of row end of line char (carriage return)
			printf("\033[%i;%iH%c",y,width + 1,char(179));
		}
		printf("\033[%i;%iH%c",height + 1,width + 1,char(217));

		// Print bottom row, only gets printed once:
		for(int x = 0; x <= width; x++){
			// ESC[y;xH moves curser to row y, col x, where ESC is \033
			printf("\033[%i;%iH%c",height + 1,x,char(196));
			if(x % 20 == 0) printf("\033[%i;%iH%i",height + 2,x,int(x/2 + corner.x));
		}
	}

	// Not particularly efficient, could be better
	void clear(){
		for(int y = 0; y <= height; y++){
			for(int x = 0; x <= width; x++){
				if(canvas[y][x].symbol != 0){
					//replace with empty space
					printf("\033[%i;%iH%c",y,x,' ');
					canvas[y][x].symbol = 0;	
				}
			}
		}
	}

	void drawP(Point p, CanvasPoint cp){
		
		// Width is twice the height to account for spacing in console
		int i = round(corner.y - p.y);
		int j = round(2 * (p.x - corner.x));
		canvas[i][j] = cp;

		//std::cout << p << " " << i ;
	}

	// posistion cursor on coordinate and print only non blank chars
	void print(){
		for(int y = 0; y <= height; y++){
			for(int x = 0; x <= width; x++){
				if(canvas[y][x].symbol != 0){
					// ESC[y;xH moves curser to row y, col x, where ESC is \033
					printf("\033[%i;%iH",y,x);
					// set color
					printf("\033[38;2;%i;%i;%im",canvas[y][x].colour[0], canvas[y][x].colour[1], canvas[y][x].colour[2]);
					putchar(canvas[y][x].symbol);
				}
			}
		}		
	}

	//Given an input canvas, update only the points that are different
	void update(Canvas in){
		for(int y = 0; y <= in.height; y++){
			for(int x = 0; x <= in.width; x++){
				if(!(in.canvas[y][x] == canvas[y][x])){
					// ESC[y;xH moves curser to row y, col x, where ESC is \033
					printf("\033[%i;%iH",y,x);
					if(in.canvas[y][x].symbol == 0){
						putchar(' ');
					} else {
						// set color
						printf("\033[38;2;%i;%i;%im",in.canvas[y][x].colour[0], in.canvas[y][x].colour[1], in.canvas[y][x].colour[2]);
						putchar(in.canvas[y][x].symbol);
					}
					// Set the current point to the new values
					canvas[y][x] = in.canvas[y][x];
				}
			}
		}
	}

	void close(){
		printf("\033[0m");
		//show cursor 
		printf("\033[?25h");
		//Move cursor at the end of the picture
		printf("\033[%i;%iH",height + 3,0);
	}
};

struct Line{
	Point P0;
	Point P1;

	Line(Point p0, Point p1){
		P0 = p0;
		P1 = p1;
	}

	Line(){
		P0 = Point(0,0);
		P1 = Point(0,1);
	}

	// function draws crude line on canvas
	void draw(Canvas& canvas, std::array<uint8_t, 3> colour){
		// how much the x changes as the calculations are made, 0.5 since for every y there are 2 x
		const double deltaX = 0.5;
		// Temp point variables in case they need to be swapped
		Point p1 = P1;
		Point p0 = P0;

		double dx = p1.x - p0.x;
		double dy = p1.y - p0.y;
		if(std::abs(dx) >= std::abs(dy)){
			// Line is more horizontal than vertical
			// if P0 > P1 swap them
			if (p0.x > p1.x){
				Point tmp = p0;
				p0 = p1;
				p1 = tmp;
			}

			double y = p0.y;
			double slope = dy/dx;

			double next_y;

			Point curr_p;

			for(double x = p0.x; x <= p1.x; x += deltaX){
				curr_p = Point(x,y);
				next_y = y + slope * deltaX;
				// working, breaks with certain numbers ending in 0.5 because of ruonding errors
				/*
																 ▄███
													  ▄████▀
											  ▄███▀
									 ▄████▀
							▄████▀
						██▀
				
			broken example:
                               ▄███
                          ▄████▀
                       ███▀
                 ▄███▄█
            ▄████▀
          ██▀				
			*/
				if(int(round(next_y)) == int(round(y))){
					canvas.drawP(curr_p,CanvasPoint(char(219),colour)); //█
				} else if(round(next_y) > round(y)){
					canvas.drawP(curr_p,CanvasPoint(char(223),colour)); //▀
					canvas.drawP(Point(x,next_y), CanvasPoint(char(220),colour)); //▄
				} else {
					canvas.drawP(curr_p,CanvasPoint(char(220),colour)); //▄
					canvas.drawP(Point(x,next_y), CanvasPoint(char(223),colour)); //▀
				}
				y = next_y;
			}
		} else {
			// Line is more vertical than horizontal
			// if P0 > P1 swap them
			if (p0.y > p1.y){
				Point tmp = p0;
				p0 = p1;
				p1 = tmp;
			}

			double x = p0.x;
			double slope = (dx/dy);
			Point curr_p;
			for(double y = p0.y; y <= p1.y; y+= 1){
				curr_p = Point(x,y);
				x = x + slope;
				canvas.drawP(curr_p,CanvasPoint(char(219),colour)); //█
			}
		}
	}
};


struct Triangle{
	// initialize vector of size 3
	std::vector<Point> vertices = std::vector<Point>(3);

	void draw(Canvas& c, std::array<uint8_t, 3> colour){
		for(int i = 0; i < 3; i++){
			Point P1 = vertices[i];
			Point P2 = vertices[i == 2 ? 0 : i+1];

			//std::cout << P1 << P2 << std::endl;
			Line l = Line(P1,P2);
			l.draw(c, colour);
		}
		for(int i = 0; i < 3; i++){
			c.drawP(vertices[i],CanvasPoint('@',colour));
		}
	}
};

// Overload ostream operator to print Triangle
std::ostream& operator<<(std::ostream& os, const Triangle& t) { 
	for (int i = 0; i < t.vertices.size(); i++){
		os << "V" << i << ": " << t.vertices[i] << (i == 2 ? "" : "\n");
	}
	return os;
}

double distance_points(Point p1, Point p2){
	return std::sqrt(std::pow((p1.x - p2.x), 2) + std::pow((p1.y - p2.y), 2));
}

double triangle_perimeter(Triangle t){
	double perimeter = 0;
	for (int i = 0; i < t.vertices.size(); i++){
		perimeter += i == 2 ? distance_points(t.vertices[i], t.vertices[0]) : distance_points(t.vertices[i], t.vertices[i+1]);
	}
	return perimeter;
}


int main(){

	std::ifstream infile;
	infile.open("triangle.txt");

	if(!infile.is_open()){
		std::cout << "error opening file" << std::endl;
		return EXIT_FAILURE;
	}

	Triangle triangle;
	std::array<uint8_t, 3> t_colour {255,0,0};
	int point_counter = 0;
	bool read_colour;

	//infile >> t_colour[0] >> t_colour[1] >> t_colour[2];

	while(infile >> triangle.vertices[point_counter].x >> triangle.vertices[point_counter].y){
		point_counter++;
		if (point_counter > 3){
			std::cout << "error, too many points" << std::endl;
			return EXIT_FAILURE;
		}
	}
	std::cout << triangle << std::endl;

	// Make canvas with size 20 and corner and -10, 10
	Canvas c = Canvas(50, 50, Point(0, 0));
	c.start();

	Canvas buff = Canvas(50, 50, Point(0, 0));


	const int DELAY = 10;

/*
	c.drawP(Point(0,0),CanvasPoint('@',std::array<uint8_t, 3> {69,69,69}));

	Line l = Line(Point(-10,3),Point(10,-6));
	l.draw(c, t_colour);
	c.print();
	*/

	for(int i = 0; i < 40; i++){		
		t_colour[2] += 5;
		t_colour[0] -= 5;
		for(int i = 0; i < 3; i++){
			triangle.vertices[i].y -= 1;
			triangle.vertices[i].x += 1;
		}
		triangle.draw(buff, t_colour);
		c.update(buff);
		std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
		buff = Canvas(50, 50, Point(0, 0));
		
	}


	c.close();
}