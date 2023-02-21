//=============================================================================================
// Mintaprogram: Z�ld h�romsz�g. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Szabó Egon Róbert
// Neptun : DEQGWW
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"
const char * const hyperbolicVertexSource = R"(
	#version 330
	precision highp float;

	uniform mat4 MVP;
	layout(location = 0) in vec2 vp;
    layout(location =1) in vec3 vc;

    out vec3 vertexColor;

	void main() {
        vec4 tp=vec4(vp.x,vp.y,0,1)*MVP;
        float w=sqrt(pow(tp.x,2)+pow(tp.y,2)+1);
		gl_Position = vec4(tp.x/(w+1), tp.y/(w+1), 0, 1);
        vertexColor=vc;
	}
)";

const char * const vertexSource = R"(
	#version 330
	precision highp float;

	uniform mat4 MVP;
	layout(location = 0) in vec2 vp;
    layout(location =1) in vec3 vc;

    out vec3 vertexColor;

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;
        vertexColor=vc;
	}
)";

const char * const fragmentSource = R"(
	#version 330
	precision highp float;

	uniform vec3 color;
	out vec4 outColor;
    in vec3 vertexColor;                                       //https://www.youtube.com/watch?v=eMpAeklxjF4 adatok (szin) atadasanak megertese
                                                               //vertex shaderbol fragment shaderbe
	void main() {
		outColor = vec4(vertexColor, 1);
	}
)";

GPUProgram gpuProgram;
GPUProgram hyperbolicGPUProgram;

unsigned int vao;
float hydrogenMass=1.6735575E-27;
float elementaryCharge=1.60217662E-19;
float epsilon=8.8541878128E-12;
float rho=1E-26;

struct Atom;
struct Molecule;

float maxPos=1.4;
float minPos=-1.4;
int maxCharge=50;
int maxMass=100;
float moleculeCircleRadius=0.6;
float atomCircleRadius=0.055;

float offSetX=0;
float offSetY=0;

int circleN=20;
int lineN=10;
int mainCircleN=50;

struct Vertex{
    vec2 position;
    vec3 color;
};

std::vector<Vertex> mainCirclePoints;
std::vector<Vertex> vertices;
int drawIdx=0;
std::vector<Molecule*> activeMolecules=std::vector<Molecule*>();

struct Atom{
    Vertex center;
    float mass;
    float charge;

public:
    Atom(){
        mass=(rand()%maxMass)+1;
    }

    float distance (vec2 pos){
        return sqrt(pow(pos.x - this->center.position.x, 2) +
                    pow(pos.y - this->center.position.y, 2));
    }
};

struct Bond{
    Atom *a1;
    Atom *a2;

    Bond(Atom *a1, Atom *a2){
        this->a1=a1;
        this->a2=a2;
    }
};

struct Molecule{
public:
    std::vector<Vertex> circlePoints;
    std::vector<Vertex> edgePoints;
    std::vector<Bond> bonds;
    Vertex centerOfMass;
    vec2 mainCirclePos;
    int numberofAtoms=0;
    std::vector<Atom*> atoms;
    vec2 velocity;
    float angularVel;

public:
    Molecule(){
        initAtoms();
        activeMolecules.push_back(this);
        refreshPoints();
        velocity=vec2(0,0);
        angularVel=0;
    }

