#pragma once

#include "cinder/gl/gl.h"

using namespace ci;

typedef std::shared_ptr<class ComputeShader> ComputeShaderRef;
class ComputeShader {
public:
	static ComputeShaderRef		create( const ci::DataSourceRef& dataSource, ivec3 workGroupSize = ivec3( 128, 1, 1 ) );
	virtual						~ComputeShader() {}

	const ivec3&				getWorkGroupSize() const { return mWorkGroupSize; }
	gl::GlslProgRef&			getGlsl() { return mUpdateProg; }
	const gl::GlslProgRef&		getGlsl() const { return mUpdateProg; }

	void dispatch( int threadGroupsX, int threadGroupsY, int threadGroupsZ );
protected:
	ComputeShader( const ci::DataSourceRef& dataSource, ivec3 workGroupSize );

	const ivec3					mWorkGroupSize;

	signals::ScopedConnection	mConnGlsl;
	gl::GlslProgRef				mUpdateProg;
};

typedef std::shared_ptr<class ComputeBuffer> ComputeBufferRef;
class ComputeBuffer {
public:
	static ComputeBufferRef		create( const void * data, int size, int blockSize );
	virtual						~ComputeBuffer() {}
	//void clear( const void * data, int size, int blockSize);

	gl::SsboRef&				getSsbo() { return mSsbo; }
	const gl::SsboRef&			getSsbo() const { return mSsbo; }

	int							getSize() const { return mSize; }
protected:
	ComputeBuffer( const void * data, int size, int blockSize );

	gl::SsboRef					mSsbo;
	int							mSize;
};


struct ScopedComputeBuffer : public Noncopyable {
	ScopedComputeBuffer( const ComputeBufferRef &bufferObj, uint8_t bufferUnit = 0 );
	~ScopedComputeBuffer();
private:
	gl::Context *	mCtx;
	gl::SsboRef		mSsbo;
};







