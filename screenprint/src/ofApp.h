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
	
	std::vector<Circle*> mCircles;
	
	// Grid configuration
	int mGridCols = 6;
	int mGridRows = 4;
	float mMarginX = 80.0f;
	float mMarginY = 60.0f;
};
