require "spec/spec_helper"

class Apache < Locasberos

  def base_file
    File.expand_path(File.dirname(__FILE__) + "/apache.cfg.erb")
  end

  def run(&proc)
    (root,cfg) = configure
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
      puts "> error.log: "
      cat(errorlog).lines.each {|l| puts ">> #{l}"}
      raise
    ensure
      clean(root)
    end
  end

  private

  def configure
    tmp_root      = File.expand_path(File.dirname(__FILE__) + "/../../tmp")
    server_root   = Dir.mktmpdir(nil, tmp_root)

    FileUtils.mkdir_p(server_root + "/logs")
    FileUtils.mkdir_p(server_root + "/www")
    FileUtils.mkdir_p(server_root + "/v_www")
    FileUtils.cp(
      url_for_fixture("/apache/passwd.db").gsub("file://",""),
      server_root + "/passwd.db"
    )
    File.open(server_root + "/www/index.txt", "w") {|f1| f1.write("Ok") }
    File.open(server_root + "/v_www/index.txt", "w") {|f1| f1.write("Ok") }
    File.open(server_root + "/apache.cfg", "w") {|f1| f1.write(template) }

    return([server_root, template])
  end

  def apache_start(server_root)
    cmd = $bin_httpd +
          " -k start" +
          " -C LogLevel\\ debug" +
          " -e error" +
          " -f "+ server_root +"/apache.cfg" +
          " -d "+ server_root
    IO.popen(cmd) { Process.wait }
    25.times do
      begin
        TCPSocket.new(@listen_host, @listen_port).close()
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
        TCPSocket.new(@listen_host, @listen_port).close()
      rescue
        return
      end
      sleep(0.2)
    end
  end

  def libexecdir
    IO.popen("#{$bin_apxs} -q LIBEXECDIR") {|f| f.read.strip}
  end

end
