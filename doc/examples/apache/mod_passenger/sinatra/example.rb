#!/usr/bin/ruby

require "rubygems"
require "sinatra"

get "/" do
  u = request.env["REMOTE_USER"]
  "Hey, #{u}!"
end
