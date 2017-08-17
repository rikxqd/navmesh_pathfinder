

local nav = require "src.nav"

local inst
function create(v_ptr,vsize,p_ptr,psize)
	inst = nav.create(v_ptr,vsize,p_ptr,psize)
end



function find(dialog,start_x,start_z,over_x,over_z)
	for i = 0,7 do
		inst:set_mask(i,1)
	end
	inst:set_mask(3,0)
	local result = inst:find(start_x,start_z,over_x,over_z)
	draw(dialog,result)
end


_G["create"] = create
_G["set_mask"] = set_mask
_G["find"] = find

