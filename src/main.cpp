#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <set>
#include <stack>
#include <unordered_map>
#include <cstring>
//#include "diagrams.h"
#include "diagrams.cpp"

using namespace std;
using namespace sf;

ifstream fin;
ofstream fout;

#define FILENAME_CODE "code.txt"
#define FILENAME_INPUT "input.txt"
#define FILENAME_OUTPUT "output.txt"
#define TEMPFILE "tempaWFtaGVyZTMK.txt"
#define TEMPFILE1 "tempasWdsaSDDFk.txt"
float SCREEN_WIDTH = 1280;
float SCREEN_HEIGHT = 720;
float MINSCR_WIDTH = 1158;
float MINSCR_HEIGHT = 585;
bool sizeScreen = false;
bool diagramIprepForPress = false;
bool oneTimeResize = false;
#define BLOCK_WIDTH 300.0
#define BLOCK_HEIGHT 60.0
#define MARGIN 60
#define BLOCK_BUTTON_WIDTH 100
// interface look
// D - diagram interface
// C - code interface
// [       ]
// [ CC DD ]
// [ CC DD ]
// [       ]
// interfata diagramei
#define DIAGRAM_MARGIN_WIDTH 60
#define DIAGRAM_MARGIN_HEIGHT MARGIN
float DIAGRAM_WIDTH = SCREEN_WIDTH/2-DIAGRAM_MARGIN_WIDTH*2;
float DIAGRAM_HEIGHT = SCREEN_HEIGHT-DIAGRAM_MARGIN_HEIGHT*2;
// interfata code
#define CODE_MARGIN_WIDTH 60
#define CODE_MARGIN_HEIGHT MARGIN
float CODE_WIDTH = SCREEN_WIDTH/2-CODE_MARGIN_WIDTH*2;
float CODE_HEIGHT = SCREEN_HEIGHT-CODE_MARGIN_HEIGHT*2;
string str_compiler_info;
#define BLOCK_CODE_WIDTH 16
#define BLOCK_CODE_HEIGHT 20
float CODEEDIT_MARGIN_WIDTH = 11.5;
float CODEEDIT_MARGIN_HEIGHT = 20;
int LIMIT_COLUMN_CODE = (CODE_WIDTH-CODEEDIT_MARGIN_WIDTH*2)/BLOCK_CODE_WIDTH-1;
int LIMIT_LINE_CODE = (CODE_HEIGHT-CODEEDIT_MARGIN_HEIGHT*2)/BLOCK_CODE_HEIGHT;

enum instructionType {EMPTY_NODE, VAR, SET, IF, WHILE, REPEAT, READ, PRINT, PASS, END, ERROR};
enum errorType {SYNTAX_ERROR_INSTRUCTION, SYNTAX_ERROR_VARTYPE,
    SYNTAX_ERROR_VARIABLE, SYNTAX_ERROR_INCOMPLETE_LINE, ERROR_UNDECLARED, ERROR_MULTIPLE_DECLARATION, ERROR_EXPRESSION, ERROR_INVALID_STRUCTURE,
    ERROR_STRING_OPERATIONS};
enum buttonType {RUN, ABOUT, SAVE, LOAD, CLEAR, CENTER, BACK, ARROW_LEFT, ARROW_RIGHT};
enum windowType {WIN_EDITOR, WIN_ABOUT};
enum variableType {INT, STRING};
enum editFileType {CODE_EDIT, INPUT_EDIT, OUTPUT_EDIT};
editFileType editFileT = CODE_EDIT;
windowType winT = WIN_EDITOR;
string errorMessage[] =
{
    "EROARE: Instructiune invalida la linia ",
    "EROARE: Tip de date declarat incorect la linia ",
    "EROARE: Variabila incorecta la linia ",
    "EROARE: Portiune de cod incompleta la linia ",
    "EROARE: Variabila nedeclarata utilizata la linia ",
    "EROARE: Variabila declarata multiplu la linia ",
    "EROARE: Expresie incorecta la linia ",
    "EROARE: Structura while/if/repeat incorecta la linia ",
    "EROARE: Operatii invalide cu stringuri la linia "
};

// pozita interfatei diagramei
Point originIDiagram = {SCREEN_WIDTH-DIAGRAM_WIDTH-DIAGRAM_MARGIN_WIDTH, DIAGRAM_MARGIN_HEIGHT};

// pozitia initiala a diagramei
Point originDiagramP = {originIDiagram.x+DIAGRAM_WIDTH/2-(float)BLOCK_WIDTH/2, originIDiagram.y+50};
Point diagramP = originDiagramP;

// pozitia interfatei code
Point originICode = {CODE_MARGIN_WIDTH, CODE_MARGIN_HEIGHT};

// variabilele pentru zoom
float zoom = 1, zoomScale = 0.1, zoomMinScale = 0.3;

// variabilele pentru move
bool moveScreen = false;
Point moveP = {0, 0};
Point amoveP = {0, 0};

// structura butonului
struct Button {
    Point topLeft, bottomRight;
    buttonType type;
    Color colorFill, colorLine, colorOnPressFill;
    bool press, prepForPress;
    string str;

    bool mouseOnButton(float x, float y) {
        return (topLeft.x < x && x < bottomRight.x && topLeft.y < y && y < bottomRight.y);
    }

    void draw(RenderWindow &window, Font &font) {
        if(!press)
            window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
        else
            window.draw(createRect(topLeft, bottomRight, colorOnPressFill, colorLine));
        Box box;
        box.x = topLeft.x+(bottomRight.x-topLeft.x)/8;
        box.y = topLeft.y+(bottomRight.y-topLeft.y)/8;
        box.length = bottomRight.x-topLeft.x-(bottomRight.x-topLeft.x)/4;
        box.height = bottomRight.y-topLeft.y-(bottomRight.y-topLeft.y)/4;
        window.draw(createText(box, str, font));
    }
} backButton;

// butoanele
unordered_map <buttonType, Button> buttons;

// editor
Point cursorCP = {0, 0};
Point codeP = {0, 0};
unordered_map <editFileType, vector <vector <char>>> codeEdit;

void createAllButtons() {

    ///variabile pentru centrarea butoanelor de sus
    int upperButtonCount = 5;
    float upperLength = originIDiagram.x + DIAGRAM_WIDTH - originICode.x; ///lungimea totala de sus
    float gapLength = 1.f * (upperLength - upperButtonCount * BLOCK_BUTTON_WIDTH) / (upperButtonCount + 1); ///distanta intre butoane
    gapLength = gapLength / 2.; ///e prea mare distanta intre butoane, mai tai din ea

    // RUN - compileaza codul si afiseaza diagrama
    buttons[RUN].topLeft = {originIDiagram.x + DIAGRAM_WIDTH - BLOCK_BUTTON_WIDTH, SCREEN_HEIGHT-(MARGIN/4)*3};
    buttons[RUN].bottomRight = {originIDiagram.x + DIAGRAM_WIDTH, SCREEN_HEIGHT-MARGIN/4};
    buttons[RUN].type = RUN;
    buttons[RUN].colorFill = Color(28, 28, 28);
    buttons[RUN].colorLine = Color(255, 0, 0);
    buttons[RUN].colorOnPressFill = Color(0, 255, 0);
    buttons[RUN].press = false;
    buttons[RUN].prepForPress = false;
    buttons[RUN].str = "RUN";

    // SAVE - salveaza codul si input-ul/output-ul
    buttons[SAVE].topLeft = {originICode.x + (upperLength - upperButtonCount * BLOCK_BUTTON_WIDTH - (upperButtonCount - 1) * gapLength) / 2.f, MARGIN/4};
    buttons[SAVE].bottomRight = {buttons[SAVE].topLeft.x + BLOCK_BUTTON_WIDTH, (MARGIN/4)*3};
    buttons[SAVE].type = SAVE;
    buttons[SAVE].colorFill = Color(28, 28, 28);
    buttons[SAVE].colorLine = Color(255, 0, 0);
    buttons[SAVE].colorOnPressFill = Color(0, 255, 0);
    buttons[SAVE].press = false;
    buttons[SAVE].prepForPress = false;
    buttons[SAVE].str = "SAVE";

    // LOAD - incarca codul si input-ul/output-ul
    buttons[LOAD].topLeft = {buttons[SAVE].bottomRight.x + gapLength, MARGIN/4};
    buttons[LOAD].bottomRight = {buttons[LOAD].topLeft.x + BLOCK_BUTTON_WIDTH, (MARGIN/4)*3};
    buttons[LOAD].type = LOAD;
    buttons[LOAD].colorFill = Color(28, 28, 28);
    buttons[LOAD].colorLine = Color(255, 0, 0);
    buttons[LOAD].colorOnPressFill = Color(0, 255, 0);
    buttons[LOAD].press = false;
    buttons[LOAD].prepForPress = false;
    buttons[LOAD].str = "LOAD";

    // ABOUT - pagina de informatii
    buttons[ABOUT].topLeft = {buttons[LOAD].bottomRight.x + gapLength, MARGIN/4};
    buttons[ABOUT].bottomRight = {buttons[ABOUT].topLeft.x + BLOCK_BUTTON_WIDTH, (MARGIN/4)*3};
    buttons[ABOUT].type = ABOUT;
    buttons[ABOUT].colorFill = Color(28, 28, 28);
    buttons[ABOUT].colorLine = Color(255, 0, 0);
    buttons[ABOUT].colorOnPressFill = Color(0, 255, 0);
    buttons[ABOUT].press = false;
    buttons[ABOUT].prepForPress = false;
    buttons[ABOUT].str = "ABOUT";

    ///CLEAR - sterge tot textul din caseta curenta
    buttons[CLEAR].topLeft = {buttons[ABOUT].bottomRight.x + gapLength, MARGIN/4};
    buttons[CLEAR].bottomRight = {buttons[CLEAR].topLeft.x + BLOCK_BUTTON_WIDTH, (MARGIN/4)*3};
    buttons[CLEAR].type = CLEAR;
    buttons[CLEAR].colorFill = Color(28, 28, 28);
    buttons[CLEAR].colorLine = Color(255, 0, 0);
    buttons[CLEAR].colorOnPressFill = Color(0, 255, 0);
    buttons[CLEAR].press = false;
    buttons[CLEAR].prepForPress = false;
    buttons[CLEAR].str = "CLEAR";

    ///CENTER - recentreaza diagrama
    buttons[CENTER].topLeft = {buttons[CLEAR].bottomRight.x + gapLength, MARGIN/4};
    buttons[CENTER].bottomRight = {buttons[CENTER].topLeft.x + BLOCK_BUTTON_WIDTH, (MARGIN/4)*3};
    buttons[CENTER].type = CENTER;
    buttons[CENTER].colorFill = Color(28, 28, 28);
    buttons[CENTER].colorLine = Color(255, 0, 0);
    buttons[CENTER].colorOnPressFill = Color(0, 255, 0);
    buttons[CENTER].press = false;
    buttons[CENTER].prepForPress = false;
    buttons[CENTER].str = "CENTER";

    // BACK - intoarcerea la editor
    backButton.topLeft = {originIDiagram.x+DIAGRAM_WIDTH-BLOCK_BUTTON_WIDTH, SCREEN_HEIGHT-(MARGIN/4)*3};
    backButton.bottomRight = {originIDiagram.x+DIAGRAM_WIDTH, SCREEN_HEIGHT-MARGIN/4};
    backButton.type = BACK;
    backButton.colorFill = Color(28, 28, 28);
    backButton.colorLine = Color(255, 0, 0);
    backButton.colorOnPressFill = Color(0, 255, 0);
    backButton.press = false;
    backButton.prepForPress = false;
    backButton.str = "BACK";

    // ARROW_LEFT - mutare caseta de text curenta la stanga
    buttons[ARROW_LEFT].topLeft = {originICode.x+CODE_WIDTH-10-2*BLOCK_BUTTON_WIDTH, SCREEN_HEIGHT-(MARGIN/4)*3};
    buttons[ARROW_LEFT].bottomRight = {originICode.x+CODE_WIDTH-10-BLOCK_BUTTON_WIDTH, SCREEN_HEIGHT-MARGIN/4};
    buttons[ARROW_LEFT].type = ARROW_LEFT;
    buttons[ARROW_LEFT].colorFill = Color(28, 28, 28);
    buttons[ARROW_LEFT].colorLine = Color(255, 0, 0);
    buttons[ARROW_LEFT].colorOnPressFill = Color(0, 255, 0);
    buttons[ARROW_LEFT].press = false;
    buttons[ARROW_LEFT].prepForPress = false;
    buttons[ARROW_LEFT].str = "<";

    // ARROW_RIGHT - mutare caseta de text curenta la dreapta
    buttons[ARROW_RIGHT].topLeft = {originICode.x+CODE_WIDTH-BLOCK_BUTTON_WIDTH, SCREEN_HEIGHT-(MARGIN/4)*3};
    buttons[ARROW_RIGHT].bottomRight = {originICode.x+CODE_WIDTH, SCREEN_HEIGHT-MARGIN/4};
    buttons[ARROW_RIGHT].type = ARROW_RIGHT;
    buttons[ARROW_RIGHT].colorFill = Color(28, 28, 28);
    buttons[ARROW_RIGHT].colorLine = Color(255, 0, 0);
    buttons[ARROW_RIGHT].colorOnPressFill = Color(0, 255, 0);
    buttons[ARROW_RIGHT].press = false;
    buttons[ARROW_RIGHT].prepForPress = false;
    buttons[ARROW_RIGHT].str = ">";
}

