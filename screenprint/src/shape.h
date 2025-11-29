#pragma once

#include <ofMain.h>

class Shape {
public:
	Shape(ofVec2f * position) : mPosition(position) {}
	virtual ~Shape() = default;

	// lifecycle
	virtual void update(float timeElapsed) = 0;

	// be careful not to modify color in draw methods, as it should set by the parent
	virtual void draw() const = 0;

protected:
	ofVec2f * mPosition;
};