// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/at_exit.h"
#include "base/bind.h"
#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "mojo/shell/application_manager.h"
#include "mojo/shell/loader.h"
#include "mojo/shell/public/cpp/shell_client.h"
#include "mojo/shell/public/cpp/shell_connection.h"
#include "mojo/shell/tests/capability_filter_unittest.mojom.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace mojo {
namespace shell {
namespace test {

class ConnectionValidator;

// This class models an application who will use the shell to interact with a
// system service. The shell may limit this application's visibility of the full
// set of interfaces exposed by that service.
class TestApplication : public ShellClient {
 public:
  TestApplication();
  ~TestApplication() override;

 private:
  // Overridden from ShellClient:
  void Initialize(Connector* connector, const std::string& name, uint32_t id,
                  uint32_t user_id) override;
  bool AcceptConnection(Connection* connection) override;

  void ConnectionClosed(const std::string& service_name);

  Connector* connector_;
  std::string name_;
  ValidatorPtr validator_;
  scoped_ptr<Connection> connection1_;
  scoped_ptr<Connection> connection2_;

  DISALLOW_COPY_AND_ASSIGN(TestApplication);
};

class CapabilityFilterTestLoader : public Loader {
 public:
  explicit CapabilityFilterTestLoader(ShellClient* delegate);
  ~CapabilityFilterTestLoader() override;

 private:
  // Overridden from Loader:
  void Load(const std::string& name,
            mojom::ShellClientRequest request) override;

  scoped_ptr<ShellClient> delegate_;
  scoped_ptr<ShellConnection> app_;

  DISALLOW_COPY_AND_ASSIGN(CapabilityFilterTestLoader);
};

class CapabilityFilterTest : public testing::Test {
 public:
   CapabilityFilterTest();
   ~CapabilityFilterTest() override;

 protected:
  template <class T>
  void CreateLoader(const std::string& name) {
    application_manager_->SetLoaderForName(
        make_scoped_ptr(new CapabilityFilterTestLoader(new T)), name);
  }

  void RunBlockingTest();
  void RunWildcardTest();

  // Overridden from testing::Test:
  void SetUp() override;
  void TearDown() override;

  base::MessageLoop* loop() { return &loop_;  }
  ApplicationManager* application_manager() {
    return application_manager_.get();
  }
  ConnectionValidator* validator() { return validator_; }

 private:
  void RunApplication(const std::string& name, const CapabilityFilter& filter);
  void InitValidator(const std::set<std::string>& expectations);
  void RunTest();

  void OnInstanceQuit(const Identity& identity);

  template<class T>
  scoped_ptr<ShellClient> CreateShellClient() {
    return scoped_ptr<ShellClient>(new T);
  }

  base::ShadowingAtExitManager at_exit_;
  base::MessageLoop loop_;
  scoped_ptr<ApplicationManager> application_manager_;
  ConnectionValidator* validator_;
  std::set<Identity> quit_identities_;

  DISALLOW_COPY_AND_ASSIGN(CapabilityFilterTest);
};

}  // namespace test
}  // namespace shell
}  // namespace mojo
