#ifndef GLAS_INPUT_H_
#define GLAS_INPUT_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../../Plain/Hashmap/hashmap.h"

#define GLAS_ANY -1

#define GLAS_RAW_MOUSEBUTTON1 512
#define GLAS_RAW_AXE1 600
#define GLAS_RAW_CONTROL 500



typedef enum{GLAS_INP_KEY_PRESS, GLAS_INP_KEY_RELEASE,
			 GLAS_INP_KEY_CONT,
			 GLAS_INP_JOYSTICK} GlasRawInputType;

typedef struct{
	GlasRawInputType type;//Input type
	int mods;//Keyboard mods
	unsigned long code;//Key or axe
	//Normal keys on [0:511]
	//Mouse buttons on [512:599]
	//Axis on [600:1023]
	//Extra keys on [1023:]
}GlasRawInput;

typedef struct{
	int id;
	float sens;
}GlasLogical;

typedef struct{
	GLFWwindow *win;
	PlainHashmap *mapping;//Maps GlasRawInputs to logical ids
	PlainHashmap *activated;//Maps Logical ids to its values
	void (*defaultAction)(GlasRawInput *raw, double axeValue);
	//Arraylist to non-released inputs
	int numContInputs;
	GlasRawInput contInputs[64];
}GlasSysInput;

GlasSysInput *glasInputInit(GLFWwindow *win);
int glasInputAddMapping(GlasSysInput *inp, GlasRawInputType type,
			int mods, unsigned long code, int logicalId, float sens);


void glasInputPoll(GlasSysInput *inp, int blockingWait);

//Get logical input value
double glasInputGet(GlasSysInput *inp, int logicalId);


//Loads/saves mapping
void glasInputLoadMapping(GlasSysInput *inp, char *inFile);
void glasInputSaveMapping(GlasSysInput *inp, char *outFile);

void glasInputFree(GlasSysInput *inp);

#endif
