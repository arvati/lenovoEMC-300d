--[[

LuCI CUPS module

Copyright 2014, CZ.NIC z.s.p.o. (http://www.nic.cz/)

This file is part of NUCI configuration server.

NUCI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

NUCI is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with NUCI.  If not, see <http://www.gnu.org/licenses/>.

]]--

module("luci.controller.cups", package.seeall)

function index()
	page = node("admin", "services", "cups")
	page.target = cbi("cups")
	page.title = _("CUPS")

	page = entry({"admin", "services", "cups_get_printers"}, call("cups_get_printers"), nil)
	page.leaf = true

	page = entry({"admin", "services", "cups_get_queue"}, call("cups_get_queue"), nil)
	page.leaf = true
end

function cups_get_printers()
	luci.http.prepare_content("application/json")

	local f = io.popen('lpstat -a', 'r')
	local data = f:read('*a')
	f:close()
	data = data:split("\n")

	luci.http.write_json(data)
end

function cups_get_queue()
	luci.http.prepare_content("application/json")

	local f = io.popen('lpstat -R', 'r')
	local data = f:read('*a')
	f:close()
	data = data:split("\n")

	luci.http.write_json(data)
end
