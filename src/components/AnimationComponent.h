#ifndef _ANIMATIONCOMPONENT_H_
#define _ANIMATIONCOMPONENT_H_

#include "../GuiComponent.h"
#include "ImageComponent.h"
#include <vector>

#define ANIMATION_TICK_SPEED 32 //this should be totally sufficient for most purposes

//------------------------------------------------------------------------------------------------------------------

typedef float InterpolationFunction(float t);

inline float interpolateLinear(float t)
{
	return t;
}

inline float interpolateEaseIn(float t)
{
	return t*t*t;
}

inline float interpolateEaseOut(float t)
{
	const float t1 = (t - 1.0f);
	return t1*t1*t1 - 1.0f; 
}

inline float interpolateSmoothStep(float t)
{
	const float t2 = t*t;
	return (3.0f - 2.0f*t) * t2;
}

//------------------------------------------------------------------------------------------------------------------

//Snapped from the C++ FAQ: http://www.parashift.com/c++-faq-lite/array-memfnptrs.html
#define CALL_MEMBER_FN(object,ptrToMember) ((object)->*(ptrToMember))

template <typename T>
class AnimationComponent
{
public:
	//typedef T (GuiComponent::*GetFunction)();
	typedef void (GuiComponent::*SetFunction)(const T);
	enum RepeatMode {REPEAT_NO, REPEAT_ALWAYS, REPEAT_PINGPONG};

protected:
	struct ChildData {
		GuiComponent * component;
		//GetFunction getValue;
		SetFunction setValue;

		bool active;
		int direction;
		float startTime;
		float accumulator;

		T start;
		T end;
		T rate;

		ChildData(GuiComponent * c, /*GetFunction get,*/ SetFunction set, int dir, bool act, float t) : component(c), /*getValue(get),*/ setValue(set), direction(dir), active(act), startTime(t) {}
	};
	std::vector<ChildData> mChildren;

	RepeatMode mRepeatMode;
	InterpolationFunction & mInterpolation;

public:
	AnimationComponent(RepeatMode repeatMode = REPEAT_NO, InterpolationFunction & function = interpolateLinear);

	void update(int deltaTime);
	void addChild(GuiComponent * component, /*GetFunction getFunction,*/ SetFunction setFunction);

	void animateWithSpeed(const T & start, const T & end, const T & rate);
	void animateWithDuration(const T & start, const T & end, float duration);

	T calcNewValue(const ChildData & T);
};

template <typename T>
inline AnimationComponent<T>::AnimationComponent(RepeatMode repeatMode, InterpolationFunction & function)
	: mRepeatMode(repeatMode), mInterpolation(function)
{
}

template <typename T>
inline void AnimationComponent<T>::addChild(GuiComponent * component, /*GetFunction getFunction,*/ SetFunction setFunction)
{
	mChildren.push_back(ChildData(component, /*getFunction,*/ setFunction, 0, false, 0));
}

template <typename T>
inline void AnimationComponent<T>::animateWithSpeed(const T & start, const T & end, const T & rate)
{
	//for every child, store current and target offset
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren[i].active = true;
		mChildren[i].direction = 1;
		mChildren[i].startTime = 0.0f;
		mChildren[i].start = start;
		mChildren[i].end = end;
		mChildren[i].rate = rate;
	}
}

template <typename T>
inline void AnimationComponent<T>::animateWithDuration(const T & start, const T & end, float duration)
{
	//for every child, store current and target offset
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren[i].active = true;
		mChildren[i].direction = 1;
		mChildren[i].startTime = 0.0f;
		mChildren[i].start = start;
		mChildren[i].end = end;
		mChildren[i].rate = (end - start) / duration;
	}
}

template <typename T>
inline void AnimationComponent<T>::update(int deltaTime)
{
	//loop through all added components
	size_t nrOfInactiveComponents = 0;
	for (unsigned int i = 0; i < mChildren.size(); i++) {
		//get child from list
		ChildData child = mChildren.at(i);
		//check if an animation is active for the component
		if (child.active) {
			//increase time passed for child
			child.accumulator += deltaTime;
			//calculate new value from time and child parameters
			T newValue = calcNewValue(child);
			//clamp offset to start or target, depending on direction
			if (child.direction > 0) {
				newValue > child.end ? child.end : newValue;
				if (newValue >= child.end) {
					//if the end has been reached, mark components as inactive and increase counter
					mChildren[i].active = false;
					nrOfInactiveComponents++;
				}
			}
			else if (child.direction < 0) {
				newValue < child.start ? child.start : newValue;
				if (newValue <= child.start) {
					//if the end has been reached, mark components as inactive and increase counter
					mChildren[i].active = false;
					nrOfInactiveComponents++;
				}
			}
			//phew. set new child offset now.
			mChildren[i] = child;
			CALL_MEMBER_FN(child.component, child.setValue)(newValue);
		}
		else {
			nrOfInactiveComponents++;
		}
	}
	//now after the loop check if all components are inactive now
	if (mChildren.size() <= nrOfInactiveComponents) {
		//all components have reached their animation target and are inactive now
		for (unsigned int i = 0; i < mChildren.size(); i++) {
			//get child from list
			ChildData & child = mChildren[i];
			//decide what to do with it
			switch (mRepeatMode) {
				//case AnimationComponent::REPEAT_NO: <-- Do nothing
			case AnimationComponent::REPEAT_ALWAYS:
				//rewind animation to the start and activate it again
				CALL_MEMBER_FN(child.component, child.setValue)(child.start);
				child.active = true;
				child.startTime = 0.0f;
				break;
			case AnimationComponent::REPEAT_PINGPONG:
				//switch animation direction and activate it again
				child.active = true;
				child.direction = -child.direction;
				child.startTime = 0.0f;
				break;
			}
			//write value back
			mChildren[i] = child;
		}
	}
}

