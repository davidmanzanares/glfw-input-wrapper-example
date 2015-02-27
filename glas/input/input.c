#include <stdlib.h>
#include "input.h"

//Auxiliary functions
static int modsEqual(GlasRawInput *a, GlasRawInput *b){
	return (a->mods==GLAS_ANY ||
		b->mods==GLAS_ANY ||
		a->mods==b->mods);
}
static int rawsAlmostEqual(GlasRawInput *a, GlasRawInput *b){
	return a->code==b->code && a->type==b->type;
}
static int rawsEqual(void *key1, void *key2){
	GlasRawInput *a=key1;
	GlasRawInput *b=key2;
	return rawsAlmostEqual(a, b) && modsEqual(a,b);
}
static int rawHash(void *key){
	return ((GlasRawInput*)key)->code;
}
static int intsEqual(void *key1, void *key2){
	int *a=key1;
	int *b=key2;
	return *a==*b;
}
static int intHash(void *key){
	return *((int*)key);
}


//Polling functionality
static GlasSysInput *sInp;

static void addCont(GlasRawInput *raw){
	if (sInp->numContInputs==64){
		return;
	}
	sInp->contInputs[sInp->numContInputs]=*raw;
	sInp->numContInputs++;
}
static void delCont(GlasRawInput *raw){
	for (int i=0; i<sInp->numContInputs; i++){
		if (rawsAlmostEqual(raw, &sInp->contInputs[i])){
			//Delete it
			sInp->numContInputs--;
			for (int j=i; j<sInp->numContInputs; j++){
				sInp->contInputs[j]=sInp->contInputs[j+1];
			}
			break;
		}
	}
}

static void registerInput(GlasSysInput *inp, GlasRawInput *raw, double axeValue){
	//Get logicalId
	GlasLogical *l=plainHashMapGet(sInp->mapping, raw);
	if (!l){
		//Raw input not mapped: do default action
		if (sInp->defaultAction){
			sInp->defaultAction(raw, axeValue);
		}
	}else{
		//Raw input mapped: add it to activated list
		//TODO: sum old value if neeeded
		double v=l->sens*axeValue;
		double *v2=plainHashMapGet(inp->activated, &l->id);
		if (v2){
			*v2=*v2+v;
		}else{
			plainHashMapInsert(inp->activated, &l->id, &v);
		}
	}
}

static void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods){
	GlasRawInput raw;
	//Set code
	if (key==GLFW_KEY_UNKNOWN){
		raw.code=1024+(unsigned int)scancode;
	}else{
		raw.code=key;
	}
	//Set mods
	raw.mods=mods;
	//Set type
	//TODO set action=glfw
	if (action==GLFW_PRESS){
		//Add to contInputs
		raw.type=GLAS_INP_KEY_CONT;
		addCont(&raw);
		raw.type=GLAS_INP_KEY_PRESS;
	}else if (action==GLFW_RELEASE){
		//Delete from contInputs
		raw.type=GLAS_INP_KEY_CONT;
		delCont(&raw);
		raw.type=GLAS_INP_KEY_RELEASE;
	}else{
		//Repeat
		return;
	}
	registerInput(sInp, &raw, 1.0);
}
static void scroll_callback(GLFWwindow *win, double x, double y){
	GlasRawInput raw;
	//Set code
	raw.code=GLAS_RAW_AXE1+2;
	//Set mods
	raw.mods=0;
	//Set type
	raw.type=GLAS_INP_JOYSTICK;
	registerInput(sInp, &raw, x);
	raw.code=GLAS_RAW_AXE1+3;
	registerInput(sInp, &raw, y);
}

static void mouse_button_callback(GLFWwindow *win, int button, int action, int mods){
	GlasRawInput raw;
	//Set code
	raw.code=button+GLAS_RAW_MOUSEBUTTON1;
	//Set mods
	raw.mods=mods;
	//Set type
	//TODO set action=glfw
	if (action==GLFW_PRESS){
		//Add to contInputs
		raw.type=GLAS_INP_KEY_CONT;
		addCont(&raw);
		raw.type=GLAS_INP_KEY_PRESS;
	}else if (action==GLFW_RELEASE){
		//Delete from contInputs
		raw.type=GLAS_INP_KEY_CONT;
		delCont(&raw);
		raw.type=GLAS_INP_KEY_RELEASE;
	}else{
		//Repeat
		return;
	}
	registerInput(sInp, &raw, 1.0);
}

