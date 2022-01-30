// character chart that seems to be working https://www.asciitable.com/

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
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

struct Canvas{
	int width;
	int height;
	Point corner;
	// pointer to two dimensional arrays
	std::vector<std::vector<char>> canvas;
	
	Canvas(const int& w, const int& h, const Point c){
		// Width is twice the height to account for spacing in console
		// Coner is in the top left and acts as a reference for all the points
		width = 2*w;
		height = h;
		corner = c;

		//std::cout << "Initializing canvas with: " << height << " " << width << corner << std::endl;
		
		std::vector<char> tmp_arr;
		for(int y = 0; y <= height; y++){
			for(int x = 0; x <= width; x++){
				tmp_arr.push_back(char(0));
			}
			canvas.push_back(tmp_arr);
			tmp_arr = std::vector<char>();
		}

		printf("\033[?25l"); //hide cursor
		
		//clear the screen;
		printf("\33[2J");

		//print empty characters at the start of line so it's preserved
		for(int y = 0; y <= height; y++){
			// start of row
			printf("\033[%i;%iH%c",y,0,char(179));
			// end of row end of line char (carriage return)
			printf("\033[%i;%iH%c",y,width + 1,char(10));
		}

		// Print bottom row, only gets printed once:
		for(int x = 0; x <= width; x++){
			// ESC[y;xH moves curser to row y, col x, where ESC is \033
			printf("\033[%i;%iH%c",height + 1,x,char(196));
			if(x % 20 == 0) printf("\033[%i;%iH%i",height + 2,x,int(x/2 + corner.x));
		}
	}

	void clear(){
		for(int y = 0; y <= height; y++){
			//position cursor at the start of the line and then erase it
			printf("\033[%i;%iH",y,1);
			// ESC[2K	erase from cursor to end of line, so as not to erase the legednd at the bottom
			printf("\033[0K");
			for(int x = 0; x <= width; x++){
				if(canvas[y][x] != char(0)) canvas[y][x] = char(0);	
			}
		}
	}

	void drawP(Point p, char symbol = '*'){
		
		// Width is twice the height to account for spacing in console
		int i = round(corner.y - p.y);
		int j = round(2 * (p.x - corner.x));
		canvas[i][j] = symbol;

		//std::cout << p << " " << i ;
	}


	// posistion cursor on coordinate and print only non blank chars
	void print(){

		for(int y = 0; y <= height; y++){
			for(int x = 0; x <= width; x++){
				if(canvas[y][x] != char(0)){
					// ESC[y;xH moves curser to row y, col x, where ESC is \033
					printf("\033[%i;%iH%c",y,x,canvas[y][x]);
				}
			}
		}

		
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
	void draw(Canvas& canvas){
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
					canvas.drawP(curr_p,char(219)); //█
				} else if(round(next_y) > round(y)){
					canvas.drawP(curr_p,char(223)); //▀
					canvas.drawP(Point(x,next_y), char(220)); //▄
				} else {
					canvas.drawP(curr_p,char(220)); //▄
					canvas.drawP(Point(x,next_y), char(223)); //▀
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
				canvas.drawP(curr_p,char(219)); //█
			}
		}
	}
};


struct Triangle{
	// initialize vector of size 3
	std::vector<Point> vertices = std::vector<Point>(3);
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
	int point_counter = 0;

	while(infile >> triangle.vertices[point_counter].x >> triangle.vertices[point_counter].y){
		point_counter++;
		if (point_counter > 3){
			std::cout << "error, too many points" << std::endl;
			return EXIT_FAILURE;
		}
	}
	std::cout << triangle << std::endl;

	// Make canvas with size 20 and corner and -10, 10
	Canvas c = Canvas(20, 20, Point(-10, 10));


/*
	// make triangle lines and add them to canvas
	for(int i = 0; i < 3; i++){
		Point P1 = triangle.vertices[i];
		Point P2 = triangle.vertices[i == 2 ? 0 : i+1];

		//std::cout << P1 << P2 << std::endl;
		Line l = Line(P1,P2);
		l.draw(c);
	}
/*
	for(int i = 0; i < 3; i++){
		c.drawP(triangle.vertices[i],'$');
	}
*/
	//c.drawP(P0,'$');
	//c.drawP(P1,'$');

	Line l = Line(Point(-7, -6), Point(6, 6));
	l.draw(c);

	for(int i = 0; i < 10; i++){		
		c.clear();
		l = Line(Point(-7, -6 + i), Point(6, 6 - i));
		l.draw(c);
		c.print();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	// print canvas


	printf("\033[?25h"); //show cursor 
}