///verificarea erorilor in arbore, definite de functia checkErrors_DFS()
bool validTree = true;
pair <errorType, int> error;
set <char> declaredGlobal;
stack < set <char> > declaredLocal;
///

///retin valorile variabilelor si tipul lor
unordered_map <char, variableType> varType;
unordered_map <char, int> valMap;
unordered_map <char, string> strMap;
///

struct node {
    int lineOfCode = 0;
    instructionType instruction;
    string line;
    vector <string> words;

    float x, y; ///coordonatele stanga sus ale blocului
    float length;
    float height;

    int verticalNodeCount; ///voi avea nevoie pentru calculul inaltimii

    vector <node*> next;

};
typedef node* tree;

///arborele de sintaxa al pseudocodului
tree Tree;

///sterg arborele din memorie
void clearTree(node* &currentNode) {
    if(currentNode != NULL) {
        for(node* nextNode : currentNode -> next)
            clearTree(nextNode);

        currentNode -> line.clear();
        currentNode -> words.clear();
        currentNode -> next.clear();
        currentNode = NULL;
        delete currentNode;
    }
}

// initializarea arborelui
void initTree() {
    clearTree(Tree);
    Tree = new node;
    Tree -> instruction = EMPTY_NODE;

    Tree -> length = BLOCK_WIDTH * zoom; ///latimea standard a unui bloc
    Tree -> height = BLOCK_HEIGHT * zoom; ///inaltimea standard a unui bloc
    Tree -> x = diagramP.x; ///coordonata x de inceput (stanga sus)
    Tree -> y = diagramP.y; ///coordonata y de inceput stanga sus
}

// citirea din fisier line by line
string readLineFromFile() {
    string str;
    getline(fin, str);
    return str;
}

// imparte un sir in cuvinte
vector<string> splitIntoWords(string str) {
    vector<string> result;
    string word;
    int strSize = str.size();

    for(int i = 0; i < strSize; i++) {
        if(str[i] != ' ') {
            word.clear();
            while(str[i] != ' ' && i < strSize) {
                word.push_back(str[i]);
                i++;
            }
            i--;
            result.push_back(word);
        }
    }

    return result;
}

// int la string
string intToString(int value) {
    string str = "";
    if(value == 0)
        str = "0";
    int p = 1, auxValue = value;
    while(auxValue) {
        p *= 10;
        auxValue /= 10;
    }
    p /= 10;
    while(p) {
        str += (char)((value/p)%10+'0');
        p /= 10;
    }
    return str;
}

// verif string into vector<vector<char>>
bool verifDataFromString(int line, int column, string str) {
    if(column+(int)str.size()-1 < (int)codeEdit[CODE_EDIT][line].size()) {
        if(column-1 >= 0 && codeEdit[CODE_EDIT][line][column-1] != ' ')
            return false;
        for(int i = 0; i < (int)str.size(); i++) {
            if(codeEdit[CODE_EDIT][line][column] != str[i])
                return false;
            column++;
        }
        if(column < (int)codeEdit[CODE_EDIT][line].size() && codeEdit[CODE_EDIT][line][column] != ' ')
            return false;
    }
    else
        return false;
    return true;
}

// stergerea din memorie a  vector<vector<char>>
void clearCodeMemory(editFileType editFT) {
    for(int i = 0; i < (int)codeEdit[editFT].size(); i++)
        codeEdit[editFT][i].clear();
    codeEdit[editFT].clear();
}

// filename la vector<vector<char>>
void getDataFromFile(string filename, editFileType editFT) {
    clearCodeMemory(editFT);
    fin.open(filename);
    if(fin.is_open()) {
        while(!fin.eof()) {
            string str = readLineFromFile();

            codeEdit[editFT].push_back(vector<char>());
            for(int i = 0 ; i < (int)str.size(); i++)
                codeEdit[editFT][codeEdit[editFT].size()-1].push_back(str[i]);
        }
    }

    if(codeEdit[editFT].empty())
        codeEdit[editFT].push_back(vector<char>());
    fin.close();
}

// vector<vector<char>> la filename
void setDataToFile(string filename, editFileType editFT) {
    fout.open(filename);

    ///daca nu am o linie libera la final, o adaug (este necesara pt executarea codului)
    ///nu mai sunt nevoit astfel sa dau enter manual din program la finalul codului
    if(codeEdit[editFT][codeEdit[editFT].size() - 1].size() > 0)
        codeEdit[editFT].push_back(vector<char>());

    for(int line = 0; line < (int)codeEdit[editFT].size(); line++) {
        for(int column = 0; column < (int)codeEdit[editFT][line].size(); column++) {
            fout << codeEdit[editFT][line][column];
        }
        if(line != (int)codeEdit[editFT].size()-1)
            fout << '\n';
    }
    fout.close();
}

// obtine instruction type
instructionType getInstructionType(vector<string> vStr) {
    ///returneaza primul cuvant
    string word = "";
    if(!vStr.empty())
        word = vStr[0];

    if(word == "var")
        return VAR;
    else if(word == "set")
        return SET;
    else if(word == "if")
        return IF;
    else if(word == "while")
        return WHILE;
    else if(word == "repeat")
        return REPEAT;
    else if(word == "pass")
        return PASS;
    else if(word == "read")
        return READ;
    else if(word == "print")
        return PRINT;
    else if(word == "else" || word == "endif" || word == "endwhile" || word == "until")
        return END;

    return ERROR;
}

// construirea arborelui
int lineCount = 0;
string untilExpr = ""; ///trebuie salvata expresia din repeat until pe revenirea din recursivitate
void buildTree(node* &currentNode) {
    //static int lineCount = 0; ///linia de cod la care ma aflu
    if(currentNode -> instruction == EMPTY_NODE) {
        bool exitWhile = false;
        while(!exitWhile) {
            string lineStr = readLineFromFile();
            lineCount++;

            if(fin.eof()) ///am ajuns la finalul fisierului
                exitWhile = true;
            else {
                ///cazul in care am linie libera sau doar cu spatii goale
                bool notEmptyLine = false;
                for(unsigned i = 0; i < lineStr.size() && !notEmptyLine; i++)
                    if(lineStr[i] != ' ')
                        notEmptyLine = true;
                if(!notEmptyLine)
                    continue;
                ///

                node* newNode = new node;
                newNode -> lineOfCode = lineCount;
                newNode -> line = lineStr;
                newNode -> words = splitIntoWords(lineStr);
                newNode -> instruction = getInstructionType(newNode -> words);

                ///daca intalnesc else, endif sau endwhile trebuie sa ma intorc in sus in arbore
                if(newNode -> instruction == END) {
                    exitWhile = true; ///ies din while chiar daca nu termin de citit fisierul

                    ///daca am altceva pe langa acel "else\endif\endwhile" atunci linia nu e valida
                    ///daca am "until" atunci trebuie sa am exact 2 cuvinte
                    if((newNode -> words[0] != "until" && newNode -> words.size() != 1)  || (newNode -> words[0] == "until" && newNode -> words.size() != 2)) {
                        validTree = false;
                        error = make_pair(SYNTAX_ERROR_INCOMPLETE_LINE, newNode -> lineOfCode);
                        delete newNode; ///nu am avut nevoie de nod
                        return;
                    }

                    if(newNode -> words[0] == "until") ///salvez expresia din until si o retin in nodul repeat (pt repeat ... until expresie)
                        untilExpr = newNode -> words[1];

                    delete newNode; ///nu am avut nevoie de nod
                }
                else {
                    ///am legat nodurile
                    (currentNode -> next).push_back(newNode);
                    buildTree(newNode);
                }
            }
        }
    }
    else if(currentNode -> instruction == IF) {
        node* trueNode = new node;
        trueNode -> instruction = EMPTY_NODE;
        (currentNode -> next).push_back(trueNode);
        buildTree(trueNode);

        node* falseNode = new node;
        falseNode -> instruction = EMPTY_NODE;
        (currentNode -> next).push_back(falseNode);
        buildTree(falseNode);
    }
    else if(currentNode -> instruction == WHILE) {
        node* newNode = new node;
        newNode -> instruction = EMPTY_NODE;
        (currentNode -> next).push_back(newNode);
        buildTree(newNode);
    }
    else if(currentNode -> instruction == REPEAT) {
        node* newNode = new node;
        newNode -> instruction = EMPTY_NODE;
        (currentNode -> next).push_back(newNode);

        buildTree(newNode);

        ///salvez expresia din until in nodul repeat
        if(untilExpr != "") {
            currentNode -> line += " ";
            currentNode -> line += untilExpr;
            currentNode -> words.push_back(untilExpr);
        }
        untilExpr = "";
    }
}