void glasInputPoll(GlasSysInput *inp, int blockingWait){
	plainHashMapDeleteEntries(inp->activated);
	for (int i=0; i<inp->numContInputs; i++){
		inp->contInputs[i].mods=-2;
	}
	//Poll keyboard: callback will be called, cont list will be refreshed
	sInp=inp;
	if (blockingWait){
		glfwWaitEvents();
	}else{
		glfwPollEvents();
	}
	//Get mods and set continous raws to them
	int alt=glfwGetKey(inp->win, GLFW_KEY_LEFT_ALT)==GLFW_PRESS ||
			glfwGetKey(inp->win, GLFW_KEY_RIGHT_ALT)==GLFW_PRESS;
	int control=glfwGetKey(inp->win, GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS ||
			glfwGetKey(inp->win, GLFW_KEY_RIGHT_CONTROL)==GLFW_PRESS;
	int shift=glfwGetKey(inp->win, GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS ||
			glfwGetKey(inp->win, GLFW_KEY_RIGHT_SHIFT)==GLFW_PRESS;
	int super=glfwGetKey(inp->win, GLFW_KEY_LEFT_SUPER)==GLFW_PRESS ||
			glfwGetKey(inp->win, GLFW_KEY_RIGHT_SUPER)==GLFW_PRESS;
	GlasRawInput r;
	r.type=GLAS_INP_KEY_CONT;
	r.code=GLAS_RAW_CONTROL;
	r.mods=0;
	registerInput(inp, &r, control);
	int mods=GLFW_MOD_ALT*alt + GLFW_MOD_CONTROL*control +
			GLFW_MOD_SHIFT*shift + GLFW_MOD_SUPER*super;
	//Iterate non-released(continue) keys
	for (int i=0; i<inp->numContInputs; i++){
		if (inp->contInputs[i].mods==-2){
			inp->contInputs[i].mods=mods;
		}
		registerInput(inp, &inp->contInputs[i], 1.0);
	}
	//Poll mouse
	double x,y;
	glfwGetCursorPos(inp->win, &x, &y);
	GlasRawInput raw;
	raw.type=GLAS_INP_JOYSTICK;
	raw.code=GLAS_RAW_AXE1;
	raw.mods=mods;
	registerInput(inp, &raw, x);
	raw.code=GLAS_RAW_AXE1+1;
	registerInput(inp, &raw, y);
}

GlasSysInput *glasInputInit(GLFWwindow *win){
	GlasSysInput *inp=malloc(sizeof(GlasSysInput));
	if (!inp){
		return NULL;
	}
	inp->win=win;
	inp->defaultAction=NULL;
	inp->numContInputs=0;
	inp->mapping=plainHashMapInit(rawsEqual, rawHash, sizeof(GlasRawInput),
					sizeof(GlasLogical), 128);
	if (!inp->mapping){
		free(inp);
		return NULL;
	}
	inp->activated=plainHashMapInit(intsEqual, intHash, sizeof(int),
					sizeof(double), 16);
	if (!inp->activated){
		plainHashMapFree(inp->mapping);
		free(inp);
		return NULL;
	}
	sInp=inp;
	glfwSetKeyCallback(win, key_callback);
	glfwSetMouseButtonCallback(win, mouse_button_callback);
	glfwSetScrollCallback(win, scroll_callback);
	return inp;
}
int glasInputAddMapping(GlasSysInput *inp, GlasRawInputType type,
			int mods, unsigned long code, int logicalId, float sens){
	GlasRawInput raw={type, mods, code};
	GlasLogical l={logicalId, sens};
	return plainHashMapInsert(inp->mapping, &raw, &l);
}

double glasInputGet(GlasSysInput *inp, int logicalId){
	double *p=((double *)plainHashMapGet(inp->activated, &logicalId));
	return p? *p : 0;
}


void glasInputLoadMapping(GlasSysInput *inp, char *inFile){
	//TODO: Depend on hashmap save
}
void glasInputSaveMapping(GlasSysInput *inp, char *outFile){

}

void glasInputFree(GlasSysInput *inp){
	plainHashMapFree(inp->mapping);
	plainHashMapFree(inp->activated);
	free(inp);
}

