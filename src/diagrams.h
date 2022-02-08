#pragma once
#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

struct Point {
    float x, y;
};

struct Box {
    float x, y, length, height;
};

Text createText(Box box, string str, Font &font);

Text createTextForCode(Box box, string str, Font &font, Color color);

CircleShape createCircle(Point middle, float r, Color colorFill, Color colorLine, Font &font);

RectangleShape createRect(Point topLeft, Point bottomRight, Color colorFill, Color colorLine);

VertexArray definitionsOrActionsCreate(Point topLeft, Point bottomRight);

VertexArray decisionCreate(Point topLeft, Point bottomRight);

// same like definitionsOrActions
VertexArray singleStepCreate(Point topLeft, Point bottomRight);

VertexArray iterationWCreate(Point topLeft, Point bottomRight, float l, float h);

VertexArray iterationUCreate(Point topLeft, Point bottomRight, float l, float h);


