#pragma once

#include "ofMain.h"

struct DitherPattern {
	std::string filename;
	ofImage image;
	int area;
};

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
		bool showInfo = true;           // draw small help overlay
		bool showPatternPreview = true; // draw pattern preview in corner
		bool animationPaused = false;   // pause/play noise animation
		float pausedTime = 0.0f;        // time when animation was paused
		std::string tempImagePath = "final_frame_temp.png";
		bool tempSaved = false;

		void saveTimestamped();
		void drawInfo();
		void drawPatternPreview();
		void exportAllPatterns();

		void generateNoise();
		void ditherAndDraw();
		
		// Reusable image data
		ofPixels pixels;      // working pixels for display
		ofPixels noisePixels; // clean noise pixels for export
		ofImage image;
		
		// Dither patterns
		std::vector<DitherPattern> ditherPatterns;
		int currentPatternIndex = 0;
		
		void loadDitherPatterns();

		bool gpuDithering = false;
		void gpuDither();
		void cpuDither();
		void applyDitherToPixels(ofPixels& targetPixels, const DitherPattern& pattern);
};
