#pragma once

#include <ofMain.h>

class Shape {
public:
	Shape(ofVec2f * position, float wiggleFactor = 0.1f) : mPosition(position), mWiggleFactor(wiggleFactor) {}
	virtual ~Shape() = default;

	// lifecycle
	virtual void update(float timeElapsed) = 0;

	// be careful not to modify color in draw methods, as it should set by the parent
	virtual void draw() const = 0;

	// wiggle factor accessors
	void setWiggleFactor(float wf) { mWiggleFactor = wf; }
	float getWiggleFactor() const { return mWiggleFactor; }

protected:
	ofVec2f * mPosition;
	float mWiggleFactor;
};