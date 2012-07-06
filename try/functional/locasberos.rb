class Locasberos
  attr_accessor :listen_host, :listen_port, :root,
                :xdir, :page, :enabled, :endpoint,
                :cas_service, :require_type,
                :auth_type, :auth_name, :auth_user_file

  def initialize
    @root         = File.expand_path(File.dirname(__FILE__) + "/../../dist")
    @listen_host  = "127.0.0.1"
    @listen_port  = 9999

    @enabled      = "On"
    @endpoint     = url_for_fixture("/auth_failure")
    @cas_service  = "undefined"
    @auth_type    = "Locasberos"
    @require_type = "valid-user"
  end

  def url_for(path)
    "http://" + @listen_host + ":" + @listen_port.to_s + path
  end

  def url_for_fixture(path)
    fixtures = File.expand_path(File.dirname(__FILE__) + "/../fixtures")
    "file://#{fixtures}#{path}"
  end

  def template
    ERB.new(File.read base_file).result(get_binding)
  end

  def display_key(key, value, endline = "")
    value ? "#{key} #{value}#{endline}" : ""
  end

  # class binding to use on ERB
  def get_binding; binding; end
end