// ------------------------------------------------------------------------------------------------------------------
//Template specializations

template <>
inline int AnimationComponent<int>::calcNewValue(const ChildData & child)
{
	//calculate value increase
	const float valueIncrease = child.rate * (child.accumulator - child.startTime) / 1000.0f;
	//calculate overall distance. this either from start or from end, depending on direction
	const float distance = (child.direction >= 0) ? (child.end - child.start) : (child.start - child.end);
	//calculate t from ratio of current offset to target offset
	float t = valueIncrease / distance;
	//t can have bad values, because of a zero-divide, take care of that
	t = is_valid(t) ? t : 0.0f;
	//clamp to a sane value
	t = clamp(t, 0.0f, 1.0f);
	//apply interpolation function to t
	t = mInterpolation(t);
	//get start value
	const float startValue = (child.direction >= 0) ? child.start : child.end;
	//calculate new value, round and return
	return startValue + t * valueIncrease * child.direction + 0.5f;
}

template <>
inline float AnimationComponent<float>::calcNewValue(const ChildData & child)
{
	//calculate value increase
	const float valueIncrease = child.rate * (child.accumulator - child.startTime) / 1000.0f;
	//calculate overall distance. this either from start or from end, depending on direction
	const float distance = (child.direction >= 0) ? (child.end - child.start) : (child.start - child.end);
	//calculate t from ratio of current offset to target offset
	float t = valueIncrease / distance;
	//t can have bad values, because of a zero-divide, take care of that
	t = is_valid(t) ? t : 0.0f;
	//clamp to a sane value
	t = clamp(t, 0.0f, 1.0f);
	//apply interpolation function to t
	t = mInterpolation(t);
	//get start value
	const float startValue = (child.direction >= 0) ? child.start : child.end;
	//calculate new value and return
	return startValue + t * valueIncrease * child.direction;
}

template <>
inline Vector2i AnimationComponent<Vector2i>::calcNewValue(const ChildData & child)
{
	//calculate value increase
	const Vector2f valueIncrease = Vector2f(child.rate) * (child.accumulator - child.startTime) / 1000.0f;
	//calculate overall distance. this either from start or from end, depending on direction
	const Vector2f distance = (child.direction >= 0) ? (child.end - child.start) : (child.start - child.end);
	//calculate t from ratio of current offset to target offset
	Vector2f t(valueIncrease.x / distance.x, valueIncrease.y / distance.y);
	//t can have bad values, because of a zero-divide, take care of that
	t = is_valid(t) ? t : Vector2f(0.0f, 0.0f);
	//clamp to a sane value
	t = clamp(t, Vector2f(0.0f, 0.0f), Vector2f(1.0f, 1.0f));
	//apply interpolation function to t
	t.x = mInterpolation(t.x);
	t.y = mInterpolation(t.y);
	//get start value
	const Vector2f startValue = Vector2f((child.direction >= 0) ? child.start : child.end);
	//calculate new value, round and return
	return startValue + cWiseProd(t, valueIncrease) * (float)child.direction;
}

template <>
inline Vector2f AnimationComponent<Vector2f>::calcNewValue(const ChildData & child)
{
	//calculate value increase
	const Vector2f valueIncrease = child.rate * (child.accumulator - child.startTime) / 1000.0f;
	//calculate overall distance. this either from start or from end, depending on direction
	const Vector2f distance = (child.direction >= 0) ? (child.end - child.start) : (child.start - child.end);
	//calculate t from ratio of current offset to target offset
	Vector2f t(valueIncrease.x / distance.x, valueIncrease.y / distance.y);
	//t can have bad values, because of a zero-divide, take care of that
	t = is_valid(t) ? t : Vector2f(0.0f, 0.0f);
	//clamp to a sane value
	t = clamp(t, Vector2f(0.0f, 0.0f), Vector2f(1.0f, 1.0f));
	//apply interpolation function to t
	t.x = mInterpolation(t.x);
	t.y = mInterpolation(t.y);
	//get start value
	const Vector2f startValue = Vector2f((child.direction >= 0) ? child.start : child.end);
	//calculate new value, round and return
	return startValue + cWiseProd(t, valueIncrease) * (float)child.direction;
}

#endif
