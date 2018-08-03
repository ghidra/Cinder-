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

#include <memory>
#include <deque>

#include "EnvironmentFilter.h"

#include "cinder/Noncopyable.h"
#include "cinder/gl/platform.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Rect.h"
#include "cinder/Vector.h"

// Cinder's forward declarations
namespace cinder { namespace gl { 
typedef std::shared_ptr<class TextureCubeMap>	TextureCubeMapRef;
typedef std::shared_ptr<class Fbo>				FboRef;
typedef std::shared_ptr<class GlslProg>			GlslProgRef;
class Context;
} } // namespace cinder::gl

namespace renderkit {

// type aliases
using EnvironmentRef	= std::shared_ptr<class Environment>;
 
//! Environment is a set of at least one cubemap texture used for image-based lighting.
// It supports basic specular reflection, pre-filtered blurry reflections and diffuse irradiance mapping.
class Environment {
public:
	// forward declaration
	class Format;
	
	//! Returns a new empty refcounted Environment object. This is usually used as a probe and should be initialized at least once with rk::ScopedEnvironmentWrite
	static EnvironmentRef create( int16_t width, int16_t height, const Format &format = Format() );
	//! Returns a new refcounted Environment object from a single cubemap texture.
	static EnvironmentRef create( const ci::gl::TextureCubeMapRef& skybox, const Format &format = Format() );
	//! Returns a new refcounted Environment object from a skybox, radiance and irradiance cubemaps. (Those textures can be generated in cmft studio https://github.com/dariomanesku/cmftStudio )
	static EnvironmentRef create( const ci::gl::TextureCubeMapRef& skybox, const ci::gl::TextureCubeMapRef& radianceMap, const ci::gl::TextureCubeMapRef& irradianceMap, const Format &format = Format() );
	//! Returns a new empty Environment object. This is usually used as a probe and should be initialized at least once with rk::ScopedEnvironmentWrite
	Environment( int16_t width, int16_t height, const Format &format = Format() );
	//! Constructs a new Environment object from a single cubemap texture.
	Environment( const ci::gl::TextureCubeMapRef& envMap, const Format &format = Format() );
	//! Constructs a new Environment object a skybox, radiance and irradiance cubemaps. (Those textures can be generated in cmft studio https://github.com/dariomanesku/cmftStudio )
	Environment( const ci::gl::TextureCubeMapRef& skybox, const ci::gl::TextureCubeMapRef& radianceMap, const ci::gl::TextureCubeMapRef& irradianceMap, const Format &format = Format() );

	class Format {
	public:
		//! Constructs a new default Environment Format object
		Format() : mRadiance( true ), mIrradiance( true ), mIsProgressive( true ), mIsProbe( false ), mPosition( 0.0f ), mSize( 0.0f ) {}

		//! Specifies whether a prefiltered environment map has to be computed. Enabled by default.
		Format& radiance( bool enabled = true );
		//! Specifies whether an irradiance map has to be computed. Enabled by default.
		Format& irradiance( bool enabled = true );
		//! Specifies the cubemap size. A cubemap with a size and a position will be local and shaders should use parallax correction, a cubemap without size will be infinite and can be used as usual.
		Format& size( const ci::vec3 &boxSize );
		//! Specifies the cubemap position. A cubemap with a size and a position will be local and shaders should use parallax correction, a cubemap without size will be infinite and can be used as usual.
		Format& position( const ci::vec3 &boxPosition );
		//! Specifies whether the environment map will be used as a probe and needs a framebuffer to capture its surrounding. Default to false.
		Format& probe( bool enabled = true );
		//! Specifies whether the generation of the underlying maps should happen accross several frames instead of being calculated at initialization.
		Format& progressive( bool enabled = true );

	protected:
		bool mRadiance, mIrradiance, mIsProgressive, mIsProbe;
		ci::vec3 mPosition, mSize;
		friend class Environment;
	};
	
	//! Returns the pre-filtered map used to calculate specular IBL.
	ci::gl::TextureCubeMapRef getRadianceMap() const;
	//! Returns the map used to calculate diffuse IBL.
	ci::gl::TextureCubeMapRef getIrradianceMap() const;
	//! Returns the non-filtered texture. Can be used as a skybox.
	ci::gl::TextureCubeMapRef getEnvironmentMap() const;
	
	//! Sets the GlslProg's EnvironmentMapping related uniforms
	void setGlslUniforms( const ci::gl::GlslProg *glsl ) const;	
	//! Sets the GlslProg's EnvironmentMapping related uniforms
	void setGlslUniforms( const ci::gl::GlslProgRef &glsl ) const;

	void update();

	// TODO:
	void write();
	void read();

	const ci::gl::FboRef& getFbo() const { return mFbo; }
	const EnvironmentFilterBaseRef& getFilter() const { return mFilter; }

protected:
	
	bool						mHasRadiance;
	bool						mHasIrradiance;

	ci::vec3					mPosition;
	ci::vec3					mSize;
	
	ci::gl::TextureCubeMapRef	mEnvironmentMap;
	ci::gl::TextureCubeMapRef	mRadianceMap;
	ci::gl::TextureCubeMapRef	mIrradianceMap;

	ci::gl::FboRef				mFbo;	

	EnvironmentFilterBaseRef	mFilter;

	friend class ScopedEnvironmentWrite;
};

//! Similar to gl::ScopedFramebuffer, this is used to capture an Environment surroundings. Takes care of settings the states and binding the framebuffer
class ScopedEnvironmentWrite : private ci::Noncopyable {
public:
	ScopedEnvironmentWrite( const EnvironmentRef &envMap );
	~ScopedEnvironmentWrite();
	
	void bindFace( uint8_t dir );
	void setViewMatrix( uint8_t dir, const ci::vec3 &eye );

protected:
	ci::gl::Context*		mGlContext;
	EnvironmentRef			mEnvironment;
};

//! Similar to gl::ScopedTextureBind. Takes care of settings the states and binding the framebuffer
class ScopedEnvironmentRead : private ci::Noncopyable {
public:
	ScopedEnvironmentRead( const EnvironmentRef &envMap, uint8_t textureUnit = 0 );
	~ScopedEnvironmentRead();

protected:
	ci::gl::Context*		mGlContext;
	uint8_t					mTextureUnit;
	EnvironmentRef			mEnvironment;
};

} // namespace renderkit

namespace rk = renderkit;