///parcurgerea arborelui; va trebui apelat de mai multe ori pt verificarea corectitudinii
///pseudocodului si calculul coordonatelor si marimii fiecarui bloc
void TreeDFS(node* currentNode, int height) {
    if(currentNode != NULL) {
        cout << "linie: " << currentNode -> lineOfCode << "\n";
        cout << "verticalCount: " << currentNode -> verticalNodeCount << "\n";
        cout << currentNode -> instruction << " ";
        if(currentNode -> instruction != EMPTY_NODE)
            cout << currentNode -> line << "\n";
        else
            cout << "\n";
        cout << "\n";

        for(node* nextNode : currentNode -> next)
            TreeDFS(nextNode, height + 1);
    }
}

/////////evaluare de expresii
int evalExpr(string expr);
int term(string expr);
int factor(string expr);

int exprPtr = 0;
int evalExpr(string expr) {
    int val = term(expr);
    while(expr[exprPtr] == '+' || expr[exprPtr] == '-') {
        if(expr[exprPtr] == '+') {
            exprPtr++;
            val += term(expr);
        }
        else {
            exprPtr++;
            val -= term(expr);
        }
    }
    return val;
}

int term(string expr) {
    int val = factor(expr);
    while(expr[exprPtr] == '*' || expr[exprPtr] == '/' || expr[exprPtr] == '%') {
        if(expr[exprPtr] == '*') {
            exprPtr++;
            val *= factor(expr);
        }
        else if(expr[exprPtr] == '/') {
            exprPtr++;
            val /= factor(expr);
        }
        else {
            exprPtr++;
            val %= factor(expr);
        }
    }
    return val;
}

int factor(string expr) {
    int val = 0;
    if(expr[exprPtr] == '(') {
        exprPtr++;
        val = evalExpr(expr);
        exprPtr++;
    }
    else {
        if(isdigit(expr[exprPtr])) {
            val = 0;
            while(isdigit(expr[exprPtr])) {
                val = val * 10 + (expr[exprPtr] - '0');
                exprPtr++;
            }
        }
        else if(isalpha(expr[exprPtr])) {
            val = valMap[expr[exprPtr]];
            exprPtr++;
        }
    }
    return val;
}

bool evalCondition(string expr) { ///pentru conditiile din if/while/repeat until
    unsigned int i = 0;
    string expr1, expr2, comp;
    for(i = 0; i < expr.size() && !strchr("!<=>", expr[i]); i++)
        expr1.push_back(expr[i]);
    for( ; i < expr.size() && strchr("!<=>", expr[i]); i++)
        comp.push_back(expr[i]);
    for( ; i < expr.size(); i++)
        expr2.push_back(expr[i]);

    if(comp.empty()) {
        int eval = (evalExpr(expr1) != 0);
        exprPtr = 0;
        return eval;
    }
    else if(comp == "==") {
        int evalExpr1 = evalExpr(expr1);
        exprPtr = 0;
        int evalExpr2 = evalExpr(expr2);
        exprPtr = 0;
        return evalExpr1 == evalExpr2;
    }
    else if(comp == ">") {
        int evalExpr1 = evalExpr(expr1);
        exprPtr = 0;
        int evalExpr2 = evalExpr(expr2);
        exprPtr = 0;
        return evalExpr1 > evalExpr2;
    }
    else if(comp == "<") {
        int evalExpr1 = evalExpr(expr1);
        exprPtr = 0;
        int evalExpr2 = evalExpr(expr2);
        exprPtr = 0;
        return evalExpr1 < evalExpr2;
    }
    else if(comp == "<=") {
        int evalExpr1 = evalExpr(expr1);
        exprPtr = 0;
        int evalExpr2 = evalExpr(expr2);
        exprPtr = 0;
        return evalExpr1 <= evalExpr2;
    }
    else if(comp == ">=") {
        int evalExpr1 = evalExpr(expr1);
        exprPtr = 0;
        int evalExpr2 = evalExpr(expr2);
        exprPtr = 0;
        return evalExpr1 >= evalExpr2;
    }
    else if(comp == "!=") {
        int evalExpr1 = evalExpr(expr1);
        exprPtr = 0;
        int evalExpr2 = evalExpr(expr2);
        exprPtr = 0;
        return evalExpr1 != evalExpr2;
    }

    return 0;
}
///////////////

///verifica daca o expresie este valida
///intr-o instructiune de tip set pot avea doar expresii "scurte" (fara comparatii, fullExpr = 0)
///in expresii din if/bucle am expresii full (fullExpr = 1)
///expresie: variabile, numere, paranteze, operatori: +, -, *, /, %, >, <, >=, <=, ==;
///expresie intreaga -> expresie_scurta1 <comp> expresie_scurta2
bool isValidExpr(string expression, int codeLine, bool fullExpr) {
    char op[] = "+-*/%";
    char comp[] = "<=>!";
    int st = 0; ///stiva
    int compCount = 0; ///sa evit erorile de genul "if a>b>c ..."

    for(char ch : expression) {
        ///verificare caractere invalide
        if(!isalnum(ch) && ch != '(' && ch != ')' && !strchr(op, ch)) {
            if((fullExpr && !strchr(comp, ch)) || !fullExpr) {
                error = make_pair(ERROR_EXPRESSION, codeLine);
                return false;
            }
        }

        ///verificare parantezare
        if(ch == '(')
            st++;
        else if(ch == ')') {
            if(st <= 0) {
                error = make_pair(ERROR_EXPRESSION, codeLine);
                return false;
            }
            else
                st--;
        }
    }
    if(st != 0) {
        error = make_pair(ERROR_EXPRESSION, codeLine);
        return false;
    }

    ///impart expresia in cuvinte
    vector<string> tokens;
    for(unsigned i = 0; i < expression.size(); i++) {
        char ch = expression[i];
        string token;

        if(isdigit(ch)) {
            while(isdigit(ch) && i < expression.size()) {
                token.push_back(ch);
                i++;
                ch = expression[i];
            }
            i--;

            tokens.push_back(token);
        }
        else if(strchr(comp, ch)) {
            compCount++;
            while(strchr(comp, ch) && i < expression.size()) {
                token.push_back(ch);
                i++;
                ch = expression[i];
            }
            i--;

            tokens.push_back(token);
        }
        else {
            token.push_back(ch);
            tokens.push_back(token);
        }
    }
    if(compCount >= 2) {
        error = make_pair(ERROR_EXPRESSION, codeLine);
        return false;
    }


    for(unsigned i = 0; i < tokens.size() - 1; i++) {
        if(tokens[i][0] == '(') {
            char ch = tokens[i + 1][0];
            if(ch == ')' || ch == '/' || ch == '*' || ch == '%' || strchr(comp, ch)) {
                error = make_pair(ERROR_EXPRESSION, codeLine);
                return false;
            }
        }
        else if(tokens[i][0] == ')') {
            char ch = tokens[i + 1][0];
            if(ch == '(' || isalnum(ch)) {
                error = make_pair(ERROR_EXPRESSION, codeLine);
                return false;
            }
        }
        else if(strchr(op, tokens[i][0])) { ///este operator
            char ch = tokens[i + 1][0];
            if(strchr(op, ch) || ch == ')' || strchr(comp, ch)) {
                error = make_pair(ERROR_EXPRESSION, codeLine);
                return false;
            }
        }
        else if(isdigit(tokens[i][0])) { ///este numar
            char ch = tokens[i + 1][0];
            if(ch == '(' || isalpha(ch)) {
                error = make_pair(ERROR_EXPRESSION, codeLine);
                return false;
            }
        }
        else if(isalpha(tokens[i][0])) { ///este variabila
            char ch = tokens[i + 1][0];
            if(isalnum(ch)) {
                error = make_pair(SYNTAX_ERROR_VARIABLE, codeLine);
                return false;
            }

            if(ch == '(') {
                error = make_pair(ERROR_EXPRESSION, codeLine);
                return false;
            }

            if(declaredGlobal.find(tokens[i][0]) == declaredGlobal.end()) { ///nu am declarat variabila
                error = make_pair(ERROR_UNDECLARED, codeLine);
                return false;
            }

            if(varType[tokens[i][0]] == STRING) {
                error = make_pair(ERROR_STRING_OPERATIONS, codeLine);
                return false;
            }
        }
        else if(fullExpr && strchr(comp, tokens[i][0])) { ///comparatie
            if(tokens[i].size() > 2) {
                error = make_pair(ERROR_EXPRESSION, codeLine);
                return false;
            }
            else if(tokens[i].size() == 1) {
                if(tokens[i][0] == '=' || tokens[i][0] == '!') {
                    error = make_pair(ERROR_EXPRESSION, codeLine);
                    return false;
                }
            }
            else {
                if(tokens[i][1] != '=') {
                    error = make_pair(ERROR_EXPRESSION, codeLine);
                    return false;
                }
            }

            char ch = tokens[i + 1][0];
            if(ch == ')' || ch == '*' || ch == '/' || strchr(comp, ch)) {
                error = make_pair(ERROR_EXPRESSION, codeLine);
                return false;
            }
        }
    }

    ///ultimul token
    char ch = tokens[tokens.size() - 1][0];
    if(strchr(op, ch) || strchr(comp, ch)) {
        error = make_pair(ERROR_EXPRESSION, codeLine);
        return false;
    }
    else if(isalpha(ch)) {
        if(declaredGlobal.find(ch) == declaredGlobal.end()) {
            error = make_pair(ERROR_UNDECLARED, codeLine);
            return false;
        }

        if(varType[ch] == STRING) {
            error = make_pair(ERROR_STRING_OPERATIONS, codeLine);
            return false;
        }
    }

    return true;

}

