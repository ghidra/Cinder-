#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"

using namespace std;
using namespace ci;
using namespace ci::app;

typedef std::shared_ptr<class CameraFollow> CameraFollowRef;
class CameraFollow{
	public:
		CameraFollow(vec3 offset = vec3(0.0f,0.3f,-1.0f), float distance = 10.0f);
		~CameraFollow(){}

		virtual void Update(const vec3 worldPosition = vec3(),const float deltaTime = 0.0f);
		virtual CameraPersp& GetPerspective(){return mPersp;}

	private:

		CameraPersp 	mPersp;
		float			mDistance;
		vec3			mEye, mCenter, mUp, mOffset;

		float			mLocalDeltaTime;

};