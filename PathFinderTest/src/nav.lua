local nav_c = require "nav.core"

local _M = {}

local meta = {}
function _M.create(v_ptr,vsize,p_ptr,psize)
	local core = nav_c.create(v_ptr,vsize,p_ptr,psize)
	local obj = setmetatable({core = core},{__index =meta,__gc = function ()
		nav_c.release(core)
	end})
	return obj
end

function meta:find(start_x,start_z,over_x,over_z)
	return nav_c.find_path(self.core,start_x,start_z,over_x,over_z)
end

function meta:raycast(start_x,start_z,over_x,over_z)
	local ok,result_x,result_z = raycast.raycast(self.core,start_x,start_z,over_x,over_z)
	return ok,result_x,result_z 
end

function meta:set_mask(mask,enable)
	nav_c.set_mask(self.core,mask,enable)
end

return _M