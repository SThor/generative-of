#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

	private:
		std::array<ofColor,3> palettePink { ofColor(202,49,98), ofColor(202,118,154), ofColor(187,141,159) };
		std::array<ofColor,3> paletteGreen { ofColor(14,96,82), ofColor(94,142,80), ofColor(168,172,158) };
		bool showInfo = true;           // draw small help overlay
		std::string tempImagePath = "final_frame_temp.png";
		bool tempSaved = false;

		void buildGradient();
		void pickPalette();
		void saveTimestamped();
		// Container of palettes for random selection
		std::vector<const std::array<ofColor,3>*> palettes { &palettePink, &paletteGreen };
		const std::array<ofColor,3>* activePalette = nullptr;
		// Simple gradient endpoints (top and bottom)
		ofColor gradientTop {0,0,0};
		ofColor gradientBottom {0,0,0};


};
