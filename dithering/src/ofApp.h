#pragma once

#include "ofMain.h"

struct DitherPattern {
	std::string filename;
	ofImage image;
	int area;
};

struct PatternGroup {
	int width;
	int height;
	std::vector<DitherPattern> patterns;
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

		ofPixels generateNoise();
		void ditherAndDraw();
		
		// Reusable image data
		bool hasBaseImage = false;
		void drawImage();
		ofPixels pixels;     // working pixels for display
		ofPixels basePixels; // undithered base image pixels
		ofImage image;
		
		// Dither patterns (grouped by size)
		std::vector<PatternGroup> patternGroups;
		int currentGroupIndex = 0;
		int currentPatternIndexInGroup = 0;
		
		void loadDitherPatterns();
		void addDitherPattern(const std::string& filename, const ofImage& image);
		std::pair<int, int> insertPatternIntoGroups(const DitherPattern& pattern);
		DitherPattern& getCurrentPattern();
		bool hasPatterns() const;

		bool gpuDithering = false;
		void gpuDither();
		void cpuDither();
		void applyDitherToPixels(ofPixels& targetPixels, const DitherPattern& pattern);
		
		// Pattern preview constants
		static const int PREVIEW_SIZE = 64;
		static const int PREVIEW_MARGIN = 10;
		static const int PREVIEW_EXTRA_MARGIN = 20; // Extra margin for easier drag targeting
};
