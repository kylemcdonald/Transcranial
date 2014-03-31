#include "testApp.h"
#include "ofAppGLFWWindow.h"

int main() {
	ofAppGLFWWindow window;
	ofSetupOpenGL(&window, 1920, 1200, OF_FULLSCREEN);
	ofRunApp(new testApp());
}