///verificarea erorilor
///mai sunt de tratat cazurile cu expresii gresite
void checkErrors_DFS(node* currentNode) {
    if(currentNode != NULL && validTree) {
        if(currentNode -> instruction == EMPTY_NODE)
            declaredLocal.push(set<char>());
        else if(currentNode -> instruction == ERROR) { ///tipul de instructiune nu este recunoscut
            validTree = false;
            error = make_pair(SYNTAX_ERROR_INSTRUCTION, currentNode -> lineOfCode);
            return;
        }
        else if(currentNode -> instruction == VAR) {
            if(currentNode -> words.size() != 3) { ///o linie de tip var trebuie sa aiba 3 cuvinte; aceeasi idee si mai jos
                validTree = false;
                error = make_pair(SYNTAX_ERROR_INCOMPLETE_LINE, currentNode -> lineOfCode);
                return;
            }
            else {
                if(currentNode -> words[1] != "int" && currentNode -> words[1] != "string") {
                    validTree = false;
                    error = make_pair(SYNTAX_ERROR_VARTYPE, currentNode -> lineOfCode);
                    return;
                }

                char ch = currentNode -> words[2][0];
                if(currentNode -> words[2].size() != 1 || !isalpha(ch)) {
                    validTree = false;
                    error = make_pair(SYNTAX_ERROR_VARIABLE, currentNode -> lineOfCode);
                    return;
                }

                ///declararea variabilelor
                if(declaredGlobal.find(ch) == declaredGlobal.end()) {
                    declaredGlobal.insert(ch);
                    declaredLocal.top().insert(ch);

                    if(currentNode -> words[1] == "int")
                        varType[ch] = INT;
                    else
                        varType[ch] = STRING;
                }
                else {
                    validTree = false;
                    error = make_pair(ERROR_MULTIPLE_DECLARATION, currentNode -> lineOfCode);
                    return;
                }
            }
        }
        else if(currentNode -> instruction == SET) {
            if(currentNode -> words.size() != 3) {
                validTree = false;
                error = make_pair(SYNTAX_ERROR_INCOMPLETE_LINE, currentNode -> lineOfCode);
                return;
            }
            else {
                char ch = currentNode -> words[1][0];
                if(currentNode -> words[1].size() != 1 || !isalpha(ch)) {
                    validTree = false;
                    error = make_pair(SYNTAX_ERROR_VARIABLE, currentNode -> lineOfCode);
                    return;
                }

                if(declaredGlobal.find(ch) == declaredGlobal.end()) { ///nu am declarat variabila ch
                    validTree = false;
                    error = make_pair(ERROR_UNDECLARED, currentNode -> lineOfCode);
                    return;
                }

                ///de verificat corectitudinea expresiei (currentNode -> words[2])
                ///daca am variabila string o sa consider al treilea cuvant drept string, nu expresie
                if(varType[ch] == INT) {
                    bool fullExpr = false;
                    validTree = isValidExpr(currentNode -> words[2], currentNode -> lineOfCode, fullExpr);
                }
            }
        }
        else if(currentNode -> instruction == READ) {
            if(currentNode -> words.size() != 2) {
                validTree = false;
                error = make_pair(SYNTAX_ERROR_INCOMPLETE_LINE, currentNode -> lineOfCode);
                return;
            }
            else {
                char ch = currentNode -> words[1][0];
                if(currentNode -> words[1].size() != 1 || !isalpha(ch)) {
                    validTree = false;
                    error = make_pair(SYNTAX_ERROR_VARIABLE, currentNode -> lineOfCode);
                    return;
                }

                if(declaredGlobal.find(ch) == declaredGlobal.end()) { ///nu am declarat variabila ch
                    validTree = false;
                    error = make_pair(ERROR_UNDECLARED, currentNode -> lineOfCode);
                    return;
                }
            }
        }
        else if(currentNode -> instruction == PRINT) {
            if(currentNode -> words.size() != 2) {
                validTree = false;
                error = make_pair(SYNTAX_ERROR_INCOMPLETE_LINE, currentNode -> lineOfCode);
                return;
            }
            else {

                char ch = currentNode -> words[1][0];
                if(currentNode -> words[1].size() != 1 || !isalpha(ch)) {
                    validTree = false;
                    error = make_pair(SYNTAX_ERROR_VARIABLE, currentNode -> lineOfCode);
                    return;
                }

                if(declaredGlobal.find(ch) == declaredGlobal.end()) { ///nu am declarat variabila ch
                    validTree = false;
                    error = make_pair(ERROR_UNDECLARED, currentNode -> lineOfCode);
                    return;
                }
            }
        }
        else if(currentNode -> instruction == PASS || currentNode -> instruction == END) {
            if(currentNode -> words.size() != 1) {
                validTree = false;
                error = make_pair(SYNTAX_ERROR_INCOMPLETE_LINE, currentNode -> lineOfCode);
                return;
            }
        }
        else if(currentNode -> instruction == IF || currentNode -> instruction == WHILE) {
            if(currentNode -> words.size() != 2) {
                validTree = false;
                error = make_pair(SYNTAX_ERROR_INCOMPLETE_LINE, currentNode -> lineOfCode);
                return;
            }
            else {
                bool fullExpr = true;
                validTree = isValidExpr(currentNode -> words[1], currentNode -> lineOfCode, fullExpr);
            }
        }
        else if(currentNode -> instruction == REPEAT) {
            if(currentNode -> words.size() > 2) {
                validTree = false;
                error = make_pair(SYNTAX_ERROR_INCOMPLETE_LINE, currentNode -> lineOfCode);
                return;
            }
            else {
                bool fullExpr = true;
                validTree = isValidExpr(currentNode -> words[1], currentNode -> lineOfCode, fullExpr);
            }
        }

        for(node* nextNode : currentNode -> next)
            checkErrors_DFS(nextNode);

        ///sterg variabilele declarate in interiorul unui if/while deoarece am ajuns la final
        if(currentNode -> instruction == EMPTY_NODE) {
            for(char ch : declaredLocal.top())
                declaredGlobal.erase(ch);
            declaredLocal.pop();
        }
    }
}

///trateaza cazurile in care pun gresit endif, else, endwhile, etc.
///pt fiecare while trebuie sa am un endwhile, iar pt fiecare if un else si dupa else un endif
///tratez problema exact la fel ca cea a parantezarii unei expresii
void checkStructure() {
    stack <char> st;
    int line = 0;
    while(!fin.eof()) {
        line++;
        string str = readLineFromFile();
        if(!str.empty()) {
            vector <string> strWords;
            string instruction;
            strWords = splitIntoWords(str);
            if(!strWords.empty())
                instruction = splitIntoWords(str)[0];

            if(instruction == "if") {
                st.push('[');
                st.push('(');
            }
            else if(instruction == "else") {
                if(st.empty() || st.top() != '(') {
                    validTree = false;
                    error = make_pair(ERROR_INVALID_STRUCTURE, line);
                    return;
                }
                else
                    st.pop();

            }
            else if(instruction == "endif") {
                if(st.empty() || st.top() != '[') {
                    validTree = false;
                    error = make_pair(ERROR_INVALID_STRUCTURE, line);
                    return;
                }
                else
                    st.pop();
            }
            else if(instruction == "while") {
                st.push('{');
            }
            else if(instruction == "endwhile") {
                if(st.empty() || st.top() != '{') {
                    validTree = false;
                    error = make_pair(ERROR_INVALID_STRUCTURE, line);
                    return;
                }
                else
                    st.pop();
            }
            else if(instruction == "repeat") {
                st.push('+');
            }
            else if(instruction == "until") {
                if(st.empty() || st.top() != '+') {
                    validTree = false;
                    error = make_pair(ERROR_INVALID_STRUCTURE, line);
                    return;
                }
                else
                    st.pop();
            }
        }
    }

    if(!st.empty()) {
        validTree = false;
        error = make_pair(ERROR_INVALID_STRUCTURE, line);
    }
}

///dp[empty_node] = noduri_simple + (1 + dp[while_empty]) + (1 + max(dp[if_T], dp[if_F]))
///dp - tinut in verticalNodeCount
void buildDP_DFS(node* &currentNode) {
    if(currentNode != NULL) {
        if(currentNode -> instruction == EMPTY_NODE)
            currentNode -> verticalNodeCount = 0;

        for(node* nextNode : currentNode -> next) {
            buildDP_DFS(nextNode);
            if(currentNode -> instruction == EMPTY_NODE)
                currentNode -> verticalNodeCount += nextNode -> verticalNodeCount;
        }

        if(currentNode -> instruction == IF)
            currentNode -> verticalNodeCount = 1 + max(currentNode -> next[0] -> verticalNodeCount, currentNode -> next[1] -> verticalNodeCount);
        else if(currentNode -> instruction == WHILE || currentNode -> instruction == REPEAT)
            currentNode -> verticalNodeCount = 1 + currentNode -> next[0] -> verticalNodeCount;
        else if(currentNode -> instruction != EMPTY_NODE)
            currentNode -> verticalNodeCount = 1;

    }
}

// pregatire pentru desenarea diagramei
void buildDiagram_DFS(node* &currentNode, node* &emptyFather) {
    if(currentNode != NULL) {
        ///creez dimensiunile blocului curent in functie de nodul tata
        if(currentNode -> instruction != EMPTY_NODE) {
            currentNode -> length = emptyFather -> length;

            if(currentNode -> instruction != REPEAT) {
                currentNode -> x = emptyFather -> x;
                currentNode -> y = emptyFather -> y;

                emptyFather -> y += emptyFather -> height;
            }

            if(currentNode -> instruction != WHILE && currentNode -> instruction != REPEAT) ///bloc de tip dreptunghi
                currentNode -> height = emptyFather -> height;
            else ///bloc de tip bucla, trebuie sa acopere toate blocurile din interior
                currentNode -> height = currentNode -> verticalNodeCount * emptyFather -> height;

        }

        ///initializez urmatoarele noduri goale care vor deveni tati
        if(currentNode -> instruction == IF) {
            currentNode -> next[0] -> length = emptyFather -> length / 2.; ///nodul gol de tip true
            currentNode -> next[1] -> length = emptyFather -> length / 2.; ///nodul gol de tip false
            currentNode -> next[0] -> x = emptyFather -> x;
            currentNode -> next[1] -> x = emptyFather -> x + emptyFather -> length / 2.;

            currentNode -> next[0] -> height = 1.f *
            (currentNode -> verticalNodeCount - 1) * emptyFather -> height / currentNode -> next[0] -> verticalNodeCount;
            currentNode -> next[1] -> height = 1.f *
            (currentNode -> verticalNodeCount - 1) * emptyFather -> height / currentNode -> next[1] -> verticalNodeCount;
            currentNode -> next[0] -> y = currentNode -> next[1] -> y = emptyFather -> y;

            emptyFather -> y += (currentNode -> verticalNodeCount - 1) * emptyFather -> height;

        }
        else if(currentNode -> instruction == WHILE || currentNode -> instruction == REPEAT) {
            float offset = emptyFather -> length / 5.; ///lungimea pentru bara din stanga
            currentNode -> next[0] -> length = emptyFather -> length - offset;
            currentNode -> next[0] -> x = emptyFather -> x + offset;

            currentNode -> next[0] -> height = emptyFather -> height;
            currentNode -> next[0] -> y = emptyFather -> y;

            emptyFather -> y += (currentNode -> verticalNodeCount - 1) * emptyFather -> height;
        }

        for(node* nextNode : currentNode -> next) {
            if(currentNode -> instruction == EMPTY_NODE)
                buildDiagram_DFS(nextNode, currentNode);
            else
                buildDiagram_DFS(nextNode, emptyFather);
        }

        if(currentNode -> instruction == REPEAT) {
            currentNode -> x = emptyFather -> x;
            currentNode -> y = emptyFather -> y;

            emptyFather -> y += emptyFather -> height;
        }
    }
}

