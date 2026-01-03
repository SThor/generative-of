#pragma once

#include "shape.h"
#include <ofMain.h>

class Stroke : public Shape {
public:
	Stroke(
		ofVec2f * startPoint,
		ofVec2f * endPoint,
		float width,
		ofColor color,
		float wiggleFactor = 0.1f);
    ~Stroke() override;

	void update(float timeElapsed) override;
	void draw() const override;

	// Length accessors
	void setLength(float length);
	float getLength() const;

	// Angle accessors
	void setAngle(float angleRad);
	float getAngle() const;

	// Width accessors
	void setWidth(float width);
	float getWidth() const;

	// Debug mode accessors
	void setDebug(bool debug);
	bool getDebug() const;

private:
	ofVec2f * mEndPoint;
	float mWidth;

	vector<ofVec2f> mControlPoints;
	ofPath* mStrokePath;

	bool mDebug;
};