    void initAtoms(){
        mainCirclePos=vec2(minPos + (float)(rand()) * (float)(maxPos - minPos) / RAND_MAX,
                           minPos + (float)(rand()) * (float)(maxPos - minPos) / RAND_MAX);

        numberofAtoms=rand()%7+2;

        int sumOfCharge=0;
        if (numberofAtoms == 2) {
            Atom *a1 = new Atom;
            Atom *a2 = new Atom;
            a1->center.position=vec2(this->mainCirclePos.x + moleculeCircleRadius * cos(2*M_PI*((float) (rand()) / (float)(RAND_MAX))),
                                     this->mainCirclePos.y + moleculeCircleRadius * sin(2*M_PI*((float) (rand()) / (float)(RAND_MAX))));
            atoms.push_back(a1);
            a2->center.position=vec2(this->mainCirclePos.x + moleculeCircleRadius * cos(2*M_PI*((float) (rand()) / (float)(RAND_MAX))),
                                     this->mainCirclePos.y + moleculeCircleRadius * sin(2*M_PI*((float) (rand()) / (float)(RAND_MAX))));
            int chargetmp=(-maxCharge + rand()%(2*maxCharge));
            a1->charge=chargetmp;
            a2->charge=-chargetmp;

            atoms.push_back(a2);
            bonds.push_back(Bond(a1,a2));;

        } else {
            Atom *a;
            for (int i = 0; i < numberofAtoms; i++) {
                a = new Atom;
                a->center.position = vec2(this->mainCirclePos.x + moleculeCircleRadius * cos(2 * M_PI *((float) (rand()) /(float) (RAND_MAX))),
                                             this->mainCirclePos.y + moleculeCircleRadius * sin(2 * M_PI *((float) (rand()) /(float) (RAND_MAX))));
                int chargetmp=(-maxCharge + rand()%(2*maxCharge));
                if(i<numberofAtoms-1) {
                    a->charge = chargetmp;
                    sumOfCharge += chargetmp;
                }
                else if(i==numberofAtoms-1){
                    a->charge=-sumOfCharge;
                }
                atoms.push_back(a);
            }
            generateTree();
        }
        calculateCenterOfMass();
    }

    void generateCircle(Atom* a){

        if(a->charge>0) {
            a->center.color=vec3((float)a->charge/maxCharge,0,0);;
        }
        else if(a->charge<0){
            a->center.color=vec3(0,0,(float)a->charge/(-1*maxCharge));
        }
        else
            a->center.color=vec3(0,0,0);

        circlePoints.push_back(a->center);

        for(int i=0; i<circleN-1;i++){
            Vertex tmp;
            tmp.position.x=a->center.position.x+ (atomCircleRadius* cosf(i*2.0*M_PI/(circleN-2)));
            tmp.position.y=a->center.position.y+ (atomCircleRadius* sinf(i*2.0*M_PI/(circleN-2)));
            tmp.color=a->center.color;
            circlePoints.push_back(tmp);
        }
    }

    void generateTree() {
            int pruferCode[20];
            for (int i = 0; i < numberofAtoms - 2; i++) {
                pruferCode[i] = rand() % (numberofAtoms - 1) + 1;
            }
            pruferCode[numberofAtoms - 2] = numberofAtoms;

            int m, min, j;
            m = numberofAtoms-1;
            for (int i = 0; i <= numberofAtoms-2; i++) {                                       //prufer code algoritmus https://hu.wikipedia.org/wiki/Pr%C3%BCfer-k%C3%B3d
                min = 0;                                                                       //az itt talalhato 1. modszer alapjan
                do {
                    min++;
                    j = i;
                    while ((j <= m) && (pruferCode[j] != min))
                        j++;
                } while (j <= m);

                m++;
                pruferCode[m] = min;

                bonds.push_back(Bond(atoms[min-1],atoms[pruferCode[i]-1]));
            }
    }

    void interpolateLine(Vertex p1, Vertex p2){
        for(int i=0; i<lineN;i++) {
            Vertex temp;
            temp.position = ((float) (i) / (lineN - 1) * p2.position) +
                            ((1 - i / (float) (lineN - 1)) * p1.position);
            edgePoints.push_back(temp);

            for(int i =0; i<edgePoints.size();i++){
                edgePoints.at(i).color=vec3(1,1,1);
            }
        }
    }

    void calculateCenterOfMass(){
        float numX=0, numY=0;
        float denom;
        for(Atom* a: atoms){
            numX+=a->mass*a->center.position.x;
            numY+=a->mass*a->center.position.y;
            denom+=a->mass;
        }
        this->centerOfMass={vec2(numX/denom,numY/denom),vec3(0,1,0)};
    }