void compileCode_DFS(node* currentNode) {
    if(currentNode != NULL) {
        if(currentNode -> instruction == EMPTY_NODE) {
            for(node* nextNode : currentNode -> next)
                compileCode_DFS(nextNode);
        }
        else if(currentNode -> instruction == SET) {
            char variable = currentNode -> words[1][0];
            if(varType[variable] == INT) {
                valMap[variable] = evalExpr(currentNode -> words[2]);
                exprPtr = 0;
            }
            else {
                strMap[variable] = currentNode -> words[2];
            }
        }
        else if(currentNode -> instruction == PRINT) {
            char variable = currentNode -> words[1][0];
            if(varType[variable] == INT)
                fout << valMap[variable] << "\n";
            else
                fout << strMap[variable] << "\n";
        }
        else if(currentNode -> instruction == READ) {
            char variable = currentNode -> words[1][0];
            if (varType[variable] == INT)
                fin >> valMap[variable];
            else
                fin >> strMap[variable];
        }
        else if(currentNode -> instruction == IF) {
            if (evalCondition(currentNode -> words[1]))
                compileCode_DFS(currentNode -> next[0]);
            else
                compileCode_DFS(currentNode -> next[1]);
        }
        else if(currentNode -> instruction == WHILE) {
            while(evalCondition(currentNode -> words[1]))
                compileCode_DFS(currentNode -> next[0]);
        }
        else if(currentNode -> instruction == REPEAT) {
            compileCode_DFS(currentNode -> next[0]);
            while (!evalCondition(currentNode -> words[1]))
                compileCode_DFS(currentNode -> next[0]);
        }
    }
}

// stergerea pozitiilor din Tree
void deletePositionDiagram_DFS(node* currentNode) {
    if(currentNode != NULL) {
        currentNode -> x = 0;
        currentNode -> y = 0;
        currentNode -> length = 0;
        currentNode -> height = 0;

        for(node* nextNode:currentNode -> next)
            deletePositionDiagram_DFS(nextNode);
    }
}

// desenarea diagramei
Font font;
void printDiagram_DFS(node* currentNode, RenderWindow &window) {
    if(currentNode != NULL) {
        if(currentNode -> instruction == IF) {
            Point topLeft;
            topLeft.x = currentNode -> x;
            topLeft.y = currentNode -> y;
            Point bottomRight;
            bottomRight.x = topLeft.x + currentNode -> length;
            bottomRight.y = topLeft.y + currentNode -> height;

            Box box;
            box.x = topLeft.x + (bottomRight.x-topLeft.x)/4;
            box.y = topLeft.y+5;
            box.length = (bottomRight.x-topLeft.x)/2;
            box.height = (bottomRight.y-topLeft.y)/2;

            string str = "";
            for(int i = 1; i < (int)currentNode -> words.size(); i++) {
                if(i > 1)
                    str += ' ';
                str += currentNode -> words[i];
            }
            str += '?';

            // desenarea conditiei pentru if
            window.draw(createText(box, str, font));

            // desenare True si False
            str = "T";
            box.x = topLeft.x;
            box.y = topLeft.y+(bottomRight.y-topLeft.y)/2;
            box.length = (bottomRight.x-topLeft.x)/4;
            box.height = (bottomRight.y-topLeft.y)/2;

            window.draw(createText(box, str, font));

            str = "F";
            box.x = bottomRight.x-(bottomRight.x-topLeft.x)/4;
            box.y = topLeft.y+(bottomRight.y-topLeft.y)/2;
            box.length = (bottomRight.x-topLeft.x)/4;
            box.height = (bottomRight.y-topLeft.y)/2;

            window.draw(createText(box, str, font));

            // desenarea blocului pentru if
            window.draw(decisionCreate(topLeft, bottomRight));
        }
        else if(currentNode -> instruction == WHILE) {
            Point topLeft;
            topLeft.x = currentNode -> x;
            topLeft.y = currentNode -> y;
            Point bottomRight;
            bottomRight.x = topLeft.x + currentNode -> length;
            bottomRight.y = topLeft.y + currentNode -> height;
            float offset = currentNode -> length / 5.; ///lungimea pentru bara din stanga
            float rectangleHeight = currentNode -> height / currentNode -> verticalNodeCount; ///inaltimea blocului fara bara din stanga

            Box box;
            box.x = topLeft.x+offset;
            box.y = topLeft.y;
            box.length = bottomRight.x-box.x;
            box.height = rectangleHeight;

            string str = "";
            for(int i = 0; i < (int)currentNode -> words.size(); i++) {
                str += currentNode -> words[i];
                if (i != (int)currentNode -> words.size() - 1)
                    str += ' ';
            }

            // desenare text pentru while
            window.draw(createText(box, str, font));

            // desenarea blocului pentru while
            window.draw(iterationWCreate(topLeft, bottomRight, offset, rectangleHeight));
        }
        else if(currentNode -> instruction == REPEAT) {
            float offset = currentNode -> length / 5.; ///lungimea pentru bara din stanga
            float rectangleHeight = currentNode -> height / currentNode -> verticalNodeCount; ///inaltimea blocului fara bara din stanga
            Point bottomRight;
            bottomRight.x = currentNode -> x + currentNode -> length;
            bottomRight.y = currentNode -> y + rectangleHeight;
            Point topLeft;
            topLeft.x = currentNode -> x;
            topLeft.y = bottomRight.y - currentNode -> height;

            Box box;
            box.x = topLeft.x+offset;
            box.y = currentNode -> y;
            box.length = bottomRight.x-box.x;
            box.height = rectangleHeight;

            string str = "";
            for(int i = 0; i < (int)currentNode -> words.size(); i++) {
                str += currentNode -> words[i];
                if (i != (int)currentNode -> words.size() - 1)
                    str += ' ';
            }

            // desenare text pentru repeat until
            window.draw(createText(box, str, font));

            // desenarea blocului pentru repeat until
            window.draw(iterationUCreate(topLeft, bottomRight, offset, rectangleHeight));

        }
        else if(currentNode -> instruction != EMPTY_NODE) {
            Point topLeft;
            topLeft.x = currentNode -> x;
            topLeft.y = currentNode -> y;
            Point bottomRight;
            bottomRight.x = topLeft.x + currentNode -> length;
            bottomRight.y = topLeft.y + currentNode -> height;

            Box box;
            box.x = topLeft.x;
            box.y = topLeft.y;
            box.length = bottomRight.x-topLeft.x;
            box.height = bottomRight.y-topLeft.y;

            string str = "";
            for(int i = 0; i < (int)currentNode -> words.size(); i++) {
                str += currentNode -> words[i];
                if (i != (int)currentNode -> words.size() - 1)
                    str += ' ';
            }

            // desenare text pentru singleStep
            window.draw(createText(box, str, font));

            // desenarea blocului pentru singleStep
            window.draw(singleStepCreate(topLeft, bottomRight));
        }

        for(node* nextNode : currentNode -> next)
            printDiagram_DFS(nextNode, window);
    }
}

// desenarea codului
void printCodeEdit(RenderWindow &window) {
    Box box;
    string str;

    int aux = codeEdit[editFileT].size(), p = 0;
    while(aux) {
        p++;
        aux /= 10;
    }

    CODEEDIT_MARGIN_WIDTH = p*10+p*1.5;
    LIMIT_COLUMN_CODE = (CODE_WIDTH-CODEEDIT_MARGIN_WIDTH*2)/BLOCK_CODE_WIDTH-1;
    LIMIT_LINE_CODE = (CODE_HEIGHT-CODEEDIT_MARGIN_HEIGHT*2)/BLOCK_CODE_HEIGHT;

    codeP = {0, 0};
    if(cursorCP.x > LIMIT_COLUMN_CODE)
        codeP.x = cursorCP.x-LIMIT_COLUMN_CODE;
    if(cursorCP.y > LIMIT_LINE_CODE-1)
        codeP.y = cursorCP.y-LIMIT_LINE_CODE+1;

    int k = 1, nr = 0;
    p = 10, aux = codeP.y+1;
    while(aux) {
        nr++;
        aux /= 10;
    }
    aux = nr;
    while(aux) {
        if(aux%2 == 1)
            aux--, k *= p;
        else
            aux /= 2, p = p*p;
    }

    Color syntaxColor = Color(234, 138, 220);
    Color baseColor = Color(255, 255, 255);

    vector <vector <Color>> colorEdit;
    for(int line = 0; line < (int)codeEdit[CODE_EDIT].size(); line++) {
        colorEdit.push_back(vector <Color>());
        for(int column = 0; column < (int)codeEdit[CODE_EDIT][line].size(); column++)
            colorEdit[line].push_back(baseColor);
    }

    vector <string> vstr;
    vstr.push_back("var");
    vstr.push_back("set");
    vstr.push_back("if");
    vstr.push_back("while");
    vstr.push_back("repeat");
    vstr.push_back("pass");
    vstr.push_back("read");
    vstr.push_back("print");
    vstr.push_back("else");
    vstr.push_back("endif");
    vstr.push_back("endwhile");
    vstr.push_back("until");

    vector <string> vstr1;
    vstr1.push_back("int");
    vstr1.push_back("string");
    for(int line = 0; line < (int)codeEdit[CODE_EDIT].size(); line++) {
        for(int column = 0; column < (int)codeEdit[CODE_EDIT][line].size(); column++) {
            for(int k = 0; k < (int)vstr.size(); k++) {
                if(verifDataFromString(line, column, vstr[k])) {
                    for(int i = 0; i < (int)vstr[k].size(); i++)
                        colorEdit[line][column+i] = syntaxColor;
                    if(vstr[k] == "var") {
                        for(int i = column+1; i < (int)codeEdit[CODE_EDIT][line].size(); i++) {
                            for(int kk = 0; kk < (int)vstr1.size(); kk++) {
                                if(verifDataFromString(line, i, vstr1[kk])) {
                                    for(int ii = 0; ii < (int)vstr1[kk].size(); ii++)
                                        colorEdit[line][i+ii] = Color(254, 128, 25);
                                    i = codeEdit[CODE_EDIT][line].size();
                                    break;
                                }
                            }
                        }
                    }
                    column = codeEdit[CODE_EDIT][line].size();
                    break;
                }
            }
        }
    }


    for(int line = 0, aLine = codeP.y; aLine < (int)codeEdit[editFileT].size() && line < (int)codeEdit[editFileT].size() && line < LIMIT_LINE_CODE; line++, aLine++) {
        box.x = originICode.x+CODEEDIT_MARGIN_WIDTH-(nr-1)*BLOCK_CODE_WIDTH;
        box.y = originICode.y+line*BLOCK_CODE_HEIGHT+CODEEDIT_MARGIN_HEIGHT;
        box.length = nr*BLOCK_CODE_WIDTH;
        box.height = BLOCK_CODE_HEIGHT;

        str = intToString(aLine+1);
        window.draw(createTextForCode(box, str, font, baseColor));
        if(aLine == k-2)
            k *= 10, nr++;
        for(int column = 0, aColumn = codeP.x; aColumn < (int)codeEdit[editFileT][aLine].size() && column < (int)codeEdit[editFileT][aLine].size() && column < LIMIT_COLUMN_CODE; column++, aColumn++) {
            box.x = originICode.x+(column+1)*BLOCK_CODE_WIDTH+CODEEDIT_MARGIN_WIDTH;
            box.y = originICode.y+line*BLOCK_CODE_HEIGHT+CODEEDIT_MARGIN_HEIGHT;
            box.length = BLOCK_CODE_WIDTH;
            box.height = BLOCK_CODE_HEIGHT;

            str = codeEdit[editFileT][aLine][aColumn];
            if(editFileT == CODE_EDIT)
                window.draw(createTextForCode(box, str, font, colorEdit[aLine][aColumn]));
            else
                window.draw(createTextForCode(box, str, font, baseColor));
        }
    }
}

