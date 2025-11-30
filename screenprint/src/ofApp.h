#pragma once

#include "ofMain.h"
#include <vector>

class Circle; 

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();
	~ofApp();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:
	bool showInfo = true; // draw small help overlay
	std::string tempImagePath = "final_frame_temp.png";
	bool tempSaved = false;

	void saveTimestamped();
	void repositionCircles(); // Helper function to recalculate grid positions
	void generateNewOffsets(); // Generate new random offset values
	void recreateCircles(); // Recreate circles with new grid/diameter parameters
	
	std::vector<Circle*> mCircles;
	
	// Shader stuff
	ofShader mShader;
	float mNoiseIntensity = 0.2f; // Controls blend intensity (0.0 = no effect, 1.0 = full effect)
	int mBlendMode = 0; // 0: Addition, 1: Screen, 2: Overlay, 3: Soft Light, 4: Lighten-Only
	
	// Fixed offset values for print misregistration
	struct PrintOffset {
		float x, y, rotation;
	};
	PrintOffset mPrintOffsets[3]; // One for each color pass
	float mHueShift = 0.0f; // Fixed hue shift for color palette
	
	// Grid configuration
	int mGridCols = 4;
	int mGridRows = 6;
	float mMarginX = 120.0f; // Increased from 80
	float mMarginY = 100.0f; // Increased from 60
	float mCircleDiameter = 30.0f; // Circle diameter (was hardcoded as 60 in setup)

	float mPrintOffset = 10.0f; // Max offset for simulating misregistration
};
