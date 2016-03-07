// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/edk/system/request_context.h"

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/threading/thread_local.h"

namespace mojo {
namespace edk {

namespace {

base::LazyInstance<base::ThreadLocalPointer<RequestContext>>::Leaky
    g_current_context = LAZY_INSTANCE_INITIALIZER;

}  // namespace

RequestContext::RequestContext() : tls_context_(g_current_context.Pointer()){
  // We allow nested RequestContexts to exist as long as they aren't actually
  // used for anything.
  if (!tls_context_->Get())
    tls_context_->Set(this);
}

RequestContext::~RequestContext() {
  // NOTE: Callbacks invoked by this destructor are allowed to initiate new
  // EDK requests on this thread, so we need to reset the thread-local context
  // pointer before calling them.
  if (IsCurrent())
    tls_context_->Set(nullptr);

  for (const WatchNotifyFinalizer& watch :
      watch_notify_finalizers_.container()) {
    // Establish a new request context for the extent of each callback to ensure
    // that they don't themselves invoke callbacks while holding a watcher lock.
    RequestContext request_context;
    watch.watcher->MaybeInvokeCallback(watch.result, watch.state);
  }

  for (const scoped_refptr<Watcher>& watcher :
          watch_cancel_finalizers_.container())
    watcher->Cancel();
}

// static
RequestContext* RequestContext::current() {
  DCHECK(g_current_context.Pointer()->Get());
  return g_current_context.Pointer()->Get();
}

void RequestContext::AddWatchNotifyFinalizer(
    scoped_refptr<Watcher> watcher,
    MojoResult result,
    const HandleSignalsState& state) {
  DCHECK(IsCurrent());
  watch_notify_finalizers_->push_back(
      WatchNotifyFinalizer(watcher, result, state));
}

void RequestContext::AddWatchCancelFinalizer(scoped_refptr<Watcher> watcher) {
  DCHECK(IsCurrent());
  watch_cancel_finalizers_->push_back(watcher);
}

bool RequestContext::IsCurrent() const {
  return tls_context_->Get() == this;
}

RequestContext::WatchNotifyFinalizer::WatchNotifyFinalizer(
    scoped_refptr<Watcher> watcher,
    MojoResult result,
    const HandleSignalsState& state)
    : watcher(watcher), result(result), state(state) {
}

RequestContext::WatchNotifyFinalizer::~WatchNotifyFinalizer() {}

}  // namespace edk
}  // namespace mojo