// desenarea margenilor
void interfaceDraw(RenderWindow &window) {
    Point topLeft, bottomRight;
    Color colorFill(20, 20,20), colorLine(255, 255, 255);

    // sus
    topLeft = {0, 0};
    bottomRight = {SCREEN_WIDTH, originIDiagram.y};
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
    // dreapta
    topLeft = {originIDiagram.x+DIAGRAM_WIDTH, 0};
    bottomRight = {SCREEN_WIDTH, SCREEN_HEIGHT};
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
    // stanga
    topLeft = {0, 0};
    bottomRight = {originICode.x, SCREEN_HEIGHT};
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
    // mijloc
    topLeft = {originICode.x+CODE_WIDTH, MARGIN+1};
    bottomRight = {originIDiagram.x, SCREEN_HEIGHT-MARGIN};
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
    // jos
    topLeft = {0, originIDiagram.y+DIAGRAM_HEIGHT};
    bottomRight = {SCREEN_WIDTH, SCREEN_HEIGHT};
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
    // margine diagrama
    topLeft = {originIDiagram.x+2, originIDiagram.y+2};
    bottomRight = {topLeft.x+DIAGRAM_WIDTH-4, topLeft.y+DIAGRAM_HEIGHT-4};
    colorFill = Color(0, 0, 0, 0);
    colorLine = Color(255, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
    // margine code
    topLeft = {originICode.x+2, originICode.y+2};
    bottomRight = {topLeft.x+CODE_WIDTH-4, topLeft.y+CODE_HEIGHT-4};
    colorFill = Color(0, 0, 0, 0);
    colorLine = Color(255, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
    // margine compiler
    topLeft = {originIDiagram.x, SCREEN_HEIGHT-(MARGIN/4)*3};
    bottomRight = {originIDiagram.x+DIAGRAM_WIDTH-BLOCK_BUTTON_WIDTH, SCREEN_HEIGHT-MARGIN/4};
    colorFill = Color(0, 0, 0, 0);
    colorLine = Color(255, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));

    // marginea in ce caseta sunt
    topLeft = {originICode.x, SCREEN_HEIGHT-(MARGIN/4)*3};
    bottomRight = {originICode.x+CODE_WIDTH-2*BLOCK_BUTTON_WIDTH-20, SCREEN_HEIGHT-MARGIN/4};
    colorFill = Color(0, 0, 0, 0);
    colorLine = Color(255, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
}

// desenarea fundalului pentru diagramCut
void backgroundDiagramDraw(RenderWindow &window) {
    Point topLeft = originIDiagram;
    Point bottomRight = {topLeft.x+DIAGRAM_WIDTH, topLeft.y+DIAGRAM_HEIGHT};
    Color colorFill = Color(14, 10, 10);
    Color colorLine = Color(0, 0, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
}

// desenarea fundalului pentru codeCut
void backgroundCodeDraw(RenderWindow &window) {
    Point topLeft = originICode;
    Point bottomRight = {topLeft.x+CODE_WIDTH, topLeft.y+CODE_HEIGHT};
    Color colorFill = Color(14, 10, 10);
    Color colorLine = Color(0, 0, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
}

// desenarea fundalului pentru compilerCut
void backgroundCompilerDraw(RenderWindow &window) {
    Point topLeft = {originIDiagram.x, SCREEN_HEIGHT-(MARGIN/4)*3};
    Point bottomRight = {originIDiagram.x+DIAGRAM_WIDTH-BLOCK_BUTTON_WIDTH, SCREEN_HEIGHT-MARGIN/4};
    Color colorFill = Color(14, 10, 10);
    Color colorLine = Color(0, 0, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
}

// desenarea fundalului pentru in ce caseta sunt
void backgroundEditFileCutDraw(RenderWindow &window) {
    Point topLeft = {originICode.x, SCREEN_HEIGHT-(MARGIN/4)*3};
    Point bottomRight = {originICode.x+CODE_WIDTH-2*BLOCK_BUTTON_WIDTH-20, SCREEN_HEIGHT-MARGIN/4};
    Color colorFill = Color(14, 10, 10);
    Color colorLine = Color(0, 0, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
}

// desenarea unei linii
void lineDraw(Point startP, Point stopP, Color color, RenderWindow &window) {
    VertexArray line(LineStrip, 2);
    line[0].position = Vector2f(startP.x, startP.y);
    line[1].position = Vector2f(stopP.x, stopP.y);
    line[0].color = color;
    line[1].color = color;
    window.draw(line);
}

// desenarea cursorului
void cursorDraw(RenderWindow &window) {
    Point startP, stopP;
    Point aCursorCP = cursorCP;
    if(cursorCP.x > LIMIT_COLUMN_CODE)
        aCursorCP.x = LIMIT_COLUMN_CODE;
    if(cursorCP.y > LIMIT_LINE_CODE-1)
        aCursorCP.y = LIMIT_LINE_CODE-1;
    startP.x = originICode.x+(aCursorCP.x+1)*BLOCK_CODE_WIDTH+CODEEDIT_MARGIN_WIDTH;
    startP.y = originICode.y+aCursorCP.y*BLOCK_CODE_HEIGHT+CODEEDIT_MARGIN_HEIGHT;
    stopP.x = startP.x;
    stopP.y = startP.y+BLOCK_CODE_HEIGHT;
    lineDraw(startP, stopP, Color(255, 0, 0), window);
}

// parte din mecanismul pentru butoane
void activateButton(Button button) {
    if(button.type == RUN) {
        validTree = true;
        lineCount = 0;
        declaredGlobal.clear();
        while(!declaredLocal.empty())
            declaredLocal.pop();
        varType.clear();
        valMap.clear();
        initTree();

        setDataToFile(TEMPFILE, CODE_EDIT);
        fin.open(TEMPFILE);
        buildTree(Tree);

        checkErrors_DFS(Tree);

        if(validTree) { ///daca arborele e corect mai am de verificat structura programului (erori pt endif, endwhile, else, etc)
            ///merg inapoi la inceputul fisierului
            fin.clear();
            fin.seekg(0);

            checkStructure();
        }

        fin.close();
        remove(TEMPFILE);

        if(validTree) {
            buildDP_DFS(Tree);
            buildDiagram_DFS(Tree, Tree);

            setDataToFile(TEMPFILE, INPUT_EDIT);
            fin.open(TEMPFILE);
            fout.open(TEMPFILE1);
            compileCode_DFS(Tree);
            fin.close();
            fout.close();
            getDataFromFile(TEMPFILE1, OUTPUT_EDIT);
            remove(TEMPFILE);
            remove(TEMPFILE1);

            if(editFileT == OUTPUT_EDIT)
                cursorCP = {0, 0};


            str_compiler_info = "Cod executat cu succes!";
        }
        else {
            str_compiler_info = errorMessage[error.first]+intToString(error.second);

        }
    }
    else if(button.type == ABOUT) {
        winT = WIN_ABOUT;
    }
    else if(button.type == SAVE) {
        setDataToFile(FILENAME_CODE, CODE_EDIT);
        setDataToFile(FILENAME_INPUT, INPUT_EDIT);
        setDataToFile(FILENAME_OUTPUT, OUTPUT_EDIT);
    }
    else if(button.type == LOAD) {
        getDataFromFile(FILENAME_CODE, CODE_EDIT);
        getDataFromFile(FILENAME_INPUT, INPUT_EDIT);
        getDataFromFile(FILENAME_OUTPUT, OUTPUT_EDIT);
        editFileT = CODE_EDIT;
        cursorCP = {0, 0};
    }
    else if(button.type == CLEAR) {
        clearCodeMemory(editFileT);
        codeEdit[editFileT].push_back(vector<char>());
        cursorCP = {0, 0};
    }
    else if(button.type == CENTER) {
        diagramP = originDiagramP;
        zoom = 1;
    }
    else if(button.type == BACK) {
        winT = WIN_EDITOR;
    }
    else if(button.type == ARROW_LEFT) {
        cursorCP = {0, 0};
        if(editFileT == CODE_EDIT)
            editFileT = OUTPUT_EDIT;
        else if(editFileT == OUTPUT_EDIT)
            editFileT = INPUT_EDIT;
        else if(editFileT == INPUT_EDIT)
            editFileT = CODE_EDIT;
    }
    else if(button.type == ARROW_RIGHT) {
        cursorCP = {0, 0};
        if(editFileT == CODE_EDIT)
            editFileT = INPUT_EDIT;
        else if(editFileT == INPUT_EDIT)
            editFileT = OUTPUT_EDIT;
        else if(editFileT == OUTPUT_EDIT)
            editFileT = CODE_EDIT;
    }
}

// mecanismul pentru butoane
void buttonsMechanics(RenderWindow &window) {
    Vector2i positionMouse = Mouse::getPosition(window);
    if(winT == WIN_EDITOR) {
        for(auto& it: buttons) {
            if(it.second.mouseOnButton(positionMouse.x, positionMouse.y)) {
                if(!it.second.prepForPress && !Mouse::isButtonPressed(Mouse::Left)) {
                    it.second.prepForPress = true;
                }
                else if(it.second.prepForPress && Mouse::isButtonPressed(Mouse::Left))
                    it.second.press = true;
                else if(it.second.press) {
                    it.second.prepForPress = false;
                    it.second.press = false;
                    activateButton(it.second);
                }
            }
            else {
                it.second.prepForPress = false;
                it.second.press = false;
            }
        }
    }
    if(winT == WIN_ABOUT) {
        if(backButton.mouseOnButton(positionMouse.x, positionMouse.y)) {
            if(!backButton.prepForPress && !Mouse::isButtonPressed(Mouse::Left)) {
                backButton.prepForPress = true;
            }
            else if(backButton.prepForPress && Mouse::isButtonPressed(Mouse::Left))
                backButton.press = true;
            else if(backButton.press) {
                backButton.prepForPress = false;
                backButton.press = false;
                activateButton(backButton);
            }
        }
        else {
            backButton.prepForPress = false;
            backButton.press = false;
        }
    }
}

// mecanismul pentru redimensionare
void resizeMechanics(RenderWindow &window) {
    View view = window.getView();
    bool correctWidth = true;
    bool correctHeight = true;
    if(window.getSize().x < MINSCR_WIDTH)
        correctWidth = false;
    if(window.getSize().y < MINSCR_HEIGHT)
        correctHeight = false;
    if(!oneTimeResize && (sizeScreen || (correctWidth && correctHeight))) {
        if(!correctWidth && !correctHeight) {
            window.setSize(Vector2u(MINSCR_WIDTH, MINSCR_HEIGHT));
            sizeScreen = false;
        }
        else if(!correctWidth) {
            window.setSize(Vector2u(MINSCR_WIDTH, window.getSize().y));
            sizeScreen = false;
        }
        else if(!correctHeight) {
            window.setSize(Vector2u(window.getSize().x, MINSCR_HEIGHT));
            sizeScreen = false;
        }
        view = window.getView();
        SCREEN_WIDTH = window.getSize().x;
        SCREEN_HEIGHT = window.getSize().y;
        originIDiagram = {SCREEN_WIDTH-DIAGRAM_WIDTH-DIAGRAM_MARGIN_WIDTH, DIAGRAM_MARGIN_HEIGHT};
        originDiagramP = {originIDiagram.x+DIAGRAM_WIDTH/2-(float)BLOCK_WIDTH/2, originIDiagram.y+50};
        diagramP = originDiagramP;
        DIAGRAM_WIDTH = SCREEN_WIDTH/2-DIAGRAM_MARGIN_WIDTH*2;
        DIAGRAM_HEIGHT = SCREEN_HEIGHT-DIAGRAM_MARGIN_HEIGHT*2;
        CODE_WIDTH = SCREEN_WIDTH/2-CODE_MARGIN_WIDTH*2;
        CODE_HEIGHT = SCREEN_HEIGHT-CODE_MARGIN_HEIGHT*2;
        LIMIT_COLUMN_CODE = (CODE_WIDTH-CODEEDIT_MARGIN_WIDTH*2)/BLOCK_CODE_WIDTH-1;
        LIMIT_LINE_CODE = (CODE_HEIGHT-CODEEDIT_MARGIN_HEIGHT*2)/BLOCK_CODE_HEIGHT;
        createAllButtons();
        view.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
        view.setCenter(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    }
    window.setView(view);
}

// mecanismul pentru zoom
void zoomMechanics() {
    deletePositionDiagram_DFS(Tree);
    Tree -> x = diagramP.x;
    Tree -> y = diagramP.y;
    Tree -> length = BLOCK_WIDTH * zoom;
    Tree -> height = BLOCK_HEIGHT * zoom;
    buildDiagram_DFS(Tree, Tree);
}

// mecanismul pentru miscarea diagramei
void moveMechanics(int direction, RenderWindow &window) {
    Vector2i positionMouse = Mouse::getPosition(window);
    if(originIDiagram.x < positionMouse.x && positionMouse.x < originIDiagram.x+DIAGRAM_WIDTH &&
       originIDiagram.y < positionMouse.y && positionMouse.y < originIDiagram.y+DIAGRAM_HEIGHT && !Mouse::isButtonPressed(Mouse::Left))
        diagramIprepForPress = true;
    if(diagramIprepForPress) {
        if(originIDiagram.x < positionMouse.x && positionMouse.x < originIDiagram.x+DIAGRAM_WIDTH &&
           originIDiagram.y < positionMouse.y && positionMouse.y < originIDiagram.y+DIAGRAM_HEIGHT) {
            if(direction > 0) {
                if(Mouse::isButtonPressed(Mouse::Left) && !moveScreen) {
                    moveScreen = true;
                    amoveP.x = positionMouse.x;
                    amoveP.y = positionMouse.y;
                }
                else if(!Mouse::isButtonPressed(Mouse::Left) && moveScreen) {
                    diagramP.x += moveP.x;
                    diagramP.y += moveP.y;
                    moveScreen = false;
                    diagramIprepForPress = false;
                    amoveP = {0, 0};
                    moveP = {0, 0};
                }
            }
            if(moveScreen) {
                if(direction > 0)
                    moveP = {positionMouse.x-amoveP.x, positionMouse.y-amoveP.y};

                diagramP.x += direction*moveP.x;
                diagramP.y += direction*moveP.y;
            }
        }
        else {
            if(moveScreen) {
                if(direction > 0) {
                    diagramP.x += direction*moveP.x;
                    diagramP.y += direction*moveP.y;
                }
                else {
                    moveScreen = false;
                    diagramIprepForPress = false;
                    amoveP = {0, 0};
                    moveP = {0, 0};
                }
            }
            else
                diagramIprepForPress = false;
        }
    }
}

// eventuri
void pollEvents(RenderWindow &window) {
    bool resizedEvent = false;
    Event event;
    while(window.pollEvent(event)) {
        if(event.type == Event::Closed)
            window.close();

        // iesire program
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::Q && Keyboard::isKeyPressed(Keyboard::LControl))
                window.close();
        }
        Vector2i positionMouse = Mouse::getPosition(window);
        if(originIDiagram.x < positionMouse.x && positionMouse.x < originIDiagram.x+DIAGRAM_WIDTH &&
           originIDiagram.y < positionMouse.y && positionMouse.y < originIDiagram.y+DIAGRAM_HEIGHT) {
            if(event.type == Event::MouseWheelMoved && Tree != NULL && validTree) {
                if(event.mouseWheel.delta == -1 && zoomMinScale < zoom && Tree -> verticalNodeCount > 0) {
                    zoom -= zoomScale;
                    float diff = BLOCK_WIDTH*((zoom+zoomScale)-zoom);
                    float diff1 = (BLOCK_WIDTH*(zoom+zoomScale))/(positionMouse.x-diagramP.x);
                    diagramP.x += diff/diff1;
                    diff = (Tree -> verticalNodeCount*BLOCK_WIDTH)*((zoom+zoomScale)-zoom);
                    diff1 = (Tree -> verticalNodeCount*BLOCK_WIDTH*(zoom+zoomScale))/(positionMouse.y-diagramP.y);
                    diagramP.y += diff/diff1;
                }
                else if(event.mouseWheel.delta == 1 && Tree -> verticalNodeCount > 0) {
                    zoom += zoomScale;
                    float diff = BLOCK_WIDTH*(zoom-(zoom-zoomScale));
                    float diff1 = (BLOCK_WIDTH*(zoom-zoomScale))/(positionMouse.x-diagramP.x);
                    diagramP.x -= diff/diff1;
                    diff = (Tree -> verticalNodeCount*BLOCK_WIDTH)*(zoom-(zoom-zoomScale));
                    diff1 = (Tree -> verticalNodeCount*BLOCK_WIDTH*(zoom-zoomScale))/(positionMouse.y-diagramP.y);
                    diagramP.y -= diff/diff1;
                }
            }
        }

        // RUN - actionarea butonului cu tastatura
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::R && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))) {
                activateButton(buttons[RUN]);
            }
        }
        // SAVE - actionarea butonului cu tastatura
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::S && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))) {
                activateButton(buttons[SAVE]);
            }
        }
        // LOAD - actionarea butonului cu tastatura
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::L && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))) {
                activateButton(buttons[LOAD]);
            }
        }

        // CENTER - actionarea butonului cu tastatura
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::O && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))) {
                activateButton(buttons[CENTER]);
            }
        }

        // CLEAR - actionarea butonului cu tastatura
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::D && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))) {
                activateButton(buttons[CLEAR]);
            }
        }

        // ARROW_RIGHT - actionarea butonului cu tastatura
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::J && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))) {
                activateButton(buttons[ARROW_RIGHT]);
            }
        }
        // ARROW_LEFT - actionarea butonului cu tastatura
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::K && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl))) {
                activateButton(buttons[ARROW_LEFT]);
            }
        }

        // mutarea cursorului cu arrow key
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::Up && cursorCP.y > 0) {
                cursorCP.y -= 1;
                if(codeEdit[editFileT].size() != 0 && cursorCP.x > codeEdit[editFileT][cursorCP.y].size())
                    cursorCP.x = codeEdit[editFileT][cursorCP.y].size();
            }
            if(event.key.code == Keyboard::Right && codeEdit[editFileT].size() != 0 && cursorCP.x < codeEdit[editFileT][cursorCP.y].size())
                cursorCP.x += 1;
            if(event.key.code == Keyboard::Left && cursorCP.x > 0)
                cursorCP.x -= 1;
            if(event.key.code == Keyboard::Down && cursorCP.y < (int)codeEdit[editFileT].size()-1) {
                cursorCP.y += 1;
                if(codeEdit[editFileT].size() != 0 && cursorCP.x > codeEdit[editFileT][cursorCP.y].size())
                    cursorCP.x = codeEdit[editFileT][cursorCP.y].size();
            }
        }

        if(event.type == sf::Event::TextEntered) {
            if(32 <= event.text.unicode && event.text.unicode <= 126) { // character
                codeEdit[editFileT][cursorCP.y].insert(codeEdit[editFileT][cursorCP.y].begin()+cursorCP.x, static_cast<char>(event.text.unicode));
                cursorCP.x += 1;
            }
            else if(event.text.unicode == 13) { // enter
                codeEdit[editFileT].insert(codeEdit[editFileT].begin()+cursorCP.y+1, vector<char>());
                codeEdit[editFileT][cursorCP.y+1].insert(codeEdit[editFileT][cursorCP.y+1].begin(), codeEdit[editFileT][cursorCP.y].begin()+cursorCP.x, codeEdit[editFileT][cursorCP.y].end());
                codeEdit[editFileT][cursorCP.y].erase(codeEdit[editFileT][cursorCP.y].begin()+cursorCP.x, codeEdit[editFileT][cursorCP.y].end());
                cursorCP.x = 0;
                cursorCP.y += 1;
            }
            else if(event.text.unicode == 8) { // backspace
                if(cursorCP.x == 0) {
                    if(cursorCP.y != 0) {
                        cursorCP.x = codeEdit[editFileT][cursorCP.y-1].size();
                        codeEdit[editFileT][cursorCP.y-1].insert(codeEdit[editFileT][cursorCP.y-1].end(), codeEdit[editFileT][cursorCP.y].begin(), codeEdit[editFileT][cursorCP.y].end());
                        codeEdit[editFileT][cursorCP.y].clear();
                        codeEdit[editFileT].erase(codeEdit[editFileT].begin()+cursorCP.y);
                        cursorCP.y -= 1;
                    }
                }
                else {
                    codeEdit[editFileT][cursorCP.y].erase(codeEdit[editFileT][cursorCP.y].begin()+cursorCP.x-1);
                    cursorCP.x -= 1;
                }
            }
            else if(event.text.unicode == 9) { // tab
                codeEdit[editFileT][cursorCP.y].insert(codeEdit[editFileT][cursorCP.y].begin()+cursorCP.x, static_cast<char>(32));
                cursorCP.x += 1;
                codeEdit[editFileT][cursorCP.y].insert(codeEdit[editFileT][cursorCP.y].begin()+cursorCP.x, static_cast<char>(32));
                cursorCP.x += 1;
            }
        }

        // resize
        if(event.type == Event::Resized)
            resizedEvent = true;
    }
    if(resizedEvent)
        oneTimeResize = false;
    if(sizeScreen) {
        oneTimeResize = true;
        sizeScreen = false;
    }
    if(!oneTimeResize && !resizedEvent && !Mouse::isButtonPressed(Mouse::Left))
        sizeScreen = true;
}

