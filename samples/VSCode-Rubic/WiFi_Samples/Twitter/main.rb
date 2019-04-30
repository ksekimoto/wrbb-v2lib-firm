#!mruby
#GR-CITRUS Version 2.28

@SSID = "Buffalo-G-B896_2"
@PASS = "9876543210"
@consumer_key = "sdfPndh6LAtEpTjVydp8wP48R"
@consumer_secret = "Jf79bawXy0u1T0LpXpvgLSIApegwghyMzXW2R85WabZsSZuKdx"
@access_token = "940756171-EbkdUMnzVWHn44ur5m4SNZOOBDv2MPiUpr19X0st"
@access_token_secret = "wl9BL1HcWTPHC6DYqq8LPZZCB5pc2gLYsggWEY8fJpjv2"

@ntp_server_ip = "ntp.nict.go.jp"
@oauth_signature_method = "HMAC-SHA1"
@oauth_version = "1.0"

@Usb = Serial.new 0
# デバッグ用のpメソッド
def p obj
    @Usb.print obj.to_s
end
def pl obj
    @Usb.println obj.to_s
end
def reset_esp8266
    pinMode(5,1)
    digitalWrite(5,0)   # LOW:Disable
    delay 500
    digitalWrite(5,1)   # LOW:Disable
    delay 500
end
def set_RTC
    @now = [2017,11,23,3,31,0];
    Rtc.init()
    Rtc.setTime(@now)
    #pl Rtc.getTime()
end
#
# http://wp-e.org/2016/06/06/6811/
# https://syncer.jp/Web/API/Twitter/REST_API/ 
#

def create_signature sec1, sec2, req_method, req_url, req_params
    sig_key = WiFi.url_encode(sec1)
    sig_key << "&"
    sig_key << WiFi.url_encode(sec2)
    #pl "sig_key = " + sig_key
    s = ""
    size = req_params.size
    i = 0
    for param in req_params do
        s << param[0].to_s
        s << "="
        s << param[1].to_s
        if (i != (size - 1)) then
            s << "&"
        end
        i = i + 1
    end
    sig_param = WiFi.url_encode(s)
    sig_data = req_method
    sig_data << "&"
    sig_data << WiFi.url_encode(req_url)
    sig_data << "&"
    sig_data << sig_param
    #pl "sig_data = " + sig_data
    hash = WiFi.hmac_sha1(sig_data, sig_data.size, sig_key, sig_key.size);
    signature = WiFi.base64_encode(hash, hash.size)
    #pl "signature = " + signature
    return signature
end

# Test result should be as follows
#sig_key = bbbbbb&dddddd
#sig_data = POST&http%3A%2F%2Fexample.com%2Fsample.php&name%3DBBB%26text%3DCCC%26title%3DAAA
#signature = mu4s4b2t4T0HsjD0z0J749fMGPA=
def test_create_signature
    req_params = [["name", "BBB"], ["text", "CCC"], ["title", "AAA"]]
    sig = create_signature "bbbbbb", "dddddd", "POST", "http://example.com/sample.php", req_params
end

def create_oauth_params oauth_params
    s = "Authorization: OAuth "
    size = oauth_params.size
    i = 0
    for param in oauth_params do
        s << WiFi.url_encode(param[0].to_s)
        s << "="
        s << WiFi.url_encode(param[1].to_s)
        if (i != (size - 1)) then
            s << ","
        end
        i = i + 1
    end
    pl "oauth_params = " + s
    return s
end

def test_create_oauth_params
    oauth_params = [["name", "BBB"], ["text", "CCC"], ["title", "AAA"]]
    param = create_oauth_params oauth_params
end

