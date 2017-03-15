-----------------------------------------------------
-- Lua扩展库之LuaSocket的使用，http://haili.me/archives/742.html
-- LuaSocket是一个Lua扩展库，它能很方便地提供SMTP、HTTP、FTP等网络议访问操作
-----------------------------------------------------
-- 1.socket方式请求
local socket = require("socket")
local host = "100.42.237.125"
local file = "/"
local sock = assert(socket.connect(host, 80))	-- 创建一个TCP连接，连接到HTTP连接的标准端口80端口上
sock:send("GET "..file.." HTTP/1.0\r\n\r\n")
repeat
    local chunk, status, partial = sock:receive(1024)	-- 以1K的字节块来接收数据，并把接收到字节块输出来
    -- print(chunk or partial)
until status ~= "closed"
sock:close()	-- 关闭TCP连接
-----------------------------------------------------
-- 2.http访问请求
local http = require("socket.http")
local result = http.request("http://ip.taobao.com/service/getIpInfo.php?ip=123.189.1.100")
print(result)
-----------------------------------------------------
-- 3.smtp方法发送mail
local smtp = require("socket.smtp")
local from = "<youmail@126.com>"	-- 发件人
local rcpt = { "<youmail@126.com>", "<youmail@qq.com>" }	-- 发送列表
local mesgt = 
{
    headers = 
	{
        to = "youmail@gmail.com",	-- 收件人
        cc = '<youmail@gmail.com>',	-- 抄送
        subject = "This is Mail Title"
    },
    body = "This is  Mail Content."
}
local r, e = smtp.send{
    server="smtp.126.com",
    user="youmail@126.com",
    password="******",
    from = from,
    rcpt = rcpt,
    source = smtp.message(mesgt)
}
if not r then
   print(e)
else
   print("send ok!")
end
-----------------------------------------------------
-- 小试LuaSocket，http://dhq.me/luasocket-network-lua-module
-- LuaSocket是Lua的网络模块库，它可以很方便地提供TCP、UDP、DNS、FTP、HTTP、SMTP、MIME等多种网络协议的访问操作。
-- 它由两部分组成：一部分是用C写的核心，提供对TCP和UDP传输层的访问支持。另外一部分是用Lua写的，负责应用功能的网络接口处理。
-- 使用LuaSocket很简单，直接用require函数加载进来就行，例如输出一个LuaSocket版本信息：
local socket = require("socket")
print(socket._VERSION)
-----------------------------------------------------
-- 模块LuaSocket内置的常量、函数的结构图如下：
-- sleep [function: 0x7feeeb40f940]
-- source [function: 0x7feeeb413570]
-- newtry [function: 0x7feeeb40f8c0]
-- _VERSION [LuaSocket 2.0.2]
-- connect [function: 0x7feeeb4122f0]
-- sink [function: 0x7feeeb410ea0]
-- __unload [function: 0x7feeeb4107e0]
-- bind [function: 0x7feeeb413380]
-- _M {.}
-- _DEBUG [true]
-- skip [function: 0x7feeeb4107b0]
-- dns	-- gethostname [function: 0x7feeeb410af0]
		-- tohostname [function: 0x7feeeb410b20]
		-- toip [function: 0x7feeeb410aa0]
-- gettime [function: 0x7feeeb40f8f0]
-- select [function: 0x7feeeb412290]
-- BLOCKSIZE [2048]
-- sinkt	-- default [function: 0x7feeeb410e20]
			-- close-when-done [function: 0x7feeeb410dc0]
			-- keep-open [function: 0x7feeeb410e20]
-- sourcet	-- by-length [function: 0x7feeeb410e50]
			-- default [function: 0x7feeeb413440]
			-- until-closed [function: 0x7feeeb413440]
-- tcp [function: 0x7feeeb412020]
-- _NAME [socket]
-- choose [function: 0x7feeeb410ce0]
-- try [function: 0x7feeeb410ca0]
-- protect [function: 0x7feeeb410760]
-- _PACKAGE []
-- udp [function: 0x7feeeb410fd0]
-----------------------------------------------------
-- 以socket的方式访问获取度娘首页数据：
local socket = require("socket")
local host = "www.baidu.com"
local file = "/"
-- 创建一个TCP连接，连接到HTTP连接的标准端口80端口上
local sock = assert(socket.connect(host, 80))
sock:send("GET " .. file .. " HTTP/1.0\r\n\r\n")
repeat
    local chunk, status, partial = sock:receive(1024)	-- 以1K的字节块来接收数据，并把接收到字节块输出来
    print(chunk or partial)
until status ~= "closed"
sock:close()	-- 关闭 TCP 连接
-----------------------------------------------------
-- 或者使用模块里内置的http方法来访问：
local http = require("socket.http")
local response = http.request("http://www.baidu.com/")
print(response)
-----------------------------------------------------
-- 一个简单的client/server通信连接
-- 本来想写成单server多client的socket聊天服务器，不过最后还是卡在客户端的数据更新上，
-- 单进程的while轮询（poll），一个io.read就把服务器数据接收给截断了。仅靠现有的LuaSocket模块不装其他第三方模块，
-- 也是很难做一个实时的聊天，虽然有socket.select在苦苦支撑，但是这还是一个填不平的坑来了。
-- 可能用上面向并发的concurrentlua模块会解决这个数据接收阻塞问题，这个以后再看看，现阶段的成果是：
-- 在客户端的终端上敲一些东西后回车会通过socket给服务器发送数据，服务器接收到数据后再返回显示在客户端的终端上。
-- 一个简单的东西，纯属练手，代码如下：
-----------------------------------------------------
-- server.lua
local socket = require("socket")
local host = "127.0.0.1"
local port = "12345"
local server = assert(socket.bind(host, port, 1024))
server:settimeout(0)
local client_tab = {}
local conn_count = 0
print("Server Start "..host..":"..port) 
while 1 do
    local conn = server:accept()
    if conn then
        conn_count = conn_count + 1
        client_tab[conn_count] = conn
        print("A client successfully connect!") 
    end
    for conn_count, client in pairs(client_tab) do
        local recvt, sendt, status = socket.select({client}, nil, 1)
        if #recvt > 0 then
            local receive, receive_status = client:receive()
            if receive_status ~= "closed" then
                if receive then
                    assert(client:send("Client "..conn_count.." Send : "))
                    assert(client:send(receive.."\n"))
                    print("Receive Client "..conn_count.." : ", receive)   
                end
            else
                table.remove(client_tab, conn_count) 
                client:close() 
                print("Client "..conn_count.." disconnect!") 
            end
        end
    end
end
-----------------------------------------------------
-- client.lua
local socket = require("socket")
local host = "127.0.0.1"
local port = 12345
local sock = assert(socket.connect(host, port))
sock:settimeout(0)
print("Press enter after input something:")
local input, recvt, sendt, status
while true do
    input = io.read()
    if #input > 0 then
        assert(sock:send(input.."\n"))
    end
    recvt, sendt, status = socket.select({sock}, nil, 1)
    while #recvt > 0 do
        local response, receive_status = sock:receive()
        if receive_status ~= "closed" then
            if response then
                print(response)
                recvt, sendt, status = socket.select({sock}, nil, 1)
            end
        else
            break
        end
    end
end
-----------------------------------------------------