// actualizarea ferestrei
void updateWindow(RenderWindow &window) {
    window.clear();

    backgroundDiagramDraw(window);
    if(Tree != NULL && validTree)
        printDiagram_DFS(Tree, window);
    backgroundCodeDraw(window);

    // afisarea codului
    printCodeEdit(window);

    // desenarea unei linii
    lineDraw({originICode.x+CODEEDIT_MARGIN_WIDTH+BLOCK_CODE_WIDTH, originICode.y+CODEEDIT_MARGIN_HEIGHT}, \
             {originICode.x+CODEEDIT_MARGIN_WIDTH+BLOCK_CODE_WIDTH, originICode.y+CODE_HEIGHT-CODEEDIT_MARGIN_HEIGHT}, Color(255, 255, 255), window);

    // afisarea cursorului
    cursorDraw(window);

    // desenarea interfatei
    interfaceDraw(window);

    // afisarea rezultatului sau syntax error
    backgroundCompilerDraw(window);

    // desenarea fundalului in ce caseta sunt
    backgroundEditFileCutDraw(window);

    // afisare butoane
    for(auto& it: buttons)
        it.second.draw(window, font);

    // afisare informatiilor in compilerCut
    Box box;
    box.x = originIDiagram.x;
    box.y = SCREEN_HEIGHT-(MARGIN/4)*3;
    box.length = DIAGRAM_WIDTH-BLOCK_BUTTON_WIDTH;
    box.height = SCREEN_HEIGHT-MARGIN/4-box.y;
    if(str_compiler_info == "")
        window.draw(createText(box, " ", font));
    else
        window.draw(createText(box, str_compiler_info, font));

    // afisare informatiilor in ce caseta sunt
    box.x = originICode.x;
    box.y = SCREEN_HEIGHT-(MARGIN/4)*3+5;
    box.length = originICode.x+CODE_WIDTH-2*BLOCK_BUTTON_WIDTH-20-box.x;
    box.height = SCREEN_HEIGHT-MARGIN/4-box.y-5;
    if(editFileT == CODE_EDIT)
        window.draw(createText(box, "Edit: Code File ", font));
    else if(editFileT == INPUT_EDIT)
        window.draw(createText(box, "Edit: In File   ", font));
    else if(editFileT == OUTPUT_EDIT)
        window.draw(createText(box, "Edit: Out File  ", font));

    window.display();
}

