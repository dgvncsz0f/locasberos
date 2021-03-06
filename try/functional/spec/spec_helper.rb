#!/usr/bin/ruby
# -*- coding: utf-8 -*-
# vim: et:ts=8:sw=2:sts=2
#
# Copyright (c) 2012 dgvncsz0f
# Copyright (c) 2012 pothix
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

require "bundler/setup"

require "tmpdir"
require "httparty"
require "fileutils"

$LOAD_PATH.unshift(File.expand_path(File.dirname(__FILE__) + "/../"))

require "locasberos"
require "apache/apache"
require "nginx/nginx"

def find_x(*candidates)
  candidates.select {|f| FileTest.executable?(f)}.first
end

$bin_apxs    = find_x("/usr/sbin/apxs",  "/usr/bin/apxs", "/usr/sbin/apxs2", "/usr/bin/apxs2")
$bin_httpd   = find_x("/usr/sbin/httpd", "/usr/bin/httpd", "/usr/sbin/apache2", "/usr/bin/apache2")


def webservers
  ws = ENV['WEBSERVERS']
  ws = "Apache" if ws.nil?

  ws.split(',').inject([]) do |acc, w|
    if Object.const_defined?(w)
      acc << Object.const_get(w)
    else
      Object.const_missing(w)
    end
    acc
  end
end

WEBSERVERS = webservers

def cat(file)
  File.open(file, "r") {|f| f.read}
end

def clean(server_root)
  FileUtils.rm_rf(server_root)
end

class AuthBasicHTTP
  include HTTParty
  basic_auth "locasberos", "locasberos"
end
