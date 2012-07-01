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

require "tmpdir"
require "fileutils"
require "rubygems"
require "bundler/setup"
require "httparty"

def find_x(*candidates)
  candidates.select {|f| FileTest.executable?(f)}.first
end

$root        = File.expand_path(File.dirname(__FILE__) + "/../../../dist")
$bin_apxs    = find_x("/usr/sbin/apxs",  "/usr/bin/apxs", "/usr/sbin/apxs2", "/usr/bin/apxs2")
$bin_httpd   = find_x("/usr/sbin/httpd", "/usr/bin/httpd", "/usr/sbin/apache2", "/usr/bin/apache2")
$listen_host = "127.0.0.1"
$listen_port = 9999
$endpoint    = "http://" + $listen_host + ":" + $listen_port.to_s

def cat(f)
  File.open(f, "r") {|f| f.read}
end

def libexecdir
  IO.popen("#{$bin_apxs} -q LIBEXECDIR") {|f| f.read.strip}
end

def url4(path, vhost=nil)
  name = (vhost.nil? ? $listen_host : vhost)
  "http://" + name + ":" + $listen_port.to_s + path
end

def url4_fixture(path)
  root = File.expand_path(File.dirname(__FILE__) + "/../../../try/fixtures")
  "file://" + root + path
end

def vhost_begin
  "<VirtualHost *:" + $listen_port.to_s + ">"
end

def vhost_end
  "</VirtualHost>"
end

def mod_auth
  xdir = libexecdir
  [ "LoadModule auth_basic_module #{xdir}/mod_auth_basic.so",
    "LoadModule authn_file_module #{xdir}/mod_authn_file.so",
    "LoadModule authz_user_module #{xdir}/mod_authz_user.so",
    "LoadModule authz_default_module #{xdir}/mod_authz_default.so",
    "LoadModule authz_host_module #{xdir}/mod_authz_host.so"
  ]
end

def mod_locasberos
  xdir = libexecdir
  "LoadModule locasberos_module #{$root}/#{xdir}/mod_locasberos.so"
end

def clean(server_root)
  FileUtils.rm_rf(server_root)
end

def configure(config=[])
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

  return([server_root, template])
end

def apache_start(server_root)
  cmd = $bin_httpd +
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

def apache_stop(server_root)
  cmd = $bin_httpd +
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

def with_apache(config=[], &proc)
  (root,cfg) = configure(config)
  errorlog   = "#{root}/logs/error.log"
  begin
    apache_start(root)
    begin
      proc.call()
    ensure
      apache_stop(root)
    end
  rescue
    puts "\n> exceptiong caught..."
    puts "> apache.cfg: "
    cfg.lines.each {|l| puts ">> #{l}"}
    puts "> error.log: "
    cat(errorlog).lines.each {|l| puts ">> #{l}"}
    raise
  ensure
    clean(root)
  end
end

class AuthBasicHTTP
  include HTTParty
  basic_auth "locasberos", "locasberos"
end
