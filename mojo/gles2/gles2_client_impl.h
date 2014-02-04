// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_GLES2_GLES2_CLIENT_IMPL_H_
#define MOJO_GLES2_GLES2_CLIENT_IMPL_H_

#include "base/memory/scoped_ptr.h"
#include "gpu/command_buffer/client/gles2_implementation.h"
#include "mojo/public/bindings/remote_ptr.h"
#include "mojo/public/gles2/gles2.h"
#include "mojom/gles2.h"

struct MojoGLES2ContextPrivate {};

namespace mojo {

template<typename S> class SyncDispatcher;

namespace gles2 {

class GLES2ClientImpl : public GLES2Client,
                        public GLES2SyncClient,
                        public MojoGLES2ContextPrivate {
 public:
  explicit GLES2ClientImpl(MojoAsyncWaiter* async_waiter,
                           ScopedMessagePipeHandle pipe,
                           MojoGLES2ContextLost lost_callback,
                           MojoGLES2DrawAnimationFrame animation_callback,
                           void* closure);
  virtual ~GLES2ClientImpl();
  bool Initialize();

  gpu::gles2::GLES2Interface* interface() const { return implementation_; }
  gpu::ContextSupport* context_support() const { return implementation_; }
  void RequestAnimationFrames();
  void CancelAnimationFrames();

 private:
  virtual void DidCreateContext(uint64_t encoded) MOJO_OVERRIDE;
  virtual void ContextLost() MOJO_OVERRIDE;
  virtual void DrawAnimationFrame() MOJO_OVERRIDE;

  bool WaitForSyncMessageAndDispatch();

  RemotePtr<GLES2> service_;
  scoped_ptr<SyncDispatcher<GLES2SyncClient> > sync_dispatcher_;
  gpu::gles2::GLES2Implementation* implementation_;
  MojoGLES2ContextLost lost_callback_;
  MojoGLES2DrawAnimationFrame animation_callback_;
  void* closure_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(GLES2ClientImpl);
};

}  // namespace gles2
}  // namespace mojo

#endif  // MOJO_GLES2_GLES2_CLIENT_IMPL_H_
