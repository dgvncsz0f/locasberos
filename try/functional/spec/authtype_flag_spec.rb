#!/usr/bin/ruby
# -*- coding: utf-8 -*-
# vim: et:ts=8:sw=2:sts=2
#
# Copyright (c) 2012 dgvncsz0f
# Copyright (c) 2012 pothix
# Copyright (c) 2012 morellon
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#   * Neither the name of the <ORGANIZATION> nor the names of its contributors
#     may be used to endorse or promote products derived from this software
#     without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

require "spec_helper"

describe :authtype_flag do
  it "must not interfere with other authentication methods" do
    config = ApacheConfig.new
    config.endpoint = "http://localhost"
    config.auth_type = "Basic"
    config.auth_name = "rspec"
    config.auth_user_file = "passwd.db"
    config.with_apache do
      response = AuthBasicHTTP.get(config.url_for("/index.txt"))
      response.code.should == 200
    end
  end

  it "must handle authentication when authtype = locasberos" do
    config = ApacheConfig.new
    config.endpoint = "http://localhost"
    config.with_apache do
      response = HTTParty.get(config.url_for("/index.txt"))
      response.code.should == 403
    end
  end
end