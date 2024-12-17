---@meta

---@alias Lvf.SettingType "TEXT"|"PASSWORD"
---@alias Lvf.SettingValues table<string, string>
---@alias Lvf.SettingsAvailable table<string, Lvf.SettingType>
---@alias Lvf.Uri string http://example.com

lvf = {}
---@param t Lvf.SettingValues
lvf.apply_settings = function(t) end
---@return Lvf.SettingsAvailable
lvf.get_settings = function() end
---@param s string
lvf.notify = function(s) end
---@param uri Lvf.Uri
lvf.open_uri = function(uri) end
---@param uri Lvf.Uri
---@param s string data
---@return string
lvf.send_post = function(uri, s) end
---@param s string
---@return string
lvf.md5 = function(s) end

---@class Lvf.ScoreResult
---@field place number
---@field total_players number
---@field clear_rate number

---@class Lvf.IrClient
---TODO: Maybe add is_online() and login() and init(lvf_version).
---@field name string Name of the IR client. May include the name of the IR service.
---@field api_version integer LVF API version the client is compatible with.
---@field apply_settings fun(t: Lvf.SettingValues):nil
---@field get_settings fun():Lvf.SettingsAvailable
---@field open_ir_page fun(md5: string):nil
---@field send_score fun(score: Lvf.Score):Lvf.ScoreResult|nil

---{EX,}GRADE is what LR2 would call HARD and NORMAL.
---@alias Lvf.Gauge "DEATH"|"EXHARD"|"HARD"|"NORMAL"|"EASY"|"ASSIST"|"GRADE"|"EXGRADE"

---@alias Lvf.Lamp Lvf.Gauge|"NOPLAY"

---@alias Lvf.Mods "NORMAL"|"MIRROR"|"RANDOM"|"S-RANDOM"|"H-RANDOM"|"ALL-SCR"

---Not for courses.
---Sometimes may mean an exact given score, sometimes a PB.
---@class Lvf.Score
---
---@field md5 string
---
---@field lamp Lvf.Lamp
---@field play_gauge Lvf.Gauge|nil Either from this play or from PB.
---@field play_mods Lvf.Mods|nil Either from this play or from PB. TODO: what about P2? Does it need separate handling?
---
---@field gauge_graph table<integer>|nil 500 elements in 0-100 range. Only added in this play.
---
---@field exscore number
---@field maxcombo number
---@field minbp number
---
---@field pgreat number
---@field great number
---@field good number
---@field bad number
---@field poor number
---@field miss number
---..Early/late - which ones do you want exactly?
---And wtf is early/late miss in beatoraja???
