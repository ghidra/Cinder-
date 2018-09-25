#include "Compute.h"

#include "cinder/Log.h"
#include "Assets.h"

ComputeShaderRef ComputeShader::create( const ci::DataSourceRef & dataSource, ivec3 workGroupSize )
{
	return ComputeShaderRef( new ComputeShader( dataSource, workGroupSize ) );
}

ComputeShader::ComputeShader( const ci::DataSourceRef& dataSource, ivec3 workGroupSize )
	: mWorkGroupSize{ workGroupSize }
{
	mConnGlsl = assets()->getFile( dataSource->getFilePath(),
		[this]( const ci::DataSourceRef& dataSource ) {
		try {
			mUpdateProg = gl::GlslProg::
				create( gl::GlslProg::Format().compute( dataSource )
					.define( "WG_SIZE_X", std::to_string( mWorkGroupSize.x ) )
					.define( "WG_SIZE_Y", std::to_string( mWorkGroupSize.y ) )
					.define( "WG_SIZE_Z", std::to_string( mWorkGroupSize.z ) )
				);
			}
			catch( const gl::GlslProgCompileExc& exc ) {
				CI_LOG_EXCEPTION( "", exc );
			}
		}
	);
}

void ComputeShader::dispatch( int threadGroupsX, int threadGroupsY, int threadGroupsZ )
{
	gl::ScopedGlslProg prog( mUpdateProg );
	gl::dispatchCompute( threadGroupsX, threadGroupsY, threadGroupsZ );
	gl::memoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );
}

///////////////////////////////
// ----------------
///////////////////////////////

ComputeBufferRef ComputeBuffer::create( const void * data, int size, int blockSize )
{
	return ComputeBufferRef( new ComputeBuffer{ data, size, blockSize } );
}

ComputeBuffer::ComputeBuffer( const void * data, int size, int blockSize )
	: mSize{ size }
{
	mSsbo = gl::Ssbo::create( mSize * blockSize, data, GL_STATIC_DRAW );
}

ScopedComputeBuffer::ScopedComputeBuffer( const ComputeBufferRef &bufferObj, uint8_t bufferUnit )
	: mCtx( gl::context() )
	, mSsbo{ bufferObj->getSsbo() }
{
	mSsbo->bindBase( static_cast<GLuint>( bufferUnit ) );
	mCtx->pushBufferBinding( mSsbo->getTarget(), mSsbo->getId() );
}

ScopedComputeBuffer::~ScopedComputeBuffer()
{
	mSsbo->unbindBase();
	mCtx->popBufferBinding( mSsbo->getTarget() );
}

