-- 焊锡检测结果表
CREATE TABLE welding_results (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    hxPort TEXT NOT NULL,                    		-- 端口
    hxId INTEGER NOT NULL,                      -- 焊锡编号
    hxBatch INTEGER,                            -- 焊锡批次
    hxWeight REAL DEFAULT 0,                    -- 焊锡长
    hxHeight REAL DEFAULT 0,                    -- 焊锡宽
    hxArea REAL DEFAULT 0,                      -- 瑕疵面积
    hxDistance REAL DEFAULT 0,                  -- 裸露铜引脚长度
    hxPianyi REAL DEFAULT 0,                    -- 焊锡偏移
    hxLTSumArea REAL DEFAULT 0,                 -- 漏铜总面积
    hxLTMaxArea REAL DEFAULT 0,                 -- 漏铜最大面积
    hxHDSumArea REAL DEFAULT 0,                 -- 黑点总面积
    hxHDMaxArea REAL DEFAULT 0,                 -- 黑点最大面积
    classifyStr TEXT DEFAULT '',                 -- 二维码
    fixtureCode TEXT DEFAULT '',                 -- 治具码
    shellCode TEXT DEFAULT '',                   -- 外壳码
    imageCode TEXT DEFAULT '',                   -- 结果图片名称
    hxReult BOOLEAN DEFAULT 0,                  -- 检测结果 (0:NG, 1:OK)
    hxReultsDescribe TEXT,                      -- 结果说明
    create_time TIMESTAMP NOT NULL DEFAULT (datetime('now', 'localtime')),
    change_time TIMESTAMP NOT NULL DEFAULT (datetime('now', 'localtime'))
);

-- 创建索引以提高查询性能
CREATE INDEX idx_welding_results_port ON welding_results (hxPort);
CREATE INDEX idx_welding_results_id ON welding_results (hxId);
CREATE INDEX idx_welding_results_batch ON welding_results (hxBatch);
CREATE INDEX idx_welding_results_time ON welding_results (create_time);

-- 更新时间触发器
CREATE TRIGGER welding_results_update_trigger 
AFTER UPDATE ON welding_results
FOR EACH ROW
BEGIN
    UPDATE welding_results 
    SET change_time = datetime('now', 'localtime') 
    WHERE id = OLD.id;
END;

-- 创建触发器，保留最近7天的数据
CREATE TRIGGER IF NOT EXISTS auto_clean_welding_results
AFTER INSERT ON welding_results
BEGIN
    DELETE FROM welding_results 
    WHERE create_time < datetime('now', '-7 days', 'localtime');
END;