    void refreshPoints(){
        circlePoints= std::vector<Vertex>();
        edgePoints= std::vector<Vertex>();
        for(Bond b:bonds){
            interpolateLine(b.a1->center,b.a2->center);
        }
        for(Atom *a: atoms){
            generateCircle(a);
        }

        vertices.insert(vertices.end(),edgePoints.begin(),edgePoints.end());
        vertices.insert(vertices.end(),circlePoints.begin(),circlePoints.end());
    }

    void draw(){
        for (int i=0; i<edgePoints.size()/lineN; i++){
            glDrawArrays(GL_LINE_STRIP, drawIdx,lineN);
            drawIdx+=lineN;
        }

        for(int i=0; i<circlePoints.size()/circleN; i++){
            glDrawArrays(GL_TRIANGLE_FAN, drawIdx,circleN);
            drawIdx+=circleN;
        }
    }

    void move(float dt){
        for(Atom* a: atoms){
            a->center.position=a->center.position+(velocity)*dt;
        }
        centerOfMass.position=centerOfMass.position+(velocity)*dt;
    }

    void rotate(float dt){
        mat4 tran1=mat4(
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,                                                              //https://www.youtube.com/watch?v=CKNjZ1mM7gc a translation matrixok
                -centerOfMass.position.x,-centerOfMass.position.y,0,1                 //megertesehez hasznalt video
        );
        mat4 tran2=mat4(
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                centerOfMass.position.x,centerOfMass.position.y,0,1
        );
        mat4 rotMatrix=mat4(
                cos(angularVel*dt),sin(angularVel*dt),0,0,
                -sin(angularVel*dt),cos(angularVel*dt),0,0,
                0,0,1,0,
                0,0,0,1
        );

        vec4 tmp;
        for(Atom* a: atoms){
            tmp=vec4(a->center.position.x,a->center.position.y,0,1);
            tmp=tmp*tran1*rotMatrix*tran2;
            a->center.position=vec2(tmp.x,tmp.y);
        }
    }

    ~Molecule(){
        for(Atom *a:atoms){
            delete a;
        }
    }
};

static void generateMainCircle(){
    mainCirclePoints.push_back({vec2(0,0),vec3(0.5,0.5,0.5)});
    for(int i=0; i<mainCircleN-1; i++){
        Vertex tmp;
        tmp.position.x=(1* cosf(i*2.0*M_PI/(mainCircleN-2)));
        tmp.position.y=(1* sinf(i*2.0*M_PI/(mainCircleN-2)));
        tmp.color=vec3(0.5,0.5,0.5);
        mainCirclePoints.push_back(tmp);
    }
}

static void drawMainCircle(){
    glDrawArrays(GL_TRIANGLE_FAN,0,mainCircleN);
    drawIdx+=mainCirclePoints.size();
}


static vec2 calculateCoulombF(Atom a1, Atom a2){
    return ((a1.charge*elementaryCharge*a2.charge*elementaryCharge)/(a1.distance(a2.center.position)*2*M_PI*epsilon))
           *((-a2.center.position+a1.center.position)/length(vec2(a2.center.position-a1.center.position)));
}

static vec2 calculateAirRes(Molecule& m, Atom a){
    vec2 radius=a.center.position-m.centerOfMass.position;
    vec2 velAirRes=-m.velocity*rho;
    vec3 radialVel=cross(vec3(0,0,m.angularVel),radius);
    vec2 rotateAirRes=-vec2(radialVel.x,radialVel.y)*rho;
    return (velAirRes+rotateAirRes);
}

