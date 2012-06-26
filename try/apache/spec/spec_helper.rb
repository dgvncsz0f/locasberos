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

require "tmpdir"
require "fileutils"
require "rubygems"
require "bundler/setup"
require "httparty"

$root        = File.expand_path(File.dirname(__FILE__) + "/../../../dist")
$libexecdir  = ENV["apxs_libexecdir"]
$listen_host = "127.0.0.1"
$listen_port = 9999
$endpoint    = "http://" + $listen_host + ":" + $listen_port.to_s

def url4(path, vhost=nil)
  name = (vhost.nil? ? $listen_host : vhost)
  if (vhost.nil?)
    "http://" + name + ":" + $listen_port.to_s + path
  else
    "http://" + name + ":" + $listen_port.to_s + path
  end
end

def vhost_begin
  "<VirtualHost *:" + $listen_port.to_s + ">"
end

def vhost_end
  "</VirtualHost>"
end

def mod_auth
  [ "LoadModule auth_basic_module "+ $libexecdir +"/mod_auth_basic.so",
    "LoadModule authn_file_module "+ $libexecdir +"/mod_authn_file.so",
    "LoadModule authz_user_module "+ $libexecdir +"/mod_authz_user.so",
    "LoadModule authz_default_module "+ $libexecdir +"/mod_authz_default.so",
    "LoadModule authz_host_module "+ $libexecdir +"/mod_authz_host.so"
  ]
end

def mod_locasberos
  "LoadModule locasberos_module " + $root + $libexecdir + "/mod_locasberos.so"
end

def clean(server_root)
  FileUtils.rm_rf(server_root)
end

def configure(config=[], apachebin="/usr/sbin/apache2")
  fixtures_root = File.expand_path(File.dirname(__FILE__) + "/../../fixtures")
  tmp_root      = File.expand_path(File.dirname(__FILE__) + "/../../tmp")
  server_root   = Dir.mktmpdir(nil, tmp_root)
  myconfig      = config.flatten.join("\n")
  template      = [ mod_auth,
                    "Listen " + $listen_host + ":" + $listen_port.to_s,
                    "NameVirtualHost *:" + $listen_port.to_s,
                    "<VirtualHost *:" + $listen_port.to_s + ">",
                    "  DocumentRoot v_www",
                    "</VirtualHost>",
                    "<Location />",
                    " Order allow,deny",
                    " Allow from all",
                    "</Location>",
                    File.open(fixtures_root + "/apache.cfg", "r") {|f| f.read().gsub(/\{config\}/, myconfig)}
                  ].join("\n")

  FileUtils.mkdir_p(server_root + "/logs")
  FileUtils.mkdir_p(server_root + "/www")
  FileUtils.mkdir_p(server_root + "/v_www")
  FileUtils.cp(fixtures_root + "/apache/passwd.db", server_root + "/passwd.db")
  File.open(server_root + "/www/index.txt", "w") {|f1| f1.write("Ok") }
  File.open(server_root + "/v_www/index.txt", "w") {|f1| f1.write("Ok") }
  File.open(server_root + "/apache.cfg", "w") {|f1| f1.write(template) }

  return(server_root)
end

def apache_start(server_root, apachebin="/usr/bin/apache2")
  cmd = apachebin +
        " -k start" +
        " -f "+ server_root +"/apache.cfg" +
        " -d "+ server_root
  IO.popen(cmd) { Process.wait }
  25.times do
    begin
      TCPSocket.new($listen_host, $listen_port).close()
      return
    rescue
    end
    sleep(0.2)
  end
  raise(RuntimeError.new("cant load apache ["+cmd+"]"))
end

def apache_stop(server_root, apachebin="/usr/bin/apache2")
  cmd = apachebin +
        " -k stop" +
        " -f "+ server_root +"/apache.cfg" +
        " -d "+ server_root +
        " 2>&1"
  IO.popen(cmd) { Process.wait }
  25.times do
    begin
      TCPSocket.new($listen_host, $listen_port).close()
    rescue
      return
    end
    sleep(0.2)
  end
end

def with_apache(config=[], apachebin="/usr/sbin/apache2", &proc)
  server_root = configure(config, apachebin)
  begin
    apache_start(server_root, apachebin)
    begin
      proc.call()
    ensure
      apache_stop(server_root, apachebin)
    end
  ensure
    clean(server_root)
  end
end

class AuthBasicHTTP
  include HTTParty
  basic_auth "locasberos", "locasberos"
end
