require "spec/spec_helper"

class Nginx < Locasberos
  NGINX_PATH = File.expand_path(File.dirname(__FILE__) + "/../../tmp/nginx")
  NGINX_BIN  = "#{NGINX_PATH}/sbin/nginx"

  def base_file
    File.expand_path(File.dirname(__FILE__) + "/nginx.conf.erb")
  end

  def run(&proc)
    root = configure
    begin
      nginx_start(root)
      begin
        proc.call()
      ensure
        nginx_stop(root)
      end
    rescue
      puts "\n> exceptiong caught..."
      raise
    ensure
      clean(root)
    end
  end

  private

  def configure
    tmp_root      = File.expand_path(File.dirname(__FILE__) + "/../../tmp")
    server_root   = Dir.mktmpdir(nil, tmp_root)
    FileUtils.cp(
      File.expand_path(File.dirname(__FILE__) + "/mime.types"),
      server_root + "/mime.types"
    )

    File.open(server_root + "/nginx.conf", "w") {|f1| f1.write(template) }
    server_root
  end

  def nginx_start(server_root)
    cmd = "#{NGINX_BIN} -c #{server_root}/nginx.conf"
    IO.popen(cmd) { Process.wait }
    25.times do
      begin
        TCPSocket.new(@listen_host, @listen_port).close()
        return
      rescue
      end
      sleep(0.2)
    end
    raise(RuntimeError.new("cant load nginx ["+cmd+"]"))
  end

  def nginx_stop(server_root)
    cmd = "#{NGINX_BIN} -s stop -c #{server_root}/nginx.conf"
    IO.popen(cmd) { Process.wait }
    25.times do
      begin
        TCPSocket.new(@listen_host, @listen_port).close()
      rescue
        return
      end
      sleep(0.2)
    end
  end
end