void interfaceDrawABOUT(RenderWindow &window) {
    Point topLeft, bottomRight;
    Color colorFill(20, 20,20), colorLine(255, 255, 255);

    topLeft = {0, 0};
    bottomRight = {SCREEN_WIDTH, SCREEN_HEIGHT};
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));

    // marginea ABOUT interface
    topLeft.x = CODE_MARGIN_WIDTH;
    topLeft.y = CODE_MARGIN_HEIGHT;
    bottomRight.x = SCREEN_WIDTH-DIAGRAM_MARGIN_WIDTH;
    bottomRight.y = SCREEN_HEIGHT-MARGIN;
    colorFill = Color(0, 0, 0, 0);
    colorLine = Color(255, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
}

// desenarea fundalului pentru ABOUT interface
void backgroundABOUTDraw(RenderWindow &window) {
    Point topLeft, bottomRight;
    topLeft.x = CODE_MARGIN_WIDTH;
    topLeft.y = CODE_MARGIN_HEIGHT;
    bottomRight.x = SCREEN_WIDTH-DIAGRAM_MARGIN_WIDTH;
    bottomRight.y = SCREEN_HEIGHT-MARGIN;
    Color colorFill = Color(14, 10, 10);
    Color colorLine = Color(0, 0, 0, 0);
    window.draw(createRect(topLeft, bottomRight, colorFill, colorLine));
}

// afisarea informatiilor in ABOUT interface
void textDrawABOUT(RenderWindow &window) {
    Box box;
    box.x = CODE_MARGIN_WIDTH;
    box.y = CODE_MARGIN_HEIGHT;
    box.length = SCREEN_WIDTH-DIAGRAM_MARGIN_WIDTH-box.x;
    box.height = SCREEN_HEIGHT-MARGIN-box.y;
    string str = "In programarea computerelor, o diagrama Nassi-Shneiderman (NSD) este o metoda de reprezentare a unui flowchart,\no reprezentare grafica de proiectare pentru programarea structurata.\nProiectul ia drept input un pseudocod simplu, care poate fi citit de la tastatura din program, si afiseaza NSD. \n\nPseudocodul are urmatoarele instructiuni:\n1. var int/string <variabila> - declararea unei variabile de tip int/string\n2. set <variabila> <expresie/string> - atribuirea catre o variabila\nDaca variabila e int se atribuie o expresie, iar daca e string se atribuie ca string tot cuvantul\n3. read <variabila> - citirea unei variabile\n4. print <variabila> - afisarea unei variabile\n5. if <expresie> ... else ... endif - instructiunea if\n6. while <expresie> ... endwhile - instructiunea while\n7. repeat ... until <expresie> - repeat until\n8. pass - instructiune dummy, nu face nimic\n\nMentiuni: \nVariabilele sunt litere mari si mici, expresiile din instructiunea set admit doar variabile, numere si operatori\nde baza (+, -, *, /, %), iar expresiile din if/while/repeat admit in plus comparatii (!=, <. ==, <=, >, >=),\nfara spatii intre ele. \nExpresiile nu pot avea operatii cu string-uri. \nToate instructiunile se scriu pe o singura linie (si else, endif, endwhile, etc se numara ca instructiuni). \nStructura unui if trebuie respectata prin folosirea if, else si endif (daca vrem sa avem o ramura if/else\ngoala se poate folosi instructiunea pass), similar pentru bucle.\n\nScurtaturi/butoane:\nctrl+r (RUN) - compileaza codul si afiseaza diagrama\nctrl+s (SAVE) - salveaza codul si input-ul/output-ul\nctrl+l (LOAD) - incarca codul si input-ul/output-ul\nctrl+o (CENTER) - aduce diagrama la forma initiala\nctrl+d (CLEAR) - sterge tot textul din caseta curenta\nctrl+j (SAGEATA DREAPTA) - mutare caseta de text curenta la dreapta\nctrl+k (SAGEATA STANGA) - mutare caseta de text curenta la stanga\n\nEchipa: Laza Gabriel si Aliciuc Alexandru, grupa B2, UAIC";
    window.draw(createText(box, str, font));
}

// eventuri in ABOUT
void pollEventsABOUT(RenderWindow &window) {
    bool resizedEvent = false;
    Event event;
    while(window.pollEvent(event)) {
        if(event.type == Event::Closed)
            window.close();
        if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::B && (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl)))
                activateButton(backButton);
        }

        // resize
        if(event.type == Event::Resized)
            resizedEvent = true;
    }
    if(resizedEvent)
        oneTimeResize = false;
    if(sizeScreen) {
        oneTimeResize = true;
        sizeScreen = false;
    }
    if(!oneTimeResize && !resizedEvent && !Mouse::isButtonPressed(Mouse::Left))
        sizeScreen = true;
}

void updateWindowABOUT(RenderWindow &window) {
    window.clear();
    interfaceDrawABOUT(window);
    backgroundABOUTDraw(window);
    textDrawABOUT(window);
    backButton.draw(window, font);
    window.display();
}

int main() {
    RenderWindow window(VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "NS Diagram");


    if(!font.loadFromFile("./assets/font.ttf")) {
        cout << "Not found file";
        exit(0);
    }
    createAllButtons();
    codeEdit[CODE_EDIT].push_back(vector<char>());
    codeEdit[INPUT_EDIT].push_back(vector<char>());
    codeEdit[OUTPUT_EDIT].push_back(vector<char>());


    while(window.isOpen()) {
        if(winT == WIN_EDITOR) {
            pollEvents(window);
            buttonsMechanics(window);
            resizeMechanics(window);
            if(Tree != NULL && validTree) {
                moveMechanics(1, window);
                zoomMechanics();
            }
            updateWindow(window);
            if(Tree != NULL && validTree) {
                moveMechanics(-1, window);
            }
        }
        else if(winT == WIN_ABOUT) {
            pollEventsABOUT(window);
            buttonsMechanics(window);
            resizeMechanics(window);
            updateWindowABOUT(window);
        }
    }

    return 0;
}
