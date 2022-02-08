#include "diagrams.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace sf;

Text createText(Box box, string str, Font &font) {
    Text text;
    text.setFont(font);
    text.setString(str);
    text.setLetterSpacing(0.5);
    float st = 1, dr = 200;
    while(st <= dr) {
        float mij = (st+dr)/2;
        text.setCharacterSize(mij);
        if(text.getLocalBounds().width <= box.length && text.getLocalBounds().height <= box.height)
            st = mij+1;
        else
            dr = mij-1;
    }
    text.setCharacterSize(st-1);
    text.setPosition(box.x+(box.length-text.getLocalBounds().width)/2, box.y+(box.height-text.getLocalBounds().height)/2);
    text.setOrigin(text.getLocalBounds().left, text.getLocalBounds().top);
    return text;
}

Text createTextForCode(Box box, string str, Font &font, Color color) {
    Text text;
    text.setFont(font);
    text.setString('1');
    if('A' <= str && str <= 'Z')
        text.setString('A');
    if('a' <= str && str <= 'z')
        text.setString('a');
    text.setCharacterSize(20);
    text.setLetterSpacing(0.5);
    float xheight = text.getLocalBounds().height;
    text.setString(str);
    if(str == 'g' || str == 'p' || str == 'q' || str == 'Q' || str == 'y')
        text.setPosition(box.x+(box.length-text.getLocalBounds().width)/2, box.y+box.height-xheight);
    else if(str == 'j')
        text.setPosition(box.x+(box.length-text.getLocalBounds().width)/2, box.y+box.height-xheight-(text.getLocalBounds().height-xheight)/2+0.5);
    else if(str == '7' || str == '9')
        text.setPosition(box.x+(box.length-text.getLocalBounds().width)/2, box.y+box.height-text.getLocalBounds().height+1);
    else if(str == '-' || str == '+' || str == '=' || str == '~')
        text.setPosition(box.x+(box.length-text.getLocalBounds().width)/2, box.y+box.height-xheight/2-text.getLocalBounds().height/2);
    else if(str == '^' || str == '`' || str == '\'' || str == '\"')
        text.setPosition(box.x+(box.length-text.getLocalBounds().width)/2, box.y+box.height-xheight);
    else
        text.setPosition(box.x+(box.length-text.getLocalBounds().width)/2, box.y+box.height-text.getLocalBounds().height);
    text.setFillColor(color);
    text.setOrigin(text.getLocalBounds().left, text.getLocalBounds().top);
    return text;
}

CircleShape createCircle(Point middle, float r, Color colorFill, Color colorLine, Font &font) {
    CircleShape circle(r);
    circle.setPosition(middle.x-r, middle.y-r);
    circle.setFillColor(colorFill);
    circle.setOutlineThickness(1);
    circle.setOutlineColor(colorLine);
    return circle;
}

RectangleShape createRect(Point topLeft, Point bottomRight, Color colorFill, Color colorLine) {
    RectangleShape rectangle;
    rectangle.setSize(Vector2f(bottomRight.x-topLeft.x, bottomRight.y-topLeft.y));
    rectangle.setFillColor(colorFill);
    rectangle.setOutlineColor(colorLine);
    rectangle.setOutlineThickness(1);
    rectangle.setPosition(topLeft.x, topLeft.y);
    return rectangle;
}

VertexArray definitionsOrActionsCreate(Point topLeft, Point bottomRight) {
    VertexArray lines(LineStrip, 5);
    lines[0].position = Vector2f(topLeft.x, topLeft.y);
    lines[1].position = Vector2f(topLeft.x, bottomRight.y);
    lines[2].position = Vector2f(bottomRight.x, bottomRight.y);
    lines[3].position = Vector2f(bottomRight.x, topLeft.y);
    lines[4].position = Vector2f(topLeft.x, topLeft.y);
    return lines;
}


VertexArray decisionCreate(Point topLeft, Point bottomRight) {
    VertexArray lines(LineStrip, 8);
    lines[0].position = Vector2f(topLeft.x, topLeft.y);
    lines[1].position = Vector2f(topLeft.x, bottomRight.y);
    lines[2].position = Vector2f(topLeft.x+(bottomRight.x-topLeft.x)/2, bottomRight.y);
    lines[3].position = Vector2f(topLeft.x, topLeft.y);
    lines[4].position = Vector2f(bottomRight.x, topLeft.y);
    lines[5].position = Vector2f(topLeft.x+(bottomRight.x-topLeft.x)/2, bottomRight.y);
    lines[6].position = Vector2f(bottomRight.x, bottomRight.y);
    lines[7].position = Vector2f(bottomRight.x, topLeft.y);
    return lines;
}


// same like definitionsOrActions
VertexArray singleStepCreate(Point topLeft, Point bottomRight) {
    VertexArray lines(LineStrip, 5);
    lines[0].position = Vector2f(topLeft.x, topLeft.y);
    lines[1].position = Vector2f(topLeft.x, bottomRight.y);
    lines[2].position = Vector2f(bottomRight.x, bottomRight.y);
    lines[3].position = Vector2f(bottomRight.x, topLeft.y);
    lines[4].position = Vector2f(topLeft.x, topLeft.y);
    return lines;
}


VertexArray iterationWCreate(Point topLeft, Point bottomRight, float l, float h) {
    VertexArray lines(LineStrip, 7);
    lines[0].position = Vector2f(topLeft.x, topLeft.y);
    lines[1].position = Vector2f(bottomRight.x, topLeft.y);
    lines[2].position = Vector2f(bottomRight.x, topLeft.y+h);
    lines[3].position = Vector2f(topLeft.x+l, topLeft.y+h);
    lines[4].position = Vector2f(topLeft.x+l, bottomRight.y);
    lines[5].position = Vector2f(topLeft.x, bottomRight.y);
    lines[6].position = Vector2f(topLeft.x, topLeft.y);
    return lines;
}


VertexArray iterationUCreate(Point topLeft, Point bottomRight, float l, float h) {
    VertexArray lines(LineStrip, 7);
    lines[0].position = Vector2f(topLeft.x, topLeft.y);
    lines[1].position = Vector2f(topLeft.x+l, topLeft.y);
    lines[2].position = Vector2f(topLeft.x+l, bottomRight.y-h);
    lines[3].position = Vector2f(bottomRight.x, bottomRight.y-h);
    lines[4].position = Vector2f(bottomRight.x, bottomRight.y);
    lines[5].position = Vector2f(topLeft.x, bottomRight.y);
    lines[6].position = Vector2f(topLeft.x, topLeft.y);
    return lines;
}


