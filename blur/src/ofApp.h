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
		// Canvas and rendering - Multi-resolution system
		ofFbo canvas;
		ofFbo fboBlurOnePass, fboBlurTwoPass;
		ofFbo highResFBO, medResFBO, lowResFBO; // Multi-resolution layers
		ofShader blurShaderX, blurShaderY;
		
		// Animation state
		float yPos;
		int frameCounter;
		unsigned long seed;
		bool firstFrame;
		bool animationComplete;
		int debugLayer; // 0=composite, 1=high, 2=med, 3=low
		
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
		
		// Multi-resolution decay parameters
		ofParameter<float> highToMedDecayStart;
		ofParameter<float> medToLowDecayStart;
		ofParameter<float> decayRate;
		ofParameter<float> verticalDecayMultiplier;
		ofParameter<float> highResAlpha;
		ofParameter<float> medResAlpha;
		ofParameter<float> lowResAlpha;
		ofParameter<bool> useNearestFiltering;
		
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
		void updateTextureFiltering(); // Update texture filtering for multi-res FBOs
		void drawToCanvas();
		void applyBlur();
		void applyBlurToFBO(ofFbo& targetFBO); // Apply blur to any specific FBO
		void applyMultiResolutionDecay(); // New multi-resolution decay system
		void compositeMultiResolutionLayers(); // Blend all resolution layers
		void resetAnimation();
		void saveTempFrame();
		void saveTimestamped();
		void drawInfo();
		float getThresholdPosition(float threshold); // Convert threshold proportion to pixel position
};
