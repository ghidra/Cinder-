/*
 RenderKit
 
 Copyright (c) 2016, Simon Geilfus, All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "cinder/Signals.h"
#include "cinder/Rect.h"
#include <memory>
#include <deque>

// Cinder's forward declarations
namespace cinder { namespace gl { 
typedef std::shared_ptr<class TextureCubeMap>	TextureCubeMapRef;
typedef std::shared_ptr<class Fbo>				FboRef;
class GlslProg;
typedef std::shared_ptr<GlslProg>				GlslProgRef;
} } // namespace cinder::gl

namespace renderkit {

// type aliases
using EnvironmentFilterBaseRef = std::shared_ptr<class EnvironmentFilterBase>;
using EnvironmentFilterRef = std::shared_ptr<class EnvironmentFilter>;
using EnvironmentFilterProgressiveRef = std::shared_ptr<class EnvironmentFilterProgressive>;

//! Environment Map Edge fixup methods enum
enum class EdgeFixup { NONE, WARP, STRETCH };

//! Environment Map Filter base class
class EnvironmentFilterBase {
public:

	class Format {
	public:
		Format() : mFaceSize( 1024 ), mNumSamples( 1024 ), mNumMips( 7 ), mGammaInput( 1.0f ), mGammaOutput( 1.0f ), mEdgeFixup( EdgeFixup::WARP ) {}

		//! Sets the output face resolution in pixels
		Format& faceSize( uint16_t size ) { mFaceSize = size; return *this; }
		//! Sets the number of samples used by the filter
		Format& samples( uint16_t numSamples ) { mNumSamples = numSamples; return *this; }
		//! Sets the number of mips desired in the output texture mipmap chain. If not specified will used the usual floor( log2( size ) ) - 1
		Format& mips( uint8_t numMips ) { mNumMips = numMips; return *this; }
		//! Filter input should be in linear space; Sets whether the input and/or output needs gamma correction.
		Format& gamma( float gammaIn, float gammaOut ) { mGammaInput = gammaIn; mGammaOutput = gammaOut; return *this; }
		//! Sets the edge fixup method.
		Format& edgeFixup( EdgeFixup fixup ) { mEdgeFixup = fixup; return *this; }
		
		//! Returns the output face resolution in pixels
		uint16_t	getFaceSize() const { return mFaceSize; }
		//! Returns the number of samples used by the filter
		uint16_t	getNumSamples() const { return mNumSamples; }
		//! Returns the number of mips desired in the output texture. 
		uint8_t		getNumMips() const { return mNumMips; }
		//! Returns the input gamma correction.
		float		getGammaInput() const { return mGammaInput; }
		//! Returns the output gamma correction.
		float		getGammaOutput() const { return mGammaOutput; }
		//! Returns the type of edge fixup method
		EdgeFixup	getEdgeFixup() const { return mEdgeFixup; }

	protected:
		EdgeFixup	mEdgeFixup;
		uint16_t	mFaceSize, mNumSamples; 
		uint8_t		mNumMips;
		float		mGammaInput, mGammaOutput;
	};

	//! Returns the un-filtered environment map. Releases the shared_ptr if filtering is done.
	virtual ci::gl::TextureCubeMapRef getEnvMap() const { return mEnvMap; }
	//! Returns the prefiltered mipmapped radiance environment map. The first split of the EnvBRDF is store for each roughness level in the mipmap chain.
	virtual ci::gl::TextureCubeMapRef getPmRadianceEnvMap() const = 0;
	
	//! Applies the filter. Called by the constructor if the input texture is specified at initialization.
	virtual void filter() = 0;
	//! Sets the input environment map texture
	virtual void setEnvMap( const ci::gl::TextureCubeMapRef &envMap ) { mEnvMap = envMap; }

	//! Returns the number of level in the output texture mipmap chain. 
	uint8_t getNumMips() const { return mNumMips; }

    EnvironmentFilterBase( const EnvironmentFilterBase& ) = delete;
    ~EnvironmentFilterBase() = default;
    EnvironmentFilterBase& operator=( const EnvironmentFilterBase& ) = delete;

protected:
	EnvironmentFilterBase( const Format &format = Format() );

	uint16_t					mFaceSize;
	uint8_t						mNumMips;
	ci::gl::GlslProgRef			mGlslProg;
	ci::gl::TextureCubeMapRef	mEnvMap;
	ci::gl::FboRef				mFilterFbo;
	EdgeFixup					mEdgeFixup;
	float						mGammaInput, mGammaOutput;
};

//! Filters the environment map in one pass. Will stall the GPU until filtered.
class EnvironmentFilter : public EnvironmentFilterBase {
public:
	class Format;
	
	static EnvironmentFilterRef create( const Format &format = Format() );
	static EnvironmentFilterRef create( const ci::gl::TextureCubeMapRef &envMap, const Format &format = Format() );
	EnvironmentFilter( const Format &format = Format() );
	EnvironmentFilter( const ci::gl::TextureCubeMapRef &envMap, const Format &format = Format() );

	class Format : public EnvironmentFilterBase::Format {
	public:
		//! Sets the output face resolution in pixels
		Format& faceSize( uint16_t size ) { mFaceSize = size; return *this; }
		//! Sets the number of samples used by the filter
		Format& samples( uint16_t numSamples ) { mNumSamples = numSamples; return *this; }
		//! Sets the number of mips desired in the output texture. If not specified will used the usual floor( log2( size ) ) - 1
		Format& mips( uint8_t numMips ) { mNumMips = numMips; return *this; }
		//! Filter input should be in linear space; Sets whether the input and/or output needs gamma correction.
		Format& gamma( float gammaIn, float gammaOut ) { mGammaInput = gammaIn; mGammaOutput = gammaOut; return *this; }
		//! Sets the edge fixup method.
		Format& edgeFixup( EdgeFixup fixup ) { mEdgeFixup = fixup; return *this; }
	};
	
	//! Returns the prefiltered mipmapped radiance environment map. The first split of the EnvBRDF is store for each roughness level in the mipmap chain.
	virtual ci::gl::TextureCubeMapRef getPmRadianceEnvMap() const;
	
	//! Applies the filter. Called by the constructor if the input texture is specified at initialization.
	virtual void filter();

protected:
	void initializeGlslProg( const Format &format );
	void initializeRenderTargets();
	
};

} // namespace renderkit