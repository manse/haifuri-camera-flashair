local PHOTO_TARGET_PATH = "/DCIM/100OLYMP"
local PHOTO_TARGET_WIDTH = 1280
local PHOTO_TARGET_HEIGHT = 960
local ENDPOINT_URL = "http://192.168.1.7/api.php"


function find_latest_path()
	local max_mod = 0
	local max_mod_path = nil
	for file in lfs.dir(PHOTO_TARGET_PATH) do
		local path = PHOTO_TARGET_PATH.."/"..file
		local last_mod = lfs.attributes(path, "modification")
		if last_mod > max_mod then
			max_mod = last_mod
			max_mod_path = path
		end
	end
	return max_mod_path
end


function read_short(fp)
	local bytes = fp:read(2)
	local b0 = bytes:sub(1, 1):byte()
	local b1 = bytes:sub(2, 2):byte()
	return bit32.lshift(b0, 8) + b1
end


function get_image_size(path)
	local fp = assert(io.open(path, "rb"))
	fp:seek("cur", 4)
	local offset = read_short(fp)
	for i = 1, 10 do
		fp:seek("cur", offset - 2)
		local marker = read_short(fp)
		if marker == 65472 then break end
		offset = read_short(fp)
	end
	fp:seek("cur", 3)
	local height = read_short(fp)
	local width = read_short(fp)
	fp:close()
	return width, height
end


function request_api(path)
	boundary = "--LGTM--BOUNDARY"
	payload = "--" ..  boundary .. "\r\n"
		.."Content-Disposition: form-data; name=\"image\"; filename=\"res.jpg\"\r\n"
		.."Content-Type: image/jpeg\r\n\r\n"
		.."<!--WLANSDFILE-->\r\n"
		.."--" .. boundary .. "--\r\n"
	fa.request{
		url = ENDPOINT_URL,
		method = "POST",
		headers = {
			["Content-Length"] = lfs.attributes(path, "size") + string.len(payload) - 17,
			["Content-Type"] = "multipart/form-data; boundary="..boundary
		},
		file = path,
		body = payload
	}
end


local path = assert(find_latest_path())
local width, height = get_image_size(path)
if width == PHOTO_TARGET_WIDTH and height == PHOTO_TARGET_HEIGHT then
	request_api(path)
end
