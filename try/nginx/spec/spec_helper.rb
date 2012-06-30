#!/usr/bin/ruby
# -*- coding: utf-8 -*-
# vim: et:ts=8:sw=2:sts=2
#
# Copyright (c) 2012 dsouza
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

require "bundler/setup"

require "tmpdir"
require "fileutils"
require "httparty"

NGINX_DIR   = File.expand_path(File.dirname(__FILE__) + "/../../tmp/nginx-1.2.1")

LISTEN_HOST = "127.0.0.1"
LISTEN_PORT = 9999

def url_for(path, vhost=nil)
  name = (vhost.nil? ? LISTEN_HOST : vhost)
  "http://" + name + ":" + LISTEN_PORT.to_s + path
end

def clean()
  FileUtils.rm_rf(NGINX_DIR)
end

def nginx_start()
  cmd = NGINX_DIR.join("/sbin/nginx", " -c #{NGINX_DIR}/conf/nginx.conf")
  IO.popen(cmd) { Process.wait }
  25.times do
    begin
      TCPSocket.new(LISTEN_HOST, LISTEN_PORT).close()
      return
    rescue
    end
    sleep(0.2)
  end
  raise(RuntimeError.new("cant load nginx ["+cmd+"]"))
end

def nginx_stop()
  cmd = NGINX_DIR.join("/sbin/nginx", " -s stop")
  IO.popen(cmd) { Process.wait }
  25.times do
    begin
      TCPSocket.new(LISTEN_HOST, LISTEN_PORT).close()
    rescue
      return
    end
    sleep(0.2)
  end
end

def with_nginx(config=[], &proc)
  begin
    nginx_start()
    begin
      proc.call()
    ensure
      nginx_stop()
    end
  ensure
    clean()
  end
end

class AuthBasicHTTP
  include HTTParty
  basic_auth "locasberos", "locasberos"
end
