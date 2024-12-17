--- ====================
--- This is only used by LVF for internal testing!
--- ====================

---@class TestIr : Lvf.IrClient
local M = {}

local state = {
	send_bogus = false,
}

---@param t table<string, string>
function M.apply_settings(t)
	if t.send_bogus ~= nil then
		state.send_bogus = t.send_bogus == "true"
	end
end

---@return table<string, Lvf.SettingType>
function M.get_settings()
	---@type table<string, Lvf.SettingType>
	return {
		send_bogus = "TEXT",
		your_secret = "PASSWORD",
	}
end

---@param md5 string
function M.open_ir_page(md5)
	--
end

---@param score Lvf.Score
---@return Lvf.ScoreResult|nil
function M.send_score(score)
	if state.send_bogus then
		return { place = "1234", total_players = "1234", clear_rate = "1234" }
	end
	---@type Lvf.ScoreResult
	return { place = 10, total_players = 200, clear_rate = 90 }
end

M.name = "The literal test client"

M.api_version = -1

return M
