#!/usr/bin/ruby

require "rubygems"
require "sinatra"

set :run, false

require (File.dirname(__FILE__) + "/example.rb")
run Sinatra::Application
