# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file helps gyp_chromium and landmines correctly set up the gyp
# environment from chromium.gyp_env on disk

import os

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
CHROME_SRC = os.path.dirname(SCRIPT_DIR)


def apply_gyp_environment_from_file(file_path):
  """Reads in a *.gyp_env file and applies the valid keys to os.environ."""
  if not os.path.exists(file_path):
    return
  with open(file_path) as f:
    file_contents = f.read()
  try:
    file_data = eval(file_contents, {'__builtins__': None}, None)
  except SyntaxError, e:
    e.filename = os.path.abspath(file_path)
    raise
  supported_vars = ( 'CC',
                     'CHROMIUM_GYP_FILE',
                     'CHROMIUM_GYP_SYNTAX_CHECK',
                     'CXX',
                     'GYP_DEFINES',
                     'GYP_GENERATOR_FLAGS',
                     'GYP_GENERATOR_OUTPUT',
                     'GYP_GENERATORS', )
  for var in supported_vars:
    file_val = file_data.get(var)
    if file_val:
      if var in os.environ:
        print 'INFO: Environment value for "%s" overrides value in %s.' % (
            var, os.path.abspath(file_path)
        )
      else:
        os.environ[var] = file_val


def apply_chromium_gyp_env():
  if 'SKIP_CHROMIUM_GYP_ENV' not in os.environ:
    # Update the environment based on chromium.gyp_env
    path = os.path.join(os.path.dirname(CHROME_SRC), 'chromium.gyp_env')
    apply_gyp_environment_from_file(path)