def test_twitter_request_token str
    @twitter_api_request_token = "https://api.twitter.com/oauth/request_token"
    unixtime = WiFi.ntp(@ntp_server_ip, 1);
    ##if (unixtime == -1) then
    ##    exit
    ##end
    t = Time.at(unixtime)
    pl t.to_s
    oauth_timestamp = WiFi.url_encode(unixtime.to_s)
    req_params = [
        ["oauth_callback", "http://127.0.0.1"],
        ["oauth_consumer_key", @consumer_key],
        ["oauth_nonce", oauth_timestamp],  
        ["oauth_signature_method", @oauth_signature_method],    
        ["oauth_timestamp", oauth_timestamp],
        ["oauth_version", @oauth_version]   
    ]
    oauth_signature = create_signature @consumer_secret, "", "POST", @twitter_api_request_token, req_params

    oauth_params = [
        ["oauth_callback", "http://127.0.0.1"],
        ["oauth_consumer_key", @consumer_key],
        ["oauth_nonce", oauth_timestamp.to_s],  
        ["oauth_signature_method", @oauth_signature_method],    
        ["oauth_timestamp", oauth_timestamp.to_s],
        ["oauth_version", @oauth_version], 
        ["oauth_signature", oauth_signature]    
    ]
    authorization_param = create_oauth_params oauth_params
    
    data = "status="
    data << WiFi.url_encode(str)
    pl "data = " + data
    headers = ["User-Agent: gr-citrus", "Content-Type: application/x-www-form-urlencoded", authorization_param]
    pl WiFi.httpsPost("api.twitter.com/oauth/request_token", headers, data, "response.txt")
end

def test_twitter_statuses_update str
    @twitter_api_statues_update = "https://api.twitter.com/1.1/statuses/update.json"
    unixtime = WiFi.ntp(@ntp_server_ip, 1);
    ##if (unixtime == -1) then
    ##    exit
    ##end
    t = Time.at(unixtime)
    pl t.to_s
    oauth_timestamp = WiFi.url_encode(unixtime.to_s)
    req_params = [
        ["oauth_consumer_key", @consumer_key],
        ["oauth_nonce", oauth_timestamp],  
        ["oauth_signature_method", @oauth_signature_method],    
        ["oauth_timestamp", oauth_timestamp],
        ["oauth_token", @access_token],
        ["oauth_version", @oauth_version] 
    ]
    oauth_signature = create_signature @consumer_secret, @access_token_secret, "POST", @twitter_api_statues_update, req_params

    oauth_params = [
        ["oauth_consumer_key", @consumer_key],
        ["oauth_nonce", oauth_timestamp.to_s],  
        ["oauth_signature_method", @oauth_signature_method],    
        ["oauth_timestamp", oauth_timestamp.to_s],
        ["oauth_token", @access_token],
        ["oauth_version", @oauth_version], 
        ["oauth_signature", oauth_signature],    
        ["status", str]    
    ]
    authorization_param = create_oauth_params oauth_params
    
    data = "status="
    data << WiFi.url_encode(str)
    data << "\r\n"
    pl "data = " + data
    headers = ["User-Agent: gr-citrus", "Content-Type: application/x-www-form-urlencoded", authorization_param]
    pl WiFi.httpsPost("api.twitter.com/1.1/statuses/update.json", headers, data, "response.txt")
end

if (System.useSD() == 0) then
#    pl "SD Card can't use."
    System.exit() 
end

if (System.useWiFi() == 0) then
#    pl "WiFi Card can't use."
    System.exit() 
end

WiFi.version
WiFi.disconnect
WiFi.setMode 1  #Station-Mode
WiFi.ipconfig
WiFi.connect(@SSID, @PASS)
WiFi.ipconfig
WiFi.multiConnect 1

#test_create_signature

#test_create_oauth_params

str = "tweet from GR-CITRUS"
test_twitter_statuses_update str

#test_twitter_request_token str

pl "WiFi disconnect"
WiFi.disconnect

SD.open 0, "response.txt", 0
# readの引数はファイル番号ファイル番号、返り値はデータ(バイト)、ファイルの最後なら-1
while (c = SD.read(0)) > 0
    p c.chr
end