static void simulatePhysics(float dt){
    for (Molecule *m1: activeMolecules) {
        vec2 moleculeSumForce=0;
        vec3 moleculeSumTorque=0;
        float moleculeTotalMass=0;
        float moleculeInertiaSum=0;
        for(Atom *a1: m1->atoms){
            vec2 atomForces=0;
            vec2 atomFMove=0;
            vec2 atomFRotate=0;
            vec2 atomCoulombForce=0;
            vec2 atomAirRes= calculateAirRes(*m1,*a1);
            moleculeTotalMass+=a1->mass*hydrogenMass;
            for(Molecule *m2: activeMolecules){
                if(m1!=m2){
                    for(Atom* a2: m2->atoms){
                        atomCoulombForce= calculateCoulombF(*a1,*a2)+atomCoulombForce;
                    }
                }
            }
            atomForces=atomCoulombForce+atomAirRes;
            vec2 vCenterOfMass=a1->center.position-m1->centerOfMass.position;
            atomFMove=vCenterOfMass*(dot(atomForces,vCenterOfMass)/pow(length(vCenterOfMass),2));
            atomFRotate=atomForces-atomFMove;

            moleculeSumForce=atomFMove+moleculeSumForce;
            moleculeSumTorque=cross(vCenterOfMass,atomFRotate);
            moleculeInertiaSum+=a1->mass*hydrogenMass*pow(length(vCenterOfMass),2)+moleculeInertiaSum;
        }

        m1->velocity=m1->velocity+(moleculeSumForce/moleculeTotalMass)*dt;
        m1->angularVel=m1->angularVel+(moleculeSumTorque.z/(moleculeInertiaSum))*dt;

    }
}

void onInitialization() {
    generateMainCircle();
    vertices.insert(vertices.end(),mainCirclePoints.begin(),mainCirclePoints.end());
	glViewport(0, 0, windowWidth, windowHeight);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(Vertex)*mainCirclePoints.size(),
		vertices.data(),
		GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (void*) offsetof(Vertex,position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (void*) offsetof(Vertex,color));

    hyperbolicGPUProgram.create(hyperbolicVertexSource, fragmentSource, "outColor");
    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

void onDisplay() {
    vertices = std::vector<Vertex>();
    vertices.insert(vertices.end(), mainCirclePoints.begin(), mainCirclePoints.end());
    for (Molecule *m: activeMolecules) {
        m->refreshPoints();
    }
    drawIdx=0;

    if(vertices.size()!=0) {
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(Vertex) * vertices.size(),
                     vertices.data(),
                     GL_DYNAMIC_DRAW);
    }
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

    int location;
	float MVPtransf[4][4] = { 1, 0, 0, 0,
							  0, 1, 0, 0,
							  0, 0, 1, 0,
							  offSetX, offSetY, 0, 1 };

	location = glGetUniformLocation(gpuProgram.getId(), "MVP");
    glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);

	glBindVertexArray(vao);
    gpuProgram.Use();
    drawMainCircle();

    hyperbolicGPUProgram.Use();
    for(Molecule* m:activeMolecules){
        m->draw();
    }
	glutSwapBuffers();
}

Molecule* m1;
Molecule* m2;
void onKeyboard(unsigned char key, int pX, int pY) {
    if(key==' '){
        delete m1;
        delete m2;
        activeMolecules=std::vector<Molecule*>();
        vertices=std::vector<Vertex>();
        vertices.insert(vertices.end(),mainCirclePoints.begin(),mainCirclePoints.end());
        for (Molecule *m: activeMolecules) {
            m->refreshPoints();
        }

        m1=new Molecule();
        m2=new Molecule();
    }
    if(key=='s'){
        offSetX-=0.1;
    }
    if(key=='d'){
        offSetX+=0.1;
    }
    if(key=='x'){
        offSetY-=0.1;
    }
    if(key=='e'){
        offSetY+=0.1;
    }
    glutPostRedisplay();
}

void onKeyboardUp(unsigned char key, int pX, int pY) {
}

void onMouseMotion(int pX, int pY) {
}

void onMouse(int button, int state, int pX, int pY) {
}

long lastTick=0;
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME);
    float T=(time-lastTick)/1000.0f;
    float dt=0.01;
    for(float t=0; t<T; t+=dt) {
        simulatePhysics(dt);

        for(Molecule *m: activeMolecules){
            m->move(dt);
            m->rotate(dt);
        }
    }
    lastTick = time;
    glutPostRedisplay();
}