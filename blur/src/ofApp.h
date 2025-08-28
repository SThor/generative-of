#pragma once

#include "ofMain.h"
#include "ofxGui.h"

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
		// Canvas and rendering
		ofFbo canvas;
		ofFbo fboBlurOnePass, fboBlurTwoPass;
		ofShader blurShaderX, blurShaderY;
		
		// Animation state
		float yPos;
		int frameCounter;
		unsigned long seed;
		bool firstFrame;
		bool animationComplete;
		
		// Parameters with automatic UI
		ofParameterGroup parameters;
		ofParameter<ofColor> backgroundColor;
		ofParameter<float> shapeSize;
		ofParameter<float> speed;
		ofParameter<float> oscillationAmplitude;
		ofParameter<float> oscillationSpeed;
		ofParameter<float> amplitudeThreshold;
		ofParameter<float> animationEndThreshold;
		ofParameter<float> hueSpeed;
		ofParameter<float> colorSaturation;
		ofParameter<float> colorBrightness;
		ofParameter<float> cornerRadiusStart;
		ofParameter<float> cornerRadiusEnd;
		ofParameter<float> blurAmount;
		ofParameter<bool> showUI;
		ofParameter<bool> autoSave;
		
		// GUI
		ofxPanel gui;
		
		// Saving system
		ofPixels tempPixels;
		std::string tempImagePath;
		bool tempSaved;
		
		// Helper functions
		void setupCanvas();
		void drawToCanvas();
		void applyBlur();
		void resetAnimation();
		void saveTempFrame();
		void saveTimestamped();
		void drawInfo();
		float getThresholdPosition(float threshold); // Convert threshold proportion to pixel position
};
