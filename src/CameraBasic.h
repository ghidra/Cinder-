#pragma once

#include "cinder/app/App.h"
//#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"

using namespace std;
using namespace ci;
using namespace ci::app;

typedef std::shared_ptr<class CameraBasic> CameraBasicRef;
class CameraBasic{
	public:
		CameraBasic(float distance = 10.0f);
		~CameraBasic(){}
		//void Update(vector<float> audioAnalysis, float deltaTime=0.0f, float elapsedTime=0.0f, float fadeBlend=1.0f, float fadeMode=1.0) override;
		//virtual void Setup(float distance = 500.0f)
		virtual void Update(float deltaTime = 0.0f);
		virtual void keyDown( KeyEvent event );
		virtual void keyUp( KeyEvent event );
		virtual CameraPersp& GetPerspective(){return mPersp;}
	private:
		CameraPersp 	mPersp;
		//Quatf			mSceneRotation;
		float			mDistance;
		vec3			mEye, mCenter, mUp;

		float			mMovementSpeed, mLocalDeltaTime;
		bool			mMovingForward, mMovingBackward, mMovingLeft, mMovingRight, mMovingUpward, mMovingDownward;

};